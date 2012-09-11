#ifndef BRACKETEDKEYWORDINCONTEXTDELEGATE_HH
#define BRACKETEDKEYWORDINCONTEXTDELEGATE_HH

#include <vector>

#include "BracketedDelegate.hh"
#include "LexItem.hh"

class BracketedKeywordInContextDelegate : public BracketedDelegate
{
    Q_OBJECT

public:
    BracketedKeywordInContextDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

private:
    void loadColorSettings();
    QString extractFragment(std::vector<LexItem> const &items, size_t first, size_t last) const;
    QColor d_keywordForeground;
    QColor d_keywordBackground;
    QColor d_contextForeground;
    QColor d_contextBackground;
};

#endif


