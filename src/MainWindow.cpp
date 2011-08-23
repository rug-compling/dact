#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFuture>
#include <QHash>
#include <QItemSelection>
#include <QLineEdit>
#include <QList>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMutexLocker>
#include <QPainter>
#include <QPoint>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QSettings>
#include <QSize>
#include <QStatusBar>
#include <QTextStream>
#include <QUrl>
#include <QtConcurrentRun>

#include <algorithm>
#include <iterator>
#include <list>
#include <stdexcept>
#include <typeinfo>

#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <AboutWindow.hh>
#include <DownloadWindow.hh>
#include <MainWindow.hh>
#include <BracketedWindow.hh>
#include <DactMacrosModel.hh>
#include <DactMacrosWindow.hh>
//#include <DactQueryHistory.hh>
#include <PreferencesWindow.hh>
#include <StatisticsWindow.hh>
#include <DactTreeScene.hh>
#include "TreeNode.hh"
#include <XPathValidator.hh>
#include <XSLTransformer.hh>
#include "ValidityColor.hh"
#include <ui_MainWindow.h>
#include <Query.hh>

#ifdef Q_WS_MAC
extern void qt_mac_set_dock_menu(QMenu *);
#endif

namespace ac = alpinocorpus;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(QSharedPointer<Ui::MainWindow>(new Ui::MainWindow)),
    d_aboutWindow(new AboutWindow(this, Qt::Window)),
    d_downloadWindow(0),
    d_macrosWindow(0),
    d_openProgressDialog(new QProgressDialog(this)),
    d_exportProgressDialog(new QProgressDialog(this)),
    d_preferencesWindow(0)
#if 0
    d_queryHistory(0)
#endif
{
    setupUi();

    // Ensure that we have a status bar.
    statusBar();

    d_macrosModel = QSharedPointer<DactMacrosModel>(new DactMacrosModel());
    
    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
#if 0
    d_queryHistory = QSharedPointer<DactQueryHistory>(new DactQueryHistory());
    d_ui->filterLineEdit->setCompleter(d_queryHistory->completer());
    d_ui->highlightLineEdit->setCompleter(d_queryHistory->completer());
#endif
        
    readSettings();
    
    initSentenceTransformer();
    
    createActions();
}

MainWindow::~MainWindow()
{
    delete d_aboutWindow;
    delete d_downloadWindow;
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
    setFilter(d_ui->sentencesWidget->filter());
    d_ui->dependencyTreeWidget->showFile(entry);
    
    activateWindow();
}

void MainWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void MainWindow::cancelQuery()
{
  d_ui->dependencyTreeWidget->cancelQuery();
  d_ui->statisticsWindow->cancelQuery();
  statusBar()->clearMessage();

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

void MainWindow::showDownloadWindow()
{
    if (d_downloadWindow == 0)
        d_downloadWindow = new DownloadWindow(this, Qt::Window);
    
    d_downloadWindow->show();
    d_downloadWindow->raise();
}

void MainWindow::showOpenCorpusError(QString const &error)
{
    QMessageBox::critical(this, "Open error", error);
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

void MainWindow::setupUi()
{
    d_ui->setupUi(this);
    
    // Move a spacer between the buttons and the inspector action button
    // This will align the inspection action button to the right
    QWidget *spacer = new QWidget(d_ui->mainToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d_ui->mainToolBar->addWidget(spacer);
    d_ui->mainToolBar->addAction(d_ui->inspectorAction);
    
    #ifdef Q_WS_MAC
        QMenu *appleDockMenu = new QMenu(this);
        appleDockMenu->addAction(d_ui->openAction);
        appleDockMenu->addAction(d_ui->openDirectoryAction);
        qt_mac_set_dock_menu(appleDockMenu);
    #endif
}

void MainWindow::createActions()
{
    connect(&d_corpusOpenWatcher, SIGNAL(finished()),
        SLOT(corpusRead()));
    connect(&d_corpusWriteWatcher, SIGNAL(resultReadyAt(int)),
        SLOT(corpusWritten(int)));
    
    connect(this, SIGNAL(exportError(QString const &)),
        SLOT(showWriteCorpusError(QString const &)));
    
    connect(this, SIGNAL(openError(QString const &)),
        SLOT(showOpenCorpusError(QString const &)));
    
    // listen to selection changes to update the next/prev node buttons accordingly.
    connect(d_ui->dependencyTreeWidget, SIGNAL(sceneChanged(DactTreeScene*)),
            SLOT(treeChanged(DactTreeScene*)));
    
    connect(d_exportProgressDialog, SIGNAL(canceled()),
        SLOT(cancelWriteCorpus()));
    connect(this, SIGNAL(exportProgressMaximum(int)), d_exportProgressDialog, SLOT(setMaximum(int)));
    connect(this, SIGNAL(exportProgress(int)), d_exportProgressDialog, SLOT(setValue(int)));
    connect(this, SIGNAL(queryCancelRequest()), SLOT(cancelQuery()),
        Qt::QueuedConnection);
    
    connect(d_ui->statisticsWindow, SIGNAL(entryActivated(QString, QString)),
            SLOT(statisticsEntryActivated(QString const &, QString const &)));
    
    connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)),
        SLOT(applyValidityColor(QString const &)));
    connect(d_ui->filterLineEdit, SIGNAL(returnPressed()),
        SLOT(filterChanged()));
    connect(d_ui->mainTabWidget, SIGNAL(currentChanged(int)),
        SLOT(tabChanged(int)));

    // Actions
    connect(d_ui->aboutAction, SIGNAL(triggered(bool)),
        SLOT(aboutDialog()));
    connect(d_ui->downloadAction, SIGNAL(triggered(bool)),
        SLOT(showDownloadWindow()));
    connect(d_ui->openAction, SIGNAL(triggered(bool)),
        SLOT(openCorpus()));
    connect(d_ui->openDirectoryAction, SIGNAL(triggered(bool)),
        SLOT(openDirectoryCorpus()));
    connect(d_ui->menuRecentFiles, SIGNAL(fileSelected(QString)),
        SLOT(readCorpus(QString)));
    connect(d_ui->saveCorpus, SIGNAL(triggered(bool)),
        SLOT(exportCorpus()));
    connect(d_ui->fitAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(fitTree()));
    connect(d_ui->helpAction, SIGNAL(triggered(bool)),
        SLOT(help()));
    connect(d_ui->nextAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(nextEntry(bool)));
    connect(d_ui->pdfExportAction, SIGNAL(triggered(bool)),
        SLOT(exportPDF()));
    connect(d_ui->xmlExportAction, SIGNAL(triggered(bool)),
        SLOT(exportXML()));
    connect(d_ui->preferencesAction, SIGNAL(triggered(bool)),
        SLOT(preferencesWindow()));
    connect(d_ui->previousAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(previousEntry(bool)));
    connect(d_ui->printAction, SIGNAL(triggered(bool)),
        SLOT(print()));
    connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomIn()));
    connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomOut()));
    connect(d_ui->nextTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusNextTreeNode()));
    connect(d_ui->previousTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusPreviousTreeNode()));
    connect(d_ui->showMacrosWindow, SIGNAL(triggered(bool)),
        SLOT(showMacrosWindow()));
    connect(d_ui->focusFilterAction, SIGNAL(triggered(bool)),
        SLOT(focusFilter()));
    connect(d_ui->focusHighlightAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusHighlight()));
    connect(d_ui->filterOnAttributeAction, SIGNAL(triggered()),
        SLOT(filterOnInspectorSelection()));
}

void MainWindow::filterOnInspectorSelection()
{
    QString query = d_filter.isEmpty() ? "//node" : d_filter;
    QMap<QString,QString> attributes = d_ui->inspector->selectedAttributes();
    
    if (attributes.size() < 1)
        return;
    
    for (QMap<QString,QString>::const_iterator itr(attributes.constBegin()),
        end(attributes.constEnd()); itr != end; itr++)
        query = ::generateQuery(query, itr.key(), itr.value());
    
    setFilter(query);
}

void MainWindow::help()
{
    static QUrl const usage("http://rug-compling.github.com/dact/manual/");
    QDesktopServices::openUrl(usage);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        statusBar()->showMessage("Cancelling the query...");
        emit queryCancelRequest();
        event->accept();
    }
    else
      QMainWindow::keyPressEvent(event);
}

void MainWindow::setFilter(QString const &query)
{
    d_ui->filterLineEdit->setText(query);
    filterChanged();
}

void MainWindow::filterChanged()
{
    QMutexLocker locker(&d_filterChangedMutex);

    d_filter = d_ui->filterLineEdit->text().trimmed();

#if 0
    if (d_queryHistory)
        d_queryHistory->addToHistory(d_filter);
#endif


    d_ui->dependencyTreeWidget->setFilter(d_macrosModel->expand(d_filter));
    d_ui->statisticsWindow->setFilter(d_macrosModel->expand(d_filter));
    d_ui->sentencesWidget->setFilter(d_macrosModel->expand(d_filter));
}

