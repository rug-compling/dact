#ifndef DACTQUERYWINDOW_H
#define DACTQUERYWINDOW_H

#include <QCloseEvent>
#include <QFileInfo>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <Qt>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class StatisticsWindow;
}

class DactMacrosModel;

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

private slots:
    void applyValidityColor(QString const &text);
    void attributeFound(QString value);
    void startQuery();
    void progressStarted(int total);
    void progressChanged(int n, int total);
    void progressStopped(int n, int total);
    void showPercentageChanged();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
    void updateResults();
    void updateResultsPercentages();
    void updateResultsTotalCount();
    void createActions();
    QSharedPointer<StatisticsWindowResultsRow> createResultsRow(QString const &value);
    void readNodeAttributes();
    void startMapper();
    void stopMapper();
    void readSettings();
    void writeSettings();

    AttributeMap *d_attrMap;
    QSharedPointer<Ui::StatisticsWindow> d_ui;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<XPathMapper> d_xpathMapper;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    
    // combination of <attribute value, hits count>
    // @TODO This one could be re-used for exporting functions. It's "finished"
    // when d_xpathMapper.data() emits it's 'stopped' signal.
    QHash<QString,int> d_results;
    QHash<QString,QSharedPointer<StatisticsWindowResultsRow> > d_resultsTable;
    int d_totalHits;
};

// Main purpose of this object is to keep the three cells and their update functions
// together, so I can keep track of them through a hashtable. Because I don't trust
// that QTableWidget thingy with its numerical rows. They change! I tell you! I saw
// them change!
class StatisticsWindowResultsRow : public QObject
{
public:
    StatisticsWindowResultsRow();
    ~StatisticsWindowResultsRow();
    void setText(QString const &);
    void setValue(int);
    void setMax(int);
    int insertIntoTable(QTableWidget *table);
    
private:
    int d_hits;
    QTableWidgetItem *d_labelItem;
    QTableWidgetItem *d_countItem;
    QTableWidgetItem *d_percentageItem;
};

#endif // DACTQUERYWINDOW_H
