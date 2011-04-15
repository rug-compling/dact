#include <cmath>
#include <stdexcept>
#include <QPainter>
#include <QFontMetrics>
#include <QPalette>
#include <QSettings>
#include <QHash>

#include "BracketedDelegates.hh"
#include "DactQueryModel.hh"
#include "XSLTransformer.hh"

#include <QtDebug>

BracketedColorDelegate::BracketedColorDelegate(CorpusReaderPtr corpus)
:
    BracketedDelegate(corpus),
    d_stylesheet(":/stylesheets/bracketed-sentence.xsl"),
    d_transformer(d_stylesheet)
{
    loadSettings();
}

void BracketedColorDelegate::loadSettings()
{
    QSettings settings;
    settings.beginGroup("CompleteSentence");
    
    d_backgroundColor = settings.value("background", QColor(Qt::green)).value<QColor>();
}

QSize BracketedColorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return option.fontMetrics.size(Qt::TextSingleLine, index.data().toString());
}

QString BracketedColorDelegate::transformXML(QString const &xml, QString const &query) const
{
    // Parameters
    QString valStr = query.trimmed().isEmpty()
        ? "'/..'"
        : QString("'%1'").arg(query);

    QHash<QString, QString> params;
    params["expr"] = valStr;
    
    return d_transformer.transform(xml, params);
}

QString const &BracketedColorDelegate::transformedCorpusXML(QModelIndex const &index) const
{
    QString filename(index.data(Qt::UserRole).toString());
    
    if (!d_cache.contains(filename))
    {
        DactQueryModel const *model = dynamic_cast<DactQueryModel const *>(index.model());
        
        d_cache.insert(filename, new QString(transformXML(
            reader().read(filename),
            model != 0 ? model->lastQuery() : ""
        )));
    }
        
    return *d_cache[filename];
}

void BracketedColorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    try {
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
    
        QString sentence(transformedCorpusXML(index));
    
        QList<Chunk> chunks(parseSentence(sentence));
        QRectF textBox(option.rect);
        QRectF usedSpace;
        QColor highlightColor(d_backgroundColor);
    
        QBrush brush(option.state & QStyle::State_Selected
            ? option.palette.highlightedText()
            : option.palette.text());
	
        foreach (Chunk chunk, chunks)
        {
            if (chunk.text().isEmpty())
                continue;
        
            QRectF chunkBox(textBox);
            chunkBox.setWidth(option.fontMetrics.width(chunk.text()));
        
    		// if the depth is greater than 0, it must be part of a matching node.
            if (chunk.depth() > 0)
            {
                highlightColor.setAlpha(std::min(85 + 42 * chunk.depth(), 255));
                painter->setPen(QColor(Qt::white));
                painter->fillRect(chunkBox, highlightColor);
            }
            else
            {
                painter->setPen(brush.color());
                painter->setBrush(brush);
            }
        
            painter->drawText(chunkBox, Qt::AlignLeft, chunk.text());
		
    		// move the left border of the box to the right to start drawing
    		// right next to the just drawn chunk of text.
            textBox.setLeft(textBox.left() + chunkBox.width());
        }
    }
    catch (std::runtime_error const &e)
    {
        qDebug() << "BracketedColorDelegate::paint: " << e.what();
    }
}