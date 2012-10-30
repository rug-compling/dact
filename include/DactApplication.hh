#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QMenuBar>
#include <QScopedPointer>
#include <QUrl>

#include "AboutWindow.hh"
#include "MainWindow.hh"
#include "PreferencesWindow.hh"

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
    /*!
      Convert a compact corpus to a Dact corpus.
     */
    void convertCompactCorpus();

    /*!
      Convert a directory corpus to a Dact corpus.
     */
    void convertDirectoryCorpus();

    void openCookbook();
    void openCorpus(QString const &filename);

    /*!
     Opens manual in the default webbrowser.
     */
    void openHelp();

    void showAboutWindow();
	void showOpenCorpus();

    /*!
     Raises the PreferencesWindow.
     */
    void showPreferencesWindow();


protected:
    bool event(QEvent *event);

private:
    void convertCorpus(QString const &path);
    void _openCorpora(QStringList const &fileNames);
    void _openMacros(QStringList const &fileNames);
    void _openUrl(QUrl const &url);

    bool d_dactStartedWithCorpus;
    QScopedPointer<QMenuBar> d_menu;
    QScopedPointer<AboutWindow> d_aboutWindow;
    QScopedPointer<PreferencesWindow> d_preferencesWindow;
};

#endif
