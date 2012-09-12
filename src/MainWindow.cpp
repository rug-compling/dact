#include <iostream>

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QHash>
#include <QItemSelection>
#include <QLineEdit>
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
#include <QUrl>
#include <QtConcurrentRun>

#include <algorithm>
#include <iterator>
#include <list>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/MultiCorpusReader.hh>
#include <AlpinoCorpus/Entry.hh>
#include <AlpinoCorpus/Error.hh>

#include <config.hh>

#include <AboutWindow.hh>
#include <AppleUtils.hh>
#ifdef USE_WEBSERVICE
#include <WebserviceWindow.hh>
#endif // USE_WEBSERVICE
#ifdef USE_REMOTE_CORPUS
#include <RemoteWindow.hh>
#endif // USE_REMOTE_CORPUS
#include <OpenCorpusDialog.hh>
#include <MainWindow.hh>
#include <BracketedWindow.hh>
#include <CorpusWidget.hh>
#include <DactMacrosModel.hh>
//#include <DactQueryHistory.hh>
#include <PreferencesWindow.hh>
#include <StatisticsWindow.hh>
#include <DactTreeScene.hh>
#include <TreeNode.hh>
#include <XPathValidator.hh>
#include <XSLTransformer.hh>
#include <ValidityColor.hh>
#include <ui_MainWindow.h>
#include <Query.hh>

#include <GlobalCopyCommand.hh>
#include <GlobalCutCommand.hh>
#include <GlobalPasteCommand.hh>

#if defined(Q_WS_MAC) && defined(USE_SPARKLE)
#include <SparkleAutoUpdater.hh>
#endif

#ifdef Q_WS_MAC
extern void qt_mac_set_dock_menu(QMenu *);
#endif

namespace ac = alpinocorpus;

typedef std::list<ac::CorpusReaderFactory::ReaderInfo> ReaderList;
typedef std::list<std::string> ExtList;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(QSharedPointer<Ui::MainWindow>(new Ui::MainWindow)),
    d_aboutWindow(new AboutWindow(this, Qt::Window)),
#ifdef USE_WEBSERVICE
    d_webserviceWindow(0),
#endif // USE_WEBSERVICE
#ifdef USE_REMOTE_CORPUS
    d_remoteWindow(0),
#endif // USE_REMOTE_CORPUS
    d_openProgressDialog(new QProgressDialog(this)),
    d_exportProgressDialog(new QProgressDialog(this)),
    d_preferencesWindow(0)
{
    setupUi();

#ifndef USE_WEBSERVICE
    d_ui->menuTools->removeAction(d_ui->webserviceAction);
#endif

#ifndef USE_REMOTE_CORPUS
    d_ui->menuFile->removeAction(d_ui->remoteAction);
#endif

    d_ui->filterComboBox->readHistory("filterHistory");

    initTaintedWidgets();

    // Ensure that we have a status bar.
    statusBar();

    d_macrosModel = QSharedPointer<DactMacrosModel>(new DactMacrosModel());

    d_ui->menuMacros->setModel(d_macrosModel);

    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    d_ui->filterComboBox->lineEdit()->setValidator(d_xpathValidator.data());

    // the dependency tree widget needs to know about our macros model
    // for the highlight queries and query validation for that same field.
    d_ui->dependencyTreeWidget->setMacrosModel(d_macrosModel);

    readSettings();

    initSentenceTransformer();

    createActions();

    d_ui->saveAsAction->setEnabled(false);

#if defined(Q_WS_MAC) && defined(USE_SPARKLE)
    d_autoUpdater = QSharedPointer<SparkleAutoUpdater>(new SparkleAutoUpdater("http://localhost/appcast.xml"));
#endif
}

