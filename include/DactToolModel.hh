#ifndef DACTTOOLMODEL_H
#define DACTTOOLMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QChar>
#include <QFile>
#include <QFileSystemWatcher>
#include <QString>

#include "DactTool.hh"

class DactToolModel : public QAbstractItemModel
{
    Q_OBJECT

    static const int ROOT_ID = 1000;

public:
    static const int COLUMN_NAME = 0;
    static const int COLUMN_COMMAND = 1;

public:
    DactToolModel(QObject *parent = 0);
    ~DactToolModel();
    
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, QModelIndex const &parent = QModelIndex()) const;
    QModelIndex parent(QModelIndex const &parent) const;

private:
    QList<DactTool *> d_tools;
};

#endif
