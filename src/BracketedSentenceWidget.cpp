#include <list>

#include <QPainter>
#include <QTextEdit>
#include <QSettings>

#include "BracketedSentenceWidget.hh"
#include "Chunk.hh"

BracketedSentenceWidget::BracketedSentenceWidget(QWidget *parent)
:
    QTextEdit(parent),
    d_stylesheet(":/stylesheets/bracketed-sentence-xml.xsl"),
    d_transformer(d_stylesheet)
{
    QFontMetrics m(document()->defaultFont());
    int ruleHeight = m.lineSpacing();
    setFixedHeight(2 * ruleHeight + (2 * document()->documentMargin()));
    setReadOnly(true);

    loadSettings();
}

void BracketedSentenceWidget::colorChanged()
{
    loadSettings();
    updateText();
}

void BracketedSentenceWidget::loadSettings()
{
    QSettings settings;
    settings.beginGroup("CompleteSentence");

    d_highlightColor =
        settings.value("background", QColor(Qt::green)).value<QColor>();
}

void BracketedSentenceWidget::setParse(QString const &parse)
{
    d_parse = parse;

    updateText();
}

void BracketedSentenceWidget::updateText()
{
    if (!d_parse.isEmpty())
    {
        QString sentence = transformXML(d_parse);
        std::list<Chunk> *chunks = Chunk::parseSentence(d_parse);

        clear();

        foreach (Chunk const &chunk, *chunks)
        {
            if (chunk.depth() > 0) {
                setTextColor(Qt::white);
                d_highlightColor.setAlpha(std::min(85 + 42 * chunk.depth(),
                    static_cast<size_t>(255)));
                setTextBackgroundColor(d_highlightColor);
            }
            else {
                setTextColor(Qt::black);
                setTextBackgroundColor(Qt::white);
            }

            insertPlainText(chunk.text());
        }

        delete chunks;
    }
}

QString BracketedSentenceWidget::transformXML(QString const &xml) const
{
    QHash<QString, QString> params;
    return d_transformer.transform(xml, params);
}