void MainWindow::focusFilter()
{
    d_ui->filterLineEdit->setFocus();
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
    d_ui->dependencyTreeWidget->renderTree(&painter);
    
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
        d_ui->dependencyTreeWidget->renderTree(&painter);
        painter.end();
    }
}

void MainWindow::readCorpus(QString const &corpusPath)
{
    d_ui->dependencyTreeWidget->cancelQuery();
    d_ui->statisticsWindow->cancelQuery();
    
    if (d_corpusOpenWatcher.isRunning()) {
        d_corpusOpenWatcher.cancel();
        d_corpusOpenWatcher.waitForFinished();
    }
    
    d_openProgressDialog->setWindowTitle(QString("Opening %1").arg(corpusPath));
    d_openProgressDialog->setLabelText(QString("Opening %1").arg(corpusPath));
    d_openProgressDialog->open();

    // Opening a corpus cannot be cancelled, but reading it (iterating the iterator) can.
    d_openProgressDialog->setCancelButton(0);
    
    QFuture< QPair< QSharedPointer<ac::CorpusReader>, QString> > corpusOpenFuture = QtConcurrent::run(this, &MainWindow::createCorpusReader, corpusPath);
    d_corpusOpenWatcher.setFuture(corpusOpenFuture);
}

QPair< QSharedPointer<ac::CorpusReader>, QString> MainWindow::createCorpusReader(QString const &path)
{
    QSharedPointer<ac::CorpusReader> reader;
    
    try {
        reader = QSharedPointer<ac::CorpusReader>(ac::CorpusReader::open(path.toUtf8().constData()));
    } catch (std::runtime_error const &e) {
        emit openError(e.what());
    }
    
    return QPair< QSharedPointer<ac::CorpusReader>, QString >(reader, path);
}

void MainWindow::cancelWriteCorpus()
{
    d_writeCorpusCancelled = true;
}

void MainWindow::setCorpusReader(QSharedPointer<ac::CorpusReader> reader, QString const &path)
{
    d_corpusReader = reader;
    
    d_xpathValidator->setCorpusReader(reader);
    
    // XXX - There seems to be no way to revalidate a QLineEdit
    QString query = d_ui->filterLineEdit->text();
    d_ui->filterLineEdit->clear();
    d_ui->filterLineEdit->insert(query);
        
    if (!reader.isNull())
    {
        // Show the canonical name in the window title, if it is implemented
        // (related: alpinocorpus issue #9)
        /*
        if (d_corpusReader->name().isEmpty())
            setWindowTitle("Dact");
        else
            setWindowTitle(QString("%1 — Dact").arg(d_corpusReader->name()));
        */
        
        setWindowTitle(QString::fromUtf8("%1 — Dact").arg(QFileInfo(path).fileName()));
        
        // On OS X, add the file icon to the window (and try alt-clicking it!)
        setWindowFilePath(path);
        
        // Add file to the recent files menu
        d_ui->menuRecentFiles->addFile(path);
    }
    else
    {
        // Make everything unhappy and empty
        setWindowTitle("Dact");
        setWindowFilePath(QString());
    }

    d_ui->dependencyTreeWidget->switchCorpus(d_corpusReader);
    d_ui->statisticsWindow->switchCorpus(d_corpusReader);
    d_ui->sentencesWidget->switchCorpus(d_corpusReader);
}

void MainWindow::corpusRead()
{
    d_openProgressDialog->accept();
    
    QPair<QSharedPointer<ac::CorpusReader>, QString> result(d_corpusOpenWatcher.result());
    
    setCorpusReader(result.first, result.second);
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

    // Move.
    move(pos);
    
    // Inspector
    d_ui->inspector->setVisible(
        settings.value("inspectorVisible").toBool());
    
    d_ui->dependencyTreeWidget->readSettings();
}

