#ifndef DACTMAINWINDOW_H
#define DACTMAINWINDOW_H

#include <QMainWindow>

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
    ~DactMainWindow();

public slots:
    void close();

private slots:
    void showSentence(QListWidgetItem *current, QListWidgetItem *previous);
    void showTree(QListWidgetItem *current, QListWidgetItem *previous);
    void nextEntry(bool);
    void previousEntry(bool);
    void queryChanged();
    void treeZoomIn(bool);
    void treeZoomOut(bool);

protected:
    void changeEvent(QEvent *e);

private:
    void addFiles();
    void readSettings();
    void writeSettings();

    Ui::DactMainWindow *d_ui;
    QString d_corpusPath;
    QString d_query;
};

#endif // DACTMAINWINDOW_H
