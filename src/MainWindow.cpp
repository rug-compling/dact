#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFuture>
#include <QItemSelection>
#include <QList>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMutexLocker>
#include <QPainter>
#include <QPair>
#include <QPoint>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QSettings>
#include <QSize>
#include <QStatusBar>
#include <QTextStream>
#include <QVector>
#include <QtConcurrentRun>

#include <list>
#include <stdexcept>
#include <string>

#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>

#include <config.hh>

#if defined(USE_SANDBOXING)
#include <QTemporaryFile>
#endif

#include <AppleUtils.hh>
#include <DactApplication.hh>
#include <DactMenuBar.hh>
#include <MainWindow.hh>
#include <BracketedWindow.hh>
#include <CorpusWidget.hh>
#include <DactMacrosModel.hh>
#include <StatisticsWindow.hh>
#include <DactTreeScene.hh>
#include <TreeNode.hh>
#include <XPathValidator.hh>
#include <ValidityColor.hh>
#include <ui_MainWindow.h>
#include <Query.hh>

#include <GlobalCopyCommand.hh>
#include <GlobalCutCommand.hh>
#include <GlobalPasteCommand.hh>

#if defined(Q_WS_MAC) && defined(USE_SPARKLE)
#include <SparkleAutoUpdater.hh>
#endif

namespace ac = alpinocorpus;

typedef std::list<ac::CorpusReaderFactory::ReaderInfo> ReaderList;
typedef std::list<std::string> ExtList;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::MainWindow),
    d_openProgressDialog(new QProgressDialog(this)),
    d_exportProgressDialog(new QProgressDialog(this)),
    d_macrosModel(QSharedPointer<DactMacrosModel>(new DactMacrosModel())),
    d_xpathValidator(new XPathValidator(d_macrosModel))
{
    setupUi();

    d_exportProgressDialog->reset();
    d_openProgressDialog->reset();

    setMenuBar(new DactMenuBar(this));

    d_ui->filterComboBox->setModel(
        dynamic_cast<DactApplication *>(qApp)->historyModel());

    initTaintedWidgets();

    // Ensure that we have a status bar.
    statusBar();

    dynamic_cast<DactMenuBar *>(menuBar())->setMacrosModel(d_macrosModel);

    d_ui->filterComboBox->lineEdit()->setValidator(d_xpathValidator.data());

    d_ui->dependencyTreeWidget->setMacrosModel(d_macrosModel);

    readSettings();

    createActions();

    DactMenuBar *menu = dynamic_cast<DactMenuBar *>(menuBar());
    menu->ui()->saveAsAction->setEnabled(false);

#if defined(Q_WS_MAC) && defined(USE_SPARKLE)
    d_autoUpdater = QSharedPointer<SparkleAutoUpdater>(new SparkleAutoUpdater("http://localhost/appcast.xml"));
    d_ui->checkForUpdatesAction->setVisible(true);
#endif
}

MainWindow::~MainWindow()
{
    cancelQuery();
}

void MainWindow::bracketedEntryActivated(const QString &entry)
{
    d_ui->mainTabWidget->setCurrentIndex(0);
    d_ui->dependencyTreeWidget->showFile(entry);
}

void MainWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void MainWindow::cancelQuery()
{
  d_ui->dependencyTreeWidget->cancelQuery();
  d_ui->statisticsWindow->cancelQuery();
  d_ui->sentencesWidget->cancelQuery();
  statusBar()->clearMessage();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    switch (e->type())
    {
        case QEvent::LanguageChange:
            d_ui->retranslateUi(this);
            break;

        default:
            break;
    }
}

void MainWindow::checkForUpdates()
{
    if (d_autoUpdater)
        d_autoUpdater->checkForUpdates();
}

void MainWindow::close()
{
    writeSettings();
    QMainWindow::close();
}

