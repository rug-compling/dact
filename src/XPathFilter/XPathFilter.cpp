#include <QByteArray>
#include <QString>
#include <QVector>
#include <QtDebug>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathFilter.hh"

using namespace std;
using namespace alpinocorpus;

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

QHash<QString,int> XPathFilter::aggregate(CorpusReader *reader, QString const &attribute) const
{
    QHash<QString,int> summaries;

    QVector<QString> corpusEntries(reader->entries());
    
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

        // When nodes are found, try to get the attribute from each found node.
        if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0) {
            for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
                QString value = getAttribute(xpathObj->nodesetval->nodeTab[i], attribute);
                if(value != 0)
                    ++summaries[value];
            }
        }

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
    }

    return summaries;
}

QString XPathFilter::getAttribute(xmlNode* node, QString const &attribute) const
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
