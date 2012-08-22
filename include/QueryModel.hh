#ifndef QUERYMODEL_HH
#define QUERYMODEL_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractTableModel>
#include <QCache>
#include <QFuture>
#include <QHash>
#include <QList>
#include <QPair>
#include <QSharedPointer>

class QueryModel : public QAbstractTableModel
{
    Q_OBJECT
    
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
    typedef QPair<QString, int> value_type;

private:
    class HitsCompare
    {
        QueryModel const &d_parent;
    public:
        HitsCompare(QueryModel const &parent);
        bool operator()(int, int);
    };
    
public:
    static QString const MISSING_ATTRIBUTE;

    QueryModel(CorpusPtr corpus, QObject *parent = 0);
    ~QueryModel();
    QString asXML() const;
    int rowCount(QModelIndex const &index) const;
    int columnCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    QString expandQuery(QString const &query, QString const &attribute) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    inline int totalHits() const { return d_totalHits; }
    
    void runQuery(QString const &query, QString const &attribute);
    void cancelQuery();
    bool validQuery(QString const &query) const;
    
signals:
    void queryFailed(QString error);
    void queryStarted(int totalEntries);
    void queryStopped(int n, int totalEntries);
    void queryFinished(int n, int totalEntries, bool cached);
    void queryEntryFound(QString entry);
    
private:
    void getEntries(EntryIterator const &i);
    void getEntriesWithQuery(QString const &query);
    
private slots:
    void mapperEntryFound(QString entry);
    void finalizeQuery(int n, int totalEntries, bool cached);
    
private:
    typedef QList<int> EntryIndex;
    typedef QList<value_type> EntryList;

    struct CacheItem {
        CacheItem(int newHits, EntryIndex newIndex, EntryList newEntries) : hits(newHits), index(newIndex), entries(newEntries) {}
        
        int hits;
        QList<int> index;
        EntryList entries;
    };

    typedef QCache<QString, CacheItem> EntryCache;

    bool volatile d_cancelled;
    CorpusPtr d_corpus;
    EntryIterator d_entryIterator;
    
    QHash<QString, int> d_valueIndex;
    EntryIndex d_hitsIndex;
    QList<value_type> d_results;
    int d_totalHits;
    QFuture<void> d_entriesFuture;
    QString d_attribute;
    QString d_query;

    mutable QMutex d_resultsMutex;
    QSharedPointer<EntryCache> d_entryCache;
};

#endif