void MainWindow::convertCorpus(QString const &convertPath,
    QString const &writePath)
{
    QSharedPointer<ac::CorpusReader> corpusReader;
    try {
        corpusReader = QSharedPointer<ac::CorpusReader>(
            ac::CorpusReaderFactory::open(convertPath.toUtf8().constData()));
    }
    catch (std::runtime_error const &e)
    {
        emit openError(e.what());
        return;
    }

    d_exportProgressDialog->setWindowTitle("Converting corpus");
    d_exportProgressDialog->setLabelText(QString("Writing corpus to:\n%1")
        .arg(writePath));
    d_exportProgressDialog->open();

    QList<QString> files;
    ac::CorpusReader::EntryIterator iter = corpusReader->entries();
    while (iter.hasNext())
        files.push_back(QString::fromUtf8(iter.next(*corpusReader).name.c_str()));

    d_writeCorpusCancelled = false;
    d_exportProgressDialog->setCancelButtonText(tr("Cancel"));

    QFuture<bool> corpusWriterFuture =
        QtConcurrent::run(this, &MainWindow::writeCorpus, writePath, corpusReader, files);
    d_corpusWriteWatcher.setFuture(corpusWriterFuture);

}

void MainWindow::macrosReadError(QString error)
{
    QMessageBox::critical(this, "Error reading macros", error);
}

void MainWindow::saveAs()
{
    switch (d_ui->mainTabWidget->currentIndex())
    {
        case 0:
            d_ui->dependencyTreeWidget->saveAs();
            break;
        case 1:
            d_ui->statisticsWindow->saveAs();
            break;
        case 2:
            d_ui->sentencesWidget->saveAs();
            break;
    }
}

void MainWindow::saveStateChanged()
{
    CorpusWidget *widget = dynamic_cast<CorpusWidget *>(QObject::sender());
    DactMenuBar *menu = dynamic_cast<DactMenuBar *>(menuBar());
    menu->ui()->saveAsAction->setEnabled(widget->saveEnabled());
}

void MainWindow::setToolbarVisible(bool visible)
{
    d_ui->mainToolBar->setVisible(visible);
    d_ui->toolbarAction->setChecked(visible);
}

void MainWindow::showOpenCorpusError(QString const &error)
{
    QMessageBox::critical(this, "Open error", error);
    close();
}

void MainWindow::showWriteCorpusError(QString const &error)
{
    d_exportProgressDialog->accept();
    QMessageBox::critical(this, "Export error", error);
    close();
}

void MainWindow::statisticsEntryActivated(QString const &value, QString const &query)
{
    d_ui->filterComboBox->setText(query);
    filterChanged();
    activateWindow();
}

