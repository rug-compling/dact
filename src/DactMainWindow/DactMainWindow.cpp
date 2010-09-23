#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QHash>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMutexLocker>
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
#include <QUrl>
#include <Qt>
#include <QtConcurrentRun>
#include <QtDebug>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

#include <AboutWindow.hh>
#include <DactMainWindow.h>
#include <BracketedWindow.hh>
#include <DactMacrosModel.h>
#include <DactMacrosWindow.h>
#include <StatisticsWindow.hh>
#include <OpenProgressDialog.hh>
#include <XPathValidator.hh>
#include <XSLTransformer.hh>
#include <ui_DactMainWindow.h>

using namespace alpinocorpus;
using namespace std;

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(QSharedPointer<Ui::DactMainWindow>(new Ui::DactMainWindow)),
    d_aboutWindow(new AboutWindow(this, Qt::Window)),
    d_bracketedWindow(0),
    d_queryWindow(0),
    d_macrosWindow(0),
    d_openProgressDialog(0)
{
    init();
}

void DactMainWindow::init()
{
    // the code shared by both the constructors.
    
    d_macrosModel = QSharedPointer<DactMacrosModel>(new DactMacrosModel());
    
    d_xpathMapper = QSharedPointer<XPathMapper>(new XPathMapper());
    
    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    
    d_ui->setupUi(this);
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    d_ui->queryLineEdit->setValidator(d_xpathValidator.data());
    
    readSettings();
    createTransformers();
    createActions();    
}

DactMainWindow::~DactMainWindow()
{
    stopMapper();
}

void DactMainWindow::aboutDialog()
{
    d_aboutWindow->show();
    d_aboutWindow->raise();
}