void MainWindow::exportCorpus()
{

    if (d_corpusWriteWatcher.isRunning()) {
        d_corpusWriteWatcher.cancel();
        d_corpusWriteWatcher.waitForFinished();
    }

    if (!d_corpusReader)
        return;

    QItemSelectionModel *selectionModel = d_ui->dependencyTreeWidget->selectionModel();
    bool selectionOnly = selectionModel->selectedIndexes().size() > 1;

    QString filename(QFileDialog::getSaveFileName(this,
        selectionOnly ? "Export selection" : "Export corpus",
        QString(), "*.dact"));
    
    if (!filename.isNull())
    {
        d_exportProgressDialog->setWindowTitle(selectionOnly ? "Exporting selection" : "Exporting corpus");
        d_exportProgressDialog->setLabelText(QString("Exporting %1 to:\n%2")
            .arg(selectionOnly ? "selection" : "corpus")
            .arg(filename));
        d_exportProgressDialog->open();
        
        // Since we make a copy of the current selection, this action doesn't really need to block
        // any interaction with the gui as long as the corpusreader supports simultanious reading
        // and writing.
        
        QList<QString> files;
        
        if (selectionOnly)
        {
            foreach (QModelIndex const &item, selectionModel->selectedRows())
                files.append(item.data(Qt::UserRole).toString());
        }
        else
            for (ac::CorpusReader::EntryIterator iter = d_corpusReader->begin();
                    iter != d_corpusReader->end(); ++iter)
                files.push_back(QString::fromUtf8((*iter).c_str()));
        
        d_writeCorpusCancelled = false;
        d_exportProgressDialog->setCancelButtonText(tr("Cancel"));
        
        QFuture<bool> corpusWriterFuture =
            QtConcurrent::run(this, &MainWindow::writeCorpus, filename, files);
        d_corpusWriteWatcher.setFuture(corpusWriterFuture);
    }
}

bool MainWindow::writeCorpus(QString const &filename, QList<QString> const &files)
{
    try {
        ac::DbCorpusWriter corpus(filename.toUtf8().constData(), true);
        
        emit exportProgressMaximum(files.size());
        emit exportProgress(0);
        int percent = files.size() / 100;
        int progress = 0;
            
        for (QList<QString>::const_iterator itr(files.constBegin()),
             end(files.constEnd());
             !d_writeCorpusCancelled && itr != end; ++itr)
        {
            corpus.write(itr->toUtf8().constData(), d_corpusReader->read(itr->toUtf8().constData()));
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
    if (!d_corpusReader || d_ui->dependencyTreeWidget->selectionModel()->selectedIndexes().size() < 1)
        return;
    
    QModelIndex index(d_ui->dependencyTreeWidget->selectionModel()->selectedIndexes()[0]);
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
    
    target.write(d_corpusReader->read(file.toUtf8().constData()).c_str());
    target.close();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    // Window geometry
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    
    // Inspector
    settings.setValue("inspectorVisible", d_ui->inspector->isVisible());
    
    d_ui->dependencyTreeWidget->writeSettings();
}

void MainWindow::tabChanged(int index)
{
  bool treeWidgetsEnabled = index == 0 ? true : false;

  d_ui->previousAction->setEnabled(treeWidgetsEnabled);
  d_ui->nextAction->setEnabled(treeWidgetsEnabled);
  d_ui->zoomInAction->setEnabled(treeWidgetsEnabled);
  d_ui->zoomOutAction->setEnabled(treeWidgetsEnabled);
  d_ui->fitAction->setEnabled(treeWidgetsEnabled);
  d_ui->nextTreeNodeAction->setEnabled(treeWidgetsEnabled);
  d_ui->previousTreeNodeAction->setEnabled(treeWidgetsEnabled);
  d_ui->xmlExportAction->setEnabled(treeWidgetsEnabled);
  d_ui->pdfExportAction->setEnabled(treeWidgetsEnabled);
  d_ui->printAction->setEnabled(treeWidgetsEnabled);
  d_ui->focusHighlightAction->setEnabled(treeWidgetsEnabled);
}

void MainWindow::treeChanged(DactTreeScene *scene)
{
    if (scene) // might be null-pointer if the scene is cleared
    {
        connect(scene, SIGNAL(selectionChanged()),
            SLOT(updateTreeNodeButtons()));
        
        connect(scene, SIGNAL(selectionChanged(TreeNode const *)),
            d_ui->inspector, SLOT(inspect(TreeNode const *)));
    }

    updateTreeNodeButtons();
}

void MainWindow::updateTreeNodeButtons()
{
    bool nodesBeforeFocussedNode = false;
    bool nodesAfterFocussedNode = false;
    bool focussedNodePassed = false;
    
    if (d_ui->dependencyTreeWidget->scene())
        foreach(TreeNode const *node, d_ui->dependencyTreeWidget->scene()->nodes())
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
