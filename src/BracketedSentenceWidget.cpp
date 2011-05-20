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

void BracketedSentenceWidget::setQuery(QString const &query)
{
    d_query = query;
    updateText();
}

void BracketedSentenceWidget::updateText()
{
    if (!d_parse.isEmpty())
    {
        QString sentence = transformXML(d_parse, d_query);
        setText(sentence.trimmed());
    }
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