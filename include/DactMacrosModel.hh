#ifndef DACTMACROSMODEL_H
#define DACTMACROSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QChar>
#include <QFile>
#include <QFileSystemWatcher>
#include <QMutex>
#include <QSharedPointer>
#include <QString>

#include "DactMacro.hh"

class DactMacrosFile;

class DactMacrosModel : public QAbstractItemModel
{
    Q_OBJECT
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
    void loadFileDelayed(QString const &path);
    void reloadFile();

signals:
    void readError(QString error);

private:
    void readFile(QString const &path);

private:
    QSharedPointer<DactMacrosFile> d_file;
    QFileSystemWatcher d_watcher;
    QMutex d_reloadMutex;

    static const QChar d_symbol;
};

#endif
