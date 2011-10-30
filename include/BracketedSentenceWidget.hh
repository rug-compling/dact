#ifndef BRACKETEDSENTENCEWIDGET_HH
#define BRACKETEDSENTENCEWIDGET_HH

#include <QFile>
#include <QList>
#include <QTextEdit>
#include <QString>

#include "XSLTransformer.hh"

class BracketedSentenceWidget : public QTextEdit
{
    Q_OBJECT
    
public:
    BracketedSentenceWidget(QWidget *parent = 0);
    void setParse(QString const &parse);

private:
    QString transformXML(QString const &xml) const;
    void updateText();

    QString d_parse;
    QFile d_stylesheet;
    XSLTransformer d_transformer;
};

#endif // BRACKETEDSENTENCEWIDGET_HH
