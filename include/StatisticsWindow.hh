#ifndef DACTQUERYWINDOW_H
#define DACTQUERYWINDOW_H

#include <QCloseEvent>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathValidator.hh"

namespace Ui {
    class StatisticsWindow;
}

class DactMacrosModel;
class DactStatisticsModel;
class StatisticsWindowResultsRow;
class QKeyEvent;

class StatisticsWindow : public QWidget {
    Q_OBJECT
public:
    StatisticsWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~StatisticsWindow();
    // When a new treebank is loaded into the main window, the corpus is switched and the results will be updated.
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
    void setFilter(QString const &text);
    void setAggregateAttribute(QString const &text);
    void showPercentage(bool show);

signals:
    void entryActivated(QString, QString);

private slots:
    void applyValidityColor(QString const &text);
    void generateQuery(QModelIndex const &index);
    void startQuery();
    void progressStarted(int total);
    void progressChanged(int n, int total);
    void progressStopped(int n, int total);
    void showPercentageChanged();
    void updateResultsTotalCount();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
    QString generateQuery(QString const &base, QString const &attribute, QString const &value) const;
    void createActions();
    void readNodeAttributes();
    void readSettings();
    void writeSettings();
    void setModel(DactStatisticsModel *model);
    
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QString d_filter;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<DactStatisticsModel> d_model;
    QSharedPointer<Ui::StatisticsWindow> d_ui;
    QSharedPointer<XPathValidator> d_xpathValidator;
};

#endif // DACTQUERYWINDOW_H
