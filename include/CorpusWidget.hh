#ifndef CORPUSWIDGET_HH
#define CORPUSWIDGET_HH

#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include <AlpinoCorpus/CorpusReader.hh>

class CorpusWidget : public QWidget
{
    Q_OBJECT
public:
    CorpusWidget(QWidget *parent = 0) : QWidget(parent) {}
    virtual ~CorpusWidget() {}
    virtual bool saveEnabled() const = 0;
    virtual void saveAs() = 0;
signals:
    void saveStateChanged();
public slots:
    virtual void setFilter(QString const &filter, QString const &raw_filter) = 0;
    virtual void cancelQuery() = 0;
    virtual void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpus) = 0;
};

#endif // CORPUSWIDGET_HH
