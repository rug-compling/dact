#include <QSettings>
#include <QtDebug>

#include "DactMacrosModel.h"

DactMacrosModel::DactMacrosModel(QObject *parent) :
    QAbstractTableModel(parent),
    d_macros(readMacros())
{
}
    
int DactMacrosModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d_macros.size();
}

int DactMacrosModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant DactMacrosModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole) 
        return QVariant();
    
    if(orientation != Qt::Horizontal)
        return QVariant();
    
    if(column == 0)
        return tr("Pattern");
    else if(column == 1)
        return tr("Replacement");
    else
        return QVariant();
}

QVariant DactMacrosModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    
    if(index.row() >= d_macros.size() || index.row() < 0)
        return QVariant();
        
    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        DactMacro macro = d_macros.at(index.row());
        
        if(index.column() == 0)
            return macro.pattern;
        else if(index.column() == 1)
            return macro.replacement;
    }
    
    return QVariant();
}

bool DactMacrosModel::insertRows(int position, int rows, const QModelIndex &index)
{
    beginInsertRows(index, position, position + rows - 1);
    
    for(int row = 0; row < rows; ++row) {
        DactMacro macro;
        macro.pattern = QString();
        macro.replacement = QString();
        d_macros.insert(position, macro);
    }
    
    endInsertRows();
    
    return true;
}

bool DactMacrosModel::removeRows(int position, int rows, const QModelIndex &index)
{
    beginRemoveRows(index, position, position + rows - 1);
    
    for(int row = 0; row < rows; ++row) {
        d_macros.removeAt(position);
    }
    
    endRemoveRows();
    
    writeMacros(d_macros);
    
    return true;
}

bool DactMacrosModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || role != Qt::EditRole)
        return false;
    
    DactMacro macro = d_macros.at(index.row());
    
    if(index.column() == 0)
        macro.pattern = value.toString();
    else if(index.column() == 1)
        macro.replacement = value.toString();
    
    d_macros.replace(index.row(), macro);
    
    emit(dataChanged(index,index));
    
    writeMacros(d_macros);
    
    return true;
}

Qt::ItemFlags DactMacrosModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::ItemIsEnabled;
    
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList<DactMacro> DactMacrosModel::readMacros() const
{
    qDebug() << "Loading macros";
    
    QList<DactMacro> macros;
    
    QSettings settings("RUG", "Dact");
    
    int size = settings.beginReadArray("macros");
    
    for(int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        
        DactMacro macro;
        macro.pattern = settings.value("pattern").toString();
        macro.replacement = settings.value("replacement").toString();
        
        macros.append(macro);
    }
    
    settings.endArray();
    
    return macros;
}
void DactMacrosModel::writeMacros(const QList<DactMacro> &macros) const
{
    qDebug() << "Saving macros";
    
    QSettings settings("RUG", "Dact");
    
    settings.beginWriteArray("macros");
    
    for(int i = 0; i < d_macros.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("pattern", d_macros.at(i).pattern);
        settings.setValue("replacement", d_macros.at(i).replacement);
    }
    
    settings.endArray();
}