MainWindow::~MainWindow()
{
    d_ui->filterComboBox->writeHistory("filterHistory");

    delete d_aboutWindow;
#ifdef USE_WEBSERVICE
    delete d_webserviceWindow;
#endif // USE_WEBSERVICE
#ifdef USE_REMOTE_CORPUS
    delete d_remoteWindow;
#endif // USE_REMOTE_CORPUS
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
    switch (e->type()) {
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

void MainWindow::clearQueryHistory()
{
    d_ui->filterComboBox->clearHistory();
}

void MainWindow::close()
{
    writeSettings();
    QMainWindow::close();
}

void MainWindow::convertCompactCorpus()
{

    QString corpusPath = QFileDialog::getOpenFileName(this, "Open compact corpus", QString(),
        QString("Compact corpora (*.data.dz)"));
    if (corpusPath.isNull())
        return;

    convertCorpus(corpusPath);
}

void MainWindow::convertCorpus(QString const &convertPath)
{
    QString newPath(QFileDialog::getSaveFileName(this,
        "New Dact corpus", QString(), "*.dact"));

    if (newPath.isNull())
        return;

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
        .arg(newPath));
    d_exportProgressDialog->open();

    QList<QString> files;
    ac::CorpusReader::EntryIterator iter = corpusReader->entries();
    while (iter.hasNext())
        files.push_back(QString::fromUtf8(iter.next(*corpusReader).name.c_str()));

    d_writeCorpusCancelled = false;
    d_exportProgressDialog->setCancelButtonText(tr("Cancel"));

    QFuture<bool> corpusWriterFuture =
        QtConcurrent::run(this, &MainWindow::writeCorpus, newPath, corpusReader, files);
    d_corpusWriteWatcher.setFuture(corpusWriterFuture);

}

void MainWindow::convertDirectoryCorpus()
{
    QString corpusPath = QFileDialog::getExistingDirectory(this,
        "Open directory corpus");
    if (corpusPath.isNull())
        return;

    convertCorpus(corpusPath);
}

void MainWindow::macrosReadError(QString error)
{
    QMessageBox::critical(this, "Error reading macros", error);
}

#ifdef USE_WEBSERVICE
void MainWindow::showWebserviceWindow()
{
    if (d_webserviceWindow == 0)
    {
        d_webserviceWindow = new WebserviceWindow(this, Qt::Window);
        d_webserviceWindow->setWindowModality(Qt::WindowModal);

        // When parsing is finished and the trees are received, load the freshly
        // created corpus.
        connect(d_webserviceWindow,
            SIGNAL(parseSentencesFinished(QString)),
            SLOT(readCorpus(QString)));
    }

    d_webserviceWindow->show();
    d_webserviceWindow->raise();
}
#endif // USE_WEBSERVICE

void MainWindow::saveAs()
{
    switch (d_ui->mainTabWidget->currentIndex()) {
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
    CorpusWidget *widget = dynamic_cast<CorpusWidget *>(
       QObject::sender());
    d_ui->saveAsAction->setEnabled(widget->saveEnabled());
}

void MainWindow::setToolbarVisible(bool visible)
{
    d_ui->mainToolBar->setVisible(visible);
    d_ui->toolbarAction->setChecked(visible);
}

#ifdef USE_REMOTE_CORPUS
void MainWindow::showRemoteWindow()
{
    if (d_remoteWindow == 0) {
        d_remoteWindow = new RemoteWindow(this, Qt::Window);
        connect(d_remoteWindow, SIGNAL(openRemote(QString const &)),
                this, SLOT(openRemoteCorpus(QString const &)));
    }

    d_remoteWindow->show();
    d_remoteWindow->raise();
}

void MainWindow::openRemoteCorpus(QString const &url)
{
    readCorpus(url);
}
#endif // USE_REMOTE_CORPUS

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

    #ifdef Q_WS_MAC
        QMenu *appleDockMenu = new QMenu(this);
        appleDockMenu->addAction(d_ui->openAction);
        appleDockMenu->addAction(d_ui->remoteAction);
        qt_mac_set_dock_menu(appleDockMenu);
    #endif
}

void MainWindow::createActions()
{
    connect(&d_corpusOpenWatcher, SIGNAL(finished()),
        SLOT(corporaRead()));
    connect(this, SIGNAL(corpusReaderCreated()),
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
//    connect(d_ui->filterComboBox, SIGNAL(activated(QString const &)),
//        SLOT(filterChanged()));
    connect(d_ui->filterComboBox, SIGNAL(returnOrClick()),
        SLOT(filterChanged()));
    //connect(d_ui->filterComboBox->lineEdit(), SIGNAL(returnPressed()),
    //    SLOT(filterChanged()));
    connect(d_ui->mainTabWidget, SIGNAL(currentChanged(int)),
        SLOT(tabChanged(int)));

    // Actions
    connect(d_ui->aboutAction, SIGNAL(triggered(bool)),
        SLOT(aboutDialog()));
#ifdef USE_REMOTE_CORPUS
    connect(d_ui->remoteAction, SIGNAL(triggered(bool)),
        SLOT(showRemoteWindow()));
#endif // USE_REMOTE_CORPUS
    connect(d_ui->openAction, SIGNAL(triggered(bool)),
        SLOT(openCorpus()));
    connect(d_ui->menuRecentFiles, SIGNAL(fileSelected(QString)),
        SLOT(readCorpus(QString)));
    connect(d_ui->saveAsAction, SIGNAL(triggered(bool)),
        SLOT(saveAs()));
    if (ac::CorpusWriter::writerAvailable(ac::CorpusWriter::DBXML_CORPUS_WRITER))
      connect(d_ui->saveCorpus, SIGNAL(triggered(bool)),
          SLOT(exportCorpus()));
    else
      d_ui->saveCorpus->setDisabled(true);
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
    connect(d_ui->focusFilterAction, SIGNAL(triggered(bool)),
        SLOT(focusFilter()));
    connect(d_ui->focusHighlightAction, SIGNAL(triggered(bool)), d_ui->dependencyTreeWidget,
        SLOT(focusHighlight()));
    connect(d_ui->filterOnAttributeAction, SIGNAL(triggered()),
        SLOT(filterOnInspectorSelection()));
    connect(d_macrosModel.data(), SIGNAL(readError(QString)),
        SLOT(macrosReadError(QString)));
    connect(d_ui->loadMacrosAction, SIGNAL(triggered()),
        SLOT(openMacrosFile()));
    connect(d_ui->toolbarAction, SIGNAL(toggled(bool)),
        SLOT(setToolbarVisible(bool)));
    connect(d_ui->mainToolBar, SIGNAL(visibilityChanged(bool)),
        SLOT(setToolbarVisible(bool)));
    #ifdef USE_WEBSERVICE
    connect(d_ui->webserviceAction, SIGNAL(triggered()),
        SLOT(showWebserviceWindow()));
    #endif // USE_WEBSERVICE
    connect(d_ui->convertCompactCorpusAction, SIGNAL(triggered()),
        SLOT(convertCompactCorpus()));
    connect(d_ui->convertDirectoryCorpusAction, SIGNAL(triggered()),
        SLOT(convertDirectoryCorpus()));
    
    new GlobalCopyCommand(d_ui->globalCopyAction);
    new GlobalCutCommand(d_ui->globalCutAction);
    new GlobalPasteCommand(d_ui->globalPasteAction);
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

void MainWindow::help()
{
    static QUrl const usage("http://rug-compling.github.com/dact/manual/");
    QDesktopServices::openUrl(usage);
}

void MainWindow::openCookbook()
{
    static QUrl const cookbook("http://rug-compling.github.com/dact/manual/cookbook.xhtml");
    QDesktopServices::openUrl(cookbook);
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

#if 0
    if (d_queryHistory)
        d_queryHistory->addToHistory(d_filter);
#endif


    taintAllWidgets();
    tabChanged(d_ui->mainTabWidget->currentIndex());
}

void MainWindow::focusFilter()
{
    d_ui->filterComboBox->setFocus();
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
    QString corpusPath = OpenCorpusDialog::getCorpusFileName(this);
    
    if (corpusPath.isNull())
        return;

    QFileInfo fi(corpusPath);

    if (fi.isDir())
        readCorpus(corpusPath, true);
    else
        readCorpus(corpusPath);
}

void MainWindow::openMacrosFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open macros file", QString(),
        "Macros file (*.*)");

    if (filePath.isNull())
        return;

    d_macrosModel->loadFile(filePath);

    d_ui->filterComboBox->revalidate();
}

void MainWindow::readMacros(QStringList const &fileNames)
{
    foreach (QString const &fileName, fileNames)
        d_macrosModel->loadFile(fileName);
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
    {
        d_preferencesWindow = new PreferencesWindow(this);

        // Propagate preference changes...
        connect(d_preferencesWindow, SIGNAL(colorChanged()),
                d_ui->dependencyTreeWidget->sentenceWidget(), SLOT(colorChanged()));
        connect(d_preferencesWindow, SIGNAL(colorChanged()),
                d_ui->sentencesWidget, SLOT(colorChanged()));
    }

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
    }

    return QPair< ac::CorpusReader*, QString >(reader, path);
}

