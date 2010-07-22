#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class DactMainWindow;
}

double const ZOOM_OUT_FACTOR = 0.8;
double const ZOOM_IN_FACTOR = 1.0 / ZOOM_OUT_FACTOR;

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
    void applyQuery();
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void showSentence(QString const &xml);
    void showTree(QString const &xml);
    void nextEntry(bool);
    void openCorpus();
    void previousEntry(bool);
    void queryChanged();
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
    QSharedPointer<XPathValidator> d_xpathValidator;
};

#endif // DACTMAINWINDOW_H