void DactMainWindow::addFiles()
{
    QMutexLocker locker(&d_addFilesMutex);

    stopMapper();

    d_ui->fileListWidget->clear();

    QVector<QString> entries;

    try {
        if (d_filter.isEmpty())
            entries = d_corpusReader->entries();
        else {
            d_xpathMapper->start(d_corpusReader.data(), d_macrosModel->expand(d_filter), &d_entryMap);
            return;
        }
    } catch (runtime_error &e) {
        QMessageBox::critical(this, QString("Error reading corpus"),
            QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
        return;
    }

    for (QVector<QString>::const_iterator iter = entries.begin();
    iter != entries.end(); ++iter)
    {
        QListWidgetItem *item = new QListWidgetItem(*iter, d_ui->fileListWidget);
        item->setData(Qt::UserRole, *iter);
    }
}

void DactMainWindow::allEntriesFound()
{
    this->statusBar()->showMessage("Finished filtering entries...", 3000);
}

void DactMainWindow::bracketedEntryActivated()
{
    raise();
    activateWindow();
}

QString DactMainWindow::sentenceForFile(QString const &filename, QString const &query)
{
    // Read XML data.
    if (d_corpusReader.isNull())
        throw runtime_error("DactMainWindow::sentenceForFile CorpusReader not available");

    QString xml = d_corpusReader->read(filename);

    if (xml.size() == 0)
        throw runtime_error("DactMainWindow::sentenceForFile: empty XML data!");

    // Parameters
    QString valStr = query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    return d_sentenceTransformer->transform(xml, params).trimmed();
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

void DactMainWindow::currentBracketedEntryChanged(const QString &entry)
{
    showFile(entry);
}

void DactMainWindow::showFilterWindow()
{
    if (d_bracketedWindow == 0) {
        d_bracketedWindow = new BracketedWindow(d_corpusReader, d_macrosModel, this, Qt::Window);
        QObject::connect(d_bracketedWindow, SIGNAL(entryActivated()), this, SLOT(bracketedEntryActivated()));
        QObject::connect(d_bracketedWindow, SIGNAL(currentEntryChanged(QString)), this, SLOT(currentBracketedEntryChanged(QString)));
    }

    d_bracketedWindow->setFilter(d_filter);
    d_bracketedWindow->show();
}

void DactMainWindow::showStatisticsWindow()
{
    if (d_queryWindow == 0)
        d_queryWindow = new StatisticsWindow(d_corpusReader, d_macrosModel, this, Qt::Window);

    d_queryWindow->setFilter(d_filter);
    d_queryWindow->show();
}

void DactMainWindow::showMacrosWindow()
{
    if(d_macrosWindow == 0) {
        d_macrosWindow = new DactMacrosWindow(d_macrosModel, this, Qt::Window);
    }
    
    d_macrosWindow->show();
}

void DactMainWindow::createActions()
{
    QObject::connect(&d_corpusOpenTimer, SIGNAL(timeout()), this, SLOT(corpusOpenTick()));
    QObject::connect(&d_corpusOpenWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(corpusRead(int)));

    QObject::connect(&d_entryMap, SIGNAL(entryFound(QString)), this,
        SLOT(entryFound(QString)));
    
    QObject::connect(d_xpathMapper.data(), SIGNAL(started(int)), this, SLOT(mapperStarted(int)));
    QObject::connect(d_xpathMapper.data(), SIGNAL(stopped(int, int)), this, SLOT(mapperStopped(int, int)));
    QObject::connect(d_xpathMapper.data(), SIGNAL(progress(int, int)), this, SLOT(mapperProgressed(int,int)));
    
    QObject::connect(d_ui->fileListWidget,
        SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)), this,
        SLOT(entrySelected(QListWidgetItem*,QListWidgetItem*)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QObject::connect(d_ui->queryLineEdit, SIGNAL(returnPressed()), this, SLOT(queryChanged()));

    // Actions
    QObject::connect(d_ui->aboutAction, SIGNAL(triggered(bool)), this, SLOT(aboutDialog()));
    QObject::connect(d_ui->openAction, SIGNAL(triggered(bool)), this, SLOT(openCorpus()));
    QObject::connect(d_ui->openDirectoryAction, SIGNAL(triggered(bool)), this, SLOT(openDirectoryCorpus()));
    QObject::connect(d_ui->fitAction, SIGNAL(triggered(bool)), this, SLOT(fitTree()));
    QObject::connect(d_ui->helpAction, SIGNAL(triggered(bool)), this, SLOT(help()));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->pdfExportAction, SIGNAL(triggered(bool)), this, SLOT(pdfExport()));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
    QObject::connect(d_ui->printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomIn(bool)));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomOut(bool)));
    QObject::connect(d_ui->showFilterWindow, SIGNAL(triggered(bool)), this, SLOT(showFilterWindow()));
    QObject::connect(d_ui->showStatisticsWindow, SIGNAL(triggered(bool)), this, SLOT(showStatisticsWindow()));
    QObject::connect(d_ui->showMacrosWindow, SIGNAL(triggered(bool)), this, SLOT(showMacrosWindow()));  

    QObject::connect(d_ui->showSentencesInFileList, SIGNAL(toggled(bool)), this, SLOT(toggleSentencesInFileList(bool)));
}

void DactMainWindow::corpusOpenTick()
{
    d_openProgressDialog->setProgress(d_corpusReader->entries().size());
}

void DactMainWindow::createTransformers()
{
    initSentenceTransformer();
    initTreeTransformer();
}

void DactMainWindow::entryFound(QString entry)
{
    QListWidgetItem *item = new QListWidgetItem(entry, d_ui->fileListWidget);
    item->setData(Qt::UserRole, entry);
}

void DactMainWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0) {
        d_ui->treeGraphicsView->setScene(0);
        return;
    }
    
    showFile(current->data(Qt::UserRole).toString());
}

void DactMainWindow::help()
{
    QDesktopServices::openUrl(QUrl("http://github.com/danieldk/dact/wiki/Usage"));
}

void DactMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        stopMapper();
    else
      QMainWindow::keyPressEvent(event);
}

