#include "TreeNodeInspector.hh"
#include "TreeNode.hh"

#include <QDebug>

TreeNodeInspector::TreeNodeInspector(QWidget *parent)
:
    QDockWidget("Inspector", parent),
    d_ui(QSharedPointer<Ui::TreeNodeInspector>(new Ui::TreeNodeInspector))
{
    d_ui->setupUi(this);
}

void TreeNodeInspector::inspect(TreeNode const &node)
{
    qDebug() << "Inspecting" << node.asString();
}