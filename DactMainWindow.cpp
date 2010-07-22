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
#include <QPoint>
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
    d_ui->queryLineEdit->setValidator(&*d_xpathValidator);
    readSettings();
    createTransformers();
    d_corpusPath = corpusPath;
    this->setWindowTitle(QString("Dact - %1").arg(corpusPath));
    addFiles();
    createActions();
}

DactMainWindow::~DactMainWindow()
{
    delete d_ui;
}

void DactMainWindow::addFiles()
{
    d_ui->fileListWidget->clear();

    indexedcorpus::ActCorpusReader corpusReader;
    QVector<QString> entries = corpusReader.entries(d_corpusPath);

    for (QVector<QString>::const_iterator iter = entries.begin();
         iter != entries.end(); ++iter)
    {
        QFileInfo entryFi(*iter);
        new QListWidgetItem(entryFi.fileName(), d_ui->fileListWidget);
    }
}

void DactMainWindow::applyQuery()
{
    if (!d_ui->queryLineEdit->hasAcceptableInput())
        return;

    queryChanged();
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
    QObject::connect(d_ui->queryLineEdit, SIGNAL(textChanged(QString const &)), this,
                     SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(returnPressed()), this, SLOT(queryChanged()));
    QObject::connect(d_ui->applyPushButton, SIGNAL(clicked()), this, SLOT(applyQuery()));

    // Actions
    QObject::connect(d_ui->openAction, SIGNAL(triggered(bool)), this, SLOT(openCorpus()));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
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

    QString xmlFilename = d_corpusPath + "/" + current->text();

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    QString xml = corpusReader.getData(xmlFilename);

    if (xml.size() == 0) {
        qWarning() << "DactMainWindow::writeSettings: empty XML data!";
        d_ui->treeGraphicsView->setScene(0);
        return;
    }

    try {
        showTree(xml);
        showSentence(xml);
    } catch(runtime_error &e) {
        QMessageBox::critical(this, QString("Tranformation error"),
            QString("A transformation error occured: %1 Corpus data could be corrupt...").arg(e.what()));
    }
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
    addFiles();
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

void DactMainWindow::showSentence(QString const &xml)
{
    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    QString sentence = d_sentenceTransformer->transform(xml, params).trimmed();

    d_ui->sentenceLineEdit->setText(sentence);
    d_ui->sentenceLineEdit->setCursorPosition(0);
}

void DactMainWindow::showTree(QString const &xml)
{
    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    QString svg;
    svg = d_treeTransformer->transform(xml, params);

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
