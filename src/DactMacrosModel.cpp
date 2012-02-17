#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>

#include "DactMacrosModel.hh"
#include "DactMacrosFile.hh"
#include "DelayedLoadFileCallback.hh"

const QChar DactMacrosModel::d_symbol('%');

DactMacrosModel::DactMacrosModel(QObject *parent)
:
    QAbstractItemModel(parent)
{
    connect(&d_watcher, SIGNAL(fileChanged(QString const &)),
        SLOT(loadFileDelayed(QString const &)));
}

DactMacrosModel::~DactMacrosModel()
{
    foreach (DactMacrosFile *file, d_files)
        delete file;
}
    
int DactMacrosModel::columnCount(const QModelIndex &parent) const
{
    return 2; // pattern and replacement
}

int DactMacrosModel::rowCount(const QModelIndex &parent) const
{
    // No index? You must be new here. A row for each file
    if (!parent.isValid())
        return d_files.size();

    // macrofiles have a row for each macro they contain
    else if (parent.internalId() == ROOT_ID)
        return d_files[parent.row()]->macros().size();
    
    // macro's themselves have no rows
    return 0;
}

QVariant DactMacrosModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) 
        return QVariant();
    
    if (orientation != Qt::Horizontal)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("Pattern");
    
        case 1:
            return tr("Replacement");
    
        default:
            return QVariant();   
    }
}

QVariant DactMacrosModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    // Aah, we are pointing at the root, also known as the list of macro files.
    if (index.internalId() == ROOT_ID)
    {
        if (index.row() >= d_files.size() || index.column() != 0)
            return QVariant();
        
        DactMacrosFile *file = d_files[index.row()];
        QFileInfo fileInfo(file->file());

        switch (role)
        {
            case Qt::DisplayRole:
                return fileInfo.baseName();
            
            case Qt::UserRole:
                return fileInfo.filePath();
            
            default:
                return QVariant();
        }
    }

    // Are we pointing to a specific macro-file?
    else if (index.internalId() >= 0 && index.internalId() < d_files.size())
    {
        DactMacrosFile *file = d_files[index.internalId()];

        if (index.row() >= file->macros().size() || index.row() < 0 || index.column() > 1)
            return QVariant();

        DactMacro const &macro(file->macros().at(index.row()));
        
        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
                case 0:
                    return macro.pattern;
                
                case 1:
                    return macro.replacement;
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (index.column())
            {
                case 0:
                    return d_symbol + macro.pattern + d_symbol;
            }
        }
    }

    // What is happening here?!
    else
    {
        //qDebug() << "Data asking for" << index.internalId() << index.row() << index.column() << "which does not exist";
        return QVariant();
    }
}

QModelIndex DactMacrosModel::index(int row, int column, QModelIndex const &parent) const
{
    // Invalid parent -> coordinates point to a file
    if (!parent.isValid() && row < d_files.size() && column == 0)
        return createIndex(row, column, ROOT_ID);
    
    // file-list is the parent -> coordinates point to a macro
    if (parent.internalId() == ROOT_ID && parent.row() < d_files.size() && column < 2)
        return createIndex(row, column, parent.row());

    // yeah, that's it. This isn't Inception.
    return QModelIndex();
}

QModelIndex DactMacrosModel::parent(QModelIndex const &index) const
{
    if (!index.isValid())
        return QModelIndex();
    
    // The First Man .. has no parents. Poor guy.
    if (index.internalId() == ROOT_ID)
        return QModelIndex();
    
    // otherwise it is probably pointing to a file,
    // so let's return its position!
    Q_ASSERT(index.internalId() >= 0);
    return createIndex(index.internalId(), 0, ROOT_ID);
}

QStringList DactMacrosModel::loadedFiles() const
{
    QStringList paths;

    foreach (DactMacrosFile const *file, d_files)
        paths << file->file().fileName();
    
    return paths;
}

void DactMacrosModel::loadFile(QString const &path)
{
    if (!d_watcher.files().contains(path))
        d_watcher.addPath(path);
    
    readFile(path);

    fileLoaded(path);
}

void DactMacrosModel::unloadFile(QString const &fileName)
{
    for (int i = 0; i < d_files.size(); ++i)
    {
        if (d_files[i]->file().fileName() == fileName)
        {
            d_files.removeAt(i);
            dataChanged(index(i, 0), index(i, 0));
            break;
        }
    }
}

void DactMacrosModel::readFile(QString const &fileName)
{
    int i;
    // Is this file already loaded? Then just reload it.
    for (i = 0; i < d_files.size(); ++i)
    {
        if (d_files[i]->file().fileName() == fileName)
        {   
            d_files[i]->reload();
            break;
        }   
    }

    // Apparently it is a new file! CooL!
    if (i == d_files.size())
    {
        DactMacrosFile *file = new DactMacrosFile(fileName);
        d_files.insert(i, file);
    }
    
    // Now tell everyone that that file changed
    dataChanged(index(i, 0), index(i, 0));
}

QString DactMacrosModel::expand(QString const &expression)
{
    QString query(expression);
    
    foreach (DactMacrosFile *file, d_files)
        foreach (DactMacro const &macro, file->macros())
            query = query.replace(d_symbol + macro.pattern + d_symbol, macro.replacement);
    
    return query;
}

void DactMacrosModel::reset()
{
    // XXX todo
}

void DactMacrosModel::loadFileDelayed(QString const &fileName)
{
    QTimer::singleShot(500, new DelayedLoadFileCallback(this, fileName), SLOT(invokeOnce()));
}

