#include "BracketedSentenceWidget.hh"

#include <QDebug>
#include <QPainter>
#include <QSettings>

BracketedSentenceWidget::BracketedSentenceWidget(QWidget *parent)
:
    QFrame(parent),
    d_chunks(0),
    d_stylesheet(":/stylesheets/bracketed-sentence.xsl"),
    d_transformer(d_stylesheet)
{}

void BracketedSentenceWidget::setParse(QString const &parse)
{
    d_parse = parse;
    updateChunks();
}

void BracketedSentenceWidget::setQuery(QString const &query)
{
    d_query = query;
    updateChunks();
}

void BracketedSentenceWidget::updateChunks()
{
    if (d_chunks)
    {
        delete d_chunks;
        d_chunks = 0;
    }
    
    if (!d_parse.isEmpty())
    {
        QString sentence = transformXML(d_parse, d_query);
        d_chunks = parseSentence(sentence.trimmed());
    }
    
    update();
}

void BracketedSentenceWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    
    if (!d_chunks)
        return;
    
    QSettings settings;
    settings.beginGroup("CompleteSentence");
    QColor highlightColor(settings.value("background", QColor(Qt::green)).value<QColor>());
    QBrush brush(palette().text());
    
    QRectF textBox(rect());
    QRectF usedSpace;
    
    QPainter painter(this);
    foreach (Chunk chunk, *d_chunks)
    {
        if (chunk.text().isEmpty())
            continue;
    
        QRectF chunkBox(textBox);
        chunkBox.setWidth(fontMetrics().width(chunk.text()));
    
        // if the depth is greater than 0, it must be part of a matching node.
        if (chunk.depth() > 0)
        {
            highlightColor.setAlpha(std::min(85 + 42 * chunk.depth(), 255));
            painter.setPen(QColor(Qt::white));
            painter.fillRect(chunkBox, highlightColor);
        }
        else
        {
            painter.setPen(brush.color());
            painter.setBrush(brush);
        }
        
        painter.drawText(chunkBox, Qt::AlignLeft, chunk.text());
    
        // move the left border of the box to the right to start drawing
        // right next to the just drawn chunk of text.
        textBox.setLeft(textBox.left() + chunkBox.width());
    }
}

QList<BracketedSentenceWidget::Chunk> *BracketedSentenceWidget::parseSentence(QString const &sentence) const
{
    QList<Chunk> *chunks = new QList<Chunk>;
    QRegExp brackets("\\[|\\]");
    
    int depth = 0, pos = -1, readTill = 0;
    
    while ((pos = sentence.indexOf(brackets, readTill)) != -1)
    {
        // reading one char less on the left and right to omit the bracktes surrounding the match.
        // @TODO use xml for this instead of silly brackets. Maybe even merge this parser with the
        // one in DactTreeScene and keep theses parsed trees in memory to speed things up.
        
        // @TODO this code is quite a mess. This could be a lot more elegant. I hope…
        
        // search backwards for the opening bracket of this match by stepping over the submatches
        int openingBracketPos = readTill;
        int subMatches = 0;
        while (openingBracketPos > 0)
        {
            openingBracketPos = sentence.lastIndexOf(brackets, openingBracketPos - 1);
            
            if (openingBracketPos == -1)
            {
                openingBracketPos = 0;
                break;
            }
            
            if (sentence[openingBracketPos] == ']')
            {
                ++subMatches;
            }
            else if(sentence[openingBracketPos] == '[')
            {
                if (!subMatches)
                    break;
                else
                    --subMatches;
            }
        }
        
        // find the closing bracket for this level of the match
        int closingBracketPos = pos - 1; // -1 because then we first look at pos. If pos is ], no need to look further.
        subMatches = 0;
        while (closingBracketPos < sentence.length())
        {
            closingBracketPos = sentence.indexOf(brackets, closingBracketPos + 1);
            
            if (closingBracketPos == -1)
            {
                closingBracketPos = pos;
                break;
            }
            
            if (sentence[closingBracketPos] == '[')
            {
                ++subMatches;
            }
            else if(sentence[closingBracketPos] == ']')
            {
                if (!subMatches)
                    break;
                else
                    --subMatches;
            }
        }
        
        
        chunks->append(Chunk(depth,
            sentence.left(readTill == 0 ? readTill : readTill - 1),
            sentence.mid(readTill, pos - readTill),
            sentence.mid(openingBracketPos + 1, closingBracketPos - openingBracketPos - 1), // -1 to omit the closing bracket
            sentence.mid(pos + 1),
            sentence.mid(closingBracketPos + 1))); // +1 to omit the closing bracket
            
        readTill = pos + 1;
                
        if (sentence[pos] == '[')
            ++depth;
        
        else if (sentence[pos] == ']')
            --depth;
    }
    
    chunks->append(Chunk(depth, sentence.left(readTill), sentence.mid(readTill), "", "", ""));
    
    return chunks;
}

QString BracketedSentenceWidget::transformXML(QString const &xml, QString const &query) const
{
    QString valStr = query.trimmed().isEmpty()
        ? "'/..'"
        : QString("'%1'").arg(query);

    QHash<QString, QString> params;
    params["expr"] = valStr;
    
    return d_transformer.transform(xml, params);
}

BracketedSentenceWidget::Chunk::Chunk(int depth, QString const &left, QString const &text, QString const &fullText, QString const &right, QString const &remainingRight)
:
    d_depth(depth),
    d_left(left),
    d_text(text),
    d_fullText(fullText),
    d_right(right),
    d_remainingRight(remainingRight)
{}

int BracketedSentenceWidget::Chunk::depth() const
{
    return d_depth;
}

QString const &BracketedSentenceWidget::Chunk::left() const
{
    return d_left;
}

QString const &BracketedSentenceWidget::Chunk::text() const
{
    return d_text;
}

QString const &BracketedSentenceWidget::Chunk::fullText() const
{
    return d_fullText;
}

QString const &BracketedSentenceWidget::Chunk::right() const
{
    return d_right;
}
                      
QString const &BracketedSentenceWidget::Chunk::remainingRight() const
{
    return d_remainingRight;
}

QString BracketedSentenceWidget::Chunk::sentence() const
{
    return d_left + d_text + d_right;
}