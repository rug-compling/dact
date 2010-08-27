#ifndef XPATH_MAPPER_HH
#define XPATH_MAPPER_HH

#include <QByteArray>
#include <QHash>
#include <QMutex>
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
	XPathMapper(map_function<QString const &, xmlXPathObjectPtr> *fun);
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
    // Please note that I made terminate blocking so when it returns, the
    // thread is actually definitely terminated.
    void terminate();

protected:
    void run();
    
private:
    bool d_running;
    QMutex d_runningMutex;
    alpinocorpus::CorpusReader *d_reader;
	QByteArray d_xpathQuery;
    map_function<QString const &, xmlXPathObjectPtr> *d_mapFunction;
};

class EntryMap : public QObject, public map_function<QString const &, xmlXPathObjectPtr>
{
    Q_OBJECT

public:
    ~EntryMap();
    void operator()(QString const &entry, xmlXPathObjectPtr xpathObj);
signals:
    void entryFound(QString entry);
};

#endif // XPATH_MAPPER_HH
