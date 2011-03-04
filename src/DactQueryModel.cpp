#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <alpinocorpus/Error.hh>
#include "DactQueryModel.hh"

DactQueryModel::DactQueryModel(CorpusPtr corpus, QObject *parent)
:
d_corpus(corpus),
QAbstractListModel(parent)
{}

DactQueryModel::~DactQueryModel()
{
	cancelQuery();
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
            && (role == Qt::DisplayRole || role == Qt::UserRole))
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
    if (d_results.contains(entry))
        return;
    
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