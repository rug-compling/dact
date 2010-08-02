#ifndef DACTQUERYWINDOW_H
#define DACTQUERYWINDOW_H

#include <QCloseEvent>
#include <QFileInfo>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <Qt>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathFilter.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
	class DactQueryWindow;
}

class QListWidgetItem;

class DactQueryWindow : public QWidget {
    Q_OBJECT
public:
    DactQueryWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DactQueryWindow();
    // When a new treebank is loaded into the main window, the corpus is switched and the results will be updated.
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
    void setFilter(QString const &text);
    void setAggregateAttribute(QString const &text);
    void showPercentage(bool show);

private slots:
    void applyValidityColor(QString const &text);
    void filterChanged();
    void showPercentageChanged();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.

private:
    void updateResults();
    void createActions();
    void readSettings();
    void writeSettings();

    QSharedPointer<Ui::DactQueryWindow> d_ui;
    QSharedPointer<XPathFilter> d_xpathFilter;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

#endif // DACTQUERYWINDOW_H
