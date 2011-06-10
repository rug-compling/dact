#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QHash>
#include <QMainWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class MainWindow;
}

class AboutWindow;
class BracketedWindow;
class DownloadWindow;
class DactMacrosModel;
class DactMacrosWindow;
class DactQueryHistory;
class FilterModel;
class StatisticsWindow;
class PreferencesWindow;
class FilterModel;
class DactQueryWindow;
class DactTreeNode;
class DactTreeScene;

class QItemSelection;
class QKeyEvent;
class QProgressDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    /*!
     Start loading a corpus
     \param corpusPath path to a .dz or directory with the XML files
    */
    void readCorpus(QString const &corpusPath);

    bool readAndShowFiles(QString const &path);

signals:
    void exportProgressMaximum(int max);
    void exportProgress(int progress);
    void exportError(QString const &error);
    void openError(QString const &error);

public slots:
    /*!
     Hide the main window
    */
    void close();
    
    /*!
     Instantiate (if not already instantiated) and raise the download window.
     */
    void showDownloadWindow();
    
    /*!
     Instantiate (if not already instantiated) and raise the filter window, BracketedWindow.
    */
    void showFilterWindow();
    
    /*!
     Instantiate (if not already done so) and raise the StatisticsWindow
    */
    void showStatisticsWindow();
    
    /*!
     Instantiate (if not already done so) and raise the DactMacrosWindow
    */
    void showMacrosWindow();
    
    /*!
     * Shows last successfully shown xml file. Used to reload the tree
     * and bracketed sentence when the highlight query changes.
     */
    void showFile();
    
    /*!
     Show a xml file in the main window's tree scene.
     \param filename path to xml file to be used.
    */
    void showFile(QString const &filename);

