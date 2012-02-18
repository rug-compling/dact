#include <QCache>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Error.hh>
#include "FilterModel.hh"

namespace ac = alpinocorpus;

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
    return 3;
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
            return d_results.at(index.row()).name;            
        case 1:
            return d_results.at(index.row()).hits;
        case 2:
            return d_results.at(index.row()).data;
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
        case 2:
            return tr("Data");
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
        if (d_results.at(i).name == filename)
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
   
    int rows;
    {
      QMutexLocker locker(&d_resultsMutex);
      rows = d_results.size();
    }

    emit dataChanged(index(d_lastRow, 0), index(rows, 1));
    emit nEntriesFound(rows, d_hits);
    
    d_lastRow = rows - 1;
}

void FilterModel::lastDataChanged(int n, int totalEntries)
{
  Q_UNUSED(n);
  Q_UNUSED(totalEntries);

  fireDataChanged();

}

void FilterModel::lastDataChanged(int n, int totalEntries, bool cached)
{
  if (cached)
    return;

  lastDataChanged(n, totalEntries);

  d_timer->stop();
}

void FilterModel::runQuery(QString const &query, QString const &stylesheet)
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
        d_entriesFuture = QtConcurrent::run(this,
            &FilterModel::getEntriesWithQuery, d_query, stylesheet);
    else {
        if (stylesheet.isNull())
            d_entriesFuture = QtConcurrent::run(this, &FilterModel::getEntries,
                d_corpus->begin(), d_corpus->end(), false);
        else
            d_entriesFuture = QtConcurrent::run(this, &FilterModel::getEntries,
                d_corpus->beginWithStylesheet(stylesheet.toUtf8().constData()),
                d_corpus->end(), true);

    }
}

QString const &FilterModel::lastQuery() const
{
    return d_query;
}

void FilterModel::cancelQuery()
{
    d_cancelled = true;
    d_entryIterator.interrupt();
    d_entriesFuture.waitForFinished();
    d_timer->stop();
}

// run async, because query() starts searching immediately
void FilterModel::getEntriesWithQuery(QString const &query,
    QString const &stylesheet)
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
    
    std::string cQuery = query.toUtf8().constData();

    try {
        if (stylesheet.isNull())
            FilterModel::getEntries(
                d_corpus->query(alpinocorpus::CorpusReader::XPATH,
                    cQuery),
                d_corpus->end(),
                false);
        else
            FilterModel::getEntries(
                d_corpus->queryWithStylesheet(alpinocorpus::CorpusReader::XPATH,
                    query.toUtf8().constData(), stylesheet.toUtf8().constData(),
                    std::list<ac::CorpusReader::MarkerQuery>(
                        1, ac::CorpusReader::MarkerQuery(cQuery, "active", "1"))),
                d_corpus->end(),
                true);

    } catch (alpinocorpus::Error const &e) {
        qDebug() << "Alpino Error in FilterModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    } catch (std::exception const &e) {
        qDebug() << "Error in FilterModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}

// run async
void FilterModel::getEntries(EntryIterator const &begin, EntryIterator const &end,
    bool withStylesheet)
{
    try {
        emit queryStarted(0); // we don't know how many entries will be found
        
        d_cancelled = false;
        d_hits = 0;
        d_entryIterator = begin;
        
        for (; !d_cancelled && d_entryIterator != end;
          ++d_entryIterator)
        {
            ++d_hits;

            QString entry(QString::fromUtf8((*d_entryIterator).c_str()));

            // Lock the results list.
            QMutexLocker locker(&d_resultsMutex);

            /*
             * WARNING: This assumes all the hits per result only occur right after
             * each other, never shuffled. Otherwise we might want to change to QHash
             * or QMap for fast lookup.
             */
            int row = d_results.size() - 1;
            if (row >= 0 && d_results[row].name == entry)
                ++d_results[row].hits;
            // Add found file to the list
            else
            {
                ++row;
                if (withStylesheet)
                    d_results.append(Entry(entry, 1,
                        QString::fromUtf8((d_entryIterator.contents(*d_corpus)).c_str())));
                else
                    d_results.append(Entry(entry, 1, QString::null));
            }
        }
        
        if (d_cancelled)
            emit queryStopped(d_results.size(), d_results.size());
        else
            emit queryFinished(d_results.size(), d_results.size(), false);
    }
    // When d_entryIterator.interrupt() is called by cancelQuery():
    catch (alpinocorpus::IterationInterrupted const &e) {
        emit queryStopped(d_results.size(), d_results.size());
    }
    // When something goes terribly terribly wrong, like entering a query
    // that doesn't yield nodes but strings.
    catch (alpinocorpus::Error const &e) {
        qDebug() << "Error in FilterModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}
