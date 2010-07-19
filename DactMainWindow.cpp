#include <QFile>
#include <QFileInfo>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

extern "C" {
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

#include <IndexedCorpus/ActCorpusReader.hh>

#include "DactMainWindow.h"
#include "ui_DactMainWindow.h"

using namespace std;

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::DactMainWindow)
{
    d_ui->setupUi(this);

    readSettings();

    if (qApp->arguments().size() == 2) {
        d_corpusPath = qApp->arguments().at(1);
        addFiles();
    }

    setWindowTitle(QString("Dact - ") + d_corpusPath);

    QObject::connect(d_ui->fileListWidget,
                     SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
                     this,
                     SLOT(showTree(QListWidgetItem *, QListWidgetItem *)));
    QObject::connect(d_ui->fileListWidget,
                     SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
                     this,
                     SLOT(showSentence(QListWidgetItem *, QListWidgetItem *)));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomIn(bool)));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomOut(bool)));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(returnPressed()), this, SLOT(queryChanged()));
    QObject::connect(d_ui->applyPushButton, SIGNAL(clicked()), this, SLOT(queryChanged()));
}

DactMainWindow::~DactMainWindow()
{
    delete d_ui;
}

void DactMainWindow::addFiles()
{
    indexedcorpus::ActCorpusReader corpusReader;
    QByteArray corpusPathData(d_corpusPath.toUtf8());
    vector<string> entries = corpusReader.entries(corpusPathData.constData());

    for (vector<string>::const_iterator iter = entries.begin();
         iter !=entries.end(); ++iter)
    {
        QFileInfo entryFi(iter->c_str());
        new QListWidgetItem(entryFi.fileName(), d_ui->fileListWidget);
    }
}

void DactMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DactMainWindow::close()
{
    writeSettings();
    QMainWindow::close();
}

void DactMainWindow::nextEntry(bool)
{
    int nextRow = d_ui->fileListWidget->currentRow() + 1;
    if (nextRow < d_ui->fileListWidget->count())
        d_ui->fileListWidget->setCurrentRow(nextRow);
}

void DactMainWindow::previousEntry(bool)
{
    int prevRow = d_ui->fileListWidget->currentRow() - 1;
    if (prevRow >= 0)
        d_ui->fileListWidget->setCurrentRow(prevRow);
}

void DactMainWindow::readSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry.
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(800, 500)).toSize();
    resize(size);

    // Splitter.
    d_ui->splitter->restoreState(
            settings.value("splitterSizes").toByteArray());

    // Move.
    move(pos);
}

void DactMainWindow::writeSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    // Splitter
    settings.setValue("splitterSizes", d_ui->splitter->saveState());
}

void DactMainWindow::showSentence(QListWidgetItem *current, QListWidgetItem *)
{
    QString xmlFilename = d_corpusPath + "/" + current->text();

    // Read stylesheet.
    QFile xslFile(":/stylesheets/bracketed-sentence.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QByteArray xslData(xslFile.readAll());
    xmlDocPtr xslDoc = xmlReadMemory(xslData.constData(), xslData.size(), 0, 0, 0);
    xsltStylesheetPtr xsl = xsltParseStylesheetDoc(xslDoc);

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    QByteArray xmlFilenameData(xmlFilename.toUtf8());
    vector<unsigned char> xmlData = corpusReader.getData(xmlFilenameData.constData());
    xmlDocPtr xmlDoc = xmlReadMemory(reinterpret_cast<char const *>(&xmlData[0]),
                                     xmlData.size(), 0, 0, 0);

    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QByteArray valData(valStr.toUtf8());
    char const *params[] = {
        "expr",
        valData.constData(),
        0
    };

    // Transform...
    xmlDocPtr res = xsltApplyStylesheet(xsl, xmlDoc, params);
    xmlChar *output;
    int outputLen = -1;
    xsltSaveResultToString(&output, &outputLen, res, xsl);
    QByteArray sentenceData(reinterpret_cast<char const *>(output), outputLen);

    // Deallocate memory used for libxml2/libxslt.
    free(output);
    xsltFreeStylesheet(xsl);
    xmlFreeDoc(xmlDoc);

    QString sentence(sentenceData);
    sentence = sentence.trimmed();
    d_ui->sentenceLineEdit->setText(sentence);
    d_ui->sentenceLineEdit->setCursorPosition(0);
}

void DactMainWindow::showTree(QListWidgetItem *current, QListWidgetItem *)
{
    QString xmlFilename = d_corpusPath + "/" + current->text();

    // Read stylesheet.
    QFile xslFile(":/stylesheets/dt2tree.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QByteArray xslData(xslFile.readAll());
    xmlDocPtr xslDoc = xmlReadMemory(xslData.constData(), xslData.size(), 0, 0, 0);
    xsltStylesheetPtr xsl = xsltParseStylesheetDoc(xslDoc);

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    QByteArray xmlFilenameData(xmlFilename.toUtf8());
    vector<unsigned char> xmlData = corpusReader.getData(xmlFilenameData.constData());
    xmlDocPtr xmlDoc = xmlReadMemory(reinterpret_cast<char const *>(&xmlData[0]),
                                     xmlData.size(), 0, 0, 0);

    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QByteArray valData(valStr.toUtf8());
    char const *params[] = {
        "expr",
        valData.constData(),
        0
    };

    // Transform...
    xmlDocPtr res = xsltApplyStylesheet(xsl, xmlDoc, params);
    xmlChar *output;
    int outputLen = -1;
    xsltSaveResultToString(&output, &outputLen, res, xsl);
    QByteArray svg(reinterpret_cast<char const *>(output), outputLen);

    // Deallocate memory used for libxml2/libxslt.
    free(output);
    xsltFreeStylesheet(xsl);
    xmlFreeDoc(xmlDoc);

    // Render SVG.
    QSvgRenderer *renderer = new QSvgRenderer(svg);
    QGraphicsScene *scene = new QGraphicsScene(d_ui->treeGraphicsView);
    QGraphicsSvgItem *item = new QGraphicsSvgItem;
    item->setSharedRenderer(renderer);
    scene->addItem(item);
    d_ui->treeGraphicsView->setScene(scene);
    d_ui->treeGraphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void DactMainWindow::queryChanged()
{
    d_query = d_ui->queryLineEdit->text();
    if (d_ui->fileListWidget->currentItem() != 0)
    {
        showSentence(d_ui->fileListWidget->currentItem(), 0);
        showTree(d_ui->fileListWidget->currentItem(), 0);
    }
}

void DactMainWindow::treeZoomIn(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_IN_FACTOR, ZOOM_IN_FACTOR);
}

void DactMainWindow::treeZoomOut(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_OUT_FACTOR, ZOOM_OUT_FACTOR);
}