void DactMainWindow::showFile(QString const &filename)
{
    // Read XML data.
    if (d_corpusReader.isNull())
        return;

    QString xml = d_corpusReader->read(filename);

    if (xml.size() == 0) {
        qWarning() << "DactMainWindow::writeSettings: empty XML data!";
        d_ui->treeGraphicsView->setScene(0);
        return;
    }

    // Parameters
    QString valStr = d_query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_macrosModel->expand(d_query) + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    try {
        showTree(xml, params);
        showSentence(xml, params);
        
        // I try to find my file back in the file list to keep the list
        // in sync with the treegraph since showFile can be called from
        // the child dialogs.
        QListWidgetItem *item;
        for(int i = 0; i < d_ui->fileListWidget->count(); ++i) {
            item = d_ui->fileListWidget->item(i);
            if(item->data(Qt::UserRole).toString() == filename) {
                d_ui->fileListWidget->setCurrentItem(item);
                break;
            }
        }
        
    } catch(runtime_error &e) {
        QMessageBox::critical(this, QString("Tranformation error"),
            QString("A transformation error occured: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
    }
}

void DactMainWindow::filterChanged()
{
    QMutexLocker locker(&d_filterChangedMutex);

    d_filter = d_ui->filterLineEdit->text().trimmed();
    
    if (d_ui->queryLineEdit->text().trimmed().isEmpty()) {
        d_ui->queryLineEdit->setText(d_filter);
        queryChanged();
    }

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

void DactMainWindow::mapperStarted(int totalEntries)
{
    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void DactMainWindow::mapperStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
}

void DactMainWindow::mapperProgressed(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setValue(processedEntries);
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
    
    readCorpus(corpusPath);
}

void DactMainWindow::openDirectoryCorpus()
{
    QString corpusPath = QFileDialog::getExistingDirectory(this,
        "Open directory corpus");
    if (corpusPath.isNull())
        return;

    readCorpus(corpusPath);
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

void DactMainWindow::readCorpus(QString const &corpusPath)
{ 
    d_corpusPath = corpusPath;
    
    this->setWindowTitle(QString("Dact - %1").arg(d_corpusPath));

    stopMapper();

    if (d_corpusOpenWatcher.isRunning()) {
        d_corpusOpenWatcher.cancel();
        d_corpusOpenWatcher.waitForFinished();
    }

    if (d_openProgressDialog == 0)
        d_openProgressDialog = new OpenProgressDialog(this);

    d_openProgressDialog->open();

    d_corpusReader = QSharedPointer<CorpusReader>(
        CorpusReader::newCorpusReader(d_corpusPath));
    d_corpusOpenTimer.start(250);
    d_corpusOpenFuture = QtConcurrent::run(d_corpusReader.data(), &CorpusReader::open);
    d_corpusOpenWatcher.setFuture(d_corpusOpenFuture);
}

void DactMainWindow::corpusRead(int idx)
{
    d_corpusOpenTimer.stop();
    d_openProgressDialog->accept();

    addFiles();

    if(d_bracketedWindow != 0)
        d_bracketedWindow->switchCorpus(d_corpusReader);

    if(d_queryWindow != 0)
        d_queryWindow->switchCorpus(d_corpusReader);
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

void DactMainWindow::toggleSentencesInFileList(bool show)
{
    d_ui->showSentencesInFileList->setChecked(show);
    filterChanged();
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
    QGraphicsScene *scene = new QGraphicsScene(d_ui->treeGraphicsView);
    QSvgRenderer *renderer = new QSvgRenderer(svgData, scene);
    d_curTreeItem = new QGraphicsSvgItem;
    d_curTreeItem->setSharedRenderer(renderer);
    scene->addItem(d_curTreeItem);
    d_ui->treeGraphicsView->setScene(scene);
    d_ui->treeGraphicsView->fitInView(d_curTreeItem, Qt::KeepAspectRatio);
}

void DactMainWindow::stopMapper()
{
    if (d_xpathMapper->isRunning()) {
        d_xpathMapper->cancel();
        d_xpathMapper->wait();
    }
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