void MainWindow::setupUi()
{
    d_ui->setupUi(this);

    // Enables the full screen button on the right window corner on OS X >= 10.7.
    enableFullScreen();

    // Move a spacer between the buttons and the inspector action button
    // This will align the inspection action button to the right
    QWidget *spacer = new QWidget(d_ui->mainToolBar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d_ui->mainToolBar->addWidget(spacer);
    d_ui->mainToolBar->addAction(d_ui->inspectorAction);
}

void MainWindow::createActions()
{
    connect(&d_corpusOpenWatcher, SIGNAL(finished()),
        SLOT(corporaRead()));
    connect(this, SIGNAL(corpusReaderCreated()),
        SLOT(corpusRead()));
    connect(this, SIGNAL(corpusWriterFinished(QString const &)),
        SLOT(corpusWritten(QString const &)));

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
    connect(d_ui->sentencesWidget, SIGNAL(entryActivated(QString)),
            SLOT(bracketedEntryActivated(QString)));

    connect(d_ui->statisticsWindow, SIGNAL(saveStateChanged()),
            SLOT(saveStateChanged()));
    connect(d_ui->sentencesWidget, SIGNAL(saveStateChanged()),
            SLOT(saveStateChanged()));
    connect(d_ui->statisticsWindow, SIGNAL(statusMessage(QString)),
            SLOT(statusMessage(QString)));
    connect(d_ui->sentencesWidget, SIGNAL(statusMessage(QString)),
            SLOT(statusMessage(QString)));

    connect(d_ui->filterComboBox->lineEdit(), SIGNAL(textChanged(QString const &)),
        SLOT(applyValidityColor(QString const &)));
    connect(d_ui->filterComboBox, SIGNAL(returnOrClick()),
        SLOT(filterChanged()));
    connect(d_ui->mainTabWidget, SIGNAL(currentChanged(int)),
        SLOT(tabChanged(int)));

    DactMenuBar *menu = dynamic_cast<DactMenuBar *>(menuBar());

    // Actions
    connect(menu->ui()->closeAction, SIGNAL(triggered(bool)),
        SLOT(close()));

    connect(menu->ui()->saveAsAction, SIGNAL(triggered(bool)),
        SLOT(saveAs()));
    if (ac::CorpusWriter::writerAvailable(ac::CorpusWriter::DBXML_CORPUS_WRITER))
      connect(menu->ui()->saveCorpus, SIGNAL(triggered(bool)), SLOT(exportCorpus()));
    else
      menu->ui()->saveCorpus->setDisabled(true);
    connect(menu->ui()->fitAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(fitTree()));
    connect(menu->ui()->nextAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(nextEntry(bool)));
    connect(menu->ui()->pdfExportAction, SIGNAL(triggered(bool)),
        SLOT(exportPDF()));
    connect(menu->ui()->xmlExportAction, SIGNAL(triggered(bool)),
        SLOT(exportXML()));
    connect(menu->ui()->previousAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(previousEntry(bool)));
    connect(menu->ui()->printAction, SIGNAL(triggered(bool)),
        SLOT(print()));
    connect(menu->ui()->zoomInAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomIn()));
    connect(menu->ui()->zoomOutAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomOut()));
    connect(menu->ui()->nextTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusNextTreeNode()));
    connect(menu->ui()->previousTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusPreviousTreeNode()));
    connect(menu->ui()->focusFilterAction, SIGNAL(triggered(bool)),
        SLOT(focusFilter()));
    connect(menu->ui()->focusHighlightAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusHighlight()));
    connect(menu->ui()->filterOnAttributeAction, SIGNAL(triggered()),
        SLOT(filterOnInspectorSelection()));
    connect(d_macrosModel.data(), SIGNAL(readError(QString)),
        SLOT(macrosReadError(QString)));
    connect(menu->ui()->loadMacrosAction, SIGNAL(triggered()),
        SLOT(openMacrosFile()));
    connect(menu->ui()->inspectorAction, SIGNAL(toggled(bool)),
        SLOT(setInspectorVisible(bool)));
    connect(menu->ui()->toolbarAction, SIGNAL(toggled(bool)),
        SLOT(setToolbarVisible(bool)));
    connect(d_ui->mainToolBar, SIGNAL(visibilityChanged(bool)),
        SLOT(setToolbarVisible(bool)));
    connect(menu->ui()->minimizeAction, SIGNAL(triggered()),
        SLOT(showMinimized()));
    connect(menu->ui()->toggleFullScreenAction, SIGNAL(triggered()),
        SLOT(toggleFullScreen()));

    // Toolbar variants
    connect(d_ui->fitAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(fitTree()));
    connect(d_ui->nextAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(nextEntry(bool)));
    connect(d_ui->previousAction, SIGNAL(triggered(bool)),
        d_ui->dependencyTreeWidget, SLOT(previousEntry(bool)));
    connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomIn()));
    connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(zoomOut()));
    connect(d_ui->nextTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusNextTreeNode()));
    connect(d_ui->previousTreeNodeAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusPreviousTreeNode()));

    // XXX: Move to DactMenuBar
    connect(menu->ui()->checkForUpdatesAction, SIGNAL(triggered()),
        SLOT(checkForUpdates()));
    
    new GlobalCopyCommand(menu->ui()->globalCopyAction);
    new GlobalCutCommand(menu->ui()->globalCutAction);
    new GlobalPasteCommand(menu->ui()->globalPasteAction);
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

void MainWindow::initTaintedWidgets()
{
    d_taintedWidgets.push_back(QPair<CorpusWidget *, bool>(d_ui->dependencyTreeWidget, false));
    d_taintedWidgets.push_back(QPair<CorpusWidget *, bool>(d_ui->statisticsWindow, false));
    d_taintedWidgets.push_back(QPair<CorpusWidget *, bool>(d_ui->sentencesWidget, false));
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
    d_ui->filterComboBox->setText(query);
    filterChanged();
}

