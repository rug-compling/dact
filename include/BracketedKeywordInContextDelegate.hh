#ifndef BRACKETEDKEYWORDINCONTEXTDELEGATE_HH
#define BRACKETEDKEYWORDINCONTEXTDELEGATE_HH

#include <vector>

#include <AlpinoCorpus/LexItem.hh>

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
    QString extractFragment(std::vector<alpinocorpus::LexItem> const &items, size_t first, size_t last) const;
    mutable QColor d_highlightColor;
};

#endif


