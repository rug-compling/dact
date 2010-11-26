#include "BracketedDelegates.hh"

QList<BracketedDelegate::Chunk> BracketedDelegate::interpretSentence(QString const &sentence) const
{
    QList<Chunk> chunks;
    
    int depth = 0, pos = -1, readTill = 0;
    
    while ((pos = sentence.indexOf(QRegExp("\\[|\\]"), readTill)) != -1)
    {
        chunks.append(Chunk(depth,
			sentence.left(readTill),
			sentence.mid(readTill, pos - readTill),
			sentence.mid(pos)));
        
        readTill = pos + 1;
                
        if (sentence[pos] == '[')
            ++depth;
        
        else if (sentence[pos] == ']')
            --depth;
    }
    
    chunks.append(Chunk(depth, sentence.left(readTill), sentence.mid(readTill), ""));
    
    return chunks;
}

BracketedDelegate::Chunk::Chunk(int depth, QString const &left, QString const &text, QString const &right)
:
	d_depth(depth),
	d_left(left),
	d_text(text),
	d_right(right)
{}

int BracketedDelegate::Chunk::depth() const
{
	return d_depth;
}

QString const &BracketedDelegate::Chunk::left() const
{
	return d_left;
}

QString const &BracketedDelegate::Chunk::text() const
{
	return d_text;
}

QString const &BracketedDelegate::Chunk::right() const
{
	return d_right;
}