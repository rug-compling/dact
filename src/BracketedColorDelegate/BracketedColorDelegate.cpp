#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedColorDelegate.hh"

#include <QtDebug>

QSize BracketedColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return option.fontMetrics.size(Qt::TextSingleLine, index.data().toString());
}

void BracketedColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString sentence = index.data().toString();
    
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    // Find the position of the matching block of text. It won't have any trouble
    // with nested blocks, but it will make a mess when two non-nested blocks are
    // found.
    int openingBracketPos = sentence.indexOf('[');
    int closingBracketPos = sentence.lastIndexOf(']') + 1;
    QRectF textBox = option.rect;
    QRectF usedSpace;
    
    QBrush brush = option.state & QStyle::State_Selected
        ? option.palette.highlightedText()
        : option.palette.text();
    
    /*
    qWarning()
        << sentence.left(openingBracketPos) << "::"
        << sentence.mid(openingBracketPos, closingBracketPos - openingBracketPos) << "::"
        << sentence.mid(closingBracketPos);
    */
    
    // Words before the match
    if (openingBracketPos > 0)
    {
        painter->setPen(brush.color());
        painter->setBrush(brush);
        painter->drawText(textBox, Qt::AlignLeft, sentence.left(openingBracketPos), &usedSpace);
        textBox.setLeft(textBox.left() + usedSpace.width());
    }
    
    // Words for the matching node
    {
        QString matchNodeText = sentence.mid(openingBracketPos, closingBracketPos - openingBracketPos);
        QRectF matchBox = textBox;
        matchBox.setWidth(option.fontMetrics.width(matchNodeText));
    
        painter->setPen(QColor(Qt::white));
        painter->fillRect(matchBox, QColor(Qt::darkGreen));
        painter->drawText(matchBox, Qt::AlignLeft, matchNodeText);
    
        textBox.setLeft(textBox.left() + matchBox.width());
    }
    
    // rest of the sentence
    if (closingBracketPos < sentence.length())
    {
        painter->setPen(brush.color());
        painter->setBrush(brush);
        painter->drawText(textBox, Qt::AlignLeft, sentence.mid(closingBracketPos), &usedSpace);
    }
}