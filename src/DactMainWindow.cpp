#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QHash>
#include <QLineEdit>
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
#include <QSvgRenderer>
#include <QTextStream>
#include <QUrl>
#include <Qt>
#include <QtConcurrentRun>
#include <QtDebug>

#include <cstdlib>
#include <stdexcept>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <AboutWindow.hh>
#include <DactMainWindow.h>
#include <BracketedWindow.hh>
#include <DactMacrosModel.h>
#include <DactMacrosWindow.h>
#include <DactQueryHistory.hh>
#include <OpenProgressDialog.hh>
#include <ExportProgressDialog.hh>
#include <PreferencesWindow.hh>
#include <StatisticsWindow.hh>
#include <DactTreeScene.h>
#include <XPathValidator.hh>
#include <XSLTransformer.hh>
#include <ui_DactMainWindow.h>

namespace ac = alpinocorpus;

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(QSharedPointer<Ui::DactMainWindow>(new Ui::DactMainWindow)),
    d_aboutWindow(new AboutWindow(this, Qt::Window)),
    d_bracketedWindow(0),
    d_statisticsWindow(0),
    d_macrosWindow(0),
    d_openProgressDialog(new OpenProgressDialog(this)),
    d_exportProgressDialog(0),
    d_preferencesWindow(0),
    d_treeScene(0),
    d_queryHistory(0)
{
    d_ui->setupUi(this);
    
    d_macrosModel = QSharedPointer<DactMacrosModel>(new DactMacrosModel());
    
    d_xpathMapper = QSharedPointer<XPathMapper>(new XPathMapper());
    
    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    d_ui->highlightLineEdit->setValidator(d_xpathValidator.data());
    /*
    d_queryHistory = QSharedPointer<DactQueryHistory>(new DactQueryHistory());
    d_ui->filterLineEdit->setCompleter(d_queryHistory->completer());
    d_ui->highlightLineEdit->setCompleter(d_queryHistory->completer());
    */
    readSettings();
    
    initSentenceTransformer();
    
    initTreeTransformer();
    
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

    d_addFilesCancelled = false;
    
    stopMapper();

    d_ui->fileListWidget->clear();

    try {
        if (!d_filter.isEmpty()) {
            d_xpathMapper->start(d_corpusReader.data(), d_macrosModel->expand(d_filter), &d_entryMap);
            return;
        }

        for (ac::CorpusReader::EntryIterator i(d_corpusReader->begin()),
                                             end(d_corpusReader->end());
            !d_addFilesCancelled && i != end; ++i) {
            QListWidgetItem *item = new QListWidgetItem(*i,
                                                        d_ui->fileListWidget);
            item->setData(Qt::UserRole, *i);
        }
    } catch (std::runtime_error const &e) {
        QMessageBox::critical(this, QString("Error reading corpus"),
                              QString("Could not read corpus: %1.")
                                .arg(e.what()));
        return;
    }
}

void DactMainWindow::bracketedEntryActivated()
{
    setHighlight(d_bracketedWindow->filter());
    raise();
    activateWindow();
}

