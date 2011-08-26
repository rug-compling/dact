#include <assert.h>
#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Error.hh>
#include "QueryModel.hh"

QueryModel::HitsCompare::HitsCompare(QueryModel const &parent)
:
    d_parent(parent)
{}

inline bool QueryModel::HitsCompare::operator()(int one_idx, int other_idx)
{
    int oneHits = d_parent.d_results[one_idx].second;
    int twoHits = d_parent.d_results[other_idx].second;
    
    if (oneHits == twoHits)
        return d_parent.d_results[one_idx].first <
            d_parent.d_results[other_idx].first;
    else
        return oneHits > twoHits;
}

QueryModel::QueryModel(CorpusPtr corpus, QObject *parent)
:
QAbstractTableModel(parent),
d_corpus(corpus)
{
    connect(this, SIGNAL(queryEntryFound(QString)),
            SLOT(mapperEntryFound(QString))); 
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
            // map positions of the hits index to the positions in d_results
            return d_results[d_hitsIndex[index.row()]].first;
        case 1:
            return d_results[d_hitsIndex[index.row()]].second;
        case 2:
            return static_cast<double>(d_results[d_hitsIndex[index.row()]].second)
                 / static_cast<double>(d_totalHits);
        default:
            return QVariant();
    }
}

QVariant QueryModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Vertical)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("Value");
        case 1:
            return tr("Nodes");
        case 2:
            return tr("Percentage");
        default:
            return QVariant();
    }
}

void QueryModel::mapperEntryFound(QString entry)
{
    if (d_cancelled)
        return;
    
    // find position in d_results using the word index
    int idx = d_valueIndex.value(entry, -1);
        
    if (idx == -1)
    {
        idx = d_results.size();
        d_results.append(QPair<QString,int>(entry, 1));
        d_valueIndex[entry] = idx;

        HitsCompare compare(*this);
        
        // find new position, just above the result with less hits.
        QList<int>::iterator insertPos = qLowerBound(
            d_hitsIndex.begin(), d_hitsIndex.end(), idx, compare);
        
        // insert at new position
        d_hitsIndex.insert(insertPos, idx);
        
        emit layoutChanged(); // notify tableview that we have new rows
    }
    else
    {
        HitsCompare compare(*this);
        
        // Find the current position to remove
        
        // Binary search: does not work? Why not? :(
        QList<int>::iterator current_pos = qBinaryFind(
            d_hitsIndex.begin(), d_hitsIndex.end(), idx,
            compare);
        
        // It is already in the words index, it has to be in the hits index as well!
        assert(current_pos != d_hitsIndex.end());
        
        // remove from current position in the index
        d_hitsIndex.erase(current_pos);
        
        // Update hits index
        int hits = d_results[idx].second;
        d_results[idx].second = hits + 1;
        
        // find new position, just above the result with less hits.
        QList<int>::iterator insertPos = qLowerBound(
            d_hitsIndex.begin(), d_hitsIndex.end(), idx, compare);
        
        // insert at new position
        d_hitsIndex.insert(insertPos, idx);
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
    d_hitsIndex.clear();
    emit layoutChanged();
    
    // clear cache and counter
    d_valueIndex.clear();
    d_totalHits = 0;
    
    // Do nothing if we where given a null-pointer
    if (!d_corpus)
        return;
    
    if (!query.isEmpty())
        d_entriesFuture = QtConcurrent::run(this, &QueryModel::getEntriesWithQuery, query);
    else
        d_entriesFuture = QtConcurrent::run(this, &QueryModel::getEntries,
            d_corpus->begin(),
            d_corpus->end());
}

void QueryModel::cancelQuery()
{
    d_cancelled = true;
    d_entriesFuture.waitForFinished();
}

// run async, because query() starts searching immediately
void QueryModel::getEntriesWithQuery(QString const &query)
{
  try {
    QueryModel::getEntries(
        d_corpus->query(alpinocorpus::CorpusReader::XPATH, query.toUtf8().constData()),
        d_corpus->end());
    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Error in QueryModel::getEntriesWithQuery: " << e.what();
        emit queryFailed(e.what());
    }
}

// run async
void QueryModel::getEntries(EntryIterator const &begin, EntryIterator const &end)
{
    try {
        queryStarted(0);
        
        d_cancelled = false;
        
        for (EntryIterator itr(begin); !d_cancelled && itr != end; ++itr)
            emit queryEntryFound(QString::fromUtf8(itr.contents(*d_corpus).c_str()));
            
        queryStopped(d_results.size(), d_results.size());
    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Error in QueryModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}
