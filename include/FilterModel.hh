#ifndef FILTERMODEL_HH
#define FILTERMODEL_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractListModel>
#include <QCache>
#include <QFuture>
#include <QList>
#include <QMutex>
#include <QPair>
#include <QSharedPointer>
#include <QTimer>

class FilterModel : public QAbstractTableModel
{
    Q_OBJECT
    
    struct Entry {
        Entry(QString nName, int nHits, QString nData) :
            name(nName), hits(nHits), data(nData) {}
        
        QString name;
        int hits;
        QString data;  
    };

    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
        
public:
    FilterModel(CorpusPtr corpus, QObject *parent = 0);
    ~FilterModel();
    QString asXML() const;
    int columnCount(QModelIndex const &index) const;
    int rowCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    int hits() const;
    QModelIndex indexOfFile(QString const &filename) const;
    
    void runQuery(QString const &xpath_query = "",
        QString const &stylesheet = QString::null);
    void cancelQuery();
    QString const &lastQuery() const;

signals:
    void queryFailed(QString error);
    void queryStarted(int totalEntries);
    void queryFinished(int n, int totalEntries, bool cached);
    void queryStopped(int n, int totalEntries);
    void nEntriesFound(int entries, int hits);
    
private:
    void getEntries(EntryIterator const &i, bool withStylesheet);
    void getEntriesWithQuery(QString const &query, QString const &stylesheet);
    
private slots:
    void fireDataChanged();
    void lastDataChanged(int n, int totalEntries);
    void lastDataChanged(int n, int totalEntries, bool cached);
    void finalizeQuery(int n, int totalEntries, bool cached);
    
private:
    typedef QList<Entry> EntryList;
    
    struct CacheItem {
        CacheItem(int newHits, EntryList newEntries) : hits(newHits), entries(newEntries) {}
        
        int hits;
        EntryList entries;
    };
    
    typedef QCache<QString, CacheItem> EntryCache;

    bool volatile d_cancelled;
    CorpusPtr d_corpus;
    EntryList d_results;
    mutable QMutex d_resultsMutex;
    QString d_query;
    QFuture<void> d_entriesFuture;
    int d_hits;
    QSharedPointer<EntryCache> d_entryCache;
    QSharedPointer<QTimer> d_timer;
    int d_lastRow;
    EntryIterator d_entryIterator;
};

inline int FilterModel::hits() const
{
    return d_hits;
}

#endif
