#ifndef STARDELEGATE_H
#define STARDELEGATE_H

#include <QList>
#include <QStyledItemDelegate>

class BracketedDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BracketedDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

protected:
	class Chunk
	{
		int d_depth;
		QString d_left;
		QString d_text;
		QString d_right;
		
	public:
		Chunk(int depth, QString const &left, QString const &text, QString const &right);
		int depth() const;
		QString const &left() const;
		QString const &text() const;
		QString const &right() const;
	};
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