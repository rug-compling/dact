#include <QAbstractItemModel>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QKeySequence>
#include <QModelIndex>
#include <QModelIndexList>
#include <QString>
#include <QStringList>

#include "DactListView3.hh"

void DactListView3::setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate)
{
    QAbstractItemView::setItemDelegateForColumn(column, delegate);

    if (model() == 0)
        return;

    // Now use dataChanged to notify the dimensions of the listitems might have changed, since
    // a new delegate means a new sizeHint().
    QModelIndex topLeft = model()->index(0,0);
    QModelIndex bottomRight = model()->index(model()->rowCount() - 1, model()->columnCount() - 1);

    dataChanged(topLeft, bottomRight);

}

