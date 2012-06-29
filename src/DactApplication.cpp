#include "DactApplication.hh"
#include <QFileOpenEvent>


DactApplication::DactApplication(int &argc, char** argv)
:
QApplication(argc, argv),
d_mainWindow(0)
{
    //d_mainWindow = new DactMainWindow();
}

void DactApplication::init()
{
    d_mainWindow.reset(new MainWindow());
    d_mainWindow->show();
}

bool DactApplication::event(QEvent *event)
{
    qDebug() << "event" << event->type();
    switch (event->type())
    {
        case QEvent::FileOpen:
        {
            QFileOpenEvent *fileEvent(static_cast<QFileOpenEvent *>(event));
            
            if (!fileEvent->file().isEmpty())
            {
                QStringList files;
                files << fileEvent->file();
                openCorpora(files);
            }

            else if (!fileEvent->url().isEmpty())
                openUrl(fileEvent->url());
            
            else
                return false;

            return true;
        }
        default:
            return QApplication::event(event);
    }
}

void DactApplication::openCorpora(QStringList const &fileNames)
{
    d_mainWindow->readCorpora(fileNames);
}

void DactApplication::openMacros(QStringList const &fileNames)
{
    d_mainWindow->readMacros(fileNames);
}

void DactApplication::openUrl(QUrl const &url)
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