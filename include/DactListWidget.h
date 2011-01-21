#ifndef DACTLISTWIDGET_HH
#define DACTLISTWIDGET_HH

#include <QListWidget>

class QAbstractItemDelegate;

class DactListWidget : public QListWidget
{
	Q_OBJECT
public:
	DactListWidget(QWidget *parent = 0) : QListWidget(parent) {};
	
	// Overrides QListWidget::setItemDelegate to call dataChanged() afterwards.
	void setItemDelegate(QAbstractItemDelegate* delegate);
};

#endif