#ifndef DACTTABLEVIEW_HH
#define DACTTABLEVIEW_HH

#include <QTableView>

class QAbstractItemDelegate;

class DactTableView : public QTableView
{
    Q_OBJECT

public:
    DactTableView(QWidget *parent = 0) : QTableView(parent) {};

    // Overrides QTableWidget::setItemDelegate to call dataChanged() afterwards.
    void setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate);
};

#endif
