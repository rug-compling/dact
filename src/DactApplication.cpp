#include "DactApplication.hh"
#include "DactApplicationEvent.hh"
#include <QDebug>
#include <QFileOpenEvent>
#include <QTimer>

DactApplication::DactApplication(int &argc, char** argv)
:
    QApplication(argc, argv),
    d_mainWindow(0)   
{
    //
}

void DactApplication::init()
{
    d_mainWindow.reset(new MainWindow());
    d_mainWindow->show();
}

int DactApplication::exec()
{
    // If there is not a corpus opened in the first 50 miliseconds after
    // starting Dact, show the Open Corpus dialog. This because OS X sends
    // signals instead of commandline arguments.
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

void DactApplication::openCorpora(QStringList const &fileNames)
{
    postEvent(this, new DactApplicationEvent(DactApplicationEvent::CorpusOpen, fileNames));
}

void DactApplication::_openCorpora(QStringList const &fileNames)
{
    d_dactStartedWithCorpus = true;
    d_mainWindow->readCorpora(fileNames, true);
}

void DactApplication::openMacros(QStringList const &fileNames)
{
    postEvent(this, new DactApplicationEvent(DactApplicationEvent::MacroOpen, fileNames));
}

void DactApplication::_openMacros(QStringList const &fileNames)
{
    d_mainWindow->readMacros(fileNames);
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
        d_mainWindow->setFilter(QUrl::fromPercentEncoding(encodedFilter));
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

void DactApplication::showOpenCorpus()
{
    if (!d_dactStartedWithCorpus)
        d_mainWindow->openCorpus();
}
