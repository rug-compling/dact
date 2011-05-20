#include "TreeNodeInspector.hh"
#include "TreeNode.hh"

#include <QDebug>

TreeNodeInspector::TreeNodeInspector(QWidget *parent)
:
    QDockWidget("Inspector", parent),
    d_ui(QSharedPointer<Ui::TreeNodeInspector>(new Ui::TreeNodeInspector))
{
    d_ui->setupUi(this);
    d_ui->attributesTree->sortByColumn(0, Qt::AscendingOrder);
}

void TreeNodeInspector::inspect(TreeNode const *node)
{
    // Clear the table
    d_ui->attributesTree->clear();
    
    // If there is no new node selected, stop.
    if (!node)
        return;
    
    // Disable sorting for performance reasons
    d_ui->attributesTree->setSortingEnabled(false);
    
    // Add key-value pairs of attributes to the cleared table
    for (QHash<QString, QString>::const_iterator itr = node->attributes().constBegin(),
        end = node->attributes().constEnd(); itr != end; itr++)
    {
        QTreeWidgetItem *line = new QTreeWidgetItem(d_ui->attributesTree);
        
        line->setText(0, itr.key());
        line->setText(1, itr.value());
        
        d_ui->attributesTree->addTopLevelItem(line);
    }
    
    d_ui->attributesTree->setSortingEnabled(true);
}