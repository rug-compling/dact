#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include <algorithm>
#include <stdexcept>
#include <typeinfo>

#include "StatisticsWindow.hh"
#include "Query.hh"
#include "QueryModel.hh"
#include "PercentageCellDelegate.hh"
#include "ValidityColor.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_StatisticsWindow.h"

StatisticsWindow::StatisticsWindow(QWidget *parent) :
    QWidget(parent),
    d_ui(QSharedPointer<Ui::StatisticsWindow>(new Ui::StatisticsWindow))
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
}

void StatisticsWindow::attributeChanged(int index)
{
  Q_UNUSED(index);
  if (!d_model.isNull())
    startQuery();
}

void StatisticsWindow::queryFailed(QString error)
{
    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void StatisticsWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
    
    //d_xpathValidator->setCorpusReader(d_corpusReader);
    
    setModel(new QueryModel(corpusReader));
}

void StatisticsWindow::setFilter(QString const &filter)
{
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

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));
    
    connect(d_model.data(), SIGNAL(queryEntryFound(QString)),
        SLOT(updateResultsTotalCount()));
    
    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));
    
    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));
}

void StatisticsWindow::updateResultsTotalCount()
{
    d_ui->totalHitsLabel->setText(QString("%1").arg(d_model->totalHits()));
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

void StatisticsWindow::copy() const
{
    QString csv = selectionAsCSV("\t");

    if (!csv.isEmpty())
        QApplication::clipboard()->setText(csv);
}

void StatisticsWindow::createActions()
{
    // @TODO: move this non action related ui code to somewhere else. The .ui file preferably.
   
    // Requires initialized UI.
    //d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTable->verticalHeader()->hide();
    d_ui->resultsTable->sortByColumn(1, Qt::DescendingOrder);
    d_ui->resultsTable->setItemDelegateForColumn(2, new PercentageCellDelegate());
    
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

    connect(d_ui->attributeComboBox, SIGNAL(currentIndexChanged(int)),
        SLOT(attributeChanged(int)));
}

void StatisticsWindow::generateQuery(QModelIndex const &index)
{
    // Get the text from the first column, that is the found value
    QString data = index.sibling(index.row(), 0).data(Qt::UserRole).toString();
    
    QString query = ::generateQuery(
        d_filter,
        d_ui->attributeComboBox->currentText(),
        data);
    
    emit entryActivated(data, query);
}

QString StatisticsWindow::selectionAsCSV(QString const &separator) const
{
    // If there is no model attached (e.g. no corpus loaded) do nothing
    if (!d_model)
        return QString();
    
    QModelIndexList rows = d_ui->resultsTable->selectionModel()->selectedRows();
    
    // If there is nothing selected, do nothing
    if (rows.isEmpty())
        return QString();
    
    QStringList output;
    
    foreach (QModelIndex const &row, rows)
    {
        // This only works if the selection behavior is SelectRows
        output << d_model->data(row).toString() // value
               << separator
               << d_model->data(row.sibling(row.row(), 1)).toString() // count
               << "\n";
    }
    
    // Remove superfluous newline separator
    output.removeLast();
    
    return output.join(QString());
}

void StatisticsWindow::showPercentage(bool show)
{
   //d_ui->resultsTable->setColumnHidden(1, show);
   d_ui->resultsTable->setColumnHidden(2, !show);
    
   d_ui->percentageCheckBox->setChecked(show);
}

void StatisticsWindow::startQuery()
{
    setAggregateAttribute(d_ui->attributeComboBox->currentText());

    d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTable->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);

    d_ui->totalHitsLabel->clear();
    
    d_model->runQuery(QString("%1/@%2")
        .arg(d_filter)
        .arg(d_ui->attributeComboBox->currentText()));
}

void StatisticsWindow::showPercentageChanged()
{
    showPercentage(d_ui->percentageCheckBox->isChecked());
}

void StatisticsWindow::progressStarted(int total)
{
    d_ui->filterProgress->setMaximum(total);
    d_ui->filterProgress->setDisabled(false);
    d_ui->filterProgress->setVisible(true);
}

void StatisticsWindow::progressChanged(int n, int total)
{
    d_ui->filterProgress->setValue(n);
}

void StatisticsWindow::progressStopped(int n, int total)
{
    d_ui->filterProgress->setVisible(false);
}

void StatisticsWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void StatisticsWindow::readNodeAttributes()
{
    QFile dtdFile(":/dtd/alpino_ds.dtd"); // XXX - hardcode?
    if (!dtdFile.open(QFile::ReadOnly)) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): Could not read DTD.";
        return;
    }
    QByteArray dtdData(dtdFile.readAll());

    xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(dtdData.constData(),
        dtdData.size(), XML_CHAR_ENCODING_8859_1);
    // Note: xmlFreeParserInputBuffer() seems to segfault in input. It's probably because
    // xmlIOParseDTD takes (some?) ownership.

    xmlDtdPtr dtd = xmlIOParseDTD(NULL, input, XML_CHAR_ENCODING_8859_1);
    if (dtd == NULL) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): Could not parse DTD.";
        return;
    }

    if (dtd->elements == NULL) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): DTD hashtable contains no elements.";
        xmlFreeDtd(dtd);
        return;
    }

    xmlNode *elem = reinterpret_cast<xmlNode *>(xmlHashLookup(
        reinterpret_cast<xmlHashTablePtr>(dtd->elements),
        reinterpret_cast<xmlChar const *>("node")));
    if (elem == NULL) {
        qWarning() << "StatisticsWindow::readNodeAttributes(): could not finde 'node' element.";
        xmlFreeDtd(dtd);
        return;
    }

    // Should be safe to clear items now...
    d_ui->attributeComboBox->clear();

    QStringList attrs;
    for (xmlAttr *attr = elem->properties; attr != NULL; attr = attr->next)
          if (attr->type == XML_ATTRIBUTE_DECL)
              attrs.push_back(reinterpret_cast<char const *>(attr->name));

    std::sort(attrs.begin(), attrs.end());

    d_ui->attributeComboBox->addItems(attrs);

    xmlFreeDtd(dtd);
}

void StatisticsWindow::readSettings()
{
    QSettings settings;

    bool show = settings.value("query_show_percentage", false).toBool();
    showPercentage(show);

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
