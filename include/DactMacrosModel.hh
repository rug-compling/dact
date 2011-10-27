#ifndef DACTMACROSMODEL_H
#define DACTMACROSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QChar>
#include <QFile>
#include <QFileSystemWatcher>
#include <QString>

#include "DactMacro.hh"

class DactMacrosFile;

class DactMacrosModel : public QAbstractItemModel
{
    Q_OBJECT

    // Make this zero or positive, and whatch the empire crumble. Seriously, don't do it.
    static const int ROOT_ID = 1000;

public:
    DactMacrosModel(QObject *parent = 0);
    ~DactMacrosModel();
    
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, QModelIndex const &parent = QModelIndex()) const;
    QModelIndex parent(QModelIndex const &parent) const;

    QString expand(QString const &query);

public slots:
    void loadFile(QString const &path);

private slots:
    void fileChanged(QString const &path);

private:
    QList<DactMacrosFile *> d_files;
    QFileSystemWatcher d_watcher;

    static const QChar d_symbol;
};

#endif
