#ifndef DEPENDENCYTREEWIDGET_HH
#define DEPENDENCYTREEWIDGET_HH

#include <QModelIndex>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include <AlpinoCorpus/CorpusReader.hh>

#include "CorpusWidget.hh"
#include "DactMacrosModel.hh"
#include "FilterModel.hh"
#include "XPathValidator.hh"
#include "ui_DependencyTreeWidget.h"

class DactMacrosModel;
class DactToolsModel;
class DactTreeScene;
class QPainter;
class QItemSelectionModel;

class DependencyTreeWidget : public CorpusWidget
{
    Q_OBJECT
public:
    DependencyTreeWidget(QWidget *parent);
    ~DependencyTreeWidget();
    
    // Provide access to the sentence widget.
    BracketedSentenceWidget *sentenceWidget();

    // XXX - do we want this?
    QItemSelectionModel *selectionModel();
    
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
    void setMacrosModel(QSharedPointer<DactMacrosModel> macrosModel);
    void readSettings();
    void renderTree(QPainter *painter);

    bool saveEnabled() const;
    
    // XXX - hack, kill asap
    DactTreeScene *scene();
    
    void writeSettings();
    
signals:
    void sceneChanged(DactTreeScene *scene);
    
public slots:
    void cancelQuery();
    void saveAs();
    
    /*!
     Focus the highlight query entry field
     */
    void focusHighlight();

    void focusNextTreeNode();
    
    void focusPreviousTreeNode();
    
    void fitTree();
    
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
     Selects the previous entry in the file list. See nextEntry.
     \sa nextEntry
     */
    void previousEntry(bool);
    
    void setFilter(QString const &filter, QString const &raw_filter);
    
    void showFile(QString const &entry);
    
    void zoomIn();
    
    void zoomOut();

    void copy();

    void showToolMenu(QPoint const &position);

private slots:
    /*!
     Attached to the highlight and filter query fields. Called every keypress to
     validate the entered query. Uses this->sender() to determine which line edit
     fired the event and should have it's background changed.
     \param text the entered query.
     */
    void applyValidityColor(QString const &text);

    /*!
     Called when entries were added to the model.
     */
    void nEntriesFound(int entries, int hits);
    
    /*!
     Called when a file in the file list is selected (or the selection is removed.)
     It loads the selected file (if any) and focusses on the first matching node.
     \sa showFile
     \sa focusFitTree
     \param current currently selected entry
     \param previous previous selected entry
     */
    void entrySelected(QModelIndex const &current, QModelIndex const &previous);

    /*!
     Focuses on the first node matching a query.
     \sa focusNextTreeNode
     */
    void focusFirstMatch();

    
    /*!
     Focusses on the first node and zooms in on that node.
     \sa forcusTreeNode
     */
    void focusFitTree();
    
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
     When the mapper stopped (because it finished), hide the
     progress bar.
     \param processedEntries number of entries searched
     \param totalEntries number of entries in the corpus
     \param cached entries were received from cache
     \sa mapperFailed
     \sa mapperStopped
     \sa mapperStarted
     \sa mapperProgressed
     */
    void mapperFinished(int processedEntries, int totalEntries, bool cached);
    
    /*!
     When the mapper stopped (because it cancelled), hide the
     progress bar.
     \param processedEntries number of entries searched
     \param totalEntries number of entries in the corpus
     \sa mapperFailed
     \sa mapperFinished
     \sa mapperStarted
     \sa mapperProgressed
     */
    void mapperStopped(int processedEntries, int totalEntries);

    /*!
     * The progress in the processing of the query changed.
     */
    void progressChanged(int percentage);
    
private:
    void addConnections();
    
    /*!
     Changes the highlight query field and calls highlightChanged.
     \param filter the XPath query which selects the nodes to highlight
     \sa setFilter
     \sa highlightChanged
     */
    void setHighlight(QString const &filter);
    
    void setModel(QSharedPointer<FilterModel> model);
    
    /*!
     * Shows last successfully shown xml file. Used to reload the tree
     * and bracketed sentence when the highlight query changes.
     */
    void showFile();
        
    /*!
     Using the sentence stylsheet transformation it generates a sentence
     from an xml file and displays it in the sentence field below the tree
     scene.
     \param xml the contents of the xml file from the corpus
     \param params key-value pairs used by the stylesheet
     */
    void showSentence(QString const &entry, QString const &query);
    
    /*!
     Displays the xml file as a tree in the tree scene. It uses the tree styleheet
     to transform the xml file from the corpus into something DactTreeScene can use.
     It replaces the current DactTreeScene with a new one.
     \param xml the contents of the xml file form the corpus
     \param params key-value pairs used by the stylesheet
     */
    void showTree(QString const &xml);
    
    QScopedPointer<Ui::DependencyTreeWidget> d_ui;
    QSharedPointer<FilterModel> d_model;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    
    /*!
     The macros model. Used to store and apply macros to XPath queries.
     */
    QSharedPointer<DactMacrosModel> d_macrosModel;

    QSharedPointer<DactToolsModel> d_toolModel;
    
    QSharedPointer<XPathValidator> d_xpathValidator;
    
    /*!
     * Last read file.
     */
    QString d_file;
    
    QString d_filter;
    
    /*!
     The XPath query currently used to highlight nodes in the tree scene.
     */
    QString d_highlight;

    bool d_treeShown;
};

#endif // DEPENDENCYTREEWIDGET

