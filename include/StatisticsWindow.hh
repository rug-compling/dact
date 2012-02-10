#ifndef DACTQUERYWINDOW_H
#define DACTQUERYWINDOW_H

#include <QCloseEvent>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

#include "CorpusWidget.hh"
#include "XPathValidator.hh"

namespace Ui {
    class StatisticsWindow;
}

class QueryModel;
class StatisticsWindowResultsRow;
class QKeyEvent;
class QTextStream;

class StatisticsWindow : public CorpusWidget {
    Q_OBJECT
public:
    StatisticsWindow(QWidget *parent = 0);
    ~StatisticsWindow();
    // When a new treebank is loaded into the main window, the corpus is switched and the results will be updated.
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
    void setFilter(QString const &text);
    void setAggregateAttribute(QString const &text);
    void showPercentage(bool show);
    void selectionAsCSV(QTextStream &output, QString const &separator, bool escape_quotes = false) const;

signals:
    void entryActivated(QString, QString);

public slots:
    void cancelQuery();
    void copy();
    void exportSelection();

private slots:
    void applyValidityColor(QString const &text);
    void attributeChanged(int index);
    void generateQuery(QModelIndex const &index);
    void startQuery();
    void progressStarted(int total);
    void progressChanged(int n, int total);
    void progressStopped(int n, int total);
    void queryFailed(QString error);
    void showPercentageChanged();
    void updateResultsTotalCount();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.

private:
    QString generateQuery(QString const &base, QString const &attribute, QString const &value) const;
    void createActions();
    void readNodeAttributes();
    void readSettings();
    void writeSettings();
    void setModel(QueryModel *model);
    
    QString d_filter;
    QSharedPointer<Ui::StatisticsWindow> d_ui;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<QueryModel> d_model;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

#endif // DACTQUERYWINDOW_H