void MainWindow::filterChanged()
{
    QMutexLocker locker(&d_filterChangedMutex);

    d_filter = d_ui->filterComboBox->text().trimmed();

    taintAllWidgets();
    tabChanged(d_ui->mainTabWidget->currentIndex());
}

void MainWindow::focusFilter()
{
    d_ui->filterComboBox->setFocus();
}

void MainWindow::openMacrosFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open macros file", QString(),
        "Macros file (*.*)");

    if (filePath.isNull())
        return;

    readMacros(QStringList(filePath));

    d_ui->filterComboBox->revalidate();
}

void MainWindow::readMacros(QStringList const &fileNames)
{
    foreach (QString const &fileName, fileNames)
        d_macrosModel->loadFile(fileName);
}

void MainWindow::exportPDF()
{
    QItemSelectionModel *selectionModel = d_ui->dependencyTreeWidget->selectionModel();
    QString entryName = selectionModel->currentIndex().data(Qt::UserRole).toString();
    QFileInfo entryFI(entryName);
    QString pdfFilename = QFileDialog::getSaveFileName(this, "Export to PDF", entryFI.baseName(), "*.pdf");
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

void MainWindow::setInspectorVisible(bool visible)
{
    bool treeWidgetsEnabled = d_ui->mainTabWidget->currentIndex() == 0;
    d_inspectorVisible = visible;

    d_ui->inspector->setVisible(treeWidgetsEnabled && d_inspectorVisible);
    d_ui->inspectorAction->setChecked(visible);
}

void MainWindow::readCorpus(QString const &corpusPath, bool recursive)
{
    return readCorpora(QStringList(corpusPath), recursive);
}

void MainWindow::readCorpora(QStringList const &corpusPaths, bool recursive)
{
    d_ui->dependencyTreeWidget->cancelQuery();
    d_ui->statisticsWindow->cancelQuery();
    d_ui->sentencesWidget->cancelQuery();

    if (d_corpusOpenWatcher.isRunning()) {
        d_corpusOpenWatcher.cancel();
        d_corpusOpenWatcher.waitForFinished();
    }

    QString actionDescription = QString("Opening %1").arg(
        corpusPaths.size() == 1
            ? deriveNameFromPath(corpusPaths[0])
            : QString("%1 corpora").arg(corpusPaths.size()));

    d_openProgressDialog->setWindowTitle(actionDescription);
    d_openProgressDialog->setLabelText(actionDescription);
    d_openProgressDialog->setRange(0, corpusPaths.size());
    d_openProgressDialog->setValue(0);
    d_openProgressDialog->open();

    // Opening a corpus cannot be cancelled, but reading it (iterating the iterator) can.
    d_openProgressDialog->setCancelButton(0);

    QFuture< QPair< ac::CorpusReader*, QString> > corpusOpenFuture = QtConcurrent::run(this, &MainWindow::createCorpusReaders, corpusPaths, recursive);
    d_corpusOpenWatcher.setFuture(corpusOpenFuture);
}

QPair< ac::CorpusReader*, QString> MainWindow::createCorpusReader(QString const &path, bool recursive)
{
    ac::CorpusReader* reader = 0;

    try {
        if (recursive) {
            if (QFileInfo(path).isDir())
              reader = ac::CorpusReaderFactory::openRecursive(path.toUtf8().constData());
            else
              // Do not attempt to open as a recursive corpus.
              reader = ac::CorpusReaderFactory::open(path.toUtf8().constData());
        }
        else
            reader = ac::CorpusReaderFactory::open(path.toUtf8().constData());
    } catch (std::runtime_error const &e) {
        emit openError(e.what());
        return QPair<ac::CorpusReader*, QString>(0, path);
    }

    return QPair<ac::CorpusReader*, QString>(reader, path);
}

QPair< ac::CorpusReader*, QString> MainWindow::createCorpusReaders(QStringList const &paths, bool recursive)
{
    // No need for the multicorpusreader if there is only one corpus to open
    if (paths.size() == 1)
        return createCorpusReader(paths[0], recursive);

    ac::MultiCorpusReader* readers = new ac::MultiCorpusReader();
    int nLoadedCorpora = 0;

    foreach (QString const &path, paths) {
        QFileInfo pathInfo(path);

        if (pathInfo.isDir())
          readers->push_back(deriveNameFromPath(path).toUtf8().constData(), path.toUtf8().constData(), recursive);
        else if (pathInfo.isFile())
          readers->push_back(deriveNameFromPath(path).toUtf8().constData(), path.toUtf8().constData(), false);
        else
        {
          qWarning() << "Corpus is not a file or directory: " << path;
          continue;
        }

        nLoadedCorpora++;
        emit corpusReaderCreated();
    }

    return QPair< ac::CorpusReader*, QString>(readers,
        nLoadedCorpora > 0
            ? QString("%1 corpora").arg(nLoadedCorpora)
            : QString());
}

QString MainWindow::deriveNameFromPath(QString const &path) const
{
    return QFileInfo(path).baseName();
}

void MainWindow::cancelWriteCorpus()
{
    d_writeCorpusCancelled = true;
}

void MainWindow::setCorpusReader(QSharedPointer<ac::CorpusReader> reader, QString const &path)
{
    d_corpusReader = reader;

    d_xpathValidator->setCorpusReader(reader);

    d_ui->filterComboBox->revalidate();

    if (!reader.isNull() && !path.isNull())
    {
        setWindowTitle(QString::fromUtf8("%1%2 \u2014 Dact").arg((path.startsWith("http://") || path.startsWith("https://")) ? "remote: " : "",
                                                             QFileInfo(path).fileName()));

        // On OS X, add the file icon to the window (and try alt-clicking it!)
        setWindowFilePath(path);

        if (QFileInfo(path).exists() || path.startsWith("http://") || path.startsWith("https://"))
            // Add file to the recent files menu
            dynamic_cast<DactMenuBar *>(menuBar())->addRecentFile(path);
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

    taintAllWidgets();
    tabChanged(d_ui->mainTabWidget->currentIndex());
}

void MainWindow::corporaRead()
{
    d_openProgressDialog->accept();

    QPair<ac::CorpusReader*, QString> result(d_corpusOpenWatcher.result());

    if (result.first == 0) {
        return;
    }

    setCorpusReader(QSharedPointer<ac::CorpusReader>(result.first), result.second);
}

void MainWindow::corpusRead()
{
    int v = d_openProgressDialog->value();
    d_openProgressDialog->setValue(v + 1);    
}

void MainWindow::corpusWritten(QString const &filename)
{
    d_exportProgressDialog->accept();
    readCorpus(filename);
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
    d_inspectorVisible = settings.value("inspectorVisible", true).toBool();
    d_ui->inspectorAction->setChecked(d_inspectorVisible);

    bool toolbarVisible = settings.value("toolbarVisible", true).toBool();
    d_ui->toolbarAction->setChecked(toolbarVisible);

    bool treeWidgetsEnabled = d_ui->mainTabWidget->currentIndex() == 0;
    d_ui->inspector->setVisible(treeWidgetsEnabled && d_inspectorVisible);

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
        QString("untitled"), "*.dact"));

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
        {
            ac::CorpusReader::EntryIterator iter = d_corpusReader->entries();
            while (iter.hasNext())
                files.push_back(QString::fromUtf8(iter.next(*d_corpusReader).name.c_str()));
            
        }

        d_writeCorpusCancelled = false;
        d_exportProgressDialog->setCancelButtonText(tr("Cancel"));

        QFuture<bool> corpusWriterFuture = QtConcurrent::run(
          this, &MainWindow::writeCorpus, filename, d_corpusReader, files);
        d_corpusWriteWatcher.setFuture(corpusWriterFuture);
    }
}

