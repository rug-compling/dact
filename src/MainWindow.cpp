#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QHash>
#include <QItemSelection>
#include <QLineEdit>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMutexLocker>
#include <QPainter>
#include <QPoint>
#include <QPrintDialog>
#include <QPrinter>
#include <QSettings>
#include <QSize>
#include <QTextStream>
#include <QUrl>
#include <QtConcurrentRun>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <AboutWindow.hh>
#include <MainWindow.hh>
#include <BracketedWindow.hh>
#include <DactMacrosModel.hh>
#include <DactMacrosWindow.hh>
//#include <DactQueryHistory.hh>
#include <FilterModel.hh>
#include <DactProgressDialog.hh>
#include <PreferencesWindow.hh>
#include <StatisticsWindow.hh>
#include <DactTreeScene.hh>
#include "TreeNode.hh"
#include <XPathValidator.hh>
#include <XSLTransformer.hh>
#include "ValidityColor.hh"
#include <ui_MainWindow.h>

namespace ac = alpinocorpus;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(QSharedPointer<Ui::MainWindow>(new Ui::MainWindow)),
    d_aboutWindow(new AboutWindow(this, Qt::Window)),
    d_bracketedWindow(0),
    d_statisticsWindow(0),
    d_macrosWindow(0),
    d_openProgressDialog(new DactProgressDialog(this)),
    d_exportProgressDialog(new DactProgressDialog(this)),
    d_preferencesWindow(0)
#if 0
    d_queryHistory(0)
#endif
{
    d_ui->setupUi(this);
    
    d_macrosModel = QSharedPointer<DactMacrosModel>(new DactMacrosModel());
    
    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    d_ui->highlightLineEdit->setValidator(d_xpathValidator.data());
#if 0
    d_queryHistory = QSharedPointer<DactQueryHistory>(new DactQueryHistory());
    d_ui->filterLineEdit->setCompleter(d_queryHistory->completer());
    d_ui->highlightLineEdit->setCompleter(d_queryHistory->completer());
#endif
    
    d_ui->hitsDescLabel->hide();
    d_ui->hitsLabel->hide();
    d_ui->statisticsLayout->setVerticalSpacing(0);
    
    readSettings();
    
    initSentenceTransformer();
    
    createActions();    
}

MainWindow::~MainWindow()
{
    delete d_aboutWindow;
    delete d_bracketedWindow;
    delete d_statisticsWindow;
    delete d_macrosWindow;
    delete d_openProgressDialog;
    delete d_exportProgressDialog;
    delete d_preferencesWindow;
}

void MainWindow::aboutDialog()
{
    d_aboutWindow->show();
    d_aboutWindow->raise();
}

void MainWindow::bracketedEntryActivated(const QString &entry)
{
    showFile(entry);
    focusFitTree();
    
    setHighlight(d_bracketedWindow->filter());
    activateWindow();
}

void MainWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void MainWindow::changeEvent(QEvent *e)
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

void MainWindow::close()
{
    writeSettings();
    QMainWindow::close();
}

void MainWindow::showFilterWindow()
{
    if (d_bracketedWindow == 0)
    {
        d_bracketedWindow = new BracketedWindow(d_corpusReader, d_macrosModel, this, Qt::Window);
        QObject::connect(d_bracketedWindow, SIGNAL(entryActivated(QString)), this, SLOT(bracketedEntryActivated(QString)));
    }

    d_bracketedWindow->setFilter(d_filter);
    d_bracketedWindow->show();
    d_bracketedWindow->raise();
}

void MainWindow::showStatisticsWindow()
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

void MainWindow::showWriteCorpusError(QString const &error)
{
    QMessageBox::critical(this, "Export error", error);
}

void MainWindow::statisticsEntryActivated(QString const &value, QString const &query)
{
    d_ui->filterLineEdit->setText(query);
    filterChanged();
    activateWindow();
}

void MainWindow::showMacrosWindow()
{
    if (d_macrosWindow == 0)
        d_macrosWindow = new DactMacrosWindow(d_macrosModel, this, Qt::Window);
    
    d_macrosWindow->show();
    d_macrosWindow->raise();
}

