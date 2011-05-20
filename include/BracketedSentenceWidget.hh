#ifndef BRACKETEDSENTENCEWIDGET_HH
#define BRACKETEDSENTENCEWIDGET_HH

#include <QFile>
#include <QLineEdit>
#include <QList>
#include <QString>

#include "XSLTransformer.hh"

class BracketedSentenceWidget : public QLineEdit
{
    Q_OBJECT
    
public:
    BracketedSentenceWidget(QWidget *parent = 0);
    void setParse(QString const &parse);
    void setQuery(QString const &query);

private:
    QString transformXML(QString const &xml, QString const &query) const;
    void updateText();

    QString d_parse;
    QString d_query;
    QFile d_stylesheet;
    XSLTransformer d_transformer;
};

#endif // BRACKETEDSENTENCEWIDGET_HH