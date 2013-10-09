#include <stdexcept>

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
    //
}
    
int DactMacrosModel::columnCount(const QModelIndex &parent) const
{
    return 2; // pattern and replacement
}

int DactMacrosModel::rowCount(const QModelIndex &parent) const
{
    // No index? You must be new here. A row for each macro
    if (!parent.isValid() && d_file)
        return d_file->macros().size();

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
    
    if (index.row() >= d_file->macros().size() || index.row() < 0 || index.column() > 1)
        return QVariant();

    DactMacro const &macro(d_file->macros().at(index.row()));
    
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
    
    // What is happening here?!
    //qDebug() << "Data asking for" << index.internalId() << index.row() << index.column() << "which does not exist";
    return QVariant();
}

QModelIndex DactMacrosModel::index(int row, int column, QModelIndex const &parent) const
{
    // Invalid parent -> coordinates point to a macro
    if (!parent.isValid() && d_file && row < d_file->macros().size() && column == 0)
        return createIndex(row, column);
    
    return QModelIndex();
}

bool DactMacrosModel::isFileBacked()
{
  return d_file;
}

QModelIndex DactMacrosModel::parent(QModelIndex const &index) const
{
    return QModelIndex();
}

void DactMacrosModel::loadFile(QString const &path)
{
    // Clear all previous telephone taps
    if (d_watcher.files().size() > 0)
        d_watcher.removePaths(d_watcher.files());

    try {
        readFile(path);

        // If we can read it now, we should watch it for updates
        d_watcher.addPath(path);
    } catch (std::runtime_error &e) {
        emit readError(QString::fromUtf8(e.what()));
    }
}

void DactMacrosModel::reloadFile()
{
    if (!d_file)
        return;

    int prevCount = d_file->macros().size();
    
    d_file->reload();

    dataChanged(index(0, 0), index(prevCount, 0));
}

void DactMacrosModel::readFile(QString const &fileName)
{
    int prevCount = 0;

    if (d_file)
        prevCount = d_file->macros().size();

    d_file = QSharedPointer<DactMacrosFile>(new DactMacrosFile(fileName));
    
    dataChanged(index(0, 0), index(prevCount, 0));
}

QString DactMacrosModel::expand(QString const &expression)
{
    QString query(expression);
    
    if (d_file)   
        foreach (DactMacro const &macro, d_file->macros())
            query = query.replace(d_symbol + macro.pattern + d_symbol, macro.replacement);

    return query;
}

void DactMacrosModel::loadFileDelayed(QString const &fileName)
{
    // Some editors seem to cause multiple fileChanged events. I am
    // looking at you vim and Sublime Text 2! Let's ignore such events
    // when we are already realoading a/the macro file.
    //
    // XXX - Maybe we should use a mutex per macro file?

    if (!d_reloadMutex.tryLock())
        return;

    QTimer::singleShot(500, new DelayedLoadFileCallback(this, &d_reloadMutex, fileName), SLOT(invokeOnce()));
}

