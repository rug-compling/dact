#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedDelegates.hh"

#include <QtDebug>

QSize BracketedKeywordInContextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return option.fontMetrics.size(Qt::TextSingleLine, index.data().toString());
}

void BracketedKeywordInContextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    QList<Chunk> chunks(interpretSentence(index.data().toString()));
    QRectF textBox(option.rect);
    
	QBrush brush(option.state & QStyle::State_Selected
				 ? option.palette.highlightedText()
				 : option.palette.text());
	
	Chunk chunk(chunks[0]);
	int i = 1;
	while (true)
	{
		if (chunks.size() < i)
			break;
		
		if (chunk.depth() > 0 && !chunk.text().isEmpty())
			break;
		
		chunk = chunks[i++];
	}
	
	QRectF leftContextBox(textBox);
	leftContextBox.setWidth(400);
	
	QRectF matchBox(textBox);
	matchBox.setLeft(matchBox.left() + leftContextBox.width());
	
	QRectF rightContextBox(textBox);
	
	painter->save();
	
	painter->setPen(QColor(Qt::darkGray));
	painter->drawText(leftContextBox, Qt::AlignRight, chunk.left());
	
	QRectF usedSpace;
	painter->setPen(QColor(Qt::black));
	painter->drawText(matchBox, Qt::AlignLeft, chunk.text(), &usedSpace);
	
	rightContextBox.setLeft(rightContextBox.left() + 400 + usedSpace.width());
	rightContextBox.setWidth(400000);
	
	painter->setPen(QColor(Qt::darkGray));
	painter->drawText(rightContextBox, Qt::AlignLeft, chunk.right());
	
	painter->restore();
}