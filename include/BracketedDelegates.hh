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

class BracketedDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BracketedDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

protected:
    QList<Chunk> interpretSentence(QString const &sentence) const;
};

class BracketedColorDelegate : public BracketedDelegate
{
    Q_OBJECT
public:
    BracketedColorDelegate(QWidget *parent = 0) : BracketedDelegate(parent) {}
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;

};

class BracketedVisibilityDelegate : public BracketedDelegate
{
    Q_OBJECT
public:
    BracketedVisibilityDelegate(QWidget *parent = 0) : BracketedDelegate(parent) {}
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
private:
    QString formatSentence(QString const &sentence) const;
};

#endif