#ifndef TREENODEINSPECTOR_HH
#define TREENODEINSPECTOR_HH

#include <QSharedPointer>
#include <QDockWidget>

#include "ui_TreeNodeInspector.h"

namespace Ui {
    class TreeNodeInspector;
}

class TreeNode;

class TreeNodeInspector : public QDockWidget
{
    Q_OBJECT

public:
    TreeNodeInspector(QWidget *parent = 0);
    QMap<QString,QString> selectedAttributes() const;

public slots:
    void inspect(TreeNode const *);

private:
    QSharedPointer<Ui::TreeNodeInspector> d_ui;
};

#endif // TREENODEINSPECTOR_HH