#ifndef DACTAPPLICATION_H
#define DACTAPPLICATION_H

#include <QApplication>

class DactMainWindow;

class DactApplication: public QApplication
{
	Q_OBJECT
public:
	DactApplication(int &argc, char** argv);
	~DactApplication();
	void init();
	void openCorpus(QString const &fileName);
protected:
	bool event(QEvent *event);

private:
	DactMainWindow* d_mainWindow;
};

#endif
