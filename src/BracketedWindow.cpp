#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QTextStream>
#include <QVector>
#include <XSLTransformer.hh>

#include <stdexcept>
#include <typeinfo>

#include "BracketedDelegate.hh"
#include "BracketedColorDelegate.hh"
#include "BracketedKeywordInContextDelegate.hh"
#include "BracketedVisibilityDelegate.hh"
#include "BracketedWindow.hh"
#include "CorpusWidget.hh"
#include "DactMacrosModel.hh"
#include "DactToolsMenu.hh"
#include "DactToolsModel.hh"
#include "FilterModel.hh"
#include "Query.hh"
#include "ValidityColor.hh"
#include "ui_BracketedWindow.h"

namespace ac = alpinocorpus;

BracketedWindow::BracketedWindow(QWidget *parent) :
    CorpusWidget(parent),
    d_ui(new Ui::BracketedWindow)
{
    d_ui->setupUi(this);

    initListDelegates();
    createActions();
    readSettings();
}

BracketedWindow::~BracketedWindow()
{
    writeSettings();
}

void BracketedWindow::cancelQuery()
{
    if (d_model)
        d_model->cancelQuery();
}

void BracketedWindow::colorChanged()
{
    reloadListDelegate();
}

void BracketedWindow::queryFailed(QString error)
{
    progressStopped(0, 0);

    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void BracketedWindow::switchCorpus(QSharedPointer<ac::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
    emit saveStateChanged();
}

void BracketedWindow::setFilter(QString const &filter, QString const &raw_filter)
{
    Q_UNUSED(raw_filter);

    d_filter = filter;
    startQuery();
}

void BracketedWindow::setModel(FilterModel *model)
{
    d_model.reset(model);
    d_ui->resultsTable->setModel(d_model.data());

    d_ui->hitsLabel->clear();
    d_ui->entriesLabel->clear();

    emit saveStateChanged();

    //d_ui->resultsTable->setColumnHidden(1, true);

    //d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(2, QHeaderView::ResizeToContents);

    d_ui->resultsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d_ui->resultsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d_ui->resultsTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // disables horizontal jumping when a sentence is selected
    d_ui->resultsTable->setAutoScroll(false);

    /*
    connect(d_model.data(), SIGNAL(queryEntryFound(QString)),
        this, SLOT(updateResultsTotalCount()));
    */

    connect(d_model.data(), SIGNAL(nEntriesFound(int, int)),
        SLOT(updateCounts(int, int)));

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));

    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));

    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(queryFinished(int, int, bool)),
            SLOT(progressFinished(int, int, bool)));

    connect(d_model.data(), SIGNAL(progressChanged(int)),
        SLOT(progressChanged(int)));
}

void BracketedWindow::startQuery()
{
    d_ui->hitsLabel->clear();
    d_ui->entriesLabel->clear();

    if (d_filter.trimmed().isEmpty())
        setModel(new FilterModel(QSharedPointer<ac::CorpusReader>()));
    else
        setModel(new FilterModel(d_corpusReader));

    // Reload the list delegate since they keep their results cached.
    // This will make sure no old cached data is used.
    reloadListDelegate();


    // This would be the correct manner to execute the query:
    //
    //d_model->runQuery(generateQuery(d_filter, "(@cat or @root)"),
    //  stylesheet);
    //
    // However, in some cases this will result in an insanely slow
    // query (#68). So, we now execute the query as-is, and in the
    // stylesheets only use the matching nodes that have a @cat or
    // @root attribute. Theoretically, this could lead to the addition
    // of sentences to the model where nothing is bracketed. If this
    // happens, this is the place to start looking.

    d_model->runQuery(d_filter, true);

    showFilenamesChanged();
}

void BracketedWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void BracketedWindow::createActions()
{
    /*
    QObject::connect(d_ui->resultsListWidget,
        SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
        this,
        SLOT(entrySelected(QListWidgetItem*,QListWidgetItem*)));
    */

    QObject::connect(d_ui->resultsTable,
        // itemActivated is triggered by a single click on some systems
        // where this is the configured behavior: it can be annoying.
        // But it also enables using [enter] to raise the main window
        // which is the expected/preferred behavior.
        SIGNAL(activated(QModelIndex const &)),
        this,
        SLOT(entryActivated(QModelIndex const &)));

    QObject::connect(d_ui->listDelegateComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(listDelegateChanged(int)));

    connect(d_ui->filenamesCheckBox, SIGNAL(toggled(bool)),
        SLOT(showFilenamesChanged()));

    connect(qApp, SIGNAL(colorPreferencesChanged()),
            SLOT(colorChanged()));
}

