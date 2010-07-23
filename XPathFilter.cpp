#include <QByteArray>
#include <QString>
#include <QVector>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <IndexedCorpus/CorpusReader.hh>

#include "XPathFilter.hh"

using namespace std;
using namespace indexedcorpus;

XPathFilter::XPathFilter(QString const &xpathQuery)
{
    d_xpathQuery = xpathQuery.toUtf8();
}

QVector<QString> XPathFilter::entries(CorpusReader *reader) const
{
    QVector<QString> corpusEntries(reader->entries());

    QVector<QString> entries;
    for (QVector<QString>::const_iterator iter = corpusEntries.constBegin();
        iter != corpusEntries.constEnd(); ++iter)
    {
        // Parse XML data...
        QString xmlStr(reader->read(*iter));
        QByteArray xmlData(xmlStr.toUtf8());
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (!doc)
            throw runtime_error("XPathFilter::entries: could not parse XML data.");

        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx) {
            xmlFreeDoc(doc);
            throw runtime_error("XPathFilter::entries: could not construct XPath context.");
        }

        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_xpathQuery.constData()), ctx);
        if (!xpathObj) {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw runtime_error("XPathFilter::entries: could not evaluate XPath expression.");
        }

        if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
            entries.push_back(*iter);

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
    }

    return entries;
}
