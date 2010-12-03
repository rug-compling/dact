#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QCloseEvent>
#include <QFileInfo>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <Qt>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class BracketedWindow;
}

class QKeyEvent;

class DactMacrosModel;
class QListWidgetItem;
class QStyledItemDelegate;
class EntryMapAndTransform;

/*!
 It is still called the bracketed window because it showed the sentences with brackets
 around the matching nodes, but it now supports multiple ways of showing the sentences
 with matches for a certain xpath query using BracketedDelegates. This class provides
 the parsing of the sentences to identify the matches and paint them accordingly.
 (Someday this will hopefully be replaced with pure xml and maybe even xml converted
 into html using xslt)
 \sa BracketedDelegate
 */
class BracketedWindow : public QWidget {
    Q_OBJECT
public:
	/*
	 Constructor
	 \param corpusReader the corpus reader that will be queried
	 \param macrosModel the current macros model which should preprocess the queries
	 \param parent
	 \param f
	*/
    BracketedWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent = 0, Qt::WindowFlags f = 0);
	
    ~BracketedWindow();
	
    /*!
	 When a new treebank is loaded into the main window, the corpus is switched and the
	 results will be updated.
	 \param corpusReader the new corpus reader
	 */
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
	
	/*!
	 Set the query filter. Used by the main window to copy the current filter query
	 into this window when opened for the first time.
	 \param text XPath query
	 */
    void setFilter(QString const &text);
	
	/*!
	 Return the current active filter. Used by the main window to highlight the nodes
	 when one of BracketedWindow's results is activated.
	 */
    inline QString const &filter() const { return d_filter; };

signals:
	/*!
	 Fired when a new entry in the list is selected.
	 Used by DactMainWindow to keep its current file in sync with the results of
	 this window.
	 \param entry corpus internal path to the xml file.
	 */
    void currentEntryChanged(QString const &entry);
	
	/*!
	 Fired when a entry is activated (by doubleclicking or pressing the enter key)
	 Used by DactMainWindow to raise its window
	 */
    void entryActivated();
    
private slots:
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void entryActivated(QListWidgetItem *subject);
    void filterChanged();
	
	/*!
	 Catches the EntryMapAndTransform's sentenceFound signal. Currently the transformed
	 xml that is returned is parsed by the BracketedDelegate when it is displayed, but
	 it is human readable.
	 \param file corpus internal path to the xml file.
	 \param sentence the transformed xml.
	 */
    void sentenceFound(QString file, QString sentence);
	
	/*!
	 Called when the search mapper started. Shows progress bar.
	 \param totalEntries number of entries to search thru.
	 */
    void mapperStarted(int totalEntries);
	
	/*!
	 Called when the search mapper progresssed. Updates the progress bar.
	 \param processedEntries number of entries searched so far.
	 \param totalEntries number of entries to search thru.
	 */
    void mapperStopped(int processedEntries, int totalEntries);
	
	/*!
	 Called when the search mapper is done or cancelled. Hides the progress bar.
	 \param processedEntries number of entries it has processed.
	 \param totalEntries number of entries it could have processed.
	 */
    void mapperProgressed(int processedEntries, int totalEntries);
	
	/*!
	 Called when another delegate is selected in the dropdown menu
	 \sa d_listDelegateFactories
	 \param index index of the delegate in the dropdown menu.
	 */
	void listDelegateChanged(int index);

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
	void addListDelegate(QString const &name, QStyledItemDelegate*(*factory)());
	void updateResults();
    void createActions();
	void initListDelegates();
    void initSentenceTransformer();
    void readSettings();
    void writeSettings();
    void stopMapper();
	
	static QStyledItemDelegate* colorDelegateFactory();
	static QStyledItemDelegate* visibilityDelegateFactory();
	static QStyledItemDelegate* keywordInContextDelegateFactory();

    EntryMapAndTransform *d_entryMap;
    QString d_filter;
	QList<QStyledItemDelegate*(*)()>d_listDelegateFactories;
    QSharedPointer<Ui::BracketedWindow> d_ui;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XPathMapper> d_xpathMapper;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

class EntryMapAndTransform : public EntryMap
{
    Q_OBJECT
public:
    EntryMapAndTransform(QSharedPointer<alpinocorpus::CorpusReader> reader, QSharedPointer<XSLTransformer> transformer, QString const &query);
    void operator()(QString const &entry, xmlXPathObjectPtr xpathObj);
private:
    QString transform(QString const &file);

signals:
    void sentenceFound(QString file, QString sentence);
    
private:
    QString d_query;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QSharedPointer<XSLTransformer> d_xslTransformer;
};

#endif // DACTFILTERWINDOW_H
