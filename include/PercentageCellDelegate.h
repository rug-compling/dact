#ifndef PERCENTAGECELL_H
#define PERCENTAGECELL_H

#include <QStyledItemDelegate>

class PercentageCellDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // PERCENTAGECELL
