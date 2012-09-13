#include <cmath>
#include <list>
#include <stdexcept>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>
#include <QHash>

#include <AlpinoCorpus/LexItem.hh>

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
    
        QRectF textBox(option.rect);
        QRectF usedSpace;
        QColor highlightColor(d_backgroundColor);
    
        QBrush brush(option.state & QStyle::State_Selected
            ? option.palette.highlightedText()
            : option.palette.text());
   
        std::vector<alpinocorpus::LexItem> const &items(retrieveSentence(index));
        int prevDepth = -1;
        bool adoptSpace = false;
         for (std::vector<alpinocorpus::LexItem>::const_iterator iter = items.begin();
            iter != items.end(); ++iter)
        {
           size_t depth = iter->matches.size();
        
            QRectF wordBox(textBox);
            QString word = QString::fromUtf8(iter->word.c_str());

            if (adoptSpace) {
                word = QString(" ") + word;
                adoptSpace = false;
            }

            std::vector<alpinocorpus::LexItem>::const_iterator next = iter + 1;
            if (next != items.end()) {
                if (next->matches.size() < depth)
                    adoptSpace = true;
                else
                    word += " ";
            }

            wordBox.setWidth(option.fontMetrics.width(word));
        
            if (depth != prevDepth) {
                if (depth == 0)
                {
                    painter->setPen(brush.color());
                    painter->setBrush(brush);
                }
                else
                {
                    highlightColor.setAlpha(std::min(85 + 42 * depth,
                        static_cast<size_t>(255)));
                    painter->setPen(QColor(Qt::white));
                }
            }

            if (depth != 0)
                painter->fillRect(wordBox, highlightColor);

            painter->drawText(wordBox, Qt::AlignLeft, word);
        
            // move the left border of the box to the right to start drawing
            // right next to the just drawn alpinocorpus::LexItem of text.
            textBox.setLeft(textBox.left() + wordBox.width());

            prevDepth = depth;
        }
    }
    catch (std::runtime_error const &e)
    {
        qDebug() << "BracketedColorDelegate::paint: " << e.what();
    }
}
