#ifndef FILTERMODEL_HH
#define FILTERMODEL_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractListModel>
#include <QCache>
#include <QFuture>
#include <QList>
#include <QPair>
#include <QSharedPointer>

class FilterModel : public QAbstractTableModel
{
    Q_OBJECT
    
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
    typedef QPair<QString, int> value_type;
        
public:
    FilterModel(CorpusPtr corpus, QObject *parent = 0);
    ~FilterModel();
    int columnCount(QModelIndex const &index) const;
    int rowCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    int hits() const;
    QModelIndex indexOfFile(QString const &filename) const;
    
    void runQuery(QString const &xpath_query = "");
    void cancelQuery();
    QString const &lastQuery() const;

signals:
    void queryFailed(QString error);
    void queryStarted(int totalEntries);
    void queryFinished(int n, int totalEntries, bool cached);
    void queryStopped(int n, int totalEntries);
    void entryFound(QString entry);
    
private:
    void getEntries(EntryIterator const &begin, EntryIterator const &end);
    void getEntriesWithQuery(QString const &query);
    
private slots:
    void mapperEntryFound(QString entry);
    void finalizeQuery(int n, int totalEntries, bool cached);
    
private:
    typedef QList<value_type> EntryList;
    
    struct CacheItem {
        CacheItem(int newHits, EntryList newEntries) : hits(newHits), entries(newEntries) {}
        
        int hits;
        EntryList entries;
    };
    
    typedef QCache<QString, CacheItem> EntryCache;

    bool volatile d_cancelled;
    CorpusPtr d_corpus;
    QList<value_type> d_results;
    QString d_query;
    QFuture<void> d_entriesFuture;
    int d_hits;
    QSharedPointer<EntryCache> d_entryCache;
};

inline int FilterModel::hits() const
{
    return d_hits;
}

#endif
