#include <QFile>
#include <QFileInfo>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QHash>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>
#include <QTextStream>
#include <QtDebug>

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

#include <IndexedCorpus/ActCorpusReader.hh>

#include "DactMainWindow.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactMainWindow.h"

using namespace std;

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::DactMainWindow),
    d_xpathValidator(new XPathValidator)
{
    d_ui->setupUi(this);

    d_ui->queryLineEdit->setValidator(&*d_xpathValidator);

    initSentenceTransformer();    
    initTreeTransformer();

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
    QVector<QString> entries = corpusReader.entries(d_corpusPath);

    for (QVector<QString>::const_iterator iter = entries.begin();
         iter != entries.end(); ++iter)
    {
        QFileInfo entryFi(*iter);
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

void DactMainWindow::initSentenceTransformer()
{
    // Read stylesheet.
    QFile xslFile(":/stylesheets/bracketed-sentence.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QTextStream xslStream(&xslFile);
    QString xsl(xslStream.readAll());
    d_sentenceTransformer = QSharedPointer<XSLTransformer>(new XSLTransformer(xsl));
}

void DactMainWindow::initTreeTransformer()
{
	// Read stylesheet.
    QFile xslFile(":/stylesheets/dt2tree.xsl");
	xslFile.open(QIODevice::ReadOnly);
	QTextStream xslStream(&xslFile);
	QString xsl(xslStream.readAll());
	d_treeTransformer = QSharedPointer<XSLTransformer>(new XSLTransformer(xsl));
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

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    QString xml = corpusReader.getData(xmlFilename);

    if (xml.size() == 0) {
        qWarning() << "DactMainWindow::writeSettings: empty XML data!";
        d_ui->sentenceLineEdit->clear();
        return;
    }

    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    QString sentence = d_sentenceTransformer->transform(xml, params).trimmed();

    d_ui->sentenceLineEdit->setText(sentence);
    d_ui->sentenceLineEdit->setCursorPosition(0);
}

void DactMainWindow::showTree(QListWidgetItem *current, QListWidgetItem *)
{
    QString xmlFilename = d_corpusPath + "/" + current->text();

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    QString xml = corpusReader.getData(xmlFilename);

    if (xml.size() == 0) {
        qWarning() << "DactMainWindow::writeSettings: empty XML data!";
        d_ui->treeGraphicsView->setScene(0);
        return;
    }

    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    QString svg = d_treeTransformer->transform(xml, params);
    QByteArray svgData(svg.toUtf8());

    // Render SVG.
    QSvgRenderer *renderer = new QSvgRenderer(svgData);
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
