#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedDelegates.hh"

#include <QtDebug>

QSize BracketedColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return option.fontMetrics.size(Qt::TextSingleLine, index.data().toString());
}

void BracketedColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    QString sentence(index.data().toString());
    QList<Chunk> chunks(interpretSentence(sentence));
    QRectF textBox(option.rect);
    QRectF usedSpace;
    QColor highlightColor(QColor(Qt::darkGreen));
    
    QBrush brush(option.state & QStyle::State_Selected
        ? option.palette.highlightedText()
        : option.palette.text());
        
    foreach (Chunk chunk, chunks)
    {
        if (chunk.text().isEmpty())
            continue;
        
        QRectF chunkBox(textBox);
        chunkBox.setWidth(option.fontMetrics.width(chunk.text()));
        
        if (chunk.depth() > 0)
        {
            highlightColor.setAlpha(std::min(85 + 42 * chunk.depth(), 255));
            painter->setPen(QColor(Qt::white));
            painter->fillRect(chunkBox, highlightColor);
        }
        else
        {
            painter->setPen(brush.color());
            painter->setBrush(brush);
        }
        
        painter->drawText(chunkBox, Qt::AlignLeft, chunk.text());
        textBox.setLeft(textBox.left() + chunkBox.width());
    }
}