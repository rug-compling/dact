#ifndef DACTLISTVIEW_HH
#define DACTLISTVIEW_HH

#include <QListView>

class QAbstractItemDelegate;

class DactListView : public QListView
{
    Q_OBJECT
public:
    DactListView(QWidget *parent = 0) : QListView(parent) {};
    
    /*
    // Change Control-C behavior to copy the whole selection.
    void keyPressEvent(QKeyEvent *event);
    */
    
    // Overrides QListWidget::setItemDelegate to call dataChanged() afterwards.
    void setItemDelegate(QAbstractItemDelegate* delegate);
};

#endif