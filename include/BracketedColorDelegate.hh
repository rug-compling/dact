#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QList>
#include <QStyledItemDelegate>

class Chunk
{
    int d_depth;
    QString d_text;
    
public:
    Chunk(int depth, QString text) : d_depth(depth), d_text(text) {}
    inline int depth() const { return d_depth; };
    inline QString const &text() const { return d_text; };
};

class BracketedColorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    BracketedColorDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QList<Chunk> interpretSentence(QString const &sentence) const;
};

#endif