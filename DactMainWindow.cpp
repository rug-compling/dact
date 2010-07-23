#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QHash>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPoint>
#include <QPrintDialog>
#include <QPrinter>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>
#include <QTextStream>
#include <QtDebug>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <IndexedCorpus/CorpusReader.hh>

#include "DactMainWindow.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactMainWindow.h"

using namespace indexedcorpus;
using namespace std;

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::DactMainWindow),
    d_xpathValidator(new XPathValidator)
{
    d_ui->setupUi(this);
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    d_ui->queryLineEdit->setValidator(d_xpathValidator.data());
    readSettings();
    createTransformers();
    createActions();
}

DactMainWindow::DactMainWindow(const QString &corpusPath, QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::DactMainWindow),
    d_xpathValidator(new XPathValidator)
{
    d_ui->setupUi(this);
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    d_ui->queryLineEdit->setValidator(&*d_xpathValidator);
    readSettings();
    createTransformers();
    d_corpusPath = corpusPath;

    this->setWindowTitle(QString("Dact - %1").arg(corpusPath));
    createActions();

    d_corpusReader = QSharedPointer<CorpusReader>(
        CorpusReader::newCorpusReader(d_corpusPath));

    addFiles();
}

DactMainWindow::~DactMainWindow()
{
    delete d_ui;
}

void DactMainWindow::addFiles()
{
    d_ui->fileListWidget->clear();

    QVector<QString> entries;
    if (d_xpathFilter.isNull())
        entries = d_corpusReader->entries();
    else
        entries = d_xpathFilter->entries(d_corpusReader.data());

    for (QVector<QString>::const_iterator iter = entries.begin();
         iter != entries.end(); ++iter)
    {
        QFileInfo entryFi(*iter);
        new QListWidgetItem(entryFi.fileName(), d_ui->fileListWidget);
    }
}

void DactMainWindow::applyValidityColor(QString const &)
{
    // Hmpf, unfortunately we get text, rather than a sender. Attempt
    // to determine the sender ourselvers.
    QObject *sender = this->sender();

    if (!sender)
        return;

    if (!sender->inherits("QLineEdit"))
        return;

    QLineEdit *widget = reinterpret_cast<QLineEdit *>(sender);

    if (widget->hasAcceptableInput())
        widget->setStyleSheet("");
    else
        widget->setStyleSheet("background-color: salmon");
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

void DactMainWindow::createActions()
{
    QObject::connect(d_ui->fileListWidget,
        SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
        this,
        SLOT(entrySelected(QListWidgetItem*,QListWidgetItem*)));

    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(returnPressed()), this, SLOT(queryChanged()));

    // Actions
    QObject::connect(d_ui->openAction, SIGNAL(triggered(bool)), this, SLOT(openCorpus()));
    QObject::connect(d_ui->fitAction, SIGNAL(triggered(bool)), this, SLOT(fitTree()));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->pdfExportAction, SIGNAL(triggered(bool)), this, SLOT(pdfExport()));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
    QObject::connect(d_ui->printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomIn(bool)));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomOut(bool)));
}

void DactMainWindow::createTransformers()
{
    initSentenceTransformer();
    initTreeTransformer();
}

void DactMainWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0) {
        d_ui->treeGraphicsView->setScene(0);
        return;
    }

    // Read XML data.
    if (d_corpusReader.isNull())
        return;

    QString xml = d_corpusReader->read(current->text());

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

    try {
        showTree(xml, params);
        showSentence(xml, params);
    } catch(runtime_error &e) {
        QMessageBox::critical(this, QString("Tranformation error"),
            QString("A transformation error occured: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
    }
}

void DactMainWindow::filterChanged()
{
    QString filter = d_ui->filterLineEdit->text();
    if (filter.trimmed().isEmpty())
        d_xpathFilter.clear();
    else
        d_xpathFilter = QSharedPointer<XPathFilter>(new XPathFilter(filter));

    if (!d_corpusReader.isNull())
        addFiles();
}


void DactMainWindow::fitTree()
{
    if (d_curTreeItem)
        d_ui->treeGraphicsView->fitInView(d_curTreeItem, Qt::KeepAspectRatio);
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

void DactMainWindow::openCorpus()
{
    QString corpusPath = QFileDialog::getOpenFileName(this, "Open corpus", QString(),
        "*.data.dz");
    if (corpusPath.isNull())
        return;

    corpusPath.chop(8);
    d_corpusPath = corpusPath;
    this->setWindowTitle(QString("Dact - %1").arg(corpusPath));
    d_corpusReader = QSharedPointer<CorpusReader>(
        CorpusReader::newCorpusReader(d_corpusPath));

    addFiles();
}

void DactMainWindow::pdfExport()
{
    QString pdfFilename = QFileDialog::getSaveFileName(this, "Export to PDF", QString(), "*.pdf");
    if (pdfFilename.isNull())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(pdfFilename);

    QPainter painter(&printer);

    // If you are asking for an empty PDF, you will get it ;).
    if (d_ui->treeGraphicsView->scene())
        d_ui->treeGraphicsView->scene()->render(&painter);

    painter.end();
}

void DactMainWindow::previousEntry(bool)
{
    int prevRow = d_ui->fileListWidget->currentRow() - 1;
    if (prevRow >= 0)
        d_ui->fileListWidget->setCurrentRow(prevRow);
}

void DactMainWindow::print()
{
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec()) {
        QPainter painter(&printer);
        d_ui->treeGraphicsView->scene()->render(&painter);
        painter.end();
    }
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

void DactMainWindow::showSentence(QString const &xml, QHash<QString, QString> const &params)
{
    QString sentence = d_sentenceTransformer->transform(xml, params).trimmed();

    d_ui->sentenceLineEdit->setText(sentence);
    d_ui->sentenceLineEdit->setCursorPosition(0);
}

void DactMainWindow::showTree(QString const &xml, QHash<QString, QString> const &params)
{
    QString svg;
    svg = d_treeTransformer->transform(xml, params);

    QByteArray svgData(svg.toUtf8());

    // Render SVG.
    QSvgRenderer *renderer = new QSvgRenderer(svgData);
    QGraphicsScene *scene = new QGraphicsScene(d_ui->treeGraphicsView);
    d_curTreeItem = new QGraphicsSvgItem;
    d_curTreeItem->setSharedRenderer(renderer);
    scene->addItem(d_curTreeItem);
    d_ui->treeGraphicsView->setScene(scene);
    d_ui->treeGraphicsView->fitInView(d_curTreeItem, Qt::KeepAspectRatio);
}

void DactMainWindow::queryChanged()
{
    d_query = d_ui->queryLineEdit->text();
    if (d_ui->fileListWidget->currentItem() != 0)
        entrySelected(d_ui->fileListWidget->currentItem(), 0);
}

void DactMainWindow::treeZoomIn(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_IN_FACTOR, ZOOM_IN_FACTOR);
}

void DactMainWindow::treeZoomOut(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_OUT_FACTOR, ZOOM_OUT_FACTOR);
}
