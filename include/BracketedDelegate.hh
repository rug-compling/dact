#ifndef BRACKETEDDELEGATE_HH
#define BRACKETEDDELEGATE_HH

#include <vector>

#include <QCache>
#include <QFile>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/LexItem.hh>

class BracketedDelegate : public QStyledItemDelegate
{
    Q_OBJECT

protected:
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusReaderPtr;
    
public:
    BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent = 0);

    virtual QString sentenceForClipboard(QModelIndex const &index) const;

protected:    
    /*!
    Parses a bracketed sentence into chunks. Chunks are textparts separated by
    open and closing brackets.
    \param sentence the brackets containing sentence to be parsed
    */
    std::vector<alpinocorpus::LexItem> retrieveSentence(QModelIndex const &index) const;

    QString bracketedSentence(QModelIndex const &index) const;
    

private:        
    mutable QCache<QString,std::vector<alpinocorpus::LexItem> > d_cache;
    CorpusReaderPtr d_corpus;
};

#endif
