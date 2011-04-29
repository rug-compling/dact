#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Error.hh>
#include "FilterModel.hh"

FilterModel::FilterModel(CorpusPtr corpus, QObject *parent)
:
QAbstractTableModel(parent),
d_corpus(corpus)
{
    connect(this, SIGNAL(entryFound(QString)),
        this, SLOT(mapperEntryFound(QString)));
}

FilterModel::~FilterModel()
{
    cancelQuery();
}

int FilterModel::columnCount(QModelIndex const &index) const
{
    return 2;
}

int FilterModel::rowCount(QModelIndex const &index) const
{
    return d_results.size();
}

QVariant FilterModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid()
        || index.row() >= d_results.size()
        || index.row() < 0
        || !(role == Qt::DisplayRole || role == Qt::UserRole))
        return QVariant();
    
    switch (index.column())
    {
        case 0:
            return d_results.at(index.row()).first;            
        case 1:
            return d_results.at(index.row()).second;
        default:
            return QVariant();
    }
}

QVariant FilterModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal
        || role != Qt::DisplayRole)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("File");
        case 1:
            return tr("Hits");
        default:
            return QVariant();
    }
}

void FilterModel::mapperEntryFound(QString entry)
{
    // WARNING: This assumes all the hits per result only occur right after
    // each other, never shuffled. Otherwise we might want to change to QHash
    // or QMap for fast lookup.
    
    // @TODO make this more robust so no assumption is required.
    
    int row = d_results.size() - 1;
    
    if (row >= 0 && d_results[row].first == entry)
        ++d_results[row].second;
    else
    {
        /*
        for (QList<value_type>::iterator it = d_results.begin(), end = d_results.end();
            it != end; it++)
            if (it->first == entry)
            qDebug() << "Your assumption is wrong!";
        */
        ++row;
        d_results.append(value_type(entry, 1));
    }
    
    emit dataChanged(index(row, 0), index(row + 1, 0));
}

void FilterModel::runQuery(QString const &query)
{
    cancelQuery(); // just in case
    
    int size = d_results.size();
    d_results.clear();
    
    emit dataChanged(index(0, 0), index(size, 0));
    
    d_query = query;
    
    if (!d_query.isEmpty())
        QtConcurrent::run(this, &FilterModel::getEntriesWithQuery, d_query);
    else
        QtConcurrent::run(this, &FilterModel::getEntries,
            d_corpus->begin(),
            d_corpus->end());
}

QString const &FilterModel::lastQuery() const
{
    return d_query;
}

void FilterModel::cancelQuery()
{
    d_cancelled = true;
}

// run async, because query() starts searching immediately
void FilterModel::getEntriesWithQuery(QString const &query)
{
    FilterModel::getEntries(
        d_corpus->query(alpinocorpus::CorpusReader::XPATH, query),
        d_corpus->end());
}

// run async
void FilterModel::getEntries(EntryIterator const &begin, EntryIterator const &end)
{
    try {
        emit queryStarted(0); // we don't know how many entries will be found
        
        d_cancelled = false;
        
        int hits = 0;
        for (EntryIterator itr(begin); !d_cancelled && itr != end; ++itr)
        {
            emit queryProgressed(++hits, 0);
            emit entryFound(*itr);
            // @TODO could we implement something a la mapperProgressed(end - itr)?
        }
            
        emit queryStopped(d_results.size(), d_results.size());
    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Error performing query: " << e.what();
    }
}