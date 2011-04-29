#ifndef XPATH_FILTER_HH
#define XPATH_FILTER_HH

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

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

class EntryFun : public QObject, public filter_function<QVector<QString> *, QString const &, xmlXPathObjectPtr, void>
{
    Q_OBJECT

public:
    ~EntryFun();
    void operator()(QVector<QString> *acc, QString const &entry, xmlXPathObjectPtr xpathObj);
signals:
    void entryFound(QString entry);
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

#endif // XPATH_FILTER_HH
