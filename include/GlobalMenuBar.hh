#ifndef GLOBAL_MENU_BAR
#define GLOBAL_MENU_BAR

#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include "ui_GlobalMenuBar.h"

class GlobalMenuBar : public QMenuBar
{
	Q_OBJECT
public:
	GlobalMenuBar(QWidget *parent = 0);
	virtual ~GlobalMenuBar() {}
private:
	QSharedPointer<Ui::GlobalMenuBar> d_ui;
};

#endif // GLOBAL_MENU_BAR