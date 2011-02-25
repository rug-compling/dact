#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QFutureWatcher>
#include <QHash>
#include <QMainWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class DactMainWindow;
}

double const ZOOM_OUT_FACTOR = 0.8;
double const ZOOM_IN_FACTOR = 1.0 / ZOOM_OUT_FACTOR;

class AboutWindow;
class BracketedWindow;
class DactMacrosModel;
class DactMacrosWindow;
class DactQueryHistory;
class DactQueryModel;
class StatisticsWindow;
class DactProgressDialog;
class PreferencesWindow;
class DactQueryModel;
class DactQueryWindow;
class DactTreeNode;
class DactTreeScene;

class QItemSelection;
class QKeyEvent;

class DactMainWindow : public QMainWindow {
    Q_OBJECT
public:
    DactMainWindow(QWidget *parent = 0);
    ~DactMainWindow();
    
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
     Show a xml file in the main window's tree scene and select it in the list of
     files (if it isn't hidden by the file list filter query).
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
    void bracketedEntryActivated();
    
    /*!
     Cancels reading the iterator in addFiles. Invoked by the open-file
     dialog's cancel button.
     */
    void cancelReadCorpus();
    
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
     Called when one of the entries in the BracketedWindow is selected. It will
     load the matching file in the tree scene, but it won't raise the window. This
     because you can then use the cursor keys to scan thru the results and see
     the tree for the currently selected result in the background. Calls fitTree.
     \sa showFile
     \sa fitTree
    */
    void currentBracketedEntryChanged(QString const &entry);
    
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
     * Save currently opened corpus to DBXML file (filename obtained from
     * dialog window).
     */
    void exportCorpus();

    /*!
     Asks for a destination and renders the current tree scene as a pdf file to
     that destination.
    */
    void exportPDF();
    
    /*!
     Called when [enter] is pressed in the filter query field, it copies the query to
     the highlight query and when a corpus is loaded, it starts filtering the files.
     \sa highlightChanged
     \sa addFiles
     */
    void filterChanged();
    
    /*!
     Zooms the tree scene just enough to fit the complete tree.
     \sa focusFitTree
    */
    void fitTree();
    
    /*!
     Focusses on the first node and zooms in on that node.
     \sa forcusTreeNode
     */
    void focusFitTree();
    
    /*!
     Focus on the next matching node. Wraps around.
     \sa focusPreviousTreeNode
     \sa focusTreeNode
     */
    void focusNextTreeNode();
    
    /*!
     Focus on the previous matching node. Wraps around.
     \sa focusNextTreeNode
     \sa focusTreeNode
     */
    void focusPreviousTreeNode();
    
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
     Resets the zoom matrix for the tree scene. 1px is 1px again.
     */
    void resetTreeZoom();

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
    void showTree(QString const &xml, QHash<QString, QString> const &params);
    
    /*!
     Displays a critical error dialog with the supplied error message.
     \sa exportError
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
     Magnifies the tree scene
     \sa treeZoomOut
     \sa resetTreeZoom
     \sa focusFitTree
    */
    void treeZoomIn();
    
    /*!
     Zooms the tree scene out.
     \sa treeZoomIn
     \sa resetTreeZoom
     \sa focusFitTree
     */
    void treeZoomOut();
    
    /*!
     Update the state of the next/previous node buttons in the toolbar.
     */
    void updateTreeNodeButtons();
    
    /*!
     When the mapper (the one used to find files that match the filter query) is
     started, this will make the progress bar visible.
     \param totalEntries number of entries in the corpus that will be searched
     \sa mapperStopped
     \sa mapperProgressed
     */
    void mapperStarted(int totalEntries);
    
    /*!
     When the mapper stopped (because it finished or was cancelled), hide the
     progress bar.
     \param processedEntries number of entries searched
     \param totalEntries number of entries in the corpus
     \sa mapperStarted
     \sa mapperProgressed
     */
    void mapperStopped(int processedEntries, int totalEntries);
    
    /*!
     Updates the file search progress bar.
     \param processedEntries number of entries searched.
     \param totalEntries number of entries in the corpus
     \sa mapperStarted
     \sa mapperStoppend
     */
    void mapperProgressed(int processedEntries, int totalEntries);
    
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
     Initialize and load the stylesheet for the tree xsl transformer
     \sa showTree
     */
    void initTreeTransformer();
    
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
    
    void setModel(DactQueryModel *model);
    
    QSharedPointer<Ui::DactMainWindow> d_ui;
    AboutWindow *d_aboutWindow;
    BracketedWindow *d_bracketedWindow;
    DactMacrosWindow *d_macrosWindow;
    DactProgressDialog *d_openProgressDialog;
    DactProgressDialog *d_exportProgressDialog;
    PreferencesWindow *d_preferencesWindow;
    StatisticsWindow *d_statisticsWindow;
    
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
    
    /*!
     XSLTransformer with a stylesheet to turn corpus xml into simple xml
     used by the DactTreeScene.
     \sa showTree
     */
    QSharedPointer<XSLTransformer> d_treeTransformer;
    
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
    
    QSharedPointer<DactQueryModel> d_model;
};

#endif // DACTMAINWINDOW_H
