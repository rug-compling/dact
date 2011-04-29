#ifndef QUERYMODEL_HH
#define QUERYMODEL_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractTableModel>
#include <QFuture>
#include <QHash>
#include <QList>
#include <QPair>

class QueryModel : public QAbstractTableModel
{
    Q_OBJECT
    
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
    
public:
    QueryModel(CorpusPtr corpus, QObject *parent = 0);
    ~QueryModel();
    int rowCount(QModelIndex const &index) const;
    int columnCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    inline int totalHits() const { return d_totalHits; }
    
    void runQuery(QString const &xpath_query = "");
    void cancelQuery();

signals:
    void queryStarted(int totalEntries);
    void queryProgressed(int n, int totalEntries);
    void queryStopped(int n, int totalEntries);
    void queryEntryFound(QString entry);
    
private:
    void getEntries(EntryIterator const &begin, EntryIterator const &end);
    void getEntriesWithQuery(QString const &query);
    
private slots:
    void mapperEntryFound(QString entry);
    
private:
    bool volatile d_cancelled;
    CorpusPtr d_corpus;
    
    QHash<QString, int> d_index;
    QList< QPair<QString, int> > d_results;
    int d_totalHits;
    
};

#endif