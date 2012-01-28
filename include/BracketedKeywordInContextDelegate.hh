#ifndef BRACKETEDKEYWORDINCONTEXTDELEGATE_HH
#define BRACKETEDKEYWORDINCONTEXTDELEGATE_HH

#include "BracketedDelegate.hh"

class BracketedKeywordInContextDelegate : public BracketedDelegate
{
    Q_OBJECT

public:
    BracketedKeywordInContextDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

private:
    void loadColorSettings();
    QColor d_keywordForeground;
    QColor d_keywordBackground;
    QColor d_contextForeground;
    QColor d_contextBackground;
};

#endif


