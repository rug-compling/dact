#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QScopedPointer>
#include <QUrl>
#include "MainWindow.hh"

class DactApplication: public QApplication
{
    Q_OBJECT
public:
    DactApplication(int &argc, char** argv);
    void init();
    void openCorpora(QStringList const &fileNames);
    void openMacros(QStringList const &fileNames);
    void openUrl(QUrl const &url);
    void showOpenCorpus();
protected:
    bool event(QEvent *event);

private:
    QScopedPointer<MainWindow> d_mainWindow;
};

#endif
