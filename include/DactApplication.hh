#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>
#include <QScopedPointer>
#include "DactMainWindow.hh"

class DactApplication: public QApplication
{
	Q_OBJECT
public:
	DactApplication(int &argc, char** argv);
	void init();
	void openCorpus(QString const &fileName);
protected:
	bool event(QEvent *event);

private:
	QScopedPointer<DactMainWindow> d_mainWindow;
};

#endif
