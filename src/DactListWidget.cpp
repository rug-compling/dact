#include "dactlistwidget.h"


void DactListWidget::setItemDelegate(QAbstractItemDelegate* delegate)
{
	QAbstractItemView::setItemDelegate(delegate);
	
	// Now use dataChanged to notify the dimensions of the listitems might have changed, since
	// a new delegate means a new sizeHint().
	QModelIndex topLeft = model()->index(0,0);
	QModelIndex bottomRight = model()->index(model()->rowCount() - 1, model()->columnCount() - 1);
	
	dataChanged(topLeft, bottomRight);
}
