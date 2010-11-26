#include "BracketedDelegates.hh"

QList<BracketedDelegate::Chunk> BracketedDelegate::interpretSentence(QString const &sentence) const
{
    QList<Chunk> chunks;
    
    int depth = 0, pos = -1, readTill = 0;
    
    while ((pos = sentence.indexOf(QRegExp("\\[|\\]"), readTill)) != -1)
    {
		// reading one char less on the left and right to omit the bracktes surrounding the match.
		// @TODO use xml for this instead of silly brackets. Maybe even merge this parser with the
		// one in DactTreeScene and keep theses parsed trees in memory to speed things up.
        chunks.append(Chunk(depth,
			sentence.left(readTill == 0 ? readTill : readTill - 1),
			sentence.mid(readTill, pos - readTill),
			sentence.mid(pos + 1)));
			
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

QString BracketedDelegate::Chunk::sentence() const
{
	return d_left + d_text + d_right;
}