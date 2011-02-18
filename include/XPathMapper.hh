#ifndef XPATH_MAPPER_HH
#define XPATH_MAPPER_HH

#include <QAtomicInt>
#include <QByteArray>
#include <QString>
#include <QThread>

#include <AlpinoCorpus/CorpusReader.hh>
#include <libxml/tree.h>
#include <libxml/xpath.h>

/*!
 A map function has to implement this interface. For every entry
 that gives some results the operator() method is called. Used
 by XPathMapper
 \sa XPathMapper
 */
template <typename Arg1, typename Arg2>
struct map_function {
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;

    virtual ~map_function() {};
    virtual void operator()(Arg1 a1, Arg2 a2) = 0;
};

/*!
 XPathMapper is ment to call a certain function, map_function, for every xml
 file the corpus reader provides for which the query finds results. It is most
 often used for searching, with the map_fuction firing a signal for each hit.
 */
class XPathMapper : public QThread
{
    Q_OBJECT
public:
	XPathMapper();
	/*!
	 Don't continue to the next entry in the corpus when this one is processed.
	 Once stopped, it has to be started again using start.
	 */
	void cancel();
	
	/*!
	 Initializes and starts the processing. For every entry in reader that provides
	 some results to the xpath query map_function is called with the corpus reader's
	 internal name for the xml file, and the xpath object with the query results.
	 \sa map_function
	 \sa run
	 \param reader Corpus reader
	 \param query the XPath query which serves as the condition whether map_function
	 is called for that entry in the corpus reader. 
	 \param fun a map_function applied to every entry.
	 */
	void start(alpinocorpus::CorpusReader *reader, QString query, map_function<QString const &, xmlXPathObjectPtr> *fun);

signals:
	/*!
	 Fired when started. Provides the total number of entries to process. Usefull
	 for progress bars.
	 \param totalEntries number of entries to process
	 */
	void started(int totalEntries);
	
	/*!
	 Fired everytime when an entry is processed.
	 \param entriesDone number of entries processed so far
	 \param totalEntries total number of entries in the corpus
	 */
	void progress(int entriesDone, int totalEntries);
	
	/*!
	 Fired when all the entries have been processed, or when the processing was
	 cancelled.
	 \param entriesDone number of entries processed
	 \param totalEntries total number of entries in the corpus
	*/
	void stopped(int entriesDone, int totalEntries);
	
protected:
	/*!
	 This contains the main loop, this is where all the fun happens.
	 */
    void run();
    
private:
	/*!
	 The main loop stops when d_cancel is not null. d_cancel is reset to 0 when
	 the main loop is stopped. (just before the stopped signal is fired)
	 \sa cancel
	 */
    QAtomicInt d_cancel;
	
	/*!
	 The corpus reader of which all the entries are processed. I have no idea
	 what happens when it is deleted while run is still running, bit I'm sure
	 it isn't pretty.
	 \sa start
	 */
    alpinocorpus::CorpusReader *d_reader;
	
	/*!
	 The search query used for testing whether map_function should be called
	 \sa start
	 */
	QByteArray d_xpathQuery;
	
	/*!
	 The fuction applied to each entry that gives some results for the xpath
	 query.
	 \sa start
	 */
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