void DactMainWindow::currentBracketedEntryChanged(const QString &entry)
{
    showFile(entry);
    focusFitTree();
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

void DactMainWindow::showFilterWindow()
{
    if (d_bracketedWindow == 0)
    {
        d_bracketedWindow = new BracketedWindow(d_corpusReader, d_macrosModel, this, Qt::Window);
        QObject::connect(d_bracketedWindow, SIGNAL(entryActivated()), this, SLOT(bracketedEntryActivated()));
        QObject::connect(d_bracketedWindow, SIGNAL(currentEntryChanged(QString)), this, SLOT(currentBracketedEntryChanged(QString)));
    }

    d_bracketedWindow->setFilter(d_filter);
    d_bracketedWindow->show();
    d_bracketedWindow->raise();
}

void DactMainWindow::showStatisticsWindow()
{
    if (d_statisticsWindow == 0)
    {
        d_statisticsWindow = new StatisticsWindow(d_corpusReader, d_macrosModel, this, Qt::Window);
        QObject::connect(d_statisticsWindow, SIGNAL(entryActivated(QString, QString)),
            this, SLOT(statisticsEntryActivated(QString const &, QString const &)));
    }

    d_statisticsWindow->setFilter(d_filter);
    d_statisticsWindow->show();
    d_statisticsWindow->raise();
}

void DactMainWindow::statisticsEntryActivated(QString const &value, QString const &query)
{
    d_ui->filterLineEdit->setText(query);
    filterChanged();
}

void DactMainWindow::showMacrosWindow()
{
    if (d_macrosWindow == 0)
        d_macrosWindow = new DactMacrosWindow(d_macrosModel, this, Qt::Window);
    
    d_macrosWindow->show();
    d_macrosWindow->raise();
}

void DactMainWindow::createActions()
{
    QObject::connect(&d_corpusOpenWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(corpusRead(int)));
    QObject::connect(d_openProgressDialog, SIGNAL(rejected()), this, SLOT(cancelReadCorpus()));
    
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
    QObject::connect(d_ui->highlightLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QObject::connect(d_ui->highlightLineEdit, SIGNAL(returnPressed()), this, SLOT(highlightChanged()));

    // Actions
    QObject::connect(d_ui->aboutAction, SIGNAL(triggered(bool)), this, SLOT(aboutDialog()));
    QObject::connect(d_ui->openAction, SIGNAL(triggered(bool)), this, SLOT(openCorpus()));
    QObject::connect(d_ui->openDirectoryAction, SIGNAL(triggered(bool)), this, SLOT(openDirectoryCorpus()));
    QObject::connect(d_ui->saveCorpus, SIGNAL(triggered(bool)), this, SLOT(exportCorpus()));
    QObject::connect(d_ui->fitAction, SIGNAL(triggered(bool)), this, SLOT(fitTree()));
    QObject::connect(d_ui->helpAction, SIGNAL(triggered(bool)), this, SLOT(help()));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->pdfExportAction, SIGNAL(triggered(bool)), this, SLOT(pdfExport()));
    QObject::connect(d_ui->preferencesAction, SIGNAL(triggered(bool)), this,
      SLOT(preferencesWindow()));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
    QObject::connect(d_ui->printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomIn(bool)));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomOut(bool)));
    QObject::connect(d_ui->nextTreeNodeAction, SIGNAL(triggered(bool)), this, SLOT(focusNextTreeNode(bool)));
    QObject::connect(d_ui->previousTreeNodeAction, SIGNAL(triggered(bool)), this, SLOT(focusPreviousTreeNode(bool)));
    QObject::connect(d_ui->showFilterWindow, SIGNAL(triggered(bool)), this, SLOT(showFilterWindow()));
    QObject::connect(d_ui->showStatisticsWindow, SIGNAL(triggered(bool)), this, SLOT(showStatisticsWindow()));
    QObject::connect(d_ui->showMacrosWindow, SIGNAL(triggered(bool)), this, SLOT(showMacrosWindow()));
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
    
    focusFitTree();
}

