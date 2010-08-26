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
    d_running = false;
    
    // We will do the termination ourselves by setting d_running to false.
    setTerminationEnabled(false);
}

void XPathMapper::setQuery(QString xpathQuery)
{
    if(d_running)
        throw std::runtime_error("XPathMapper::setQuery: Cannot change query when mapper is running");
    
    d_xpathQuery = xpathQuery.toUtf8();
}

void XPathMapper::start(alpinocorpus::CorpusReader *reader)
{
    if(d_running)
        throw std::runtime_error("XPathMapper::start: XPathMapper cannot start when already running");
    
    if(d_xpathQuery.isEmpty())
        throw std::runtime_error("XPathMapper::start: Cannot run without a query");
    
    d_reader = reader;
    QThread::start();
}

void XPathMapper::terminate()
{
    d_running = false;
    QThread::terminate();
    wait();
}

void XPathMapper::run()
{
    d_running = true;
    
    QVector<QString> corpusEntries(d_reader->entries());
    for (QVector<QString>::const_iterator iter = corpusEntries.constBegin();
        d_running && iter != corpusEntries.constEnd(); ++iter)
    {
        // Parse XML data...
        QString xmlStr(d_reader->read(*iter));
        QByteArray xmlData(xmlStr.toUtf8());
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (!doc)
            throw std::runtime_error("XPathMapper::run: could not parse XML data.");

        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx) {
            xmlFreeDoc(doc);
            throw std::runtime_error("XPathMapper::run: could not construct XPath context.");
        }

        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_xpathQuery.constData()), ctx);
        if (!xpathObj) {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw std::runtime_error("XPathMapper::run: could not evaluate XPath expression.");
        }

        (*d_mapFunction)(*iter, xpathObj);

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        
        // Easy testing
        //msleep(10);
    }
    
    d_running = false;
}


EntryMap::~EntryMap() {}


void EntryMap::operator()(QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
    {
        emit entryFound(entry);
    }
}