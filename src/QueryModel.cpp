#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Error.hh>
#include "QueryModel.hh"

QueryModel::QueryModel(CorpusPtr corpus, QObject *parent)
:
QAbstractTableModel(parent),
d_corpus(corpus)
{
    connect(this, SIGNAL(queryEntryFound(QString)),
            this, SLOT(mapperEntryFound(QString))); 
}

QueryModel::~QueryModel()
{
    cancelQuery();
}

int QueryModel::columnCount(QModelIndex const &index) const
{
    return 3;
}

int QueryModel::rowCount(QModelIndex const &index) const
{
    return d_results.size();
}

QVariant QueryModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid()
        || (role != Qt::DisplayRole && role != Qt::UserRole)
        || index.row() >= d_results.size()
        || index.row() < 0
        || index.column() > 2
        || index.column() < 0)
        return QVariant();
    
    switch (index.column())
    {
        case 0:
            return d_results[index.row()].first;
        case 1:
            return d_results[index.row()].second;
        case 2:
            return static_cast<double>(d_results[index.row()].second)
                 / static_cast<double>(d_totalHits);
        default:
            return QVariant();
    }
}

QVariant QueryModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    switch (column)
    {
        case 0:
            return tr("Value");
        case 1:
            return tr("Hits");
        case 2:
            return tr("Relative");
        default:
            return QVariant();
    }
}

void QueryModel::mapperEntryFound(QString entry)
{
    if (d_cancelled)
        return;
    
    int idx = d_index.value(entry, -1);
    
    if (idx == -1)
    {
        d_index[entry] = d_results.size();
        d_results.append(QPair<QString,int>(entry, 1));
        
        emit layoutChanged(); // notify tableview that we have new rows
    }
    else
    {
        ++d_results[idx].second;
    }
    
    ++d_totalHits;
    
    emit dataChanged(index(idx, 0), index(idx + 1, 2));
}

void QueryModel::runQuery(QString const &query)
{
    cancelQuery(); // just in case
    
    // clean results cache and notify the table of the loss of rows
    //int size = d_results.size();
    d_results.clear();
    emit layoutChanged();
    
    // clear cache and counter
    d_index.clear();
    d_totalHits = 0;
    
    if (!query.isEmpty())
        QtConcurrent::run(this, &QueryModel::getEntriesWithQuery, query);
    else
        QtConcurrent::run(this, &QueryModel::getEntries,
            d_corpus->begin(),
            d_corpus->end());
}

void QueryModel::cancelQuery()
{
    d_cancelled = true;
}

// run async, because query() starts searching immediately
void QueryModel::getEntriesWithQuery(QString const &query)
{
    QueryModel::getEntries(
        d_corpus->query(alpinocorpus::CorpusReader::XPATH, query),
        d_corpus->end());
}

// run async
void QueryModel::getEntries(EntryIterator const &begin, EntryIterator const &end)
{
    try {
        queryStarted(0);
        
        d_cancelled = false;
        
        for (EntryIterator itr(begin); !d_cancelled && itr != end; ++itr)
            emit queryEntryFound(itr.contents(*d_corpus));
            
        queryStopped(d_results.size(), d_results.size());
    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Error performing query: " << e.what();
    }
}
