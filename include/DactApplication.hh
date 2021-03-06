#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QMenuBar>
#include <QScopedPointer>
#include <QStandardItemModel>
#include <QStringList>
#include <QUrl>

#include "config.hh"
#include "AboutWindow.hh"
#include "MainWindow.hh"
#include "PreferencesWindow.hh"
#include "QtSingleApplication.hh"
#ifdef USE_WEBSERVICE
#include "WebserviceWindow.hh"
#endif // USE_WEBSERVICE

QString const CORPUS_OPEN_MESSAGE = "openCorpus:";
QString const CORPUS_SEPARATOR = "+:+";

class DactApplication: public QtSingleApplication
{
    Q_OBJECT
public:
    DactApplication(int &argc, char** argv);
    QStandardItemModel *historyModel();
    void init();
    void openCorpora(QStringList const &fileNames);
    void openMacros(QStringList const &fileNames);
    void openUrl(QUrl const &url);
    int exec();

signals:
    void colorPreferencesChanged();

public slots:
    /*!
     Clear the query history
     */
    void clearHistory();

    /*!
      Convert a compact corpus to a Dact corpus.
     */
    void convertCompactCorpus();

    /*!
      Convert a directory corpus to a Dact corpus.
     */
    void convertDirectoryCorpus();

    void openCookbook();
    MainWindow *openCorpus(QString const &filename);

    /*!
     Opens manual in the default webbrowser.
     */
    void openHelp();

    void showAboutWindow();
    MainWindow *showOpenCorpus();

    /*!
     Raises the PreferencesWindow.
     */
    void showPreferencesWindow();

#ifdef USE_WEBSERVICE
    /*!
     Instantiate (if not already instantiated) and raise the Alpinowebservice query window.
     */
    void showWebserviceWindow();
#endif // USE_WEBSERVICE

protected:
    bool event(QEvent *event);

private slots:
    void emitColorPreferencesChanged();
    void handleMessage(QString const &msg);
    void prepareQuit();
    void updateLastMainWindow(QWidget *old, QWidget *now);

    /*!
     Used when starting Dact, does not show the dialog if a corpus was provided
     as a command-line argument.
     */
    void showOpenCorpusLaunch();

private:
    void convertCorpus(QString const &path);
    void _openCorpora(QStringList const &fileNames);
    void _openMacros(QStringList const &fileNames);
    void _openUrl(QUrl const &url);
    void readHistory(QString const &settingsKey);
    void writeHistory(QString const &settingsKey);

    bool d_dactStartedWithCorpus;
    QStringList d_argMacros;
    QScopedPointer<QStandardItemModel> d_historyModel;
    QScopedPointer<QMenuBar> d_menu;
    QScopedPointer<AboutWindow> d_aboutWindow;
#ifdef USE_WEBSERVICE
    QScopedPointer<WebserviceWindow> d_webserviceWindow;
#endif // USE_WEBSERVICE
    QScopedPointer<PreferencesWindow> d_preferencesWindow;
    MainWindow *d_lastMainWindow;
};

#endif
