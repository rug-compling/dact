#include <assert.h>
#include <sstream>

#include <QtConcurrentRun>
#include <QDateTime>
#include <QDebug>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QTimer>

#include <algorithm>
#include <set>

#include <AlpinoCorpus/CorpusInfo.hh>
#include <AlpinoCorpus/Error.hh>

#include "config.hh"
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
    d_corpus(corpus),
    d_entryCache(new EntryCache()),
    d_timer(new QTimer)
{
    connect(this, SIGNAL(queryEntryFound(QString)),
        SLOT(mapperEntryFound(QString)));
    connect(this, SIGNAL(queryFinished(int, int, QString, QString, bool, bool)),
        SLOT(finalizeQuery(int, int, QString, QString, bool, bool)));

    // Timer for progress updates.
    connect(d_timer.data(), SIGNAL(timeout()),
        SLOT(updateProgress()));
    connect(this, SIGNAL(queryStopped(int, int)),
        SLOT(stopProgress()));
    connect(this, SIGNAL(queryFinished(int, int, QString, QString, bool, bool)),
        SLOT(stopProgress()));
    connect(this, SIGNAL(queryFailed(QString)),
        SLOT(stopProgress()));
}

QueryModel::~QueryModel()
{
    cancelQuery();
}

QString QueryModel::asXML() const
{
    int rows = rowCount(QModelIndex());

    // TODO: Remove selected attribute from the filter...

    QStringList outList;
    outList.append("<statistics>");
    outList.append("<statisticsinfo>");
    outList.append(QString("<corpus>%1</corpus>")
        .arg(QString::fromUtf8(d_corpus->name().c_str())));
    outList.append(QString("<filter>%1</filter>")
        .arg(d_query));
    outList.append(QString("<attribute>%1</attribute>")
        .arg(d_attribute));
    outList.append(QString("<variants>%1</variants>")
        .arg(rows));
    outList.append(QString("<hits>%1</hits>")
        .arg(totalHits()));
    QString date(QDateTime::currentDateTime().toLocalTime().toString());
    outList.append(QString("<date>%1</date>")
        .arg(date));
    outList.append("</statisticsinfo>");

    for (int i = 0; i < rows; ++i) {
        outList.append("<statistic>");
        outList.append(QString("<value>%1</value>")
            .arg(Qt::escape(data(index(i, 0)).toString())));
        outList.append(QString("<frequency>%1</frequency>")
            .arg(data(index(i, 1)).toString()));
        outList.append(QString("<percentage>%1</percentage>")
            .arg(data(index(i, 2)).toDouble() * 100.0, 0, 'f', 1));
        outList.append("</statistic>");
    }

    outList.append("</statistics>");

    return outList.join("\n");
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
        || index.row() >= d_results.size()
        || index.row() < 0
        || index.column() > 2
        || index.column() < 0)
        return QVariant();

    switch (role)
    {
        case Qt::UserRole:
        case Qt::DisplayRole:
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

        case Qt::TextAlignmentRole:
            switch (index.column())
            {
                case 1:
                case 2:
                    return Qt::AlignRight;

                default:
                    return Qt::AlignLeft;
            }

        default:
            return QVariant();
    }
}

QString QueryModel::expandQuery(QString const &query,
    QString const &attribute) const
{
    QString expandedQuery = QString("%1/(@%2/string(), '%3')[1]")
        .arg(query)
        .arg(attribute)
        .arg(MISSING_ATTRIBUTE);

    // Not all corpus readers support this styntax.
    if (!validQuery(expandedQuery))
        expandedQuery = QString("%1/@%2")
            .arg(query)
            .arg(attribute);

    return expandedQuery;
}

void QueryModel::updateProgress()
{
    emit progressChanged(d_entryIterator.progress());
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

void QueryModel::runQuery(QString const &query, QString const &attribute,
    bool yield)
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
    
    d_query = query;
    d_attribute = attribute;
       
    // Do nothing if we where given a null-pointer
    if (!d_corpus)
        return;

    if (!query.isEmpty())
    {
        d_timer->setInterval(100);
        d_timer->setSingleShot(false);
        d_timer->start();

        if (yield)
          d_entriesFuture = QtConcurrent::run(this, &QueryModel::getEntriesWithQuery,
              query, attribute, yield);
        else
          d_entriesFuture = QtConcurrent::run(this, &QueryModel::getEntriesWithQuery,
              expandQuery(query, attribute), attribute, yield);
    }
    // If the query is empty, QueryModel is not supposed to do anything.
}

