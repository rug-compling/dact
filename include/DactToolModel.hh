#ifndef DACTTOOLMODEL_H
#define DACTTOOLMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QFile>
#include <QList>

#include "DactTool.hh"

class DactToolModel : public QAbstractItemModel
{
    Q_OBJECT

    static const int ROOT_ID = 1000;

public:
    static const int COLUMN_NAME = 0;
    static const int COLUMN_COMMAND = 1;

private:
    static const QString s_assignment_symbol;
    static const QString s_start_replacement_symbol;
    static const QString s_end_replacement_symbol;

public:
    DactToolModel(QList<DactTool*> tools = QList<DactTool*>(), QObject *parent = 0);
    ~DactToolModel();
    
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, QModelIndex const &parent = QModelIndex()) const;
    QModelIndex parent(QModelIndex const &parent) const;

    static QSharedPointer<DactToolModel> loadFromFile(QFile &file);

private:
    QList<DactTool *> d_tools;
};

#endif
