#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileOpenEvent>
#include <QScopedPointer>
#include <QSettings>
#include <QStandardItemModel>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <config.hh>
#include <AboutWindow.hh>
#include <DactApplication.hh>
#include <DactApplicationEvent.hh>
#include <DactMenuBar.hh>
#include <MainWindow.hh>
#include <OpenCorpusDialog.hh>
#include <PreferencesWindow.hh>
#ifdef USE_REMOTE_CORPUS
#include <RemoteWindow.hh>
#endif // USE_REMOTE_CORPUS
#ifdef USE_WEBSERVICE
#include <WebserviceWindow.hh>
#endif // USE_WEBSERVICE

DactApplication::DactApplication(int &argc, char** argv) :
    QApplication(argc, argv),
    d_dactStartedWithCorpus(false),
    d_historyModel(new QStandardItemModel),
    d_menu(new DactMenuBar(0, true)),
    d_aboutWindow(new AboutWindow(0, Qt::Window)),
#ifdef USE_REMOTE_CORPUS
    d_remoteWindow(new RemoteWindow(0, Qt::Window)),
#endif // USE_REMOTE_CORPUS
#ifdef USE_WEBSERVICE
    d_webserviceWindow(new WebserviceWindow),
#endif // USE_WEBSERVICE
    d_preferencesWindow(new PreferencesWindow)
{
#if defined(Q_WS_MAC)
    setQuitOnLastWindowClosed(false);
#endif

    readHistory("filterHistory");

    connect(this, SIGNAL(aboutToQuit()),
        SLOT(prepareQuit()));

#ifdef USE_REMOTE_CORPUS
    connect(d_remoteWindow.data(), SIGNAL(openRemote(QString const &)),
        SLOT(openCorpus(QString const &)));
#endif // USE_REMOTE_CORPUS

#ifdef USE_WEBSERVICE
    // Open a corpus if we have parsed some sentences.
    connect(d_webserviceWindow.data(),
        SIGNAL(parseSentencesFinished(QString)),
        SLOT(openCorpus(QString)));
#endif // USE_WEBSERVICE
}

void DactApplication::clearHistory()
{
  d_historyModel->clear();
}

void DactApplication::convertCorpus(QString const &path)
{
    QString newPath = QFileDialog::getSaveFileName(0,
        "New Dact corpus", QString("untitled"), "*.dact");
    if (newPath.isNull())
        return;

    MainWindow *window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();
    window->convertCorpus(path, newPath);

}


void DactApplication::convertCompactCorpus()
{
    QString corpusPath = QFileDialog::getOpenFileName(0,
        "Open compact corpus", QString(),
        QString("Compact corpora (*.data.dz)"));
    if (corpusPath.isNull())
        return;

    convertCorpus(corpusPath);
}

void DactApplication::convertDirectoryCorpus()
{
    QString corpusPath = QFileDialog::getExistingDirectory(0,
        "Open directory corpus");
    if (corpusPath.isNull())
        return;

    convertCorpus(corpusPath);
}

void DactApplication::emitColorPreferencesChanged()
{
    emit colorPreferencesChanged();
}

int DactApplication::exec()
{
    // If there is not a corpus opened in the first 50 miliseconds after
    // starting Dact, show the Open Corpus dialog. This because OS X sends
    // signals instead of commandline arguments.
    QTimer::singleShot(50, this, SLOT(showOpenCorpusLaunch()));

    return QCoreApplication::exec();
}

bool DactApplication::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *fileEvent(static_cast<QFileOpenEvent *>(event));
        
        if (!fileEvent->file().isEmpty())
            _openCorpora(QStringList(fileEvent->file()));

        else if (!fileEvent->url().isEmpty())
            _openUrl(fileEvent->url());
        
        else
            return false;

        return true;
    }

    if (event->type() == DactApplicationEvent::CorpusOpen)
    {
        DactApplicationEvent *appEvent(static_cast<DactApplicationEvent *>(event));

        _openCorpora(appEvent->data().toStringList());

        return true;   
    }

    if (event->type() == DactApplicationEvent::MacroOpen)
    {
        DactApplicationEvent *appEvent(static_cast<DactApplicationEvent *>(event));

        _openMacros(appEvent->data().toStringList());

        return true;
    }

    if (event->type() == DactApplicationEvent::UrlOpen)
    {
        DactApplicationEvent *appEvent(static_cast<DactApplicationEvent *>(event));

        _openUrl(appEvent->data().toUrl());

        return true;
    }
        
    
    return QApplication::event(event);
}

QStandardItemModel *DactApplication::historyModel()
{
    return d_historyModel.data();
}


void DactApplication::init()
{
}

