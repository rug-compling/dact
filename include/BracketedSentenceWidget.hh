#ifndef BRACKETEDSENTENCEWIDGET_HH
#define BRACKETEDSENTENCEWIDGET_HH

#include <QFile>
#include <QFrame>
#include <QList>
#include <QString>

#include "XSLTransformer.hh"

class BracketedSentenceWidget : public QFrame
{
    Q_OBJECT
    
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
    
public:
    BracketedSentenceWidget(QWidget *parent = 0);
    void setParse(QString const &parse);
    void setQuery(QString const &query);

protected:
    void paintEvent(QPaintEvent *event);
    
private:
    void updateChunks();
    QList<Chunk> *parseSentence(QString const &sentence) const;
    QString transformXML(QString const &xml, QString const &query) const;
    
    QList<Chunk> *d_chunks;
    QString d_parse;
    QString d_query;
    QFile d_stylesheet;
    XSLTransformer d_transformer;
};

#endif // BRACKETEDSENTENCEWIDGET_HH