private slots:
    /*!
     Raise the about window
    */
    void aboutDialog();
    
    /*!
     Attached to the highlight and filter query fields. Called every keypress to
     validate the entered query. Uses this->sender() to determine which line edit
     fired the event and should have it's background changed.
     \param text the entered query.
    */
    void applyValidityColor(QString const &text);
    
    /*!
     Called when one of the entries in the BracketedWindow is activated to display
     the file for that entry in the tree scene. The file is actually loaded at the
     currentBracketedEntryChanged signal which is fired when a entry is selected,
     but this slot will raise the main window to the foreground and replace the
     highlight query with the query from the BracketedWindow.
     \sa currentBracketedEntryChanged
    */
    void bracketedEntryActivated(QString const &file);
    
    /*!
     Cancels the writing loop when exporting files to a corpus.
     \sa exportCorpus
     \sa writeCorpus
     */
    void cancelWriteCorpus();
    
    /*!
     Listens for the resultReadyAt signal from the corpus reader. When heard, it hides
     the OpenProgressDialog, calls addFiles to start loading the file list and changes
     the current corpus used by the bracketed window and statistics window.
     \sa addFiles
     \param idx files available so far. Not used by corpusRead.
    */
    void corpusRead(int idx);

    void corpusWritten(int idx);
    
    /*!
     Called when an entry was added to the model.
     */
    void entryFound(QString entry);
    
    /*!
     Called when a file in the file list is selected (or the selection is removed.)
     It loads the selected file (if any) and focusses on the first matching node.
     \sa showFile
     \sa focusFitTree
     \param current currently selected entry
     \param previous previous selected entry
     */
    void entrySelected(QItemSelection const &current, QItemSelection const &previous);
    
    /*!
     * Save currently selected sentences to DBXML file (filename obtained from
     * dialog window).
     */
    void exportCorpus();

    /*!
     Asks for a destination and renders the current tree scene as a pdf file to
     that destination.
    */
    void exportPDF();
    
    /*!
     *  Write the currently selected sentences to a directory
     */
    void exportXML();
    
    /*!
     Called when [enter] is pressed in the filter query field, it copies the query to
     the highlight query and when a corpus is loaded, it starts filtering the files.
     \sa highlightChanged
     \sa addFiles
     */
    void filterChanged();
    
    void filterOnInspectorSelection();
    
    /*!
     Focusses on the first node and zooms in on that node.
     \sa forcusTreeNode
     */
    void focusFitTree();
    
    /*!
     Focus the filter query entry field
     */
    void focusFilter();
    
    /*!
     Focus the highlight query entry field
     */
    void focusHighlight();
    
    /*!
     Opens the wiki in the default webbrowser.
     */
    void help();
    
    /*!
     Called when the highlight query changed. Calls showFile to 'refresh' the
     tree scene.
     \sa filterChanged
     \sa showFile
     */
    void highlightChanged();
    
    /*!
     Selects the next entry in the file list. (This triggers the entrySelected
     slot which subsequenly calls showFile)
     \sa previousEntry
     */
    void nextEntry(bool);
    
    /*!
     Calls the open file dialog and filters on the .data.dz extension
     \sa openDirectoryCorpus
     \sa readCorpus
     */
    void openCorpus();
    
    /*!
     Calls the open file dialog and filters only allows you to select directories.
     \sa openCorpus
     \sa readCorpus
     */
    void openDirectoryCorpus();
    
    /*!
     Instantiates (if not already done so) and raises the PreferencesWindow
     */
    void preferencesWindow();
    
    /*!
     Selects the previous entry in the file list. See nextEntry.
     \sa nextEntry
     */
    void previousEntry(bool);
    
    /*!
     Renders the current tree scene to the printer.
     */
    void print();
    
    /*!
     Changes the filter query field used to filter the file list and calls
     filterChanged. Used to set the filter from one of the child windows.
     Currently the statistics window uses it to display all the entries
     that make up one of it's results rows.
     \param filter the XPath query.
     \sa setHighlight
     \sa filterChanged
     */
    void setFilter(QString const &filter);
    
    /*!
     Changes the highlight query field and calls highlightChanged.
     \param filter the XPath query which selects the nodes to highlight
     \sa setFilter
     \sa highlightChanged
     */
    void setHighlight(QString const &filter);
    
    /*!
     Displays a critical error dialog with the suplied error message.
     \sa exportError
     \sa showWriteCorpusError
     */
    void showOpenCorpusError(QString const &error);
    
    /*!
     Using the sentence stylsheet transformation it generates a sentence
     from an xml file and displays it in the sentence field below the tree
     scene.
     \param xml the contents of the xml file from the corpus
     \param params key-value pairs used by the stylesheet
     */
    void showSentence(QString const &xml, QHash<QString, QString> const &params);
    
    /*!
     Displays the xml file as a tree in the tree scene. It uses the tree styleheet
     to transform the xml file from the corpus into something DactTreeScene can use.
     It replaces the current DactTreeScene with a new one.
     \param xml the contents of the xml file form the corpus
     \param params key-value pairs used by the stylesheet
     */
    void showTree(QString const &xml);
    
    /*!
     Displays a critical error dialog with the supplied error message.
     \sa exportError
     \sa showOpenCorpusError
     */
    void showWriteCorpusError(QString const &error);
    
    /*!
     Listens to the entryActivated event from the statistics window which passes
     along the query to find all the results that toghether form the clicked row.
     \param value the value represented by the activated row
     \param query the XPath query which can be used to find all the nodes that
     represent the row.
     */
    void statisticsEntryActivated(QString const &value, QString const &query);
    
    /*!
     When the tree scene changes because of a new tree (or new highlight query)
     Add the selection listener to the new scene.
    */
    void treeChanged(DactTreeScene *scene);
    
    /*!
     Update the state of the next/previous node buttons in the toolbar.
     */
    void updateTreeNodeButtons();

    /*!
     When the mapper failed (e.g. due to an error in the corpus reader), hide
     the progress bar and display an error.
     \param error error message 
     \sa mapperStarted
     \sa mapperProgressed
     \sa mapperStopped
     */
    void mapperFailed(QString error);
    
    /*!
     When the mapper (the one used to find files that match the filter query) is
     started, this will make the progress bar visible.
     \param totalEntries number of entries in the corpus that will be searched
     \sa mapperFailed
     \sa mapperStopped
     \sa mapperProgressed
     */
    void mapperStarted(int totalEntries);
    
    /*!
     When the mapper stopped (because it finished or was cancelled), hide the
     progress bar.
     \param processedEntries number of entries searched
     \param totalEntries number of entries in the corpus
     \sa mapperFailed
     \sa mapperStarted
     \sa mapperProgressed
     */
    void mapperStopped(int processedEntries, int totalEntries);
    
protected:
    void changeEvent(QEvent *e);
    /*!
     Used to listen for the esc key to stop d_mapper filtering.
     */
    void keyPressEvent(QKeyEvent *event);

