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

XPathMapper::XPathMapper(map_function<QString const &, xmlXPathObjectPtr> *fun)
{
    d_mapFunction = fun;
    d_cancel = 0;
    
    // We will do the termination ourselves by setting d_running to false.
    //setTerminationEnabled(false);
}

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

void XPathMapper::start(alpinocorpus::CorpusReader *reader)
{
    if(isRunning())
        throw std::runtime_error("XPathMapper::start: XPathMapper cannot start when already running");
    
    if(d_xpathQuery.isEmpty())
        throw std::runtime_error("XPathMapper::start: Cannot run without a query");
    
    d_reader = reader;
    QThread::start();
}

void XPathMapper::run()
{    
    QVector<QString> corpusEntries(d_reader->entries());
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
    }

    d_cancel = 0;
}


EntryMap::~EntryMap() {}


void EntryMap::operator()(QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
    {
        emit entryFound(entry);
    }
}
