#include "DactApplication.hh"
#include "DactMainWindow.hh"
#include <QFileOpenEvent>


DactApplication::DactApplication(int &argc, char** argv)
:
QApplication(argc, argv),
d_mainWindow(0)
{
	//d_mainWindow = new DactMainWindow();
}

DactApplication::~DactApplication()
{
	delete d_mainWindow;
}

void DactApplication::init()
{
	d_mainWindow = new DactMainWindow();
	
	d_mainWindow->show();
}

bool DactApplication::event(QEvent *event)
{
	switch (event->type())
	{
        case QEvent::FileOpen:
            openCorpus(static_cast<QFileOpenEvent *>(event)->file());
            return true;
        default:
            return QApplication::event(event);
	}
}

void DactApplication::openCorpus(QString const &fileName)
{
    d_mainWindow->readCorpus(fileName);
}