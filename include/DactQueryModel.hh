#ifndef DACTQUERYMODEL_HH
#define DACTQUERYMODEL_HH

#include "XPathMapper.hh"
#include <AlpinoCorpus/CorpusReader.hh>
#include <QAbstractListModel>
#include <QFuture>
#include <QList>

class DactQueryModel : public QAbstractListModel
{
    Q_OBJECT
    
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusPtr;
    typedef alpinocorpus::CorpusReader::EntryIterator EntryIterator;
    
public:
    DactQueryModel(CorpusPtr corpus, QObject *parent = 0);
	~DactQueryModel();
    int rowCount(QModelIndex const &index) const;
    QVariant data(QModelIndex const &index, int role) const;
    QVariant headerData(int column, Qt::Orientation orientation, int role) const;
    
    void runQuery(QString const &xpath_query = "");
    void cancelQuery();

signals:
    void queryStarted(int totalEntries);
    void queryProgressed(int n, int totalEntries);
    void queryStopped(int n, int totalEntries);
    
private:
    void getEntries(EntryIterator const &begin, EntryIterator const &end);
    void getEntriesWithQuery(QString const &query);
    
private slots:
    void mapperEntryFound(QString entry);
    void mapperStarted(int totalEntries);
    void mapperProgressed(int n, int totalEntries);
    void mapperStopped(int n, int totalEntries);
    
private:
    bool d_cancelled;
    CorpusPtr d_corpus;
    QList<QString> d_results;
    
};

#endif