#include <QByteArray>
#include <QString>
#include <QThread>
#include <QtDebug>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"

namespace ac = alpinocorpus;

XPathMapper::XPathMapper() {}

void XPathMapper::cancel()
{
    d_cancel = 1;
}

void XPathMapper::start(alpinocorpus::CorpusReader *reader, QString query, map_function<QString const &, xmlXPathObjectPtr> *fun)
{
    if(isRunning())
        throw std::runtime_error("XPathMapper::start: XPathMapper cannot start when already running");
    
	d_reader = reader;
	
	d_xpathQuery = query.toUtf8();
	
	d_mapFunction = fun;
	
    if(d_xpathQuery.isEmpty())
        throw std::runtime_error("XPathMapper::start: Cannot run without a query");
    
    QThread::start();
}

void XPathMapper::run()
{    
    size_t n = 0, totalEntries = d_reader->size();

    emit started(totalEntries);

    for (ac::CorpusReader::EntryIterator i(d_reader->begin()),
                                         end(d_reader->end());
        !d_cancel && i != end; ++i) {
        // Parse XML data...
        QString xmlStr(d_reader->read(*i));
        QByteArray xmlData(xmlStr.toUtf8());
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (!doc) {
            qWarning() << "XPathMapper::run: could not parse XML data: " << *i;
            continue;
        }

        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx) {
            xmlFreeDoc(doc);
            qWarning() << "XPathMapper::run: could not construct XPath context from document: " << *i;
            continue;
        }

        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_xpathQuery.constData()), ctx);
        if (!xpathObj) {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            qWarning() << "XPathMapper::run: could not evaluate XPath expression.";
            break;
        }

        (*d_mapFunction)(*i, xpathObj);

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
		
		// This could be _a lot_ of signals. Maybe a performance killer? It's currently
		// only used for a progress bar in the statistics window.
		emit progress(++n, totalEntries);
    }

    d_cancel = 0;
	
	emit stopped(n, totalEntries);
}


void EntryMap::operator()(QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
    {
        emit entryFound(entry);
    }
}


AttributeMap::AttributeMap() {}

void AttributeMap::operator()(QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0) {
        for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
            QString value = getAttribute(xpathObj->nodesetval->nodeTab[i], d_attribute);
            if(value != 0)
                emit attributeFound(value);
        }
    }
}

QString AttributeMap::getAttribute(xmlNode* node, QString const &attribute) const
{
    for (xmlAttrPtr attr = node->properties; attr; attr = attr->next)
    {
        // @TODO: it's probably faster if this uses strcmp and const char* instead of QString
        // but I coudn't get it to work correctly with Qstring.constData().
        if (QString(reinterpret_cast<const char *>(attr->name)).compare(attribute) == 0) {
            for (xmlNode* child = attr->children; child; child = child->next) {
                return QString::fromUtf8(reinterpret_cast<char const *>(child->content));
            }
        }
    }
	
    return 0;
}

