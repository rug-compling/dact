#include <cmath>
#include <list>
#include <set>

#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>

#include <AlpinoCorpus/LexItem.hh>

#include "BracketedKeywordInContextDelegate.hh"

#include <QtDebug>

BracketedKeywordInContextDelegate::BracketedKeywordInContextDelegate(CorpusReaderPtr corpus)
:
    BracketedDelegate(corpus)
{
    loadColorSettings();
}

QString BracketedKeywordInContextDelegate::extractFragment(
    std::vector<alpinocorpus::LexItem> const &items, size_t first, size_t last) const
{
    QStringList fragment;

    for (size_t i = first; i <= last; ++i)
        fragment.append(QString::fromUtf8(items[i].word.c_str()));

    return fragment.join(" ");
}


void BracketedKeywordInContextDelegate::loadColorSettings()
{
    QSettings settings;

    settings.beginGroup("CompleteSentence");
    d_highlightColor = settings.value("background", QColor(Qt::green)).value<QColor>();
    settings.endGroup();
}

QSize BracketedKeywordInContextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(
        2400, // @TODO yes, this number is completely random. Please please find a way to calculate a sane guess.
        option.fontMetrics.height() * index.sibling(index.row(), 1).data().toInt()
    );
}

void BracketedKeywordInContextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
        
    QBrush brush(option.state & QStyle::State_Selected
                 ? option.palette.highlightedText()
                 : option.palette.text());

    int fontHeight = option.fontMetrics.height();
    
    std::vector<alpinocorpus::LexItem> lexItems(retrieveSentence(index));

    // Strategy: we recurse over the sentence, word by word. If a word belongs
    // to one or more match and that match has not been displayed before, we
    // known that we have to display this in a KWIC. We then scan forward, to
    // extract the words that belong to the KWIC.
    QSet<size_t> matchesSeen;
    QRectF containerBox(option.rect);

    for (size_t i = 0; i < lexItems.size(); ++i)
    {
        for (std::set<size_t>::const_iterator matchIter = lexItems[i].matches.begin();
            matchIter != lexItems[i].matches.end(); ++matchIter)
        {
            // Did we already display this match?
            if (matchesSeen.contains(*matchIter))
                continue;

            matchesSeen.insert(*matchIter);

            // Extract the text left of the match.
            QString left = i > 0 ? extractFragment(lexItems, 0, i - 1) : QString();

            QRectF leftContextBox(containerBox);
            leftContextBox.setWidth(400);
            
            QRectF textBox(containerBox);
            textBox.moveLeft(textBox.left() + leftContextBox.width());
            
            painter->save();

            QRectF usedSpace;

            painter->setPen(brush.color());
            painter->setBrush(brush);
            painter->drawText(leftContextBox, Qt::AlignRight, left + " ", &usedSpace);
            
            bool adoptSpace = false;
            size_t prevDepth = 0;
            for (int j = i; j < lexItems.size(); ++j)
            {
                size_t depth = lexItems[j].matches.size();
            
                QRectF wordBox(textBox);
                QString word = QString::fromUtf8(lexItems[j].word.c_str());

                if (adoptSpace) {
                    word = QString(" ") + word;
                    adoptSpace = false;
                }

                if (j + 1 != lexItems.size()) {
                    if (lexItems[j + 1].matches.size() < depth)
                        adoptSpace = true;
                    else
                        word += " ";
                }

                wordBox.setWidth(option.fontMetrics.width(word));
                wordBox.setHeight(fontHeight);

                if (depth == 0 || lexItems[j].matches.count(*matchIter) == 0)
                {
                    painter->setPen(brush.color());
                    painter->setBrush(brush);
                }
                else
                {
                    d_highlightColor.setAlpha(std::min(85 + 42 * depth,
                        static_cast<size_t>(255)));
                    painter->setPen(QColor(Qt::white));
                    painter->fillRect(wordBox, d_highlightColor);
                }

                painter->drawText(wordBox, Qt::AlignLeft, word);
            
                // move the left border of the box to the right to start drawing
                // right next to the just drawn word.
                textBox.setLeft(textBox.left() + wordBox.width());

                prevDepth = depth;
            }
            
            painter->restore();
            
            // move textBox one line lower.
            containerBox.setTop(containerBox.top() + usedSpace.height());
        }
    }
}
