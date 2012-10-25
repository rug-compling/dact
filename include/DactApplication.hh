#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QMenuBar>
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
    int exec();
   
public slots:
	void showOpenCorpus();

protected:
    bool event(QEvent *event);

private:
    void _openCorpora(QStringList const &fileNames);
    void _openMacros(QStringList const &fileNames);
    void _openUrl(QUrl const &url);
    QScopedPointer<MainWindow> d_mainWindow;
    bool d_dactStartedWithCorpus;
    QScopedPointer<QMenuBar> d_menu;
};

#endif
