#include <QCache>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Error.hh>
#include "FilterModel.hh"

FilterModel::FilterModel(CorpusPtr corpus, QObject *parent)
:
    QAbstractTableModel(parent),
    d_corpus(corpus),
    d_hits(0),
    d_entryCache(new EntryCache(1000000)),
    d_timer(new QTimer)
{
    connect(d_timer.data(), SIGNAL(timeout()),
        SLOT(fireDataChanged()));
    connect(this, SIGNAL(queryStopped(int, int)),
        SLOT(lastDataChanged(int, int)));
    connect(this, SIGNAL(queryFinished(int, int, bool)),
        SLOT(lastDataChanged(int, int, bool)));
    connect(this, SIGNAL(queryFinished(int, int, bool)),
        SLOT(finalizeQuery(int, int, bool)));
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
    QMutexLocker locker(&d_resultsMutex);
    return d_results.size();
}

QVariant FilterModel::data(QModelIndex const &index, int role) const
{
    QMutexLocker locker(&d_resultsMutex);

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

void FilterModel::finalizeQuery(int n, int totalEntries, bool cached)
{
    if (!cached) {
        // Cache the query
        d_entryCache->insert(d_query, new CacheItem(d_hits, d_results));
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

QModelIndex FilterModel::indexOfFile(QString const &filename) const
{
    QMutexLocker locker(&d_resultsMutex);
    int index = -1;
    for (int i = 0, size = d_results.size(); i < size; ++i)
    {
        if (d_results.at(i).first == filename)
        {
            index = i;
            break;
        }
    }
    
    return createIndex(index, 0);
}

void FilterModel::fireDataChanged()
{
    // @TODO make this more robust so no assumption is required.
   
    int row;
    {
      QMutexLocker locker(&d_resultsMutex);
      row = d_results.size() - 1;
    }

    emit dataChanged(index(d_lastRow, 0), index(row + 1, 1));
    
    d_lastRow = row;
}

void FilterModel::lastDataChanged(int n, int totalEntries)
{
  Q_UNUSED(n);
  Q_UNUSED(totalEntries);

  fireDataChanged();

  d_timer->stop();
}

void FilterModel::lastDataChanged(int n, int totalEntries, bool cached)
{
  if (cached)
    return;

  lastDataChanged(n, totalEntries);
}

void FilterModel::runQuery(QString const &query)
{
    cancelQuery(); // just in case
    
    int size;
    {
      QMutexLocker locker(&d_resultsMutex);
      size = d_results.size();
      d_results.clear();
    }
    
    emit dataChanged(index(0, 0), index(size, 0));
    
    d_query = query;
    
    // Do nothing if this is a dummy filter model with a stupid null pointer
    if (!d_corpus)
        return;
   
    d_timer->setInterval(100);
    d_timer->setSingleShot(false);
    d_lastRow = 0;
    d_timer->start();

    if (!d_query.isEmpty())
        d_entriesFuture = QtConcurrent::run(this, &FilterModel::getEntriesWithQuery, d_query);
    else
        d_entriesFuture = QtConcurrent::run(this, &FilterModel::getEntries,
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
    d_entriesFuture.waitForFinished();
}

// run async, because query() starts searching immediately
void FilterModel::getEntriesWithQuery(QString const &query)
{
    if (d_entryCache->contains(query)) {
        {
          QMutexLocker locker(&d_resultsMutex);
          d_results = d_entryCache->object(query)->entries;
        }

        d_hits = d_entryCache->object(query)->hits;

        emit queryFinished(d_results.size(), d_results.size(), true);
        return;
    }
    
    FilterModel::getEntries(
        d_corpus->query(alpinocorpus::CorpusReader::XPATH, query.toUtf8().constData()),
        d_corpus->end());    
}

// run async
void FilterModel::getEntries(EntryIterator const &begin, EntryIterator const &end)
{
    try {
        emit queryStarted(0); // we don't know how many entries will be found
        
        d_cancelled = false;
        d_hits = 0;
        
        for (EntryIterator itr(begin); !d_cancelled && itr != end; ++itr)
        {
            ++d_hits;

            QString entry(QString::fromUtf8((*itr).c_str()));

            // Lock the results list.
            QMutexLocker locker(&d_resultsMutex);

            /*
             * WARNING: This assumes all the hits per result only occur right after
             * each other, never shuffled. Otherwise we might want to change to QHash
             * or QMap for fast lookup.
             */
            int row = d_results.size() - 1;
            if (row >= 0 && d_results[row].first == entry)
                ++d_results[row].second;
            // Add found file to the list
            else
            {
                ++row;
                d_results.append(value_type(entry, 1));
            }
        }
        
        if (d_cancelled)
            emit queryStopped(d_results.size(), d_results.size());
        else
            emit queryFinished(d_results.size(), d_results.size(), false);
    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Error in FilterModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}
