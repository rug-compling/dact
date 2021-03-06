#include <iostream>

#include <stdexcept>

#include <QClipboard>
#include <QMessageBox>
#include <QSettings>
#include <QSharedPointer>
#include <QWidget>
#include <QFile>

#include <CorpusWidget.hh>
#include <DactMacrosModel.hh>
#include <DactToolsMenu.hh>
#include <DactToolsModel.hh>
#include <DactTreeScene.hh>
#include <DactTreeView.hh>
#include <DependencyTreeWidget.hh>
#include <FilterModel.hh>
#include <ValidityColor.hh>
#include <XPathValidator.hh>

#include "ui_DependencyTreeWidget.h"

// Note that there is a race in this class, when a new filter is set.
// d_model->runQuery may cancel an existing query, delivering a queryStopped()
// signal. However, at that point d_filter is already reset. Since queries
// run in a separate thread, we cannot connect signals directly.
//
// Consequence: don't rely on d_filter if the code path is treating stopping
// of an older query.


DependencyTreeWidget::DependencyTreeWidget(QWidget *parent) :
    CorpusWidget(parent),
    d_ui(new Ui::DependencyTreeWidget),
    d_macrosModel(QSharedPointer<DactMacrosModel>(new DactMacrosModel()))
{
    d_ui->setupUi(this);
    
    addConnections();

    // Statistics are only shown after we have all entries, or when a
    // query is executed...
    d_ui->statisticsGroupBox->hide();

    d_ui->hitsDescLabel->hide();
    d_ui->hitsLabel->hide();
    d_ui->statisticsLayout->setVerticalSpacing(0);
}

DependencyTreeWidget::~DependencyTreeWidget()
{
    // Define a destructor here to make sure the qsharedpointers are implemented where all
    // the proper header files are available (not just forward declarations)
}

void DependencyTreeWidget::addConnections()
{
    connect(d_ui->highlightLineEdit, SIGNAL(textChanged(QString const &)),
            SLOT(applyValidityColor(QString const &)));
    connect(d_ui->highlightLineEdit, SIGNAL(returnPressed()),
            SLOT(highlightChanged()));    
    connect(d_ui->treeGraphicsView, SIGNAL(sceneChanged(DactTreeScene*)),
            SIGNAL(sceneChanged(DactTreeScene*)));
}

void DependencyTreeWidget::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

BracketedSentenceWidget *DependencyTreeWidget::sentenceWidget()
{
    return d_ui->sentenceWidget;
}


void DependencyTreeWidget::cancelQuery()
{
    if (d_model)
        d_model->cancelQuery();
}

void DependencyTreeWidget::saveAs()
{
    std::cerr << "Dependency Tree Widget Save As" << std::endl;
}

bool DependencyTreeWidget::saveEnabled() const
{
    return false;
}

void DependencyTreeWidget::copy()
{
    if (!d_model)
        return;
    
    QStringList filenames;
    
    QModelIndexList indices = d_ui->fileListWidget->selectionModel()->selectedIndexes();
    
    for (QModelIndexList::const_iterator iter = indices.begin();
        iter != indices.end(); ++iter)
    {
        QVariant v = d_model->data(*iter, Qt::DisplayRole);
        if (v.type() == QVariant::String)
            filenames.push_back(v.toString());
    }

    QApplication::clipboard()->setText(filenames.join("\n")); // XXX - Good enough for Windows?
}

void DependencyTreeWidget::nEntriesFound(int entries, int hits) {
    d_ui->entriesLabel->setText(QString("%L1").arg(entries));
    d_ui->hitsLabel->setText(QString("%L1").arg(hits));
    
    if (!d_treeShown) {
        d_ui->fileListWidget->selectionModel()->clear();
        QModelIndex idx(d_model->index(0, 0));
        d_ui->fileListWidget->selectionModel()->setCurrentIndex(idx,
                                                                QItemSelectionModel::ClearAndSelect);
        d_treeShown = true;
    }
}

void DependencyTreeWidget::entrySelected(QModelIndex const &current, QModelIndex const &prev)
{
    Q_UNUSED(prev);
    
    if (!current.isValid()) {
        d_ui->treeGraphicsView->setScene(0);
        d_ui->sentenceWidget->clear();
        return;
    }
    
    showFile(current.data(Qt::UserRole).toString());
    
    //focusFitTree();
    focusFirstMatch();
}

void DependencyTreeWidget::fitTree()
{
    d_ui->treeGraphicsView->fitTree();
}

void DependencyTreeWidget::focusFirstMatch()
{
    if (d_ui->treeGraphicsView->scene() &&
        d_ui->treeGraphicsView->scene()->activeNodes().length() > 0)
      d_ui->treeGraphicsView->focusTreeNode(1);
}

void DependencyTreeWidget::focusFitTree()
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

void DependencyTreeWidget::focusHighlight()
{
    d_ui->highlightLineEdit->setFocus();
}

void DependencyTreeWidget::focusNextTreeNode()
{
    d_ui->treeGraphicsView->focusNextTreeNode();
}

