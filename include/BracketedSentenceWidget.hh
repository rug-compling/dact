#ifndef BRACKETEDSENTENCEWIDGET_HH
#define BRACKETEDSENTENCEWIDGET_HH

#include <QColor>
#include <QFile>
#include <QList>
#include <QTextEdit>
#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XSLTransformer.hh"

class BracketedSentenceWidget : public QTextEdit
{
    Q_OBJECT
    
public:
    BracketedSentenceWidget(QWidget *parent = 0);
    void setEntry(QString const &entry, QString const &query);
    void setCorpusReader(QSharedPointer<alpinocorpus::CorpusReader> reader);

public slots:
	void colorChanged();

private:
	void loadSettings();
    void updateText();

    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QColor d_highlightColor;
    QString d_entry;
    QString d_query;
};

#endif // BRACKETEDSENTENCEWIDGET_HH
