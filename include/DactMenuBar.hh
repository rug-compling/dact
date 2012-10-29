#ifndef GLOBAL_MENU_BAR
#define GLOBAL_MENU_BAR

#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include "ui_DactMenuBar.h"

class DactMenuBar : public QMenuBar
{
	Q_OBJECT
public:
	DactMenuBar(QWidget *parent = 0);
	virtual ~DactMenuBar() {}
private:
	QSharedPointer<Ui::DactMenuBar> d_ui;
};

#endif // GLOBAL_MENU_BAR