void DactMainWindow::help()
{
    static QUrl const usage("http://rug-compling.github.com/dact/Usage.html");
    QDesktopServices::openUrl(usage);
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
    QString valStr = d_highlight.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + d_macrosModel->expand(d_highlight) + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

    try {
        showTree(xml, params);
        showSentence(xml, params);
        
        // I try to find my file back in the file list to keep the list
        // in sync with the treegraph since showFile can be called from
        // the child dialogs.
        
        // TODO: disabled this for now because it messes up the selection
        // when deselecting files by ctrl/cmd-clicking on them.
        /*
        QListWidgetItem *item;
        for(int i = 0; i < d_ui->fileListWidget->count(); ++i) {
            item = d_ui->fileListWidget->item(i);
            if(item->data(Qt::UserRole).toString() == filename) {
                d_ui->fileListWidget->setCurrentItem(item);
                break;
            }
        }
        */
        
    } catch (std::runtime_error const &e) {
        QMessageBox::critical(this, QString("Tranformation error"),
            QString("A transformation error occured: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
    }
}

void DactMainWindow::setFilter(QString const &query)
{
    d_ui->filterLineEdit->setText(query);
    filterChanged();
}

void DactMainWindow::filterChanged()
{
    QMutexLocker locker(&d_filterChangedMutex);

    d_filter = d_ui->filterLineEdit->text().trimmed();
    
    if (d_queryHistory)
        d_queryHistory->addToHistory(d_filter);
    
    d_ui->highlightLineEdit->setText(d_filter);
    highlightChanged();

    if (!d_corpusReader.isNull())
        addFiles();
}


void DactMainWindow::fitTree()
{
    if (!d_treeScene)
        return;
   
    if (d_treeScene->rootNode() != 0)
      d_ui->treeGraphicsView->fitInView(d_treeScene->rootNode()->boundingRect(), Qt::KeepAspectRatio);
}

void DactMainWindow::focusFitTree()
{
    if (!d_treeScene)
        return;
    
    if (d_treeScene->activeNodes().length())
    {
        resetTreeZoom();
        focusTreeNode(1);
    }
    else
        fitTree();
}

void DactMainWindow::resetTreeZoom()
{
    d_ui->treeGraphicsView->setMatrix(QMatrix());
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
    QFile xslFile(":/stylesheets/tree.xsl");
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
        "*.dbxml;;*.data.dz");
    if (corpusPath.isNull())
        return;

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

void DactMainWindow::preferencesWindow()
{
  if (d_preferencesWindow == 0)
    d_preferencesWindow = new PreferencesWindow(this);
  d_preferencesWindow->show();
  d_preferencesWindow->raise();
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
    stopMapper();

    if (d_corpusOpenWatcher.isRunning()) {
        d_corpusOpenWatcher.cancel();
        d_corpusOpenWatcher.waitForFinished();
    }

    d_openProgressDialog->open();

    // Opening a corpus cannot be cancelled, but reading it (iterating the iterator) can.
    d_openProgressDialog->setCancelable(false);
    
    QFuture<bool> corpusOpenFuture = QtConcurrent::run(this, &DactMainWindow::readAndShowFiles, corpusPath);
    d_corpusOpenWatcher.setFuture(corpusOpenFuture);
}

bool DactMainWindow::readAndShowFiles(QString const &path)
{
    try {
        d_corpusReader = QSharedPointer<ac::CorpusReader>(ac::CorpusReader::open(path));
        d_openProgressDialog->setCancelable(true);
        addFiles();
    } catch (std::runtime_error const &e) {
        // TODO display a nice error window here
        return false;
    }

    return true;
}

void DactMainWindow::cancelReadCorpus()
{
    d_addFilesCancelled = true;
}

void DactMainWindow::corpusRead(int idx)
{
    d_openProgressDialog->accept();

    if(d_bracketedWindow != 0)
        d_bracketedWindow->switchCorpus(d_corpusReader);

    if(d_statisticsWindow != 0)
        d_statisticsWindow->switchCorpus(d_corpusReader);
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

void DactMainWindow::exportCorpus()
{
    QString filename(QFileDialog::getSaveFileName(this,
        d_ui->fileListWidget->selectedItems().size() ? "Export selection" : "Export corpus",
        QString(), "*.dbxml"));
    
    if (!filename.isNull() && d_corpusReader)
    {
        if (d_exportProgressDialog == 0)
            d_exportProgressDialog = new ExportProgressDialog(this);
        
        d_exportProgressDialog->open();
        
        // Since we make a copy of the current selection, this action doesn't really need to block
        // any interaction with the gui as long as the corpusreader supports simultanious reading
        // and writing.
        
        QList<QString> files;
        
        if (d_ui->fileListWidget->selectedItems().size())
        {
            foreach (QListWidgetItem *item,
                    d_ui->fileListWidget->selectedItems())
                files.append(item->data(Qt::UserRole).toString());
        }
        else
            std::copy(d_corpusReader->begin(), d_corpusReader->end(), std::back_inserter(files));
        
        QtConcurrent::run(this, &DactMainWindow::writeCorpus, filename, files);
    }
}

void DactMainWindow::writeCorpus(QString const &filename, QList<QString> const &files)
{
    try {
        ac::DbCorpusWriter corpus(filename, true);
        
        d_exportProgressDialog->setMaximum(files.size());
        int progress = 0;
            
        for (QList<QString>::const_iterator itr(files.constBegin()),
             end(files.constEnd()); itr != end; ++itr)
        {
            d_exportProgressDialog->setProgress(++progress);
            corpus.write(*itr, d_corpusReader->read(*itr));
        }
    } catch (ac::OpenError const &e) {
        QString const msg(
                          "Could not open %1 for exporting:\n\n%2"
                          );
        QMessageBox::critical(this, "Export error",
                              msg.arg(filename).arg(e.what()));
    } catch (std::runtime_error const &e) {
        QString msg("Could not export to %1:\n\n%2");
        if (not QFile::remove(filename))
            msg += QString("\n\nCheck or delete the file %1").arg(filename);
        QMessageBox::critical(this, "Export error",
                              msg.arg(filename).arg(e.what()));
    }
    
    d_exportProgressDialog->accept();
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
    // @TODO why is the scene recreated for every tree, and not just cleared and reused?
    // (well, currently because calling parseTree twice will cause a segfault)
    if (d_treeScene)
        delete d_treeScene;
    
    d_treeScene = new DactTreeScene(d_ui->treeGraphicsView);
    
    QString xml_tree = d_treeTransformer->transform(xml, params);
    
    d_treeScene->parseTree(xml_tree);
    
    d_ui->treeGraphicsView->setScene(d_treeScene);
}

void DactMainWindow::stopMapper()
{
    if (d_xpathMapper->isRunning()) {
        d_xpathMapper->cancel();
        d_xpathMapper->wait();
    }
}

void DactMainWindow::setHighlight(QString const &query)
{
    d_ui->highlightLineEdit->setText(query);
    highlightChanged();
}

void DactMainWindow::highlightChanged()
{
    d_highlight = d_ui->highlightLineEdit->text().trimmed();
    
    if (d_queryHistory)
        d_queryHistory->addToHistory(d_highlight);
    
    if (d_ui->fileListWidget->currentItem() != 0)
        showFile(d_ui->fileListWidget->currentItem()->data(Qt::UserRole).toString());
}

void DactMainWindow::treeZoomIn(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_IN_FACTOR, ZOOM_IN_FACTOR);
}

void DactMainWindow::treeZoomOut(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_OUT_FACTOR, ZOOM_OUT_FACTOR);
}

void DactMainWindow::focusNextTreeNode(bool)
{
    focusTreeNode(1);
}

void DactMainWindow::focusPreviousTreeNode(bool)
{
    focusTreeNode(-1);
}

void DactMainWindow::focusTreeNode(int direction)
{
    if (!d_treeScene)
        return;
    
    QList<DactTreeNode*> nodes = d_treeScene->nodes();
    
    int offset = 0;
    int nodeCount = nodes.length();
    
    // Find the currently focussed node
    for (int i = 0; i < nodeCount; ++i)
    {
        if (nodes[i]->hasFocus())
        {
            offset = i + direction;
            break;
        }
    }
    
    // then find the next node that's active
    for (int i = 0; i < nodeCount; ++i)
    {
        // nodeCount + offset + direction * i is positive for [0..nodeCount]
        // whichever the direction, and modulo nodeCount makes shure it wraps around.
        DactTreeNode &node = *nodes[(nodeCount + offset + direction * i) % nodeCount];
        
        if (node.isActive())
        {
            node.setFocus();
            
            // reset the matrix to undo the scale operation done by fitTree.
            // I don't like this yet, because it always resets the zoom.
            //d_ui->treeGraphicsView->setMatrix(QMatrix());
            
            // move the focus to the center of the focussed leaf
            d_ui->treeGraphicsView->centerOn(node.mapToScene(node.leafRect().center()));
            break;
        }
    }
}
