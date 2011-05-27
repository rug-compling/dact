#include "BracketedSentenceWidget.hh"

#include <QDebug>
#include <QPainter>
#include <QSettings>

BracketedSentenceWidget::BracketedSentenceWidget(QWidget *parent)
:
    QLineEdit(parent),
    d_stylesheet(":/stylesheets/bracketed-sentence.xsl"),
    d_transformer(d_stylesheet)
{}

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
        setText(sentence.trimmed());
    }
}

QString BracketedSentenceWidget::transformXML(QString const &xml) const
{
    QHash<QString, QString> params;
    return d_transformer.transform(xml, params);
}
