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

void TreeNodeInspector::inspect(TreeNode const *node)
{
    // Clear the table
    d_ui->attributesTable->clearContents();
    qDebug() << "Cleared";
    
    // If there is no new node selected, stop.
    if (!node)
        return;
    
    // Add key-value pairs of attributes to the cleared table
    int row = 0;
    for (QHash<QString, QString>::const_iterator itr = node->attributes().constBegin(),
        end = node->attributes().constEnd(); itr != end; itr++)
    {
        d_ui->attributesTable->setItem(row, 0, new QTableWidgetItem(itr.key()));
        d_ui->attributesTable->setItem(row, 1, new QTableWidgetItem(itr.value()));
        qDebug() << row << itr.key() << ":" << itr.value();
        ++row;
    }
}