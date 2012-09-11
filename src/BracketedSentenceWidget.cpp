#include <list>

#include <QPainter>
#include <QTextEdit>
#include <QSettings>

#include "BracketedSentenceWidget.hh"
#include "LexItem.hh"

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
        //std::list<LexItem> *LexItems = LexItem::parseSentence(d_parse);
        //std::list<LexItem> *LexItems = new std::list<LexItem>;
        std::vector<LexItem> *items = LexItem::parseSentence(d_parse);

        clear();

        size_t prevDepth = 0;
        foreach (LexItem const &item, *items)
        {
            size_t depth = item.matches.size();

            if (depth != prevDepth) {
                if (depth == 0)
                {
                    setTextColor(Qt::black);
                    setTextBackgroundColor(Qt::white);
                }
                else
                {
                    setTextColor(Qt::white);
                    d_highlightColor.setAlpha(std::min(85 + 42 * depth,
                        static_cast<size_t>(255)));
                    setTextBackgroundColor(d_highlightColor);
                }

                prevDepth = depth;
            }

            insertPlainText(item.word);
            insertPlainText(" ");
        }

        delete items;
    }
}

QString BracketedSentenceWidget::transformXML(QString const &xml) const
{
    QHash<QString, QString> params;
    return d_transformer.transform(xml, params);
}
