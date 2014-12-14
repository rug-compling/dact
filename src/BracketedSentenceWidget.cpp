#include <list>

#include <QApplication>
#include <QPainter>
#include <QTextEdit>
#include <QSettings>

#include <AlpinoCorpus/LexItem.hh>

#include "config.hh"
#include "BracketedSentenceWidget.hh"

BracketedSentenceWidget::BracketedSentenceWidget(QWidget *parent)
:
    QTextEdit(parent)
{
    QFontMetrics m(document()->defaultFont());
    int ruleHeight = m.lineSpacing();
    setFixedHeight(2 * ruleHeight + (2 * document()->documentMargin()));
    setReadOnly(true);

    loadSettings();

    connect(qApp, SIGNAL(colorPreferencesChanged()),
            SLOT(colorChanged()));
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
        settings.value("background", QColor(140, 50, 255)).value<QColor>();
}

void BracketedSentenceWidget::setCorpusReader(QSharedPointer<alpinocorpus::CorpusReader> reader)
{
    d_corpusReader = reader;
}


void BracketedSentenceWidget::setEntry(QString const &entry, QString const &query)
{
    d_entry = entry;
    d_query = query;

    updateText();
}

void BracketedSentenceWidget::updateText()
{
    std::string wordAttr = d_corpusReader->type() == "tueba_tree" ? "form" : "word";

    if (!d_entry.isEmpty() && d_corpusReader)
    {
        std::vector<alpinocorpus::LexItem> items = d_corpusReader->sentence(
            d_entry.toUtf8().constData(), d_query.toUtf8().constData(), wordAttr,
            MISSING_ATTRIBUTE, wordAttr);

        clear();

        // Reset colors.
        setTextColor(Qt::black);
        setTextBackgroundColor(Qt::white);

        bool adoptSpace = false;
        size_t prevDepth = 0;
        for (std::vector<alpinocorpus::LexItem>::const_iterator iter = items.begin();
            iter != items.end(); ++iter)
        {
            size_t depth = iter->matches.size();

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

            if (adoptSpace) {
                insertPlainText(" ");
                adoptSpace = false;
            }

            insertPlainText(QString::fromUtf8(iter->word.c_str()));

            std::vector<alpinocorpus::LexItem>::const_iterator next = iter + 1;
            if (next != items.end()) {
                if (next->matches.size() < depth)
                    adoptSpace = true;
                else
                    insertPlainText(" ");
            }
        }
    }
}
