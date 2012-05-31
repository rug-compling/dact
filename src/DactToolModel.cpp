#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>

#include "DactToolModel.hh"

DactToolModel::DactToolModel(QObject *parent)
:
    QAbstractItemModel(parent)
{
    d_tools.append(new DactTool("edit", "subl %1"));
    d_tools.append(new DactTool("diff", "opendiff %1"));
}

DactToolModel::~DactToolModel()
{
    foreach (DactTool *tool, d_tools)
        delete tool;
}
    
int DactToolModel::columnCount(const QModelIndex &parent) const
{
    return 2; // name and command
}

int DactToolModel::rowCount(const QModelIndex &parent) const
{
    // No index? You must be new here. A row for each file
    if (!parent.isValid())
        return d_tools.size();

    // Tool's themselves have no rows
    return 0;
}

QVariant DactToolModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) 
        return QVariant();
    
    if (orientation != Qt::Horizontal)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("Name");
    
        case 1:
            return tr("Command");
    
        default:
            return QVariant();   
    }
}

QVariant DactToolModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d_tools.size())
        return QVariant();
    
    DactTool *tool = d_tools[index.row()];
    
    if (role == Qt::DisplayRole)
        switch (index.column())
        {
            case COLUMN_NAME:
                return tool->name();
            
            case COLUMN_COMMAND:
                return tool->command();
            
            default:
                return QVariant();
        }

    else
        return QVariant();
}

QModelIndex DactToolModel::index(int row, int column, QModelIndex const &parent) const
{
    // Invalid parent -> coordinates point to a file
    if (!parent.isValid() && row < d_tools.size() && (column == 0 || column == 1))
        return createIndex(row, column);
    
    return QModelIndex();
}

QModelIndex DactToolModel::parent(QModelIndex const &index) const
{
    return QModelIndex();
}
