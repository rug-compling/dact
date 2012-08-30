#ifndef CHUNK_HH
#define CHUNK_HH

#include <list>
#include <QString>

struct _xmlNode;
typedef struct _xmlNode xmlNode;

class Chunk
{
    size_t d_depth;
    QString d_left;
    QString d_text;
    QString d_fullText;
    QString d_right;
    QString d_remainingRight;

public:
    static std::list<Chunk> *parseSentence(QString sentence);


    /*!
    Chunks are used to store parts of text matched by parseSentence.
     \param depth depth of the match (is it a match in a match in a match
     etc.) 1 or higher means the chunk is part of something that matched.
     */
    Chunk(size_t depth, QString const &left, QString const &text,
        QString const &fullText, QString const &right,
        QString const &remainingRight);

    /*!
    Returns the depth of a chunk. Depth = 0 means this chunk is not part of
    a matching node in the tree.
    */
    size_t depth() const;

    /*!
    Returns all the text left of the chunk.
    */
    QString const &left() const;

    /*!
    Set the text left of the chunk.
    */
    void setLeft(QString const &left);

    /*!
    Set the text right of the chunk.
    */
    void setRemainingRight(QString const &right);

    /*!
    Set the text right of the text chunk.
    */
    void setRight(QString const &right);

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

private:

    static void processTree(xmlNode *node, size_t depth,
        std::list<Chunk> *chunks);
};

#endif // CHUNK_HH
