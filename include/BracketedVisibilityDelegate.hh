#ifndef BRACKETEDVISIBILITYDELEGATE_HH
#define BRACKETEDVISIBILITYDELEGATE_HH

#include "BracketedDelegate.hh"

class BracketedVisibilityDelegate : public BracketedDelegate
{
    Q_OBJECT

public:
    BracketedVisibilityDelegate(CorpusReaderPtr);
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

private:
    QString formatSentence(QModelIndex const &index) const;
};

#endif
