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
    switch (event->type())
    {
        case QEvent::FileOpen:
            openCorpora(QStringList(static_cast<QFileOpenEvent *>(event)->file()));
            return true;
        default:
            return QApplication::event(event);
    }
}

void DactApplication::openCorpora(QStringList const &fileNames)
{
    d_mainWindow->readCorpora(fileNames);
}