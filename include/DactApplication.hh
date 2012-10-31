#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QMenuBar>
#include <QScopedPointer>
#include <QStringList>
#include <QUrl>

#include "config.hh"
#include "AboutWindow.hh"
#include "MainWindow.hh"
#include "PreferencesWindow.hh"
#ifdef USE_REMOTE_CORPUS
#include <RemoteWindow.hh>
#endif // USE_REMOTE_CORPUS
#ifdef USE_WEBSERVICE
#include "WebserviceWindow.hh"
#endif // USE_WEBSERVICE

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

signals:
    void colorPreferencesChanged();

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

#ifdef USE_REMOTE_CORPUS
    /*!
     Instantiate (if not already instantiated) and raise the remote window.
     */
    void showRemoteWindow();
#endif // USE_REMOTE_CORPUS


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
    void prepareQuit();

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

    bool d_dactStartedWithCorpus;
    QStringList d_argMacros;
    QScopedPointer<QMenuBar> d_menu;
    QScopedPointer<AboutWindow> d_aboutWindow;
#ifdef USE_REMOTE_CORPUS
    QScopedPointer<RemoteWindow> d_remoteWindow;
#endif // USE_REMOTE_CORPUS
#ifdef USE_WEBSERVICE
    QScopedPointer<WebserviceWindow> d_webserviceWindow;
#endif // USE_WEBSERVICE
    QScopedPointer<PreferencesWindow> d_preferencesWindow;
};

#endif