bool QueryModel::validQuery(QString const &query) const
{
    return d_corpus->isValidQuery(alpinocorpus::CorpusReader::XPATH,
        false, query.toUtf8().constData()).isRight();
}

void QueryModel::cancelQuery()
{
    d_cancelled = true;
    d_entryIterator.interrupt();
    d_entriesFuture.waitForFinished();
    d_timer->stop();
}

void QueryModel::finalizeQuery(int n, int totalEntries, QString query,
    QString attribute, bool cached, bool yield)
{
    // Just to make sure, otherwise data() will go completely crazy
    Q_ASSERT(d_hitsIndex.size() == d_results.size());

    layoutChanged(); // Please, don't do this. Let's copy FilterModel's implementation.

    if (!cached)
    {
        d_entryCache->insert(CacheKey(query, attribute, yield),
            new CacheItem(d_totalHits, d_hitsIndex, d_results));

        // this index is no longer needed, as it is only used for constructing d_hitsIndex
        d_valueIndex.clear();
    }
}

// run async, because query() starts searching immediately
void QueryModel::getEntriesWithQuery(QString const &query,
    QString const &attribute, bool yield)
{
    try {
        if (d_entryCache->contains(CacheKey(query, attribute, yield)))
        {
            {
              QMutexLocker locker(&d_resultsMutex);
              CacheItem *cachedResult = d_entryCache->object(
                  CacheKey(query, attribute, yield));
              d_totalHits = cachedResult->hits;
              d_hitsIndex = cachedResult->index;
              d_results   = cachedResult->entries;
            }

            emit queryFinished(d_results.size(), d_results.size(), query,
                attribute, true, yield);
            return;
        }

        QueryModel::getEntries(
            d_corpus->query(alpinocorpus::CorpusReader::XPATH, query.toUtf8().constData()),
            query.toUtf8().constData(), attribute.toUtf8().constData(), yield);
    } catch (std::exception const &e) {
        qDebug() << "Error in QueryModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}

// run async
void QueryModel::getEntries(EntryIterator const &i, std::string const &query,
    std::string const &attribute, bool yield)
{
    alpinocorpus::CorpusInfo corpusInfo =
        alpinocorpus::predefinedCorpusOrFallback(d_corpus->type());

    if (i.hasProgress())
      emit queryStarted(100);
    else
      emit queryStarted(0);
        
    try {
        d_cancelled = false;
        d_entryIterator = i;

        std::set<std::string> seen;
       
        while (!d_cancelled && d_entryIterator.hasNext())
        {
            alpinocorpus::Entry e = d_entryIterator.next(*d_corpus);

            if (yield) {
                // Check whether we have already processed this tree before.
                if (seen.find(e.name) != seen.end())
                  continue;
                seen.insert(e.name);

                std::vector<alpinocorpus::LexItem> sent =
                  d_corpus->sentence(e.name, query, attribute,
                      MISSING_ATTRIBUTE_SHORT, corpusInfo);

                // Find all match ids.
                std::set<size_t> ids;
                foreach (alpinocorpus::LexItem const &lexItem, sent)
                  ids.insert(lexItem.matches.begin(), lexItem.matches.end());

                foreach (size_t id, ids) {
                    std::ostringstream oss;
                    bool firstToken = true;

                    foreach (alpinocorpus::LexItem const &lexItem, sent) {
                        if (lexItem.matches.find(id) != lexItem.matches.end()) {
                            if (firstToken)
                              firstToken = false;
                            else
                              oss << " ";

                            oss << lexItem.word;
                        }
                    }

                    std::string result = oss.str();
                    emit queryEntryFound(QString::fromUtf8(result.c_str()));
                }
                size_t prevDepth = 0;
            }
            else
              emit queryEntryFound(QString::fromUtf8(e.contents.c_str()));
        }
            
        if (d_cancelled)
            emit queryStopped(d_results.size(), d_results.size());
        else
            emit queryFinished(d_results.size(), d_results.size(),
                QString::fromUtf8(query.c_str()),
                QString::fromUtf8(attribute.c_str()), false, yield);
    }
    // Catch d_entryIterator.interrupt()'s shockwaves of terror
    catch (alpinocorpus::IterationInterrupted const &e) {
        emit queryStopped(d_results.size(), d_results.size());
    }
    catch (alpinocorpus::Error const &e) {
        qDebug() << "Error in QueryModel::getEntries: " << e.what();
        emit queryFailed(e.what());
    }
}

void QueryModel::stopProgress()
{
    d_timer->stop();
}

