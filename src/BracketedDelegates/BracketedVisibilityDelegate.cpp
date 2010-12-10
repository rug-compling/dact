#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedDelegates.hh"

#include <QtDebug>

QString BracketedVisibilityDelegate::formatSentence(QString const &sentence) const
{
    QString result;
    
    foreach (Chunk chunk, interpretSentence(sentence))
    {
        if (!chunk.depth() || chunk.text().isEmpty())
            continue;
        
        // each result on a new line, like they are individual entries.
        if (!result.isEmpty())
            //result += '\n'; // use this for one line per match
            result += ", "; // or this for one line per file
        
        // note that text() currently does not contain the text of the full match
        // because it could contain submatches, which are separate chunks.
        result += chunk.text();
    }
    
    return result;
}

QSize BracketedVisibilityDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    QString result(formatSentence(index.data().toString()));
    
    return option.fontMetrics.size(0, result);
}

void BracketedVisibilityDelegate::paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    QString result(formatSentence(index.data().toString()));
    
    QBrush brush = option.state & QStyle::State_Selected
        ? option.palette.highlightedText()
        : option.palette.text();
    
    painter->save();
    
    painter->setPen(brush.color());
    painter->setBrush(brush);
    
    painter->drawText(option.rect, Qt::AlignLeft, result);
    
    painter->restore();
}