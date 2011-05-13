#ifndef FILTERMODEL_HH
#define FILTERMODEL_HH

#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractListModel>
#include <QFuture>
#include <QList>
#include <QPair>

class FilterModel : public QAbstractTableModel
{
    Q_OBJECT
    
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
    typedef QPair<QString,int> value_type;
    
public:
    FilterModel(CorpusPtr corpus, QObject *parent = 0);
    ~FilterModel();
    int columnCount(QModelIndex const &index) const;
    int rowCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    int hits() const;
    
    void runQuery(QString const &xpath_query = "");
    void cancelQuery();
    QString const &lastQuery() const;

signals:
    void queryStarted(int totalEntries);
    void queryStopped(int n, int totalEntries);
    void entryFound(QString entry);
    
private:
    void getEntries(EntryIterator const &begin, EntryIterator const &end);
    void getEntriesWithQuery(QString const &query);
    
private slots:
    void mapperEntryFound(QString entry);
    
private:
    bool volatile d_cancelled;
    CorpusPtr d_corpus;
    QList<value_type> d_results;
    QString d_query;
    QFuture<void> d_entriesFuture;
    int d_hits;
};

inline int FilterModel::hits() const
{
    return d_hits;
}

#endif