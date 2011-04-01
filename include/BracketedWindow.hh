#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QCloseEvent>
#include <QHash>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class BracketedWindow;
}

class QKeyEvent;

class DactMacrosModel;
class DactQueryModel;
class QListWidgetItem;
class QStyledItemDelegate;

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
	 when one of BracketedWindow's results is activated.::progress
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
    /*
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void entryActivated(QListWidgetItem *subject);
    */
    
	/*!
	 Called when the search mapper started. Shows progress bar.
	 \param totalEntries number of entries to search thru.
	 */
    void progressStarted(int totalEntries);
	
	/*!
	 Called when the search mapper is done or cancelled. Hides the progress bar.
	 \param processedEntries number of entries it has processed.
	 \param totalEntries number of entries it could have processed.
	 */
    void progressChanged(int processedEntries, int totalEntries);
	
	/*!
	 Called when the search mapper progresssed. Updates the progress bar.
	 \param processedEntries number of entries searched so far.
	 \param totalEntries number of entries to search thru.
	 */
    void progressStopped(int processedEntries, int totalEntries);
	
	/*!
	 Called when another delegate is selected in the dropdown menu
	 \sa d_listDelegateFactories
	 \param index index of the delegate in the dropdown menu.
	 */
	void listDelegateChanged(int index);
	
	void startQuery();
    void stopQuery();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
	void addListDelegate(QString const &name, QStyledItemDelegate*(*factory)());
	void updateResults();
    void createActions();
	void initListDelegates();
    void initSentenceTransformer();
    void setModel(DactQueryModel* model);
    void readSettings();
    void writeSettings();
	
	static QStyledItemDelegate* colorDelegateFactory();
	static QStyledItemDelegate* visibilityDelegateFactory();
	static QStyledItemDelegate* keywordInContextDelegateFactory();

    QString d_filter;
	QList<QStyledItemDelegate*(*)()>d_listDelegateFactories;
    QSharedPointer<Ui::BracketedWindow> d_ui;
	QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<DactQueryModel> d_model;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XPathValidator> d_xpathValidator;
};

#endif // DACTFILTERWINDOW_H
