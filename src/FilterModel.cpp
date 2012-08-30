#include <QCache>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QtConcurrentRun>

#include <algorithm>

#include <AlpinoCorpus/Entry.hh>
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
    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 1)
            return (Qt::AlignTop + Qt::AlignHCenter);
        else
            return Qt::AlignTop;
    }


    size_t nResults;
    {
        QMutexLocker locker(&d_resultsMutex);
        nResults = d_results.size();
    }

    if (!index.isValid()
        || index.row() >= nResults
        || index.row() < 0
        || !(role == Qt::DisplayRole || role == Qt::UserRole))
        return QVariant();

    QMutexLocker locker(&d_resultsMutex);
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

QString FilterModel::asXML() const
{
    QStringList docList;
    docList.append("<entries>");

    docList.append("<entriesinfo>");
    docList.append(QString("<corpus>%1</corpus>")
        .arg(QString::fromUtf8(d_corpus->name().c_str())));
    docList.append(QString("<filter>%1</filter>").arg(d_query));
    QString date(QDateTime::currentDateTime().toLocalTime().toString());
    docList.append(QString("<date>%1</date>").arg(date));
    docList.append("</entriesinfo>");

    int count = rowCount(QModelIndex());
    for (size_t i = 0; i < count; i++) {
        QString filename = data(index(i, 0), Qt::DisplayRole).toString();
        QString count = data(index(i, 1), Qt::DisplayRole).toString();
        QString xmlData = data(index(i, 2), Qt::DisplayRole).toString().trimmed()
            .replace("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", "");

        QString line = QString("<entry><filename>%1</filename><count>%2</count>%3</entry>")
            .arg(filename)
            .arg(count)
            .arg(xmlData);

        docList.append(line);
    }
    docList.append("</entries>");

    QString doc = docList.join("\n");

    return doc;
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

    emit layoutAboutToBeChanged();
    emit dataChanged(index(d_lastRow, 0), index(rows - 1, 2));
    emit nEntriesFound(rows, d_hits);
    emit layoutChanged();

    if (d_entryIterator.hasProgress())
      emit progressChanged(static_cast<int>(d_entryIterator.progress()));

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
  d_timer->stop();

  if (cached)
    return;

  lastDataChanged(n, totalEntries);
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
                d_corpus->entries(), false);
        else
            d_entriesFuture = QtConcurrent::run(this, &FilterModel::getEntries,
                d_corpus->entriesWithStylesheet(stylesheet.toUtf8().constData()),
                true);

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
                false);
        else
            FilterModel::getEntries(
                d_corpus->queryWithStylesheet(alpinocorpus::CorpusReader::XPATH,
                    query.toUtf8().constData(), stylesheet.toUtf8().constData(),
                    std::list<ac::CorpusReader::MarkerQuery>(
                        1, ac::CorpusReader::MarkerQuery(cQuery, "active", "1"))),
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
void FilterModel::getEntries(EntryIterator const &i, bool withStylesheet)
{
    if (i.hasProgress())
        emit queryStarted(100);
    else
        emit queryStarted(0); // we don't know how many entries will be found


    try {
        d_cancelled = false;
        d_hits = 0;
        d_entryIterator = i;

        while (!d_cancelled && d_entryIterator.hasNext())
        {
            ++d_hits;

            alpinocorpus::Entry e = d_entryIterator.next(*d_corpus);

            QString name(QString::fromUtf8(e.name.c_str()));

            /*
             * WARNING: This assumes all the hits per result only occur right after
             * each other, never shuffled. Otherwise we might want to change to QHash
             * or QMap for fast lookup.
             */
            int row = d_results.size() - 1;
            if (row >= 0 && d_results[row].name == name) {
                QMutexLocker locker(&d_resultsMutex);
                ++d_results[row].hits;
            }
            // Add found file to the list
            else
            {
                ++row;
                if (withStylesheet) {
                    QString contents =
                        QString::fromUtf8((e.contents.c_str()));
                    d_results.append(Entry(name, 1, contents));
                }
                else {
                    QMutexLocker locker(&d_resultsMutex);
                    d_results.append(Entry(name, 1, QString::null));
                }
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
