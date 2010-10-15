#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QStyledItemDelegate>

class BracketedColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    BracketedColorDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif