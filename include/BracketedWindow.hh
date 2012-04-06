#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QCloseEvent>
#include <QHash>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include <AlpinoCorpus/CorpusReader.hh>

#include "CorpusWidget.hh"
#include "DactMacrosModel.hh"
#include "FilterModel.hh"
#include "XPathValidator.hh"
#include "ui_BracketedWindow.h"

class QKeyEvent;
class QModelIndex;
class QStyledItemDelegate;
class QTextStream;

/*!
 It is still called the bracketed window because it showed the sentences with brackets
 around the matching nodes, but it now supports multiple ways of showing the sentences
 with matches for a certain xpath query using BracketedDelegates. This class provides
 the parsing of the sentences to identify the matches and paint them accordingly.
 (Someday this will hopefully be replaced with pure xml and maybe even xml converted
 into html using xslt)
 \sa BracketedDelegate
 */
class BracketedWindow : public CorpusWidget {
    Q_OBJECT

    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusReaderPtr;
    typedef QStyledItemDelegate*(*DelegateFactory)(CorpusReaderPtr);

public:
    BracketedWindow(QWidget *parent = 0);
    ~BracketedWindow();

    /*!
     When a new treebank is loaded into the main window, the corpus is switched and the
     results will be updated.
     \param corpusReader the new corpus reader
     */
    void switchCorpus(CorpusReaderPtr corpusReader);

    /*!
     Set the query filter. Used by the main window to copy the current filter query
     into this window when opened for the first time.
     \param text XPath query
     */
    void setFilter(QString const &text, QString const &raw_filter);

    /*!
     Return the current active filter. Used by the main window to highlight the nodes
     when one of BracketedWindow's results is activated.::progress
     */
    inline QString const &filter() const { return d_filter; };

    void showFilenames(bool show);

signals:
    /*!
     Fired when a entry is activated (by doubleclicking or pressing the enter key)
     Used by DactMainWindow to raise its window and show the tree
     */
    void entryActivated(QString file);

public slots:
    void cancelQuery();

    void copy();
    void exportSelection();
    void saveAs();


private slots:
    void applyValidityColor(QString const &text);
    /*
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    */
    /*!
     * Called when an item in the results list is activated
     */
    void entryActivated(QModelIndex const &index);

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
     Called when the search mapper was finished. Cancels the progress bar.
     \param processedEntries number of entries searched so far.
     \param totalEntries number of entries to search thru.
     */
    void progressFinished(int processedEntries, int totalEntries, bool cached);

    /*!
     Called when the search mapper was canceled. Cancels the progress bar.
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

    /*! Called when the execution of a query failed. */
    void queryFailed(QString error);

    void startQuery();

    void showFilenamesChanged();


protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.

private:
    void addListDelegate(QString const &name, DelegateFactory);
    void updateResults();
    void createActions();
    void initListDelegates();
    void reloadListDelegate();
    void setModel(FilterModel* model);
    void readSettings();
    void writeSettings();
    void selectionAsCSV(QTextStream &output);

    static QStyledItemDelegate* colorDelegateFactory(CorpusReaderPtr);
    static QStyledItemDelegate* visibilityDelegateFactory(CorpusReaderPtr);
    static QStyledItemDelegate* keywordInContextDelegateFactory(CorpusReaderPtr);

    QString d_filter;
    QList<DelegateFactory>d_listDelegateFactories;
    QSharedPointer<Ui::BracketedWindow> d_ui;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<FilterModel> d_model;
};

#endif // DACTFILTERWINDOW_H
