#ifndef DACTLISTVIEWIII_HH
#define DACTLISTVIEWIII_HH

#include <QTableView>
//#include <QListView>

class QAbstractItemDelegate;

class DactListView3 : public QTableView
{
    Q_OBJECT

public:
    DactListView3(QWidget *parent = 0) : QTableView(parent) {};

    // Overrides QListWidget::setItemDelegate to call dataChanged() afterwards.
    void setItemDelegate(QAbstractItemDelegate* delegate);
};

#endif



