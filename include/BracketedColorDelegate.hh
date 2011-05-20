#ifndef BRACKETEDCOLORDELEGATE_HH
#define BRACKETEDCOLORDELEGATE_HH

#include "BracketedDelegate.hh"

class BracketedColorDelegate : public BracketedDelegate
{
    Q_OBJECT

public:
    BracketedColorDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

private:
    void loadSettings();    
    QColor d_backgroundColor;
};

#endif