#include "TreeNodeInspector.hh"
#include "TreeNode.hh"

#include <QDebug>

TreeNodeInspector::TreeNodeInspector(QWidget *parent)
:
    QDockWidget(parent),
    d_ui(QSharedPointer<Ui::TreeNodeInspector>(new Ui::TreeNodeInspector))
{
    d_ui->setupUi(this);
    d_ui->attributesTree->sortByColumn(0, Qt::AscendingOrder);
}

void TreeNodeInspector::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    d_ui->attributesTree->setContextMenuPolicy(policy);
}

void TreeNodeInspector::addAction(QAction *action)
{
    d_ui->attributesTree->addAction(action);
}

QMap<QString, QString> TreeNodeInspector::selectedAttributes() const
{
    QMap<QString,QString> pairs;
    QList<QTreeWidgetItem *> selection = d_ui->attributesTree->selectedItems();

    foreach (QTreeWidgetItem *attribute, selection)
        pairs[attribute->data(0, Qt::DisplayRole).toString()]
          = attribute->data(1, Qt::DisplayRole).toString();
    
    return pairs;
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
