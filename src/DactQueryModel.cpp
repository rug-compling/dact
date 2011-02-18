#include "DactQueryModel.hh"
#include <AlpinoCorpus/CorpusReader.hh>
#include <QtDebug>

DactQueryModel::DactQueryModel(CorpusPtr corpus, QObject *parent)
:
d_corpus(corpus),
QAbstractListModel(parent)
{
    QObject::connect(&d_entryMap, SIGNAL(entryFound(QString)),
        this, SLOT(mapperEntryFound(QString)));
    
    QObject::connect(&d_mapper, SIGNAL(started(int)),
        this, SLOT(mapperStarted(int)));
    
    QObject::connect(&d_mapper, SIGNAL(progress(int, int)),
        this, SLOT(mapperProgressed(int, int)));
    
    QObject::connect(&d_mapper, SIGNAL(stopped(int, int)),
        this, SLOT(mapperStopped(int, int)));
}

int DactQueryModel::rowCount(QModelIndex const &index) const
{
    return d_results.size();
}

QVariant DactQueryModel::data(QModelIndex const &index, int role) const
{
    return (index.isValid()
            && index.column() == 0
            && (index.row() < d_results.size() && index.row() >= 0)
            && role == Qt::DisplayRole)
        ? d_results.at(index.row())
        : QVariant();
}

QVariant DactQueryModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    return (column == 0
            && orientation == Qt::Horizontal
            && role == Qt::DisplayRole)
        ? tr("File")
        : QVariant();
}

void DactQueryModel::mapperEntryFound(QString entry)
{
    int row = d_results.size();
    d_results.append(entry);
    
    emit dataChanged(index(row, 0), index(row + 1, 0));
}

void DactQueryModel::mapperStarted(int totalEntries)
{
    emit queryStarted(totalEntries);
}

void DactQueryModel::mapperProgressed(int n, int totalEntries)
{
    emit queryProgressed(n, totalEntries);
}

void DactQueryModel::mapperStopped(int n, int totalEntries)
{
    emit queryStopped(n, totalEntries);
}

void DactQueryModel::runQuery(QString const &query)
{
    int size = d_results.size();
    d_results.clear();
    
    emit dataChanged(index(0, 0), index(size, 0));
    
    if (!query.isEmpty())
        getEntriesWithQuery(query);
    else
        getEntries();
}

void DactQueryModel::cancelQuery()
{
    d_mapper.cancel();
}

void DactQueryModel::getEntries()
{
    // @TODO make this async
    for (alpinocorpus::CorpusReader::EntryIterator itr(d_corpus->begin()), end(d_corpus->end());
         itr != end; itr++)
        mapperEntryFound(*itr);
}

void DactQueryModel::getEntriesWithQuery(QString const &query)
{
    d_mapper.start(d_corpus.data(), query, &d_entryMap);
}