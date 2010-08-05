#ifndef XPATH_FILTER_HH
#define XPATH_FILTER_HH

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

#include <stdexcept>

#include <AlpinoCorpus/CorpusReader.hh>
#include <libxml/tree.h>
#include <libxml/xpath.h>

template <typename Arg1, typename Arg2, typename Arg3, typename Result>
struct filter_function {
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;
    typedef Arg3 third_argument_type;
    typedef Result result_type;

    virtual ~filter_function() {};
    virtual Result operator()(Arg1 a1, Arg2 a2, Arg3 a3) = 0;
};

class XPathFilter
{
public:
	XPathFilter(QString const &xpathQuery);
    template <typename T>
    T fold(alpinocorpus::CorpusReader *reader,
        filter_function<T*, QString const &, xmlXPathObjectPtr, void> *fun);
private:
	QByteArray d_xpathQuery;
};

struct EntryFun : public filter_function<QVector<QString> *, QString const &, xmlXPathObjectPtr, void>
{
    ~EntryFun();
    void operator()(QVector<QString> *acc, QString const &entry, xmlXPathObjectPtr xpathObj);
};

struct AggregateFun : public filter_function<QHash<QString, int> *, QString const &, xmlXPathObjectPtr, void>
{
    AggregateFun(QString const &attribute) : d_attribute(attribute) {}
    ~AggregateFun();
    void operator()(QHash<QString, int> *acc, QString const &entry, xmlXPathObjectPtr xpathObj);
private:
    QString getAttribute(xmlNode* node, QString const &attribute) const;

    QString d_attribute;
};


template <typename T>
T XPathFilter::fold(alpinocorpus::CorpusReader *reader,
    filter_function<T*, QString const &, xmlXPathObjectPtr, void> *fun)
{
    T result;

    QVector<QString> corpusEntries(reader->entries());
    for (QVector<QString>::const_iterator iter = corpusEntries.constBegin();
        iter != corpusEntries.constEnd(); ++iter)
    {
        // Parse XML data...
        QString xmlStr(reader->read(*iter));
        QByteArray xmlData(xmlStr.toUtf8());
        xmlDocPtr doc = xmlParseMemory(xmlData.constData(), xmlData.size());
        if (!doc)
            throw std::runtime_error("XPathFilter::filter: could not parse XML data.");

        // Parse XPath query
        xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
        if (!ctx) {
            xmlFreeDoc(doc);
            throw std::runtime_error("XPathFilter::filter: could not construct XPath context.");
        }

        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
            reinterpret_cast<xmlChar const *>(d_xpathQuery.constData()), ctx);
        if (!xpathObj) {
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            throw std::runtime_error("XPathFilter::filter: could not evaluate XPath expression.");
        }

        (*fun)(&result, *iter, xpathObj);

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
    }

    return result;
}

#endif // XPATH_FILTER_HH
