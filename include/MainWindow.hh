#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QMainWindow>
#include <QMutex>
#include <QPair>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "AutoUpdater.hh"
#include "CorpusWidget.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "config.hh"

namespace Ui {
    class MainWindow;
}

class DactMacrosModel;
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

signals:
    void corpusReaderCreated();
    void corpusWriterFinished(QString const &filename);
    void queryCancelRequest();
    void exportProgressMaximum(int max);
    void exportProgress(int progress);
    void exportError(QString const &error);
    void openError(QString const &error);

public slots:
    /*!
     Cancel a running query
     */
    void cancelQuery();

    /*!
     Hide the main window
    */
    void close();

    /*!
     Convert the given corpus to a Dact corpus.
     */
    void convertCorpus(QString const &path, QString const &writePath);

    /*!
     Start loading a corpus
     \param corpusPath path to a .dz or directory with the XML files
    */
    void readCorpus(QString const &corpusPath, bool recursive = false);

    void readCorpora(QStringList const &corpusPaths, bool recursive = false);

    void readMacros(QStringList const &macroPaths);

    /*!
     The save state of a widget was modified.
    */
    void saveStateChanged();

    /*!
     Changes the filter query field used to filter the file list and calls
     filterChanged. Used to set the filter from one of the child windows.
     Currently the statistics window uses it to display all the entries
     that make up one of it's results rows.
     \param filter the XPath query.
     \sa filterChanged
     */
    void setFilter(QString const &filter);

    /*!
     Toggle toolbar visibility.
    */
    void setToolbarVisible(bool visible);

    void saveAs();

    void statusMessage(QString message);

    void checkForUpdates();

    void makeActiveWindow();

private slots:
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
     Listens for the finished signal from the corpus readers. When heard, it hides
     the OpenProgressDialog, calls addFiles to start loading the file list and changes
     the current corpus used by the bracketed window and statics window.
     \sa addFiles
    */
    void corporaRead();

    /*!
     One out of multiple corpora is read. Updates the opening progressbar.
     */
    void corpusRead();

    void corpusWritten(QString const &filename);

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
     Focus the filter query entry field
     */
    void focusFilter();

    /*!
     Report an error in reading macros.
     */
    void macrosReadError(QString error);

    void openMacrosFile();

    /*!
     Renders the current tree scene to the printer.
     */
    void print();

    void setCorpusReader(QSharedPointer<ac::CorpusReader> reader, QString const &path);

    /*!
     Displays a critical error dialog with the suplied error message.
     \sa exportError
     \sa showWriteCorpusError
     */
    void showOpenCorpusError(QString const &error);

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
     * The tab in the main window is changed.
     */
    void tabChanged(int index);

    /*!
     When the tree scene changes because of a new tree (or new highlight query)
     Add the selection listener to the new scene.
    */
    void treeChanged(DactTreeScene *scene);

    /*!
     Update the state of the next/previous node buttons in the toolbar.
     */
    void updateTreeNodeButtons();

    void setInspectorVisible(bool);

    void toggleFullScreen();

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
     Initialize the tainted widget list.
     */
    void initTaintedWidgets();

    /*!
     Taint all widget in the taint list.
     */
    void taintAllWidgets();

    /*!
     Export a set of sentences as a dbxml .dact file to the given location.
     This can be run (and is run) on a different thread, and sends signals to
     inform the ui-thread of its progress.
     \sa exportProgressMaximum
     \sa exportProgress
     \sa exportError
     */
    bool writeCorpus(QString const &filename,
        QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QList<QString> const &files);

    /*!
     Given a path create one CorpusReader. In an error occurs, an openError(QString)
     event is emitted and the pointer is a null-pointer.
     */
    QPair<ac::CorpusReader*, QString> createCorpusReader(QString const &path, bool recursive = false);

    /*!
     Creates one CorpusReader for all the corpora found at paths. If only one path is provided,
     the CorpusReader will not be wrapped by a MultiCorpusReader. If opening one of the paths results
     in an error, an openError(QString) event is emitted and the reader is not added to the wrapping list.
     \sa createCorpusReaders
     */
    QPair<ac::CorpusReader*, QString> createCorpusReaders(QStringList const &paths, bool recursive = false);

    /*!
     Create a reasonable name for the corpus based on its path.
     */
    QString deriveNameFromPath(QString const &path) const;

    /*!
     Read settings like the main window position and dimensions
     */
    void readSettings();

    /*!
     Write settings like the main window position and dimensions
     */
    void writeSettings();

    /*!
     * Finishes the ui loading. Its main purpose is to align the toolbar
     * correctly. If this could be done in the ui file itself...
     */
    void setupUi();

    void enableFullScreen();

    QScopedPointer<Ui::MainWindow> d_ui;
    QProgressDialog *d_openProgressDialog;
    QProgressDialog *d_exportProgressDialog;

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
    
    QMutex d_addFilesMutex;
    QMutex d_filterChangedMutex;

    /*!
     XPath validator used by the filter and highlight query fields.
     \sa applyValidityColor
     */
    QScopedPointer<XPathValidator> d_xpathValidator;

    /*!
     \sa d_corpusOpenFuture
     \sa corpusRead
     */
    QFutureWatcher< QPair< ac::CorpusReader*, QString> > d_corpusOpenWatcher;

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
     Used in the for-loop in writeCorpus, the exportProgressDialog's cancel
     button can set this to false to stop the writing of sentences to the
     exported corpus.
     \sa writeCorpus
     */
    bool d_writeCorpusCancelled;

    /*!
     Keep track of tabs/widgets that are 'tainted'. Widgets that are tainted
     need to have their query reset when switching to their tab. This makes
     querying of corpora a bit more lazy.
     */
    QVector<QPair<CorpusWidget *, bool> > d_taintedWidgets;

    bool d_inspectorVisible;

    QSharedPointer<AutoUpdater> d_autoUpdater;
};

#endif // MAINWINDOW_H