void DactApplication::openCookbook()
{
    static QUrl const cookbook("http://rug-compling.github.com/dact/manual/cookbook.xhtml");
    QDesktopServices::openUrl(cookbook);
}

void DactApplication::openCorpus(QString const &filename)
{    
    QFileInfo fi(filename);

    MainWindow *window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();

    if (fi.isDir())
        window->readCorpus(filename, true);
    else
        window->readCorpus(filename);

    window->readMacros(d_argMacros);
}

void DactApplication::openCorpora(QStringList const &fileNames)
{
    postEvent(this, new DactApplicationEvent(DactApplicationEvent::CorpusOpen, fileNames));
}

void DactApplication::_openCorpora(QStringList const &fileNames)
{
    d_dactStartedWithCorpus = true;
    MainWindow *window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose, true);
    window->show();
    window->readCorpora(fileNames, true);
    window->readMacros(d_argMacros);
}

void DactApplication::openHelp()
{
    static QUrl const usage("http://rug-compling.github.com/dact/manual/");
    QDesktopServices::openUrl(usage);
}

void DactApplication::openMacros(QStringList const &fileNames)
{
    postEvent(this, new DactApplicationEvent(DactApplicationEvent::MacroOpen, fileNames));
}

void DactApplication::_openMacros(QStringList const &fileNames)
{
    d_argMacros = fileNames;
}

void DactApplication::openUrl(QUrl const &url)
{
    postEvent(this, new DactApplicationEvent(DactApplicationEvent::UrlOpen, url));
}

void DactApplication::_openUrl(QUrl const &url)
{
    if (url.scheme() != "dact")
        return;

    if (url.hasQueryItem("filter"))
    {
        QByteArray encodedFilter(url.queryItemValue("filter").toUtf8());
        // XXX: Reimplement
        //d_mainWindow->setFilter(QUrl::fromPercentEncoding(encodedFilter));
    }

    // Disabled because I don't trust this functionality. I think it can
    // be easily abused to let Dact open arbritray files. We then have to
    // trust dbxml for doing nothing stupid.
    #if 0
    if (url.hasQueryItem("corpus"))
    {
        QStringList fileNames;

        foreach (QString fileName, url.allQueryItemValues("corpus"))
        {
            QByteArray encodedFileName(fileName.toUtf8());
            fileNames << QUrl::fromPercentEncoding(encodedFileName);
        }

        d_mainWindow->readCorpora(fileNames);
    }
    #endif
}

void DactApplication::prepareQuit()
{
    // Explicitly close all windows before quitting. This will cancel
    // any running queries. If we do not do this, ~QCoreApplication
    // will wait until every thread in the thread pool is finished.
    closeAllWindows();

    writeHistory("filterHistory");
}

void DactApplication::readHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QSettings settings;
    QVariant value = settings.value(settingsKey, QStringList());

    if (value.type() == QVariant::StringList) {
      QStringList history(value.toStringList());

      QStringListIterator iter(history);
      while (iter.hasNext())
        d_historyModel->appendRow(new QStandardItem(iter.next()));
    }
    else
      qWarning() << "Read history, but it is not a QStringList.";
  }
}


void DactApplication::showAboutWindow()
{
    d_aboutWindow->show();
    d_aboutWindow->raise();
}

void DactApplication::showOpenCorpusLaunch()
{
    if (!d_dactStartedWithCorpus)
        showOpenCorpus();
}

void DactApplication::showOpenCorpus()
{
    QString corpusPath = OpenCorpusDialog::getCorpusFileName(0);
    
    if (corpusPath.isNull())
        return;

    openCorpus(corpusPath);
}

void DactApplication::showPreferencesWindow()
{
    connect(d_preferencesWindow.data(), SIGNAL(colorChanged()),
        SLOT(emitColorPreferencesChanged()));

    d_preferencesWindow->show();
    d_preferencesWindow->raise();
}

#ifdef USE_REMOTE_CORPUS
void DactApplication::showRemoteWindow()
{
    d_remoteWindow->show();
    d_remoteWindow->raise();
}
#endif // USE_REMOTE_CORPUS

#ifdef USE_WEBSERVICE
void DactApplication::showWebserviceWindow()
{
    d_webserviceWindow->setWindowModality(Qt::WindowModal);

    d_webserviceWindow->show();
    d_webserviceWindow->raise();
}
#endif // USE_WEBSERVICE

void DactApplication::writeHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QStringList history;

    for (int i = 0; i < d_historyModel->rowCount(); ++i)
      history << d_historyModel->item(i)->text();

    QSettings settings;
    settings.setValue(settingsKey, history);
  }
}