void BracketedWindow::showFilenames(bool show)
{
   d_ui->resultsTable->setColumnHidden(0, !show);
   d_ui->resultsTable->setColumnHidden(1, !show);
   d_ui->filenamesCheckBox->setChecked(show);
}

void BracketedWindow::showFilenamesChanged()
{
    showFilenames(d_ui->filenamesCheckBox->isChecked());
}

/*
void BracketedWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0)
        return;

    emit currentEntryChanged(current->data(Qt::UserRole).toString());

    // Raises this window again when using cursor keys after using
    // [enter] to raise the main window.
    raise();
}
*/


void BracketedWindow::entryActivated(QModelIndex const &index)
{
    emit entryActivated(index.sibling(index.row(), 0).data(Qt::UserRole).toString());
}

void BracketedWindow::addOutputType(QString const &outputType,
    QString const &description, DelegateFactory factory)
{
    d_outputTypes.append(outputType);
    d_ui->listDelegateComboBox->addItem(description,
        d_listDelegateFactories.size());
    d_listDelegateFactories.append(factory);
}

void BracketedWindow::listDelegateChanged(int index)
{
    int delegateIndex = d_ui->listDelegateComboBox->itemData(index, Qt::UserRole).toInt();

    if (delegateIndex >= d_listDelegateFactories.size())
    {
        qWarning() << QString("Trying to select a list delegate (%1) beyond the boundary "
                              "of the d_listDelegateFactories list (%2)")
                             .arg(delegateIndex).arg(d_listDelegateFactories.size());
        return;
    }

    d_delegate.reset(d_listDelegateFactories[delegateIndex](d_corpusReader));
    d_ui->resultsTable->setItemDelegateForColumn(2, d_delegate.data());
    d_ui->resultsTable->resizeRowsToContents();
}

void BracketedWindow::initListDelegates()
{
    addOutputType("sentence", "Complete sentence",
        &BracketedWindow::colorDelegateFactory);
    addOutputType("match", "Only matches",
        &BracketedWindow::visibilityDelegateFactory);
    addOutputType("kwic",
        "Keyword in Context", &BracketedWindow::keywordInContextDelegateFactory);
}

void BracketedWindow::reloadListDelegate()
{
    listDelegateChanged(d_ui->listDelegateComboBox->currentIndex());
}

void BracketedWindow::progressStarted(int totalEntries)
{
    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void BracketedWindow::progressChanged(int percentage)
{
    d_ui->filterProgressBar->setValue(percentage);
}

void BracketedWindow::progressFinished(int processedEntries, int totalEntries, bool cached)
{
    progressStopped(processedEntries, totalEntries);
}


void BracketedWindow::progressStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
    emit saveStateChanged();
}

void BracketedWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void BracketedWindow::readSettings()
{
    QSettings settings;

    bool show = settings.value("bracketed_show_filenames", false).toBool();
    showFilenames(show);

    /*

    // restore last selected display method
    int delegateIndex = settings.value("filter_list_delegate", 0).toInt();
    listDelegateChanged(delegateIndex);
    d_ui->listDelegateComboBox->setCurrentIndex(delegateIndex);

    */

}

void BracketedWindow::writeSettings()
{
    QSettings settings;

    settings.setValue("bracketed_show_filenames", d_ui->filenamesCheckBox->isChecked());

    // display method
    settings.setValue("filter_list_delegate", d_ui->listDelegateComboBox->currentIndex());
}

void BracketedWindow::copy()
{
    QString output;
    QTextStream textstream(&output, QIODevice::WriteOnly | QIODevice::Text);

    selectionAsCSV(textstream);

    if (!output.isEmpty())
        QApplication::clipboard()->setText(output);
}

void BracketedWindow::exportSelection()
{
    QString filename(QFileDialog::getSaveFileName(this,
        "Export selection",
        QString("untitled"), "*.txt"));

    if (filename.isNull())
        return;

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,
            tr("Error exporting selection"),
            tr("Could open file for writing."),
            QMessageBox::Ok);

        return;
    }

    QTextStream textstream(&file);

    textstream.setGenerateByteOrderMark(true);
    selectionAsCSV(textstream);
}

