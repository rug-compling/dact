#ifndef DACTLISTVIEW_HH
#define DACTLISTVIEW_HH

#include <QListView>

class QAbstractItemDelegate;

class DactListView : public QListView
{
    Q_OBJECT
public:
    DactListView(QWidget *parent = 0) : QListView(parent) {};
    
    // Overrides QListWidget::setItemDelegate to call dataChanged() afterwards.
    void setItemDelegate(QAbstractItemDelegate* delegate);
};

#endif