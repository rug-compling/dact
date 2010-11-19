#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QFuture>
#include <QFutureWatcher>
#include <QHash>
#include <QMainWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QTimer>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathFilter.hh"
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
class StatisticsWindow;
class OpenProgressDialog;
class PreferencesWindow;
class DactQueryWindow;
class DactTreeNode;
class DactTreeScene;

class QListWidgetItem;
class QKeyEvent;

class DactMainWindow : public QMainWindow {
    Q_OBJECT
public:
    DactMainWindow(QWidget *parent = 0);
    ~DactMainWindow();
    void readCorpus(QString const &corpusPath);

public slots:
    void close();
    void showFilterWindow();
    void showStatisticsWindow();
    void showMacrosWindow();
    void showFile(QString const &filename);

private slots:
    void aboutDialog();
    void allEntriesFound();
    void applyValidityColor(QString const &text);
    void bracketedEntryActivated();
    void corpusOpenTick();
    void corpusRead(int idx);
    void currentBracketedEntryChanged(QString const &entry);
    void entryFound(QString entry);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void filterChanged();
    void fitTree();
	void focusFitTree();
    void focusNextTreeNode(bool);
    void focusPreviousTreeNode(bool);
    void help();
    void highlightChanged();
    void nextEntry(bool);
    void openCorpus();
    void openDirectoryCorpus();
    void pdfExport();
    void preferencesWindow();
    void previousEntry(bool);
    void print();
	void resetTreeZoom();
	void setFilter(QString const &filter);
	void setHighlight(QString const &filter);
    void showSentence(QString const &xml, QHash<QString, QString> const &params);
    void showTree(QString const &xml, QHash<QString, QString> const &params);
	void statisticsEntryActivated(QString const &value, QString const &query);
    void treeZoomIn(bool);
    void treeZoomOut(bool);
    void toggleSentencesInFileList(bool show);
    void mapperStarted(int);
    void mapperStopped(int, int);
    void mapperProgressed(int, int);

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *event);

private:
    void addFiles();
    void createActions();
    void createTransformers();
    void init();
    void focusTreeNode(int direction);
    QString getHighlightQuery();
    void initSentenceTransformer();
    void initTreeTransformer();
    void readSettings();
    void stopMapper();
    void writeSettings();
    QString sentenceForFile(QString const &filename, QString const &query);

    QSharedPointer<Ui::DactMainWindow> d_ui;
    AboutWindow *d_aboutWindow;
    BracketedWindow *d_bracketedWindow;
    DactMacrosWindow *d_macrosWindow;
    OpenProgressDialog *d_openProgressDialog;
    PreferencesWindow *d_preferencesWindow;
    StatisticsWindow *d_statisticsWindow;
    
    QString d_corpusPath;
    QString d_highlight;
    QString d_filter;
    QString d_filterExpr; // Stores the raw, unexpanded filter expression
    QSharedPointer<DactMacrosModel> d_macrosModel;
	QSharedPointer<DactQueryHistory> d_queryHistory;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XSLTransformer> d_treeTransformer;
    EntryFun d_entryFun;
    EntryMap d_entryMap;
    QSharedPointer<XPathMapper> d_xpathMapper;
    QMutex d_addFilesMutex;
    QMutex d_filterChangedMutex;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QFuture<bool> d_corpusOpenFuture;
    QTimer d_corpusOpenTimer;
    QFutureWatcher<bool> d_corpusOpenWatcher;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    DactTreeScene *d_treeScene;
};

#endif // DACTMAINWINDOW_H
