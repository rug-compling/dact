#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileOpenEvent>
#include <QScopedPointer>
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
#ifdef USE_WEBSERVICE
#include <WebserviceWindow.hh>
#endif // USE_WEBSERVICE

DactApplication::DactApplication(int &argc, char** argv) :
    QApplication(argc, argv),
    d_menu(new DactMenuBar(0, true)),
    d_aboutWindow(new AboutWindow(0, Qt::Window)),
#ifdef USE_WEBSERVICE
    d_webserviceWindow(new WebserviceWindow),
#endif // USE_WEBSERVICE
    d_preferencesWindow(new PreferencesWindow)
{
#if defined(Q_WS_MAC)
    setQuitOnLastWindowClosed(false);
#endif
    //

#ifdef USE_WEBSERVICE
    // Open a corpus if we have parsed some sentences.
    connect(d_webserviceWindow.data(),
        SIGNAL(parseSentencesFinished(QString)),
        SLOT(openCorpus(QString)));
#endif // USE_WEBSERVICE
}

void DactApplication::convertCorpus(QString const &path)
{
    QString newPath = QFileDialog::getSaveFileName(0,
        "New Dact corpus", QString(), "*.dact");
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

void DactApplication::init()
{
}

int DactApplication::exec()
{
    // If there is not a corpus opened in the first 50 miliseconds after
    // starting Dact, show the Open Corpus dialog. This because OS X sends
    // signals instead of commandline arguments.
    if (!d_dactStartedWithCorpus)
        QTimer::singleShot(50, this, SLOT(showOpenCorpus()));

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
/*    d_mainWindow->readMacros(fileNames);*/
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

void DactApplication::showAboutWindow()
{
    d_aboutWindow->show();
    d_aboutWindow->raise();
}

void DactApplication::showOpenCorpus()
{
//    if (!d_dactStartedWithCorpus)
//        d_mainWindow->openCorpus();
    QString corpusPath = OpenCorpusDialog::getCorpusFileName(0);
    
    if (corpusPath.isNull())
        return;

    openCorpus(corpusPath);
}

void DactApplication::showPreferencesWindow()
{

    // Propagate preference changes...
    /* XXX(OSX): make some DactApplication signal.
    connect(d_preferencesWindow, SIGNAL(colorChanged()),
            d_ui->dependencyTreeWidget->sentenceWidget(), SLOT(colorChanged()));
    connect(d_preferencesWindow, SIGNAL(colorChanged()),
            d_ui->sentencesWidget, SLOT(colorChanged()));
    */

    d_preferencesWindow->show();
    d_preferencesWindow->raise();
}

#ifdef USE_WEBSERVICE
void DactApplication::showWebserviceWindow()
{
    d_webserviceWindow->setWindowModality(Qt::WindowModal);

    d_webserviceWindow->show();
    d_webserviceWindow->raise();
}
#endif // USE_WEBSERVICE
