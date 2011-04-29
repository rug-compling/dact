#include <cmath>
#include <stdexcept>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>
#include <QHash>

#include "BracketedDelegates.hh"

#include <QtDebug>

BracketedColorDelegate::BracketedColorDelegate(CorpusReaderPtr corpus)
:
    BracketedDelegate(corpus)
{
    loadSettings();
}

void BracketedColorDelegate::loadSettings()
{
    QSettings settings;
    settings.beginGroup("CompleteSentence");
    
    d_backgroundColor = settings.value("background", QColor(Qt::green)).value<QColor>();
}

QSize BracketedColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // @TODO this is incorrect, these are the dimensions required to display the filename!
    return option.fontMetrics.size(Qt::TextSingleLine, index.data().toString());
}

void BracketedColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    try {
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
    
        QList<Chunk> chunks(parseChunks(index));
        QRectF textBox(option.rect);
        QRectF usedSpace;
        QColor highlightColor(d_backgroundColor);
    
        QBrush brush(option.state & QStyle::State_Selected
            ? option.palette.highlightedText()
            : option.palette.text());
    
        foreach (Chunk chunk, chunks)
        {
            if (chunk.text().isEmpty())
                continue;
        
            QRectF chunkBox(textBox);
            chunkBox.setWidth(option.fontMetrics.width(chunk.text()));
        
            // if the depth is greater than 0, it must be part of a matching node.
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
        
            // move the left border of the box to the right to start drawing
            // right next to the just drawn chunk of text.
            textBox.setLeft(textBox.left() + chunkBox.width());
        }
    }
    catch (std::runtime_error const &e)
    {
        qDebug() << "BracketedColorDelegate::paint: " << e.what();
    }
}