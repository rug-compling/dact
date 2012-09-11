#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedVisibilityDelegate.hh"
#include "../parseString.hh"

#include <QtDebug>

BracketedVisibilityDelegate::BracketedVisibilityDelegate(CorpusReaderPtr corpus)
:
    BracketedDelegate(corpus)
{}

QString BracketedVisibilityDelegate::formatSentence(QModelIndex const &index) const
{
    QStringList result;
    
    std::vector<LexItem> items = retrieveSentence(index);
    int hits = index.sibling(index.row(), 1).data().toInt();

    for (int i = 0; i < hits; ++i)
    {
        QStringList line;

        foreach (LexItem const &item, items)
            if (item.matches.contains(i))
                line.append(item.word);

        result.append(line.join(" "));
    }
    
    return result.join("\n");
}

QSize BracketedVisibilityDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    return QSize(
        2400, // @TODO yes, this number is completely random. Please please find a way to calculate a sane guess.
        option.fontMetrics.height() * index.sibling(index.row(), 1).data().toInt()
    );
}

void BracketedVisibilityDelegate::paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    QString result(formatSentence(index));
    
    QBrush brush = option.state & QStyle::State_Selected
        ? option.palette.highlightedText()
        : option.palette.text();
    
    painter->save();
    
    painter->setPen(brush.color());
    painter->setBrush(brush);
    
    painter->drawText(option.rect, Qt::AlignLeft, result);
    
    painter->restore();
}

QString BracketedVisibilityDelegate::sentenceForClipboard(QModelIndex const &index) const
{
    return formatSentence(index).trimmed();
}