QPair< ac::CorpusReader*, QString> MainWindow::createCorpusReaders(QStringList const &paths, bool recursive)
{
    // No need for the multicorpusreader if there is only one corpus to open
    if (paths.size() == 1)
        return createCorpusReader(paths[0], recursive);

    ac::MultiCorpusReader* readers = new ac::MultiCorpusReader();
    int nLoadedCorpora = 0;

    foreach (QString const &path, paths) {
        if (QFileInfo(path).isDir())
          readers->push_back(deriveNameFromPath(path).toUtf8().constData(), path.toUtf8().constData(), recursive);
        else
          readers->push_back(deriveNameFromPath(path).toUtf8().constData(), path.toUtf8().constData(), false);
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
        setWindowTitle(QString::fromUtf8("%1%2 -- Dact").arg((path.startsWith("http://") || path.startsWith("https://")) ? "remote: " : "",
                                                             QFileInfo(path).fileName()));

        // On OS X, add the file icon to the window (and try alt-clicking it!)
        setWindowFilePath(path);

        if (QFileInfo(path).exists() || path.startsWith("http://") || path.startsWith("https://"))
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

    taintAllWidgets();
    tabChanged(d_ui->mainTabWidget->currentIndex());
}

void MainWindow::corporaRead()
{
    d_openProgressDialog->accept();

    QPair<ac::CorpusReader*, QString> result(d_corpusOpenWatcher.result());

    setCorpusReader(QSharedPointer<ac::CorpusReader>(result.first), result.second);
}

void MainWindow::corpusRead()
{
    int v = d_openProgressDialog->value();
    d_openProgressDialog->setValue(v + 1);    
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
        {
            ac::CorpusReader::EntryIterator iter = d_corpusReader->entries();
            while (iter.hasNext())
                files.push_back(QString::fromUtf8(iter.next(*d_corpusReader).name.c_str()));
            
        }

        d_writeCorpusCancelled = false;
        d_exportProgressDialog->setCancelButtonText(tr("Cancel"));

        QFuture<bool> corpusWriterFuture =
            QtConcurrent::run(this, &MainWindow::writeCorpus, filename, d_corpusReader, files);
        d_corpusWriteWatcher.setFuture(corpusWriterFuture);
    }
}

bool MainWindow::writeCorpus(QString const &filename,
    QSharedPointer<ac::CorpusReader> corpusReader,
    QList<QString> const &files)
{
    try {
        QSharedPointer<ac::CorpusWriter> corpus(
          ac::CorpusWriter::open(filename.toUtf8().constData(), true,
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

    // Hide inspector on other tabs
    d_ui->inspectorAction->setEnabled(treeWidgetsEnabled);
    d_ui->inspector->setVisible(treeWidgetsEnabled && d_inspectorVisible);

    Q_ASSERT(index < d_taintedWidgets.size());

    if (d_taintedWidgets[index].second)
    {
        d_taintedWidgets[index].first->setFilter(d_macrosModel->expand(d_filter), d_filter);
        d_taintedWidgets[index].second = false;
    }

    d_ui->saveAsAction->setEnabled(d_taintedWidgets[index].first->saveEnabled());
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