private:
    /*!
     Attaches all the signals from the ui and mapper to the various functions.
     */
    void createActions();
    
    /*!
     Focus on the next or previous tree node in the current tree scene. It finds the
     currently focussed node, and then walks using direction towards the next node
     that is active (which means it matched the highlight query) When the last node
     is reached, it continues with the first node again. It never walks more than 
     one round. Note that it walks the tree depth-first.
     \param direction number that is added to the position each step. -1 is effectively
     walk to the left while 1 is walk to the right.
     */
    void focusTreeNode(int direction);
    
    /*!
     Initialize and load stylesheet for the the sentence xsl transformer
     \sa sentenceForFile
     */
    void initSentenceTransformer();
        
    /*!
     Export a set of sentences as a dbxml .dact file to the given location.
     This can be run (and is run) on a different thread, and sends signals to
     inform the ui-thread of its progress.
     \sa exportProgressMaximum
     \sa exportProgress
     \sa exportError
     */
    bool writeCorpus(QString const &filename, QList<QString> const &files);
    
    /*!
     Read settings like the main window position and dimensions
     */
    void readSettings();
    
    /*!
     Write settings like the main window position and dimensions
     */
    void writeSettings();
    
    void setModel(FilterModel *model);
    
    /*!
     * Finishes the ui loading. Its main purpose is to align the toolbar
     * correctly. If this could be done in the ui file itself...
     */
    void setupUi();
    
    QSharedPointer<Ui::MainWindow> d_ui;
    AboutWindow *d_aboutWindow;
    BracketedWindow *d_bracketedWindow;
    DownloadWindow *d_downloadWindow;
    StatisticsWindow *d_statisticsWindow;
    DactMacrosWindow *d_macrosWindow;
    QProgressDialog *d_openProgressDialog;
    QProgressDialog *d_exportProgressDialog;
    PreferencesWindow *d_preferencesWindow;
    
    /*!
     The XPath query currently used to highlight nodes in the tree scene.
     */
    QString d_highlight;
    
    /*!
     The XPath query currently used for filtering files. This is after
     the macros have been expanded (so it's real XPath)
     */
    QString d_filter;
    
    /*!
     The XPath query currently used for filtering the files. This is
     the query as it was entered, before all the macros where replaced
     by their xpath counterparts.
     */
    QString d_filterExpr; // Stores the raw, unexpanded filter expression
    
    /*!
     * Last read file.
     */
    QString d_file;
    
    /*!
     The macros model. Used to store and apply macros to XPath queries.
     */
    QSharedPointer<DactMacrosModel> d_macrosModel;

    /*!
     Query history. Used to store the last x queries entered. It should come
     with an autocomplete function for the query fields, but it doesn't.. yet.
     */
#if 0
    QSharedPointer<DactQueryHistory> d_queryHistory;
#endif

    /*!
     XSLTransformer with a stylesheet loaded to transform a corpus xml
     into a plain text sentence.
     \sa showSentence
     */
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    
    QMutex d_addFilesMutex;
    QMutex d_filterChangedMutex;
    
    /*!
     XPath validator used by the filter and highlight query fields.
     \sa applyValidityColor
     */
    QSharedPointer<XPathValidator> d_xpathValidator;
        
    /*!
     \sa d_corpusOpenFuture
     \sa corpusRead
     */
    QFutureWatcher<void> d_corpusOpenWatcher;

    /*!
     \sa d_corpusWriteFuture
     \sa corpusWritten
     */
    QFutureWatcher<bool> d_corpusWriteWatcher;
    
    /*!
     Currently loaded corpus. Shared between all the windows that might need
     something from the corpus.
     */
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    
    /*!
     Current tree scene, used to display tree xml as a interactive visual tree.
     */
    DactTreeScene *d_treeScene;
    
    /*!
     Used in the for-loop in addFiles, the openProgressDialog's cancel button
     can set this to false to stop the loop.
     \sa addFiles
     */
    bool d_addFilesCancelled;
    
    /*!
     Used in the for-loop in writeCorpus, the exportProgressDialog's cancel
     button can set this to false to stop the writing of sentences to the 
     exported corpus.
     \sa writeCorpus
     */
    bool d_writeCorpusCancelled;
    
    QSharedPointer<FilterModel> d_model;
};

#endif // MAINWINDOW_H
