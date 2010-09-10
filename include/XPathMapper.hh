#ifndef XPATH_MAPPER_HH
#define XPATH_MAPPER_HH

#include <QAtomicInt>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QThread>
#include <QVector>

#include <stdexcept>

#include <AlpinoCorpus/CorpusReader.hh>
#include <libxml/tree.h>
#include <libxml/xpath.h>

template <typename Arg1, typename Arg2>
struct map_function {
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;

    virtual ~map_function() {};
    virtual void operator()(Arg1 a1, Arg2 a2) = 0;
};

class XPathMapper : public QThread
{
    Q_OBJECT
public:
	XPathMapper();
	void cancel();
	void start(alpinocorpus::CorpusReader *reader, QString query, map_function<QString const &, xmlXPathObjectPtr> *fun);

signals:
	void started(int totalEntries);
	void progress(int entriesDone, int totalEntries);
	void stopped(int entriesDone, int totalEntries);
	
protected:
    void run();
    
private:
    QAtomicInt d_cancel;
    alpinocorpus::CorpusReader *d_reader;
	QByteArray d_xpathQuery;
    map_function<QString const &, xmlXPathObjectPtr> *d_mapFunction;
};

class EntryMap : public QObject, public map_function<QString const &, xmlXPathObjectPtr>
{
    Q_OBJECT

public:
    void operator()(QString const &entry, xmlXPathObjectPtr xpathObj);
signals:
    void entryFound(QString entry);
};

class AttributeMap : public QObject, public map_function<QString const &, xmlXPathObjectPtr>
{
	Q_OBJECT
public:
    AttributeMap(QString const &attribute) : d_attribute(attribute) {}
    AttributeMap();
    void operator()(QString const &entry, xmlXPathObjectPtr xpathObj);
private:
    QString getAttribute(xmlNode* node, QString const &attribute) const;
    QString d_attribute;
signals:
	void attributeFound(QString value);
};

#endif // XPATH_MAPPER_HH
