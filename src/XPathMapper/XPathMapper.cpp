#include <QByteArray>
#include <QString>
#include <QThread>
#include <QVector>
#include <QtDebug>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"

using namespace std;
using namespace alpinocorpus;

// depricated
XPathMapper::XPathMapper(map_function<QString const &, xmlXPathObjectPtr> *fun)
{
    d_mapFunction = fun;
    d_cancel = 0;
    
    // We will do the termination ourselves by setting d_running to false.
    //setTerminationEnabled(false);
}

XPathMapper::XPathMapper() {}

void XPathMapper::cancel()
{
    d_cancel = 1;
}

void XPathMapper::setQuery(QString xpathQuery)
{
    if (isRunning())
        throw std::runtime_error("XPathMapper::setQuery: Cannot change query when mapper is running");
    
    d_xpathQuery = xpathQuery.toUtf8();
}

// depricated in favor of the one with more arguments. That one allows us to change
// the map_function–which is required by the statistics window–without creating a
// new instance each time.
void XPathMapper::start(alpinocorpus::CorpusReader *reader)
{
    if(isRunning())
        throw std::runtime_error("XPathMapper::start: XPathMapper cannot start when already running");
    
    if(d_xpathQuery.isEmpty())
        throw std::runtime_error("XPathMapper::start: Cannot run without a query");
    
    d_reader = reader;
    QThread::start();
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
    QVector<QString> corpusEntries(d_reader->entries());
	
	int n = 0, totalEntries = corpusEntries.count();
	
	emit started(totalEntries);
	
    for (QVector<QString>::const_iterator iter = corpusEntries.constBegin();
        d_cancel == 0 && iter != corpusEntries.constEnd(); ++iter)
    {
        // Parse XML data...
        QString xmlStr(d_reader->read(*iter));
        QByteArray xmlData(xmlStr.toUtf8());
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (!doc) {
            qWarning() << "XPathMapper::run: could not parse XML data: " << *iter;
            continue;
        }

        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx) {
            xmlFreeDoc(doc);
            qWarning() << "XPathMapper::run: could not construct XPath context from document: " << *iter;
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

        (*d_mapFunction)(*iter, xpathObj);

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