bool MainWindow::writeCorpus(QString const &filename,
    QSharedPointer<ac::CorpusReader> corpusReader,
    QList<QString> const &files)
{
#if defined(USE_SANDBOXING)
    QTemporaryFile tmpFile;
    // Ensure the temp file name is created.
    tmpFile.open();
    tmpFile.close();

    QString const writeTarget = tmpFile.fileName();
#else
    QString const writeTarget = filename;
#endif // defined(USE_SANDBOXING)


    try {
        QSharedPointer<ac::CorpusWriter> corpus(
          ac::CorpusWriter::open(writeTarget.toUtf8().constData(), true,
          ac::CorpusWriter::DBXML_CORPUS_WRITER));

        emit exportProgressMaximum(files.size());
        emit exportProgress(0);
        int percent = files.size() / 100;
        int progress = 0;

        for (QList<QString>::const_iterator itr(files.constBegin()),
             end(files.constEnd());
             !d_writeCorpusCancelled && itr != end; ++itr)
        {
            corpus->write(itr->toUtf8().constData(), corpusReader->read(itr->toUtf8().constData()));
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

#if defined(USE_SANDBOXING)
    bool failed = false;
    QFile::remove(filename);
    if (!QFile::rename(writeTarget, filename))
      if (!QFile::copy(writeTarget, filename))
        failed = true;
    
    QFile::remove(writeTarget);

    if (failed)
      emit exportError(QString("Could not move temporary file %1 to %2")
          .arg(writeTarget).arg(filename));
      return false;
#endif

    emit corpusWriterFinished(filename);

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
    settings.setValue("inspectorVisible", d_inspectorVisible);

    // Toolbar
    settings.setValue("toolbarVisible", d_ui->mainToolBar->isVisible());

    d_ui->dependencyTreeWidget->writeSettings();
}

void MainWindow::tabChanged(int index)
{
    bool treeWidgetsEnabled = index == 0 ? true : false;

    DactMenuBar *menu = dynamic_cast<DactMenuBar *>(menuBar());
    menu->ui()->previousAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->nextAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->zoomInAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->zoomOutAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->fitAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->nextTreeNodeAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->previousTreeNodeAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->xmlExportAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->pdfExportAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->printAction->setEnabled(treeWidgetsEnabled);
    if (ac::CorpusWriter::writerAvailable(ac::CorpusWriter::DBXML_CORPUS_WRITER))
      menu->ui()->saveCorpus->setEnabled(treeWidgetsEnabled);
    menu->ui()->focusHighlightAction->setEnabled(treeWidgetsEnabled);
    menu->ui()->inspectorAction->setEnabled(treeWidgetsEnabled);

    d_ui->previousAction->setEnabled(treeWidgetsEnabled);
    d_ui->nextAction->setEnabled(treeWidgetsEnabled);
    d_ui->zoomInAction->setEnabled(treeWidgetsEnabled);
    d_ui->zoomOutAction->setEnabled(treeWidgetsEnabled);
    d_ui->fitAction->setEnabled(treeWidgetsEnabled);
    d_ui->nextTreeNodeAction->setEnabled(treeWidgetsEnabled);
    d_ui->previousTreeNodeAction->setEnabled(treeWidgetsEnabled);

    // Hide inspector on other tabs
    d_ui->inspectorAction->setEnabled(treeWidgetsEnabled);
    d_ui->inspector->setVisible(treeWidgetsEnabled && d_inspectorVisible);

    Q_ASSERT(index < d_taintedWidgets.size());

    if (d_taintedWidgets[index].second)
    {
        d_taintedWidgets[index].first->setFilter(d_macrosModel->expand(d_filter), d_filter);
        d_taintedWidgets[index].second = false;
    }

    menu->ui()->saveAsAction->setEnabled(d_taintedWidgets[index].first->saveEnabled());
}

void MainWindow::taintAllWidgets()
{
    for (QVector<QPair<CorpusWidget *, bool> >::iterator iter = d_taintedWidgets.begin();
            iter != d_taintedWidgets.end(); ++iter)
        iter->second = true;
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

void MainWindow::statusMessage(QString message)
{
    statusBar()->showMessage(message, 4000);
}

void MainWindow::enableFullScreen()
{
#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    enableFullScreenOnMac(this);
#endif
}

void MainWindow::toggleFullScreen()
{
#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    toggleFullScreenOnMac(this);
#else
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
#endif
}

void MainWindow::makeActiveWindow()
{
    activateWindow();
    raise();
}