void BracketedWindow::selectionAsCSV(QTextStream &output)
{
    // Nothing loaded? Do nothing.
    if (!d_model)
        return;

    QModelIndexList rows = d_ui->resultsTable->selectionModel()->selectedRows();

    // If there is nothing selected, do nothing
    if (rows.isEmpty())
        return;

    BracketedDelegate* delegate = dynamic_cast<BracketedDelegate*>(d_ui->resultsTable->itemDelegateForColumn(2));

    // Could not cast QAbstractItemDelegate to BracketedDelegate? Typical, but it is possible.
    if (!delegate)
        return;

    foreach (QModelIndex const &row, rows)
    {
        // This only works if the selection behavior is SelectRows
        output << delegate->sentenceForClipboard(row)
               << '\n';
    }
}

QStyledItemDelegate* BracketedWindow::colorDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedColorDelegate(reader);
}

QStyledItemDelegate* BracketedWindow::visibilityDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedVisibilityDelegate(reader);
}

QStyledItemDelegate* BracketedWindow::keywordInContextDelegateFactory(CorpusReaderPtr reader)
{
    return new BracketedKeywordInContextDelegate(reader);
}

void BracketedWindow::saveAs()
{
    if (d_model.isNull())
        return;

    int nlines = d_model->rowCount(QModelIndex());

    if (nlines == 0)
        return;

    QString filename;
    QStringList filenames;

    QFileDialog fd(this, tr("Save"), QString(), tr("Text (*.txt);;HTML (*.html *.htm)"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.setConfirmOverwrite(true);
    fd.setLabelText(QFileDialog::Accept, tr("Save"));
    if (d_lastfilterchoice.size())
        fd.selectNameFilter(d_lastfilterchoice);
    if (fd.exec())
        filenames = fd.selectedFiles();
    else
        return;
    if (filenames.size() < 1)
        return;
    filename = filenames[0];
    if (! filename.length())
        return;

    OutputFormat format;
    d_lastfilterchoice = fd.selectedNameFilter();
    if (d_lastfilterchoice.contains("*.txt"))
        format = FormatText;
    else
        format = FormatHTML;

    QFile data(filename);
    if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(this,
                              tr("Save file error"),
                              tr("Cannot save file %1 (error code %2)").arg(filename).arg(data.error()),
                              QMessageBox::Ok);
        return;
    }

    QTextStream out(&data);
    out.setCodec("UTF-8");

    QString xmlEntries = d_model->asXML();

    QSharedPointer<QFile> stylesheet;

    if (format == FormatText)
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-text.xsl"));
    else
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-html.xsl"));

    XSLTransformer::ParamHash params;

    int idx = d_ui->listDelegateComboBox->currentIndex();
    int outputIdx = d_ui->listDelegateComboBox->itemData(idx, Qt::UserRole).toInt();
    params["outputType"] = QString("'%1'").arg(d_outputTypes[outputIdx]);

    if (d_ui->filenamesCheckBox->isChecked())
        params["showFilenames"] = "1";

    XSLTransformer trans(*stylesheet);
    out << trans.transform(xmlEntries, params);

    emit statusMessage(tr("File saved as %1").arg(filename));

    /*
    QMessageBox::information(this,
                             tr("File saved"),
                             tr("File saved as %1").arg(filename),
                             QMessageBox::Ok);
    */
}

bool BracketedWindow::saveEnabled() const
{
    if (d_model.isNull())
        return false;
    if (d_model->rowCount(QModelIndex()) == 0)
        return false;

    return true;
}

void BracketedWindow::showToolsMenu(QPoint const &position)
{
    // Don't even try to access the current selection if there is no data.
    if (!d_ui->resultsTable->model())
        return;

    QModelIndexList rows = d_ui->resultsTable->selectionModel()->selectedRows();
    QList<QString> selectedFiles;

    // Don't show a menu with actions if there are no files selected
    if (rows.isEmpty())
        return;

    foreach (QModelIndex const &row, rows)
        selectedFiles << row.data().toString();

    DactToolsMenu::exec(
        DactToolsModel::sharedInstance()->tools(QString::fromStdString(d_corpusReader->name())),
        selectedFiles,
        mapToGlobal(position),
        d_ui->resultsTable->actions());
}

void BracketedWindow::updateCounts(int entries, int hits)
{
    d_ui->hitsLabel->setText(QString("%L1").arg(hits));
    d_ui->entriesLabel->setText(QString("%L1").arg(entries));
}
