#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>

#include "BracketedDelegates.hh"

#include <QtDebug>

QSize BracketedKeywordInContextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<Chunk> chunks(interpretSentence(index.data().toString()));
	QSize lineBox;
	
	int previousDepth = 0;
	int lineCount = 0;
	
	foreach (Chunk chunk, chunks)
	{
		bool skip = false;
		
		if (chunk.depth() <= previousDepth)
			skip = true;
		
		if (chunk.text().isEmpty())
			skip = true;
		
		previousDepth = chunk.depth();
		
		if (skip)
			continue;
		
		++lineCount;
		
		previousDepth = chunk.depth();
		
		QSize textBox = option.fontMetrics.size(Qt::TextSingleLine, chunk.text() + chunk.right());
		lineBox.setHeight(lineBox.height() + textBox.height());
		if (textBox.width() > lineBox.width())
			lineBox.setWidth(textBox.width());
	}
	
	lineBox.setWidth(lineBox.width() + 400);
	
    return lineBox;
}

BracketedDelegate::Chunk &BracketedKeywordInContextDelegate::chooseChunk(QList<Chunk> &chunks) const
{
	Chunk *chunk = &chunks[0];
	int i = 0;
	while (i++ < chunks.size())
	{
		if (chunk->depth() > 0 && !chunk->text().isEmpty())
			break;
		
		chunk = &chunks[i];
	}
	
	return *chunk;
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
	
	int previousDepth = 0;
	
	foreach (Chunk chunk, chunks)
	{
		bool skip = false;
		
		if (chunk.depth() <= previousDepth)
			skip = true;
		
		if (chunk.text().isEmpty())
			skip = true;
		
		previousDepth = chunk.depth();
		
		if (skip)
			continue;
		
		QRectF leftContextBox(textBox);
		leftContextBox.setWidth(400);
		
		QRectF matchBox(textBox);
		matchBox.moveLeft(matchBox.left() + leftContextBox.width());
		
		QRectF rightContextBox(textBox);
		
		painter->save();
		
		painter->setPen(QColor(Qt::darkGray));
		painter->drawText(leftContextBox, Qt::AlignRight, chunk.left());
		
		QRectF usedSpace;
		painter->setPen(QColor(Qt::black));
		painter->drawText(matchBox, Qt::AlignLeft, chunk.fullText(), &usedSpace);
		
		rightContextBox.moveLeft(rightContextBox.left() + 400 + usedSpace.width());
		//rightContextBox.setWidth(400000);
		
		painter->setPen(QColor(Qt::darkGray));
		painter->drawText(rightContextBox, Qt::AlignLeft, chunk.remainingRight());
		
		painter->restore();
		
		textBox.setTop(textBox.top() + usedSpace.height());
	}
}