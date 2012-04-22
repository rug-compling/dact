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
#include "FilterModel.hh"
#include "Query.hh"
#include "ValidityColor.hh"
#include "ui_BracketedWindow.h"

namespace ac = alpinocorpus;

BracketedWindow::BracketedWindow(QWidget *parent) :
    CorpusWidget(parent),
    d_ui(QSharedPointer<Ui::BracketedWindow>(new Ui::BracketedWindow))
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

void BracketedWindow::queryFailed(QString error)
{
    progressStopped(0, 0);

    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void BracketedWindow::switchCorpus(QSharedPointer<ac::CorpusReader> corpusReader)
{
    setReady(2, false);
    d_corpusReader = corpusReader;
}

void BracketedWindow::setFilter(QString const &filter, QString const &raw_filter)
{
    Q_UNUSED(raw_filter);

    setReady(2, false);
    d_filter = filter;
    startQuery();
}

void BracketedWindow::setModel(FilterModel *model)
{
    d_model = QSharedPointer<FilterModel>(model);
    d_ui->resultsTable->setModel(d_model.data());

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

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));

    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));

    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(queryFinished(int, int, bool)),
            SLOT(progressFinished(int, int, bool)));

}

void BracketedWindow::startQuery()
{
    // XXX - only once
    QFile file(":/stylesheets/bracketed-sentence-xml.xsl");
    file.open(QIODevice::ReadOnly);
    QTextStream xslStream(&file);
    QString stylesheet = xslStream.readAll();

    if (d_filter.trimmed().isEmpty())
        setModel(new FilterModel(QSharedPointer<ac::CorpusReader>()));
    else
        setModel(new FilterModel(d_corpusReader));

    // Reload the list delegate since they keep their results cached.
    // This will make sure no old cached data is used.
    reloadListDelegate();

    d_model->runQuery(generateQuery(d_filter, "(@cat or @root)"),
        stylesheet);

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

void BracketedWindow::addListDelegate(QString const &name, DelegateFactory factory)
{
    d_ui->listDelegateComboBox->addItem(name, d_listDelegateFactories.size());
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

    QAbstractItemDelegate* prevItemDelegate = d_ui->resultsTable->itemDelegateForColumn(2);
    d_ui->resultsTable->setItemDelegateForColumn(2, d_listDelegateFactories[delegateIndex](d_corpusReader));
    delete prevItemDelegate;
    d_ui->resultsTable->resizeRowsToContents();
}

void BracketedWindow::initListDelegates()
{
    addListDelegate("Complete sentence", &BracketedWindow::colorDelegateFactory);
    addListDelegate("Only matches", &BracketedWindow::visibilityDelegateFactory);
    addListDelegate("Keyword in Context", &BracketedWindow::keywordInContextDelegateFactory);
}

void BracketedWindow::reloadListDelegate()
{
    listDelegateChanged(d_ui->listDelegateComboBox->currentIndex());
}

void BracketedWindow::progressStarted(int totalEntries)
{
    setReady(2, false);
    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void BracketedWindow::progressChanged(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setValue(processedEntries);
}

void BracketedWindow::progressFinished(int processedEntries, int totalEntries, bool cached)
{
    progressStopped(processedEntries, totalEntries);
}


void BracketedWindow::progressStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
    if (d_model.isNull())
        setReady(2, false);
    else
        setReady(2, d_model->rowCount(QModelIndex()) ? true : false);
}

void BracketedWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void BracketedWindow::readSettings()
{
    QSettings settings;

    bool show = settings.value("bracketed_show_filenames", true).toBool();
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
        QString(), "*.txt"));

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

    file.close();
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

    output.flush();
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

    QFileDialog::QFileDialog fd(this, tr("Save"), QString(), tr("Text (*.txt);;HTML (*.html *.htm)"));
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

    switch (d_ui->listDelegateComboBox->currentIndex()) {
    case 0:
        if (format == FormatText)
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-sentence-text.xsl"));
        else
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-sentence-html.xsl"));
        break;
    case 1:
        if (format == FormatText)
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-match-text.xsl"));
        else
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-match-html.xsl"));
        break;
    case 2:
        if (format == FormatText)
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-kwic-text.xsl"));
        else
            stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/bracketed-kwic-html.xsl"));
        break;
    }

    XSLTransformer::ParamHash params;
    if (d_ui->filenamesCheckBox->isChecked())
        params["showFilenames"] = "1";

    XSLTransformer trans(*stylesheet);
    out << trans.transform(xmlEntries, params);

    out.flush();
    data.close();

    emit statusMessage(tr("File saved as %1").arg(filename));

    /*
    QMessageBox::information(this,
                             tr("File saved"),
                             tr("File saved as %1").arg(filename),
                             QMessageBox::Ok);
    */
}
