#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QSharedPointer>
#include <QString>

#include <IndexedCorpus/CorpusReader.hh>

#include "XPathFilter.hh"
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

private slots:
    void aboutDialog();
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void filterChanged();
    void fitTree();
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

    Ui::DactMainWindow *d_ui;
    QString d_corpusPath;
    QString d_query;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XSLTransformer> d_treeTransformer;
    QSharedPointer<XPathFilter> d_xpathFilter;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<indexedcorpus::CorpusReader> d_corpusReader;
    QGraphicsSvgItem *d_curTreeItem; // Scene-managed
};

#endif // DACTMAINWINDOW_H
