#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QFuture>
#include <QFutureWatcher>
#include <QHash>
#include <QMainWindow>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "DactFilterWindow.h"
#include "DactHelpWindow.h"
#include "DactQueryWindow.h"
#include "DactMacrosWindow.h"
#include "XPathFilter.hh"
#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
	class DactMainWindow;
}

double const ZOOM_OUT_FACTOR = 0.8;
double const ZOOM_IN_FACTOR = 1.0 / ZOOM_OUT_FACTOR;

class QGraphicsSvgItem;
class QListWidgetItem;

class DactMainWindow : public QMainWindow {
    Q_OBJECT
public:
    DactMainWindow(QWidget *parent = 0);
    DactMainWindow(QString const &corpusPath, QWidget *parent = 0);
    ~DactMainWindow();

public slots:
    void close();
    void showFilterWindow();
    void showQueryWindow();
    void showMacrosWindow();
    void showFile(QString const &filename);

private slots:
    void aboutDialog();
    void allEntriesFound();
    void applyValidityColor(QString const &text);
    void bracketedEntryActivated();
    void currentBracketedEntryChanged(QString const &entry);
    void entryFound(QString entry);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void filterChanged();
    void fitTree();
    void help();
    void nextEntry(bool);
    void openCorpus();
    void pdfExport();
    void previousEntry(bool);
    void print();
    void queryChanged();
    void showSentence(QString const &xml, QHash<QString, QString> const &params);
    void showTree(QString const &xml, QHash<QString, QString> const &params);
    void treeZoomIn(bool);
    void treeZoomOut(bool);
    void toggleSentencesInFileList(bool show);

protected:
    void changeEvent(QEvent *e);

private:
    void addFiles();
    void createActions();
    void createTransformers();
    void initSentenceTransformer();
    void initTreeTransformer();
    void readSettings();
    void writeSettings();
    QString sentenceForFile(QString const &filename, QString const &query);

    QSharedPointer<Ui::DactMainWindow> d_ui;
    DactFilterWindow *d_filterWindow;
    DactQueryWindow *d_queryWindow;
    DactHelpWindow *d_dactHelpWindow;
    DactMacrosWindow *d_macrosWindow;
    
    QString d_corpusPath;
    QString d_query;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XSLTransformer> d_treeTransformer;
    EntryFun d_entryFun;
    EntryMap d_entryMap;
    QSharedPointer<XPathMapper> d_xpathMapper;
    QSharedPointer<XPathFilter> d_xpathFilter;
    QMutex d_addFilesMutex;
    QMutex d_filterChangedMutex;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QGraphicsSvgItem *d_curTreeItem; // Scene-managed
};

#endif // DACTMAINWINDOW_H
