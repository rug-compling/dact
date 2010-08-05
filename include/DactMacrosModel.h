#ifndef DACTMACROSMODEL_H
#define DACTMACROSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QString>

#include "DactMacro.h"

// This model is used by the tableview in the macros edit window. It also
// contains the logic for loading and saving macros from the settings file.
// This could be separated and put into another dedicated class that will
// provide all the macro functionality like storing and applying macros.

// (Should such a class be made singleton, or should an instance be carried
// around by all the Dact* classes in a QSharedPointer, and should they
// call the replace procedure themselves before they send a xpath query
// to one of the XPath* or XSLT* classes?)

class DactMacrosModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DactMacrosModel(QObject *parent = 0);
    
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index);
    bool removeRows(int position, int rows, const QModelIndex &index);
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    QList<DactMacro> readMacros() const;
    void writeMacros(const QList<DactMacro> &macros) const;

    QList<DactMacro> d_macros;
};

#endif
