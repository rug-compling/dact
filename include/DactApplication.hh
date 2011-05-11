#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QScopedPointer>
#include "MainWindow.hh"

class DactApplication: public QApplication
{
    Q_OBJECT
public:
    DactApplication(int &argc, char** argv);
    void init();
    void openCorpus(QString const &fileName);
protected:
    bool event(QEvent *event);

private:
    QScopedPointer<MainWindow> d_mainWindow;
};

#endif
