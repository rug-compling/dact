#ifndef DACTTOOLSMODEL_H
#define DACTTOOLSMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QFile>
#include <QList>

#include "DactTool.hh"

class DactToolsModel : public QAbstractItemModel
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

    static QSharedPointer<DactToolsModel> s_sharedInstance;

public:
    DactToolsModel(QList<DactTool*> tools = QList<DactTool*>(), QObject *parent = 0);
    ~DactToolsModel();
    
    static QSharedPointer<DactToolsModel> sharedInstance();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, QModelIndex const &parent = QModelIndex()) const;
    QModelIndex parent(QModelIndex const &parent) const;

    void readFromFile(QFile &file);
    void clear();

private slots:
    void preferenceChanged(QString const &key, QVariant const &value);

private:
    QList<DactTool *> d_tools;
};

#endif