void DependencyTreeWidget::focusPreviousTreeNode()
{
    d_ui->treeGraphicsView->focusPreviousTreeNode();
}

void DependencyTreeWidget::highlightChanged()
{
    setHighlight(d_ui->highlightLineEdit->text().trimmed());
}

void DependencyTreeWidget::mapperStarted(int totalEntries)
{
    d_ui->entriesLabel->setText(QString::number(0));
    d_ui->hitsLabel->setText(QString::number(0));
    
    if (!d_filter.isEmpty()) {
        d_ui->filterProgressBar->setMinimum(0);
        d_ui->filterProgressBar->setMaximum(totalEntries);
        d_ui->filterProgressBar->setValue(0);
        d_ui->filterProgressBar->setVisible(true);
    }
}

void DependencyTreeWidget::mapperFailed(QString error)
{
    d_ui->filterProgressBar->setVisible(false);
    QMessageBox::critical(this, tr("Error processing query"),
                          tr("Could not process query: ") + error,
                          QMessageBox::Ok);
}

void DependencyTreeWidget::mapperFinished(int processedEntries, int totalEntries, bool cached)
{
    if (cached) {
        d_ui->fileListWidget->selectionModel()->clear();
        QModelIndex idx(d_model->index(0, 0));
        d_ui->fileListWidget->selectionModel()->setCurrentIndex(idx,
            QItemSelectionModel::ClearAndSelect);
    }
    
    mapperStopped(processedEntries, totalEntries);
}

void DependencyTreeWidget::mapperStopped(int processedEntries, int totalEntries)
{    
    d_ui->filterProgressBar->setVisible(false);
    
    // Final counts. Doing this again is necessary, because the query may
    // have been cached. If so, it doesn't emit a signal for every entry.
    int entries = d_model->rowCount(QModelIndex());
    int hits = d_model->hits();
    d_ui->entriesLabel->setText(QString("%L1").arg(entries));
    d_ui->hitsLabel->setText(QString("%L1").arg(hits));
    
    if (!d_file.isNull())
    {
        QModelIndex current = d_model->indexOfFile(d_file);
        d_ui->fileListWidget->setCurrentIndex(current);
    }
}

/* Next- and prev entry buttons */

void DependencyTreeWidget::nextEntry(bool)
{
    QModelIndex current(d_ui->fileListWidget->currentIndex());
    QModelIndex next = current.sibling(current.row() + 1, current.column());
    if (next.isValid())
        d_ui->fileListWidget->setCurrentIndex(next);
}

void DependencyTreeWidget::previousEntry(bool)
{
    QModelIndex current(d_ui->fileListWidget->currentIndex());
    QModelIndex previous = current.sibling(current.row() - 1, current.column());
    if (previous.isValid())
        d_ui->fileListWidget->setCurrentIndex(previous);
}

void DependencyTreeWidget::progressChanged(int progress)
{
    d_ui->filterProgressBar->setValue(progress);
}

void DependencyTreeWidget::readSettings()
{
    QSettings settings;
    
    // Splitter.
    d_ui->splitter->restoreState(
        settings.value("splitterSizes").toByteArray());
}

void DependencyTreeWidget::renderTree(QPainter *painter)
{
    if (d_ui->treeGraphicsView->scene())
        d_ui->treeGraphicsView->scene()->render(painter);
}

DactTreeScene *DependencyTreeWidget::scene()
{
    return d_ui->treeGraphicsView->scene();
}

QItemSelectionModel *DependencyTreeWidget::selectionModel()
{
    return d_ui->fileListWidget->selectionModel();
}

void DependencyTreeWidget::setFilter(QString const &filter, QString const &raw_filter)
{
    d_treeShown = false;
    d_file = QString();
    
    if (filter.isEmpty()) {
        d_ui->statisticsGroupBox->hide();
        d_ui->hitsDescLabel->hide();
        d_ui->hitsLabel->hide();
        d_ui->statisticsLayout->setVerticalSpacing(0);
    } else {
        d_ui->statisticsGroupBox->show();
        d_ui->statisticsLayout->setVerticalSpacing(-1);
        d_ui->hitsDescLabel->show();
        d_ui->hitsLabel->show();
    }

    setHighlight(raw_filter);

    if (d_model)
        d_model->runQuery(filter);

    d_filter = filter;
}

void DependencyTreeWidget::setModel(QSharedPointer<FilterModel> model)
{
    d_model = model;
    d_ui->fileListWidget->setModel(d_model.data());
    
    connect(d_model.data(), SIGNAL(queryFailed(QString)),
            SLOT(mapperFailed(QString)));
    connect(d_model.data(), SIGNAL(queryStarted(int)),
            SLOT(mapperStarted(int)));
    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
            SLOT(mapperStopped(int, int)));
    connect(d_model.data(), SIGNAL(queryFinished(int, int, bool)),
            SLOT(mapperFinished(int, int, bool)));
    connect(d_model.data(), SIGNAL(nEntriesFound(int, int)),
            SLOT(nEntriesFound(int, int)));
    connect(d_model.data(), SIGNAL(progressChanged(int)),
            SLOT(progressChanged(int)));
    
    connect(d_ui->fileListWidget->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(entrySelected(QModelIndex,QModelIndex)));
}

