#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QTextStream>

#include <algorithm>
#include <stdexcept>
#include <typeinfo>

#include "config.hh"
#include "CorpusWidget.hh"
#include "SimpleDTD.hh"
#include "StatisticsWindow.hh"
#include "Query.hh"
#include "QueryModel.hh"
#include "PercentageCellDelegate.hh"
#include "ValidityColor.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_StatisticsWindow.h"

StatisticsWindow::StatisticsWindow(QWidget *parent) :
    CorpusWidget(parent),
    d_ui(QSharedPointer<Ui::StatisticsWindow>(new Ui::StatisticsWindow)),
    d_percentageCellDelegate(new PercentageCellDelegate)
{
    d_ui->setupUi(this);

    createActions();
    readNodeAttributes();
    readSettings();

    // Pick a sane default attribute.
    int idx = d_ui->attributeComboBox->findText("word");
    if (idx != -1)
        d_ui->attributeComboBox->setCurrentIndex(idx);
}

StatisticsWindow::~StatisticsWindow()
{
    writeSettings();
}

void StatisticsWindow::attributeChanged(int index)
{
    Q_UNUSED(index);
    if (!d_model.isNull())
        startQuery();
}

void StatisticsWindow::queryFailed(QString error)
{
    progressStopped(0, 0);

    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void StatisticsWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
    readNodeAttributes();
    emit saveStateChanged();

    //d_xpathValidator->setCorpusReader(d_corpusReader);

    setModel(new QueryModel(corpusReader));

    // Ensure that percentage column is hidden when necessary.
    showPercentageChanged();
}

void StatisticsWindow::setFilter(QString const &filter, QString const &raw_filter)
{
    Q_UNUSED(raw_filter);

    d_filter = filter;
    startQuery();

}

void StatisticsWindow::setAggregateAttribute(QString const &detail)
{
    // @TODO: update d_ui->attributeComboBox.currentIndex when changed from outside
    // to reflect the current (changed) state of the window.
}

void StatisticsWindow::setModel(QueryModel *model)
{
    d_model = QSharedPointer<QueryModel>(model);
    d_ui->resultsTable->setModel(d_model.data());

    d_ui->distinctValuesLabel->setText(QString(""));
    d_ui->totalHitsLabel->setText(QString(""));

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));

    connect(d_model.data(), SIGNAL(queryEntryFound(QString)),
        SLOT(updateResultsTotalCount()));

    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));

    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(queryFinished(int, int, QString, QString, bool, bool)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(progressChanged(int)),
        SLOT(progressChanged(int)));
}

void StatisticsWindow::updateResultsTotalCount()
{
    d_ui->totalHitsLabel->setText(QString("%L1").arg(d_model->totalHits()));
    d_ui->distinctValuesLabel->setText(QString("%L1").arg(d_model->rowCount(QModelIndex())));
}

void StatisticsWindow::applyValidityColor(QString const &)
{
    ::applyValidityColor(sender());
}

void StatisticsWindow::cancelQuery()
{
    if (d_model)
        d_model->cancelQuery();
}


