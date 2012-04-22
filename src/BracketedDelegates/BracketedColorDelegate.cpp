#include <cmath>
#include <list>
#include <stdexcept>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>
#include <QHash>

#include "BracketedColorDelegate.hh"

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
    // Calculating the atual width is far to slow :(, we need to retrieve
    // the entry, retrieve its sentence, and do an expensive
    // fontMetrics.size() call.

    //QString filename(index.data(Qt::UserRole).toString());
    //QString sent(sentence(index));
    //if (d_sizes.contains(sent.length()))
    //  return d_sizes.value(sent.length());
    //QSize sentSize = option.fontMetrics.size(Qt::TextSingleLine, sentence(index));
    //d_sizes.insert(sent.length(), sentSize);
    
    return QSize(
        2400, // @TODO yes, this number is completely random. Please please find a way to calculate a sane guess.
        option.fontMetrics.height()
    );
}

void BracketedColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    try {
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
    
        std::list<Chunk> chunks(parseChunks(index));
        QRectF textBox(option.rect);
        QRectF usedSpace;
        QColor highlightColor(d_backgroundColor);
    
        QBrush brush(option.state & QStyle::State_Selected
            ? option.palette.highlightedText()
            : option.palette.text());
    
        foreach (Chunk const &chunk, chunks)
        {
            if (chunk.text().isEmpty())
                continue;
        
            QRectF chunkBox(textBox);
            chunkBox.setWidth(option.fontMetrics.width(chunk.text()));
        
            // if the depth is greater than 0, it must be part of a matching node.
            if (chunk.depth() > 0)
            {
                highlightColor.setAlpha(std::min(85 + 42 * chunk.depth(),
                    static_cast<size_t>(255)));
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
