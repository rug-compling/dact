#ifndef DACTFILTERWINDOW_H
#define DACTFILTERWINDOW_H

#include <QHash>
#include <QWidget>
#include <QSharedPointer>
#include <QString>
#include <QFileInfo>

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
    DactFilterWindow(DactMainWindow *delegate, QSharedPointer<alpinocorpus::CorpusReader> corpusReader, QWidget *parent = 0);
    ~DactFilterWindow();
    void switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader);

public slots:
    void hide();

private slots:
    void applyValidityColor(QString const &text);
    void entrySelected(QListWidgetItem *current, QListWidgetItem *previous);
    void filterChanged();

protected:
    //void changeEvent(QEvent *e);

private:
    void addFiles();
    void createActions();
    void initSentenceTransformer();
    void readSettings();
    void writeSettings();
    QString sentenceForFile(QFileInfo const &file, QString const &query);

    Ui::DactFilterWindow *d_ui;
    DactMainWindow *d_mainWindow;
    QSharedPointer<XSLTransformer> d_sentenceTransformer;
    QSharedPointer<XPathFilter> d_xpathFilter;
    QSharedPointer<XPathValidator> d_xpathValidator;
    QSharedPointer<alpinocorpus::CorpusReader> d_corpusReader;
};

#endif // DACTFILTERWINDOW_H
