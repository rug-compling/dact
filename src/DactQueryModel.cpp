#include <QDebug>
#include <QtConcurrentRun>

#include <algorithm>

#include <alpinocorpus/Error.hh>
#include "DactQueryModel.hh"

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
        QtConcurrent::run(this, &DactQueryModel::getEntries);
}

void DactQueryModel::cancelQuery()
{
    // @TODO this does not cancel the other methods
    if (d_mapper.isRunning())
    {
        d_mapper.cancel();
        d_mapper.wait();
    }
}

// run async
void DactQueryModel::getEntries()
{
    // @TODO make this stoppable
    for (alpinocorpus::CorpusReader::EntryIterator itr(d_corpus->begin()), end(d_corpus->end());
         itr != end; itr++)
        mapperEntryFound(*itr);
}

// run async
void DactQueryModel::getEntriesWithQuery(QString const &query)
{
	try {
        mapperStarted(0); // we don't know how many entries will be found
		
		std::for_each(
		    d_corpus->query(alpinocorpus::CorpusReader::XPATH, query),
			d_corpus->end(),
			std::bind1st(std::mem_fun(&DactQueryModel::mapperEntryFound), this)
		);
			
        mapperStopped(d_results.size(), d_results.size());
	} catch (alpinocorpus::NotImplemented const &e) {
		d_mapper.start(d_corpus.data(), query, &d_entryMap);
	} catch (alpinocorpus::Error const &e) {
		qDebug() << "Error performing query: " << e.what();
	}
}