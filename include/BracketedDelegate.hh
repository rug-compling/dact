#ifndef BRACKETEDDELEGATE_HH
#define BRACKETEDDELEGATE_HH

#include <QCache>
#include <QFile>
#include <QList>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <AlpinoCorpus/CorpusReader.hh>

class BracketedDelegate : public QStyledItemDelegate
{
    Q_OBJECT

protected:
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusReaderPtr;
    
public:
    BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent = 0);

    QString sentenceForClipboard(QModelIndex const &index) const;
    
protected:
    class Chunk
    {
        int d_depth;
        QString d_left;
        QString d_text;
        QString d_fullText;
        QString d_right;
        QString d_remainingRight;
        
    public:
        /*!
        Chunks are used to store parts of text matched by parseSentence.
         \param depth depth of the match (is it a match in a match in a match
         etc.) 1 or higher means the chunk is part of something that matched.
         */
        Chunk(int depth, QString const &left, QString const &text,
            QString const &fullText, QString const &right,
            QString const &remainingRight);
        /*!
        Returns the depth of a chunk. Depth = 0 means this chunk is not part of
        a matching node in the tree.
        */
        int depth() const;
        
        /*!
        Returns all the text left of the chunk.
        */
        QString const &left() const;
        
        /*!
        Returns all the text of the chunk itself.
        */
        QString const &text() const;
        
        /*!
        Returns all the text of the match this chunk is part of, including all 
        the submatches.
        */
        QString const &fullText() const;
        
        /*!
        Returns all the text right of the chunk.
        */
        QString const &right() const;
        
        /*!
        Returns all the text right of the match this chunk is part of.
        \sa fullText
        */
        QString const &remainingRight() const;
        
        /*!
        Returns the complete sentence, reconstructed from left + text + right.
        */
        QString sentence() const;
    };

protected:    
    /*!
    Parses a bracketed sentence into chunks. Chunks are textparts separated by
    open and closing brackets.
    \param sentence the brackets containing sentence to be parsed
    */
    QList<Chunk> parseChunks(QModelIndex const &index) const;

    QString sentence(QModelIndex const &index) const;

    QString bracketedSentence(QModelIndex const &index) const;
    

private:
    QList<Chunk> *parseSentence(QString const &sentence) const;
        
    mutable QCache<QString,QList<Chunk> > d_cache;
    CorpusReaderPtr d_corpus;
};

#endif
