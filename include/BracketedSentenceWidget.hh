#ifndef BRACKETEDSENTENCEWIDGET_HH
#define BRACKETEDSENTENCEWIDGET_HH

#include <QColor>
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

public slots:
	void colorChanged();

private:
	void loadSettings();
    QString transformXML(QString const &xml) const;
    void updateText();

    QColor d_highlightColor;
    QString d_parse;
    QFile d_stylesheet;
    XSLTransformer d_transformer;
};

#endif // BRACKETEDSENTENCEWIDGET_HH
