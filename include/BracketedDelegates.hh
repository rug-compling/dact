#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QCache>
#include <QFile>
#include <QList>
#include <QModelIndex>
#include <QStyledItemDelegate>
#include <AlpinoCorpus/CorpusReader.hh>
#include "XSLTransformer.hh"

class BracketedDelegate : public QStyledItemDelegate
{
    Q_OBJECT

protected:
    typedef QSharedPointer<alpinocorpus::CorpusReader> CorpusReaderPtr;
    
public:
    BracketedDelegate(CorpusReaderPtr corpus, QWidget *parent = 0);
    
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
	
    /*!
    Access the corpusreader provided to the constructor.
    */
    alpinocorpus::CorpusReader const &reader() const;
    
    /*!
	Parses a bracketed sentence into chunks. Chunks are textparts separated by
	open and closing brackets.
	\param sentence the brackets containing sentence to be parsed
	*/
    QList<Chunk> parseSentence(QString const &sentence) const;

private:
    CorpusReaderPtr d_corpus;

};

inline alpinocorpus::CorpusReader const &BracketedDelegate::reader() const
{
    return *d_corpus.data();
}

class BracketedColorDelegate : public BracketedDelegate
{
    Q_OBJECT
public:
    BracketedColorDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
private:
    void loadSettings();
    QString transformXML(QString const &xml, QString const &query) const;
    QString const &transformedCorpusXML(QModelIndex const &index) const;
    
    QColor d_backgroundColor;
    mutable QCache<QString,QString> d_cache;
    QFile d_stylesheet;
    XSLTransformer d_transformer;
};

class BracketedVisibilityDelegate : public BracketedDelegate
{
    Q_OBJECT
public:
    BracketedVisibilityDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
private:
    QString formatSentence(QString const &sentence) const;
};

class BracketedKeywordInContextDelegate : public BracketedDelegate
{
    Q_OBJECT
public:
    BracketedKeywordInContextDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
private:
    void loadColorSettings();
    QColor d_keywordForeground;
    QColor d_keywordBackground;
    QColor d_contextForeground;
    QColor d_contextBackground;
};

#endif