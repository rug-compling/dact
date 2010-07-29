#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <QFileInfo>
#include <QCloseEvent>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathFilter.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
	class DactFilterWindow;
}

class DactMainWindow;
class QListWidgetItem;

class DactFilterWindow : public QWidget {
    Q_OBJECT
public:
    DactFilterWindow(DactMainWindow *mainWindow, QSharedPointer<alpinocorpus::CorpusReader> corpusReader, QWidget *parent = 0);
    ~DactFilterWindow();
    // When a new treebank is loaded into the main window, the corpus is switched and the results will be updated.
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);

public slots:
    
private slots:
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void filterChanged();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.

private:
    void addFiles();
    void createActions();
    void initSentenceTransformer();
    void readSettings();
    void writeSettings();
    QString sentenceForFile(QFileInfo const &file, QString const &query);

    QSharedPointer<Ui::DactFilterWindow> d_ui;
    DactMainWindow *d_mainWindow;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XPathFilter> d_xpathFilter;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

#endif // DACTFILTERWINDOW_H
