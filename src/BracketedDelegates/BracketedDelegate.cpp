#include "BracketedDelegates.hh"

QList<Chunk> BracketedDelegate::interpretSentence(QString const &sentence) const
{
    QList<Chunk> chunks;
    
    int depth = 0, pos = -1, readTill = 0;
    
    while ((pos = sentence.indexOf(QRegExp("\\[|\\]"), readTill)) != -1)
    {
        QString text(sentence.mid(readTill, pos - readTill));
        
        chunks.append(Chunk(depth, text));
        
        readTill = pos + 1;
                
        if (sentence[pos] == '[')
            ++depth;
        
        else if (sentence[pos] == ']')
            --depth;
    }
    
    chunks.append(Chunk(depth, sentence.mid(readTill)));
    
    return chunks;
}