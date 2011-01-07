#ifndef DACTTREEVIEW_H
#define DACTTREEVIEW_H

#include <QGraphicsView>

class DactTreeView : public QGraphicsView
{
	Q_OBJECT
	
public:
	DactTreeView(QWidget *parent = 0) : QGraphicsView(parent) {};
	
protected:
	void wheelEvent(QWheelEvent * event);
};

#endif