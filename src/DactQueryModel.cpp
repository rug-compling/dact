#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <alpinocorpus/Error.hh>
#include "DactQueryModel.hh"

DactQueryModel::DactQueryModel(CorpusPtr corpus, QObject *parent)
:
d_corpus(corpus),
QAbstractTableModel(parent)
{}

DactQueryModel::~DactQueryModel()
{
	cancelQuery();
}

int DactQueryModel::columnCount(QModelIndex const &index) const
{
    return 2;
}

int DactQueryModel::rowCount(QModelIndex const &index) const
{
    return d_results.size();
}

QVariant DactQueryModel::data(QModelIndex const &index, int role) const
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

QVariant DactQueryModel::headerData(int column, Qt::Orientation orientation, int role) const
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

void DactQueryModel::mapperEntryFound(QString entry)
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
    cancelQuery(); // just in case
    
    int size = d_results.size();
    d_results.clear();
    
    emit dataChanged(index(0, 0), index(size, 0));
    
    if (!query.isEmpty())
        QtConcurrent::run(this, &DactQueryModel::getEntriesWithQuery, query);
    else
        QtConcurrent::run(this, &DactQueryModel::getEntries,
            d_corpus->begin(),
            d_corpus->end());
}

void DactQueryModel::cancelQuery()
{
    d_cancelled = true;
}

// run async, because query() starts searching immediately
void DactQueryModel::getEntriesWithQuery(QString const &query)
{
    DactQueryModel::getEntries(
        d_corpus->query(alpinocorpus::CorpusReader::XPATH, query),
        d_corpus->end());
}

// run async
void DactQueryModel::getEntries(EntryIterator const &begin, EntryIterator const &end)
{
	try {
        mapperStarted(0); // we don't know how many entries will be found
		
        d_cancelled = false;
		
        for (EntryIterator itr(begin); !d_cancelled && itr != end; ++itr)
            mapperEntryFound(*itr);
            // @TODO could we implement something a la mapperProgressed(end - itr)?
			
        mapperStopped(d_results.size(), d_results.size());
	} catch (alpinocorpus::Error const &e) {
		qDebug() << "Error performing query: " << e.what();
	}
}