void DependencyTreeWidget::setMacrosModel(QSharedPointer<DactMacrosModel> macrosModel)
{
    d_macrosModel = macrosModel;
    d_xpathValidator = QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel));
    d_ui->highlightLineEdit->setValidator(d_xpathValidator.data());
}

void DependencyTreeWidget::setHighlight(QString const &query)
{
    // Get the last component of the query pipeline
    QStringList queryParts = query.split("+|+");
    d_highlight = queryParts[queryParts.size() - 1].trimmed();
    d_ui->highlightLineEdit->setText(d_highlight);
    showFile(); // to force-reload the tree and bracketed sentence
}

void DependencyTreeWidget::showFile()
{
    if (!d_file.isNull())
        showFile(d_file);
}

void DependencyTreeWidget::showFile(QString const &entry)
{    
    // Read XML data.
    if (d_corpusReader.isNull())
        return;
    
    try {
        QString xml;
        if (d_highlight.trimmed().isEmpty())
            xml = QString::fromUtf8(d_corpusReader->read(entry.toUtf8().constData()).c_str());
        else {
            ac::CorpusReader::MarkerQuery query(d_macrosModel->expand(d_highlight).toUtf8().constData(),
                                                "active", "1");
            std::list<ac::CorpusReader::MarkerQuery> queries;
            queries.push_back(query);
            xml = QString::fromUtf8(d_corpusReader->read(entry.toUtf8().constData(), queries).c_str());
        }
        
        if (xml.size() == 0) {
            qWarning() << "MainWindow::writeSettings: empty XML data!";
            d_ui->treeGraphicsView->setScene(0);
            return;
        }
        
        // Remember file for when we need to redraw the tree
        d_file = entry;
        
        // Parameters
        QString valStr = d_highlight.trimmed().isEmpty() ? "'/..'" :
        QString("'") + d_macrosModel->expand(d_highlight) + QString("'");
        QHash<QString, QString> params;
        params["expr"] = valStr;
        
        try {
            showTree(xml);
            showSentence(entry, d_highlight);
            
            // I try to find my file back in the file list to keep the list
            // in sync with the treegraph since showFile can be called from
            // the child dialogs.
        
            QModelIndexList matches =
              d_ui->fileListWidget->model()->match(
                  d_ui->fileListWidget->model()->index(0, 0),
                  Qt::DisplayRole, entry, 1,
                  Qt::MatchFixedString | Qt::MatchCaseSensitive);
            if (matches.size() > 0) {
              d_ui->fileListWidget->selectionModel()->select(matches.at(0),
                  QItemSelectionModel::ClearAndSelect);
              d_ui->fileListWidget->scrollTo(matches.at(0));
            }
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

void DependencyTreeWidget::showSentence(QString const &entry, QString const &query)
{
    d_ui->sentenceWidget->setEntry(entry, d_macrosModel->expand(query));
}

void DependencyTreeWidget::showTree(QString const &xml)
{
    d_ui->treeGraphicsView->showTree(xml);
}

void DependencyTreeWidget::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;

    QString stylesheetFilename = ":/stylesheets/tree.xsl";
    if (d_corpusReader->type() == "tueba_tree")
        stylesheetFilename = ":/stylesheets/tueba-tree.xsl";
    else if (d_corpusReader->type() == "conllx_ds")
        stylesheetFilename = ":/stylesheets/conllx_ds.xsl";

    QFile stylesheet(stylesheetFilename);
    d_ui->treeGraphicsView->setStylesheet(&stylesheet);
    
    d_xpathValidator->setCorpusReader(d_corpusReader);  
    
    setModel(QSharedPointer<FilterModel>(new FilterModel(d_corpusReader)));
    
    QString query = d_ui->highlightLineEdit->text();
    d_ui->highlightLineEdit->clear();
    d_ui->highlightLineEdit->insert(query);
    
    d_model->runQuery(d_macrosModel->expand(d_filter));

    d_ui->sentenceWidget->setCorpusReader(d_corpusReader);
}

void DependencyTreeWidget::writeSettings()
{
    QSettings settings;
    
    // Splitter
    settings.setValue("splitterSizes", d_ui->splitter->saveState());
}

void DependencyTreeWidget::zoomIn()
{
    d_ui->treeGraphicsView->zoomIn();
}

void DependencyTreeWidget::zoomOut()
{
    d_ui->treeGraphicsView->zoomOut();
}

void DependencyTreeWidget::showToolMenu(QPoint const &position)
{
    if (!d_ui->fileListWidget->model())
        return;

    QModelIndexList rows = d_ui->fileListWidget->selectionModel()->selectedRows();
    QList<QString> selectedFiles;

    if (rows.isEmpty())
        return;

    foreach (QModelIndex const &row, rows)
        selectedFiles << row.data().toString();

    DactToolsMenu::exec(
        DactToolsModel::sharedInstance()->tools(QString::fromStdString(d_corpusReader->name())),
        selectedFiles,
        mapToGlobal(position),
        d_ui->fileListWidget->actions());
}
