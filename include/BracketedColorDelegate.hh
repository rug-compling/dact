#ifndef BRACKETEDCOLORDELEGATE_HH
#define BRACKETEDCOLORDELEGATE_HH

#include <QSize>

#include "BracketedDelegate.hh"


class BracketedColorDelegate : public BracketedDelegate
{
    Q_OBJECT

public:
    BracketedColorDelegate(CorpusReaderPtr);
    virtual ~BracketedColorDelegate() {}
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

private:
    void loadSettings();    
    QColor d_backgroundColor;
};

#endif