void MainWindow::createActions()
{
    QObject::connect(&d_corpusOpenWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(corpusRead(int)));
    QObject::connect(&d_corpusWriteWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(corpusWritten(int)));
    
    QObject::connect(d_openProgressDialog, SIGNAL(rejected()), this, SLOT(cancelReadCorpus()));
    QObject::connect(this, SIGNAL(exportError(QString const &)), this, SLOT(showWriteCorpusError(QString const &)));
    
    // @TODO don't steal other peoples error dialogs!
    QObject::connect(this, SIGNAL(openError(QString const &)), this, SLOT(showWriteCorpusError(QString const &)));
    
    QObject::connect(d_exportProgressDialog, SIGNAL(rejected()), this, SLOT(cancelWriteCorpus()));
    QObject::connect(this, SIGNAL(exportProgressMaximum(int)), d_exportProgressDialog, SLOT(setMaximum(int)));
    QObject::connect(this, SIGNAL(exportProgress(int)), d_exportProgressDialog, SLOT(setProgress(int)));
    
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->highlightLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    QObject::connect(d_ui->highlightLineEdit, SIGNAL(returnPressed()), this, SLOT(highlightChanged()));

    // listen to selection changes to update the next/prev node buttons accordingly.
    QObject::connect(d_ui->treeGraphicsView, SIGNAL(sceneChanged(DactTreeScene*)),
        this, SLOT(treeChanged(DactTreeScene*)));

    // Actions
    QObject::connect(d_ui->aboutAction, SIGNAL(triggered(bool)), this, SLOT(aboutDialog()));
    QObject::connect(d_ui->openAction, SIGNAL(triggered(bool)), this, SLOT(openCorpus()));
    QObject::connect(d_ui->openDirectoryAction, SIGNAL(triggered(bool)), this, SLOT(openDirectoryCorpus()));
    QObject::connect(d_ui->saveCorpus, SIGNAL(triggered(bool)), this, SLOT(exportCorpus()));
    QObject::connect(d_ui->xmlExportAction, SIGNAL(triggered(bool)), this, SLOT(exportXML()));
    QObject::connect(d_ui->fitAction, SIGNAL(triggered(bool)), d_ui->treeGraphicsView, SLOT(fitTree()));
    QObject::connect(d_ui->helpAction, SIGNAL(triggered(bool)), this, SLOT(help()));
    QObject::connect(d_ui->nextAction, SIGNAL(triggered(bool)), this, SLOT(nextEntry(bool)));
    QObject::connect(d_ui->pdfExportAction, SIGNAL(triggered(bool)), this, SLOT(exportPDF()));
    QObject::connect(d_ui->preferencesAction, SIGNAL(triggered(bool)), this, SLOT(preferencesWindow()));
    QObject::connect(d_ui->previousAction, SIGNAL(triggered(bool)), this, SLOT(previousEntry(bool)));
    QObject::connect(d_ui->printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), d_ui->treeGraphicsView, SLOT(zoomIn()));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), d_ui->treeGraphicsView, SLOT(zoomOut()));
    QObject::connect(d_ui->nextTreeNodeAction, SIGNAL(triggered(bool)), d_ui->treeGraphicsView, SLOT(focusNextTreeNode()));
    QObject::connect(d_ui->previousTreeNodeAction, SIGNAL(triggered(bool)), d_ui->treeGraphicsView, SLOT(focusPreviousTreeNode()));
    QObject::connect(d_ui->showFilterWindow, SIGNAL(triggered(bool)), this, SLOT(showFilterWindow()));
    QObject::connect(d_ui->showStatisticsWindow, SIGNAL(triggered(bool)), this, SLOT(showStatisticsWindow()));
    QObject::connect(d_ui->showMacrosWindow, SIGNAL(triggered(bool)), this, SLOT(showMacrosWindow()));
    QObject::connect(d_ui->focusFilterAction, SIGNAL(triggered(bool)), this, SLOT(focusFilter()));
    QObject::connect(d_ui->focusHighlightAction, SIGNAL(triggered(bool)), this, SLOT(focusHighlight()));
}

void MainWindow::entryFound(QString) {
    int entries = d_model->rowCount(QModelIndex());
    int hits = d_model->hits();
    d_ui->entriesLabel->setText(QString::number(entries));
    d_ui->hitsLabel->setText(QString::number(hits));
}

void MainWindow::entrySelected(QItemSelection const &current, QItemSelection const &prev)
{
    Q_UNUSED(prev);
    
    if (!current.size()) {
        d_ui->treeGraphicsView->setScene(0);
        return;
    }
    
    showFile(current.indexes().at(0).data(Qt::UserRole).toString());
    
    focusFitTree();
}

