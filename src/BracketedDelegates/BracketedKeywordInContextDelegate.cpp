#include <cmath>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>

#include "BracketedDelegates.hh"

#include <QtDebug>

BracketedKeywordInContextDelegate::BracketedKeywordInContextDelegate(QWidget* parent)
:
BracketedDelegate(parent)
{
    loadColorSettings();
}

void BracketedKeywordInContextDelegate::loadColorSettings()
{
    QSettings settings;
    settings.beginGroup("KeywordsInContext");
    
    d_keywordForeground = settings.value("keywordForeground",
        QColor(Qt::black)).value<QColor>();
    d_keywordBackground = settings.value("keywordBackground",
        QColor(Qt::white)).value<QColor>();
    d_contextForeground = settings.value("contextForeground",
        QColor(Qt::darkGray)).value<QColor>();
    d_contextBackground = settings.value("contextBackground",
        QColor(Qt::white)).value<QColor>();
}

QSize BracketedKeywordInContextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<Chunk> chunks(parseSentence(index.data().toString()));
	QSize lineBox;
	
	int previousDepth = 0;
	
	foreach (Chunk chunk, chunks)
	{
		bool skip = false;
		
		// if this chunk comes after a submatch (a chunk at a deeper level)
		// than we can ignore this chunk because we already printed it at
		// the start. (@TODO we did, did we?)
		if (chunk.depth() <= previousDepth)
			skip = true;
		
		// If it is empty, there is no real use printing it.
		// (It's only empty in cases like the chunk between the first and
		// second opening bracket in "I [[love]] peanutbutter".
		if (chunk.text().isEmpty())
			skip = true;
		
		previousDepth = chunk.depth();
		
		if (skip)
			continue;
		
		previousDepth = chunk.depth();
		
		QSize textBox = option.fontMetrics.size(Qt::TextSingleLine, chunk.text() + chunk.right());
		lineBox.setHeight(lineBox.height() + textBox.height());
		if (textBox.width() > lineBox.width())
			lineBox.setWidth(textBox.width());
	}
	
	lineBox.setWidth(lineBox.width() + 400);
	
    return lineBox;
}

void BracketedKeywordInContextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    
    QList<Chunk> chunks(parseSentence(index.data().toString()));
    QRectF textBox(option.rect);
    
	QBrush brush(option.state & QStyle::State_Selected
				 ? option.palette.highlightedText()
				 : option.palette.text());
	
	int previousDepth = 0;
	
	foreach (Chunk chunk, chunks)
	{
		// Indeed, this is copy-pastework from ~20
		bool skip = false;
		
		if (chunk.depth() <= previousDepth)
			skip = true;
		
		if (chunk.text().isEmpty())
			skip = true;
		
		previousDepth = chunk.depth();
		
		if (skip)
			continue;
		
		/*
		          This is [my sentence] for now
		      and this is [my other sentence] also
		  leftContentBox |---matchBox--|-----| rightContentBox
		|--------------------textBox---------------------------|
		 
	    leftContentBox has a static width,
		matchBox is moved $that_static_width to the right, and gets
		its dimensions once drawn. rightContentBox is then moved
		$that_static_width + $just_drawn_text_width to the right
		and is drawn. Width of this box doen't really matter,
		sizeHint determines the area that is shown.
		textBox is the box that is used to draw a line, and is then
		moved one line downwards to draw the next one. sizeHint is
		quite important here since we are allowed to draw on other
		ListItems.
		*/
		 
		QRectF leftContextBox(textBox);
		leftContextBox.setWidth(400);
		
		QRectF matchBox(textBox);
		matchBox.moveLeft(matchBox.left() + leftContextBox.width());
		
		QRectF rightContextBox(textBox);
		
		painter->save();
		
		painter->setPen(option.state & QStyle::State_Selected
    				 ? option.palette.highlightedText().color()
    				 : d_contextForeground);
		painter->drawText(leftContextBox, Qt::AlignRight, chunk.left());
		
		QRectF usedSpace;
		painter->setPen(option.state & QStyle::State_Selected
    				 ? option.palette.highlightedText().color()
    				 : d_keywordForeground);
		painter->drawText(matchBox, Qt::AlignLeft, chunk.fullText(), &usedSpace);
		
		rightContextBox.moveLeft(rightContextBox.left() + 400 + usedSpace.width());
		
		painter->setPen(option.state & QStyle::State_Selected
    				 ? option.palette.highlightedText().color()
    				 : d_contextForeground);
		painter->drawText(rightContextBox, Qt::AlignLeft, chunk.remainingRight());
		
		painter->restore();
		
		// move textBox one line lower.
		textBox.setTop(textBox.top() + usedSpace.height());
	}
}