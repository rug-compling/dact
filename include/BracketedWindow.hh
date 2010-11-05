#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QCloseEvent>
#include <QFileInfo>
#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <Qt>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathMapper.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"

namespace Ui {
    class BracketedWindow;
}

class QKeyEvent;

class DactMacrosModel;
class QListWidgetItem;
class EntryMapAndTransform;

class BracketedWindow : public QWidget {
    Q_OBJECT
public:
    BracketedWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~BracketedWindow();
    // When a new treebank is loaded into the main window, the corpus is switched and the results will be updated.
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);
    void setFilter(QString const &text);
    inline QString const &filter() const { return d_filter; };

signals:
    void currentEntryChanged(QString const &entry);
    void entryActivated();
    
private slots:
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void entryActivated(QListWidgetItem *subject);
    void filterChanged();
    void sentenceFound(QString file, QString sentence);
    void mapperStarted(int);
    void mapperStopped(int, int);
    void mapperProgressed(int, int);

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
    void updateResults();
    void createActions();
    void initSentenceTransformer();
    void readSettings();
    void writeSettings();
    void stopMapper();

    EntryMapAndTransform *d_entryMap;
    QString d_filter;
    QSharedPointer<Ui::BracketedWindow> d_ui;
    QSharedPointer<DactMacrosModel> d_macrosModel;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XPathMapper> d_xpathMapper;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

class EntryMapAndTransform : public EntryMap
{
    Q_OBJECT
public:
    EntryMapAndTransform(QSharedPointer<alpinocorpus::CorpusReader> reader, QSharedPointer<XSLTransformer> transformer, QString const &query);
    void operator()(QString const &entry, xmlXPathObjectPtr xpathObj);
private:
    QString transform(QString const &file);

signals:
    void sentenceFound(QString file, QString sentence);
    
private:
    QString d_query;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
    QSharedPointer<XSLTransformer> d_xslTransformer;
};

#endif // DACTFILTERWINDOW_H
