#include "DactApplication.hh"
#include <QDesktopServices>
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
    QDesktopServices::setUrlHandler("dact", this, "openUrl");

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

    if (!url.allQueryItemValues("corpus").isEmpty())
        d_mainWindow->readCorpora(url.allQueryItemValues("corpus"));
}