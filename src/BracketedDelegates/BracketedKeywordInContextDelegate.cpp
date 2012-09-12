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
    settings.beginGroup("KeywordsInContext");
    
    d_keywordForeground = settings.value("keywordForeground",
        QColor(Qt::black)).value<QColor>();
    d_keywordBackground = settings.value("keywordBackground",
        QColor(Qt::white)).value<QColor>();
    d_contextForeground = settings.value("contextForeground",
        QColor(Qt::darkGray)).value<QColor>();
    d_contextBackground = settings.value("contextBackground",
        QColor(Qt::white)).value<QColor>();
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
    
    std::vector<alpinocorpus::LexItem> lexItems(retrieveSentence(index));

    QRectF textBox(option.rect);
    
    QBrush brush(option.state & QStyle::State_Selected
                 ? option.palette.highlightedText()
                 : option.palette.text());
    
    // Strategy: we recurse over the sentence, word by word. If a word belongs
    // to one or more match and that match has not been displayed before, we
    // known that we have to display this in a KWIC. We then scan forward, to
    // extract the words that belong to the KWIC.
    QSet<size_t> matchesSeen;
    for (size_t i = 0; i < lexItems.size(); ++i)
    {
        for (std::set<size_t>::const_iterator matchIter = lexItems[i].matches.begin();
            matchIter != lexItems[i].matches.end(); ++matchIter)
        {
            // Did we already display this match?
            if (matchesSeen.contains(*matchIter))
                continue;

            matchesSeen.insert(*matchIter);

            // Find the last word that belongs to this match.
            size_t lastMatchedWord = i;
            for (size_t j = lastMatchedWord + 1; j < lexItems.size(); ++j)
                if (lexItems[j].matches.count(*matchIter) != 0)
                    lastMatchedWord = j;

            // We now know the first and last words, let's build the fields.
            QString left = i > 0 ? extractFragment(lexItems, 0, i - 1) : QString();
            QString match = extractFragment(lexItems, i, lastMatchedWord);
            QString right = lastMatchedWord < lexItems.size() - 1 ?
                extractFragment(lexItems, lastMatchedWord + 1, lexItems.size() - 1) :
                QString();

            QRectF leftContextBox(textBox);
            leftContextBox.setWidth(400);
            
            QRectF matchBox(textBox);
            matchBox.moveLeft(matchBox.left() + leftContextBox.width());
            
            QRectF rightContextBox(textBox);
            
            painter->save();

            painter->setPen(option.state & QStyle::State_Selected
                         ? option.palette.highlightedText().color()
                         : d_contextForeground);
            painter->drawText(leftContextBox, Qt::AlignRight, left + " ");
            
            QRectF usedSpace;
            painter->setPen(option.state & QStyle::State_Selected
                         ? option.palette.highlightedText().color()
                         : d_keywordForeground);
            painter->drawText(matchBox, Qt::AlignLeft, match, &usedSpace);
            
            rightContextBox.moveLeft(rightContextBox.left() + 400 + usedSpace.width());
            
            painter->setPen(option.state & QStyle::State_Selected
                         ? option.palette.highlightedText().color()
                         : d_contextForeground);
            painter->drawText(rightContextBox, Qt::AlignLeft, QString(" ") + right);
            
            painter->restore();
            
            // move textBox one line lower.
            textBox.setTop(textBox.top() + usedSpace.height());
        }
    }
}
