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
#include <QVector>

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
class StatisticsWindow;
class OpenProgressDialog;

class QGraphicsSvgItem;
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
    void help();
    void nextEntry(bool);
    void openCorpus();
    void openDirectoryCorpus();
    void pdfExport();
    void previousEntry(bool);
    void print();
    void queryChanged();
    void showSentence(QString const &xml, QHash<QString, QString> const &params);
    void showTree(QString const &xml, QHash<QString, QString> const &params);
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
    void initSentenceTransformer();
    void initTreeTransformer();
    void readSettings();
    void stopMapper();
    void writeSettings();
    QString sentenceForFile(QString const &filename, QString const &query);

    QSharedPointer<Ui::DactMainWindow> d_ui;
	AboutWindow *d_aboutWindow;
    BracketedWindow *d_bracketedWindow;
    StatisticsWindow *d_queryWindow;
    DactMacrosWindow *d_macrosWindow;
    OpenProgressDialog *d_openProgressDialog;
    
    QString d_corpusPath;
    QString d_query;
	QString d_filter;
	QString d_filterExpr; // Stores the raw, unexpanded filter expression
	QSharedPointer<DactMacrosModel> d_macrosModel;
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
    QGraphicsSvgItem *d_curTreeItem; // Scene-managed
};

#endif // DACTMAINWINDOW_H