void MainWindow::help()
{
    static QUrl const usage("http://rug-compling.github.com/dact/Usage.html");
    QDesktopServices::openUrl(usage);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && d_model)
    {
        d_model->cancelQuery();
        event->accept();
    }
    else
      QMainWindow::keyPressEvent(event);
}

void MainWindow::showFile(QString const &filename)
{
    // Read XML data.
    if (d_corpusReader.isNull())
        return;
    
    try {
        QString xml = d_corpusReader->read(filename);
    
        if (xml.size() == 0) {
            qWarning() << "MainWindow::writeSettings: empty XML data!";
            d_ui->treeGraphicsView->setScene(0);
            return;
        }

        // Parameters
        QString valStr = d_highlight.trimmed().isEmpty() ? "'/..'" :
                         QString("'") + d_macrosModel->expand(d_highlight) + QString("'");
        QHash<QString, QString> params;
        params["expr"] = valStr;

        try {
            showTree(xml);
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
    catch(std::runtime_error const &e)
    {
        QMessageBox::critical(this, QString("Read error"),
            QString("An error occured while trying to read a corpus file: %1").arg(e.what()));
    }
}

void MainWindow::setFilter(QString const &query)
{
    d_ui->filterLineEdit->setText(query);
    filterChanged();
}

void MainWindow::filterChanged()
{
    if (!d_model)
      return;

    QMutexLocker locker(&d_filterChangedMutex);

    d_filter = d_ui->filterLineEdit->text().trimmed();

#if 0
    if (d_queryHistory)
        d_queryHistory->addToHistory(d_filter);
#endif

    
    if (d_filter.isEmpty()) {
        d_ui->hitsDescLabel->hide();
        d_ui->hitsLabel->hide();
        d_ui->statisticsLayout->setVerticalSpacing(0);
    } else {
        d_ui->statisticsLayout->setVerticalSpacing(-1);
        d_ui->hitsDescLabel->show();
        d_ui->hitsLabel->show();
    }
    
    setHighlight(d_filter);

    d_model->runQuery(d_macrosModel->expand(d_filter));
}

void MainWindow::focusFitTree()
{
    if (d_ui->treeGraphicsView->scene()
        && d_ui->treeGraphicsView->scene()->activeNodes().length())
    {
        d_ui->treeGraphicsView->resetZoom();
        d_ui->treeGraphicsView->focusTreeNode(1);
    }
    else
        d_ui->treeGraphicsView->fitTree();
}

void MainWindow::focusFilter()
{
    d_ui->filterLineEdit->setFocus();
}

void MainWindow::focusHighlight()
{
    d_ui->highlightLineEdit->setFocus();
}

void MainWindow::initSentenceTransformer()
{
    // Read stylesheet.
    QFile xslFile(":/stylesheets/bracketed-sentence.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QTextStream xslStream(&xslFile);
    QString xsl(xslStream.readAll());
    d_sentenceTransformer = QSharedPointer<XSLTransformer>(new XSLTransformer(xsl));
}

void MainWindow::mapperStarted(int totalEntries)
{
    d_ui->entriesLabel->setText(QString::number(0));
    d_ui->hitsLabel->setText(QString::number(0));

    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void MainWindow::mapperStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
}

/* Next- and prev entry buttons */

void MainWindow::nextEntry(bool)
{
    QModelIndex current(d_ui->fileListWidget->currentIndex());
    d_ui->fileListWidget->setCurrentIndex(
        current.sibling(current.row() + 1, current.column()));
}

void MainWindow::previousEntry(bool)
{
    QModelIndex current(d_ui->fileListWidget->currentIndex());
    d_ui->fileListWidget->setCurrentIndex(
        current.sibling(current.row() - 1, current.column()));
}

/* Open corpus dialogs */

void MainWindow::openCorpus()
{
    QString corpusPath = QFileDialog::getOpenFileName(this, "Open corpus", QString(),
        "Dact corpora (*.dact *.data.dz)");
    if (corpusPath.isNull())
        return;

    readCorpus(corpusPath);
}

void MainWindow::openDirectoryCorpus()
{
    QString corpusPath = QFileDialog::getExistingDirectory(this,
        "Open directory corpus");
    if (corpusPath.isNull())
        return;

    readCorpus(corpusPath);
}

void MainWindow::exportPDF()
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

void MainWindow::preferencesWindow()
{
    if (d_preferencesWindow == 0)
        d_preferencesWindow = new PreferencesWindow(this);

    d_preferencesWindow->show();
    d_preferencesWindow->raise();
}

void MainWindow::print()
{
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec()) {
        QPainter painter(&printer);
        d_ui->treeGraphicsView->scene()->render(&painter);
        painter.end();
    }
}

void MainWindow::readCorpus(QString const &corpusPath)
{ 
    if (d_model)
        d_model->cancelQuery();
    
    if (d_corpusOpenWatcher.isRunning()) {
        d_corpusOpenWatcher.cancel();
        d_corpusOpenWatcher.waitForFinished();
    }

    d_openProgressDialog->setWindowTitle(QString("Opening %1").arg(corpusPath));
    d_openProgressDialog->setDescription(QString("Opening %1").arg(corpusPath));
    d_openProgressDialog->open();

    // Opening a corpus cannot be cancelled, but reading it (iterating the iterator) can.
    d_openProgressDialog->setCancelable(false);
    
    QFuture<bool> corpusOpenFuture = QtConcurrent::run(this, &MainWindow::readAndShowFiles, corpusPath);
    d_corpusOpenWatcher.setFuture(corpusOpenFuture);
}

bool MainWindow::readAndShowFiles(QString const &path)
{
    try {
        d_corpusReader = QSharedPointer<ac::CorpusReader>(ac::CorpusReader::open(path));
    } catch (std::runtime_error const &e) {
        d_corpusReader.clear();
        emit openError(e.what());
        return false;
    }

    return true;
}

void MainWindow::cancelReadCorpus()
{
    //
}

void MainWindow::cancelWriteCorpus()
{
    d_writeCorpusCancelled = true;
}

void MainWindow::corpusRead(int idx)
{
    d_openProgressDialog->accept();
    
    // If opening the corpus failed, don't do anything.
    if (!d_corpusReader)
        return;
    
    setModel(new FilterModel(d_corpusReader));
    
    // runQuery has to be called explicitly to give you the time to connect to
    // any signals before we start searching.
    d_model->runQuery();
    
    if(d_bracketedWindow != 0)
        d_bracketedWindow->switchCorpus(d_corpusReader);

    if(d_statisticsWindow != 0)
        d_statisticsWindow->switchCorpus(d_corpusReader);
}

void MainWindow::corpusWritten(int idx)
{
    d_exportProgressDialog->accept();
}

void MainWindow::readSettings()
{
    QSettings settings;

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

void MainWindow::exportCorpus()
{

    if (d_corpusWriteWatcher.isRunning()) {
        d_corpusWriteWatcher.cancel();
        d_corpusWriteWatcher.waitForFinished();
    }

    QItemSelectionModel *selectionModel = d_ui->fileListWidget->selectionModel();
    bool selectionOnly = selectionModel->selectedIndexes().size() > 1;

    QString filename(QFileDialog::getSaveFileName(this,
        selectionOnly ? "Export selection" : "Export corpus",
        QString(), "*.dact"));
    
    if (!filename.isNull() && d_corpusReader)
    {
        d_exportProgressDialog->setWindowTitle(selectionOnly ? "Exporting selection" : "Exporting corpus");
        d_exportProgressDialog->setDescription(QString("Exporting %1 to:\n%2")
            .arg(selectionOnly ? "selection" : "corpus")
            .arg(filename));
        d_exportProgressDialog->open();
        
        // Since we make a copy of the current selection, this action doesn't really need to block
        // any interaction with the gui as long as the corpusreader supports simultanious reading
        // and writing.
        
        QList<QString> files;
        
        if (selectionOnly)
        {
            foreach (QModelIndex item, selectionModel->selectedRows())
                files.append(item.data(Qt::UserRole).toString());
        }
        else
            std::copy(d_corpusReader->begin(), d_corpusReader->end(), std::back_inserter(files));
        
        d_writeCorpusCancelled = false;
        d_exportProgressDialog->setCancelable(true);
        
        QFuture<bool> corpusWriterFuture =
            QtConcurrent::run(this, &MainWindow::writeCorpus, filename, files);
        d_corpusWriteWatcher.setFuture(corpusWriterFuture);
    }
}

bool MainWindow::writeCorpus(QString const &filename, QList<QString> const &files)
{
    try {
        ac::DbCorpusWriter corpus(filename, true);
        
        emit exportProgressMaximum(files.size());
        emit exportProgress(0);
        int percent = files.size() / 100;
        int progress = 0;
            
        for (QList<QString>::const_iterator itr(files.constBegin()),
             end(files.constEnd());
             !d_writeCorpusCancelled && itr != end; ++itr)
        {
            corpus.write(*itr, d_corpusReader->read(*itr));
            ++progress;
            if (percent == 0 || progress % percent == 0)
              emit exportProgress(progress);
        }
    } catch (ac::OpenError const &e) {
        emit exportError(e.what());
        return false;
    } catch (std::runtime_error const &e) {
        emit exportError(QString("Could not export %1:\n%2").arg(filename).arg(e.what()));
        return false;
    }
    
    return true;
}

void MainWindow::exportXML()
{
    if (d_ui->fileListWidget->selectionModel()->selectedIndexes().size() < 1)
        return;
    
    QModelIndex index(d_ui->fileListWidget->selectionModel()->selectedIndexes()[0]);
    QString file(index.data(Qt::UserRole).toString());
    
    QString targetName(QFileDialog::getSaveFileName(this,
        QString("Export %1").arg(file),
        file, "*.xml"));
    
    if (targetName.isNull())
        return;
    
    QFile target(targetName);
    
    if (!target.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this,
            QString("Failed exporting %1").arg(target.fileName()),
            "Could not open file for writing");
        return;
    }
    
    target.write(d_corpusReader->read(file).toUtf8());
    target.close();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    // Window geometry
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    // Splitter
    settings.setValue("splitterSizes", d_ui->splitter->saveState());
}

void MainWindow::showSentence(QString const &xml, QHash<QString, QString> const &params)
{
    d_ui->sentenceWidget->setParse(xml);
}

void MainWindow::showTree(QString const &xml)
{
    d_ui->treeGraphicsView->showTree(xml);
}

void MainWindow::setHighlight(QString const &query)
{
    d_highlight = query;
    d_ui->highlightLineEdit->setText(query);
    d_ui->sentenceWidget->setQuery(query);
    d_ui->treeGraphicsView->setHighlightQuery(d_macrosModel->expand(query));
}

void MainWindow::highlightChanged()
{
    setHighlight(d_ui->highlightLineEdit->text().trimmed());
}

void MainWindow::setModel(FilterModel *model)
{
    d_model = QSharedPointer<FilterModel>(model);
    d_ui->fileListWidget->setModel(d_model.data());
    
    QObject::connect(model, SIGNAL(queryStarted(int)),
        this, SLOT(mapperStarted(int)));
    QObject::connect(model, SIGNAL(queryStopped(int, int)),
        this, SLOT(mapperStopped(int, int)));
    QObject::connect(model, SIGNAL(entryFound(QString)),
        this, SLOT(entryFound(QString)));
    
    QObject::connect(d_ui->fileListWidget->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
        SLOT(entrySelected(QItemSelection,QItemSelection)));
}

void MainWindow::treeChanged(DactTreeScene *scene)
{
    if (scene) // might be null-pointer if the scene is cleared
        QObject::connect(scene, SIGNAL(selectionChanged()),
            this, SLOT(updateTreeNodeButtons()));
    
    updateTreeNodeButtons();
}

void MainWindow::updateTreeNodeButtons()
{
    bool nodesBeforeFocussedNode = false;
    bool nodesAfterFocussedNode = false;
    bool focussedNodePassed = false;
    
    if (d_ui->treeGraphicsView->scene())
        foreach(TreeNode* node, d_ui->treeGraphicsView->scene()->nodes())
        {
            if (node->hasFocus())
                focussedNodePassed = true;
    
            else if (node->isActive())
            {
                if (!focussedNodePassed)
                    nodesBeforeFocussedNode = true;
                else
                    nodesAfterFocussedNode = true;
            }
        }
    
    // When focussedNodePassed is false, none of the nodes has focus. Then what
    // is the "next" node? Therefore use nodesBeforeFocussedUpdate when none of
    // the nodes is focussed, which will be true if at least one node is active.
    d_ui->previousTreeNodeAction->setEnabled(nodesBeforeFocussedNode);
    d_ui->nextTreeNodeAction->setEnabled(
        focussedNodePassed ? nodesAfterFocussedNode : nodesBeforeFocussedNode);
}
