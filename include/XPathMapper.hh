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
	XPathMapper(map_function<QString const &, xmlXPathObjectPtr> *fun);
    void cancel();
	// The query can be changed (will throw when trying to change it on a
	// running mapper) which allows for reuse of the same mapper. This might
	// come in handy when the ui wants to bind the mapper for progress events.
	// @TODO this could be combined with start() if DactMainWindow would save
	// the last-entered query somewhere.
    void setQuery(QString xpathQuery);
    // Reader is inserted at start which allows the mapper to be created when
    // there isn't a reader present in DactMainWindow. It will throw when you
    // try to start an already running mapper.
    void start(alpinocorpus::CorpusReader *reader);
	
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