void StatisticsWindow::saveAs()
{
    if (d_model.isNull())
        return;

    int nlines = d_model->rowCount(QModelIndex());

    if (nlines == 0)
        return;

    QString filename;
    QStringList filenames;

    QFileDialog fd(this, tr("Save"), QString("untitled"), tr("Microsoft Excel 2003 XML (*.xls);;Text (*.txt);;HTML (*.html *.htm);;CSV (*.csv)"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.setLabelText(QFileDialog::Accept, tr("Save"));
    if (d_lastFilterChoice.size())
        fd.selectNameFilter(d_lastFilterChoice);
    if (fd.exec())
        filenames = fd.selectedFiles();
    else
        return;
    if (filenames.size() < 1)
        return;
    filename = filenames[0];
    if (! filename.length())
        return;

    QSharedPointer<QFile> stylesheet;

    d_lastFilterChoice = fd.selectedNameFilter();
    if (d_lastFilterChoice.contains("*.txt"))
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/stats-text.xsl"));
    else if (d_lastFilterChoice.contains("*.html"))
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/stats-html.xsl"));
    else if (d_lastFilterChoice.contains("*.xls"))
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/stats-officexml.xsl"));
    else
        stylesheet = QSharedPointer<QFile>(new QFile(":/stylesheets/stats-csv.xsl"));

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

    QString xmlStats = d_model->asXML();

    XSLTransformer trans(stylesheet.data());
    out << trans.transform(xmlStats);

    emit statusMessage(tr("File saved as %1").arg(filename));
}


void StatisticsWindow::copy()
{
    QString csv;
    QTextStream textstream(&csv, QIODevice::WriteOnly | QIODevice::Text);

    selectionAsCSV(textstream, "\t");

    if (!csv.isEmpty())
        QApplication::clipboard()->setText(csv);
}

void StatisticsWindow::exportSelection()
{
    QString filename(QFileDialog::getSaveFileName(this,
        "Export selection",
        QString(), "*.csv"));

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
    selectionAsCSV(textstream, ";", true);
}

void StatisticsWindow::createActions()
{
    // @TODO: move this non action related ui code to somewhere else. The .ui file preferably.

    // Requires initialized UI.
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTable->verticalHeader()->hide();
    d_ui->resultsTable->sortByColumn(1, Qt::DescendingOrder);
    d_ui->resultsTable->setItemDelegateForColumn(2, d_percentageCellDelegate.data());

    // Only allow valid xpath queries to be submitted
    //d_ui->filterLineEdit->setValidator(d_xpathValidator.data());

    // When a row is activated, generate a query to be used in the main window to
    // filter all the results so only the results which are accumulated in this
    // row will be shown.
    connect(d_ui->resultsTable, SIGNAL(activated(QModelIndex const &)),
        SLOT(generateQuery(QModelIndex const &)));

    // Toggle percentage column checkbox (is this needed?)
    connect(d_ui->percentageCheckBox, SIGNAL(toggled(bool)),
        SLOT(showPercentageChanged()));

    connect(d_ui->yieldCheckBox, SIGNAL(toggled(bool)),
        SLOT(showYieldChanged()));

    connect(d_ui->attributeComboBox, SIGNAL(currentIndexChanged(int)),
        SLOT(attributeChanged(int)));
}

void StatisticsWindow::generateQuery(QModelIndex const &index)
{
    // We are not able to generate a query (yet) for yield items.
    // I guess such queries become really long and tedious.
    if (d_ui->yieldCheckBox->isChecked())
      return;

    // Get the text from the first column, that is the found value
    QString data = index.sibling(index.row(), 0).data(Qt::UserRole).toString();

    if (data == MISSING_ATTRIBUTE)
      return;

    QString query = ::generateQuery(
        d_filter,
        d_ui->attributeComboBox->currentText(),
        data);

    emit entryActivated(data, query);
}

void StatisticsWindow::selectionAsCSV(QTextStream &output, QString const &separator, bool escape_quotes) const
{
    // If there is no model attached (e.g. no corpus loaded) do nothing
    if (!d_model)
        return;

    QModelIndexList rows = d_ui->resultsTable->selectionModel()->selectedRows();

    // If there is nothing selected, do nothing
    if (rows.isEmpty())
        return;

    foreach (QModelIndex const &row, rows)
    {
        // This only works if the selection behavior is SelectRows
        if (escape_quotes)
            output << '"' << d_model->data(row).toString().replace("\"", "\"\"") << '"'; // value
        else
            output << d_model->data(row).toString();

        output
            << separator
            << d_model->data(row.sibling(row.row(), 1)).toString() // count
            << '\n';
    }
}

void StatisticsWindow::startQuery()
{
    setAggregateAttribute(d_ui->attributeComboBox->currentText());

    d_ui->resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    d_ui->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    d_ui->totalHitsLabel->clear();
    d_ui->distinctValuesLabel->clear();

    bool yield = d_ui->yieldCheckBox->isChecked();

    d_model->runQuery(d_filter, d_ui->attributeComboBox->currentText(), yield);

    emit saveStateChanged();
}

void StatisticsWindow::showPercentageChanged()
{
    d_ui->resultsTable->setColumnHidden(2, !d_ui->percentageCheckBox->isChecked());
}

void StatisticsWindow::showYieldChanged()
{
    if (!d_model.isNull())
        startQuery();
}

void StatisticsWindow::progressStarted(int total)
{
    d_ui->filterProgress->setMaximum(total);
    d_ui->filterProgress->setDisabled(false);
    d_ui->filterProgress->setVisible(true);
}

void StatisticsWindow::progressChanged(int percentage)
{
    d_ui->filterProgress->setValue(percentage);
}

void StatisticsWindow::progressStopped(int n, int total)
{
    updateResultsTotalCount();
    d_ui->filterProgress->setVisible(false);
    emit saveStateChanged();
}

void StatisticsWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void StatisticsWindow::readNodeAttributes()
{
    QString dtdPath = (d_corpusReader && d_corpusReader->type() == "tueba_tree") ?
      ":/dtd/tueba_tree.dtd" : ":/dtd/alpino_ds.dtd"; // XXX - hardcode?
    QFile dtdFile(dtdPath);

    if (!dtdFile.open(QFile::ReadOnly)) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): Could not read DTD.";
        return;
    }
    QByteArray dtdData(dtdFile.readAll());

    SimpleDTD sDTD(dtdData.constData());

    // Should be safe to clear items now...
    d_ui->attributeComboBox->clear();

    // Do we have a node element?
    ElementMap::const_iterator iter = sDTD.elementMap().find("node");
    if (iter == sDTD.elementMap().end())
        return;

    std::set<std::string> attrs = iter->second;

    QStringList attrList;
    for (std::set<std::string>::const_iterator attrIter = attrs.begin();
            attrIter != attrs.end(); ++ attrIter)
        attrList.push_back(QString::fromUtf8(attrIter->c_str()));

    std::sort(attrList.begin(), attrList.end());

    d_ui->attributeComboBox->addItems(attrList);
}

bool StatisticsWindow::saveEnabled() const
{
    if (d_model.isNull())
        return false;
    if (d_model->rowCount(QModelIndex()) == 0)
        return false;

    return true;
}

void StatisticsWindow::readSettings()
{
    QSettings settings;

    bool show = settings.value("query_show_percentage", true).toBool();
    d_ui->percentageCheckBox->setChecked(show);

    // Window geometry.
    QPoint pos = settings.value("query_pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("query_size", QSize(350, 400)).toSize();
    resize(size);

    // Move.
    move(pos);
}

void StatisticsWindow::writeSettings()
{
    QSettings settings;

    settings.setValue("query_show_percentage", d_ui->percentageCheckBox->isChecked());

    // Window geometry
    settings.setValue("query_pos", pos());
    settings.setValue("query_size", size());
}

