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

#include "CorpusWidget.hh"
#include "StatisticsWindow.hh"
#include "Query.hh"
#include "QueryModel.hh"
#include "PercentageCellDelegate.hh"
#include "ValidityColor.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_StatisticsWindow.h"

QString const MISSING_ATTRIBUTE("[missing attribute]");

StatisticsWindow::StatisticsWindow(QWidget *parent) :
    CorpusWidget(parent),
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
    writeSettings();
}

void StatisticsWindow::attributeChanged(int index)
{
    Q_UNUSED(index);
    setReady(1, false);
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
    setReady(1, false);
    d_corpusReader = corpusReader;

    //d_xpathValidator->setCorpusReader(d_corpusReader);

    setModel(new QueryModel(corpusReader));
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

    connect(d_model.data(), SIGNAL(queryFailed(QString)),
        SLOT(queryFailed(QString)));

    connect(d_model.data(), SIGNAL(queryEntryFound(QString)),
        SLOT(updateResultsTotalCount()));

    connect(d_model.data(), SIGNAL(queryStarted(int)),
        SLOT(progressStarted(int)));

    connect(d_model.data(), SIGNAL(queryStopped(int, int)),
        SLOT(progressStopped(int, int)));

    connect(d_model.data(), SIGNAL(queryFinished(int, int, bool)),
        SLOT(progressStopped(int, int)));
}

void StatisticsWindow::updateResultsTotalCount()
{
    d_ui->totalHitsLabel->setText(QString("%L1").arg(d_model->totalHits()));
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

    int
        nlines = d_model->rowCount(QModelIndex());

    if (nlines == 0)
        return;

    QString
        filename;
    QStringList
        filenames;

    QFileDialog::QFileDialog
        fd(this, tr("Save"), QString(), tr("Text (*.txt);;HTML (*.html *.htm);;Microsoft Excel 2003 XML (*.xml);;CSV (*.csv)"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.setConfirmOverwrite(true);
    fd.setLabelText(QFileDialog::Accept, tr("Save"));
    if (fd.exec())
        filenames = fd.selectedFiles();
    else
        return;
    if (filenames.size() < 1)
        return;
    filename = filenames[0];
    if (! filename.length())
        return;

    bool
        txt = false,
        html = false,
        xml = false,
        csv = false;
    if (fd.selectedNameFilter().contains("*.txt"))
        txt = true;
    else if (fd.selectedNameFilter().contains("*.html"))
        html = true;
    else if (fd.selectedNameFilter().contains("*.xml"))
        xml = true;
    else
        csv = true;

    QFile
        data(filename);
    if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(this,
                              tr("Save file error"),
                              tr("Cannot save file %1 (error code %2)").arg(filename).arg(data.error()),
                              QMessageBox::Ok);
        return;
    }

    QString
        lbl,
        date(QDateTime::currentDateTime().toLocalTime().toString());
    qreal
        perc;
    int
        count;
    QTextStream
        out(&data);

    out.setCodec("UTF-8");
    out.setRealNumberNotation(QTextStream::FixedNotation);

    if (txt) {
        out.setRealNumberPrecision(1);
        out << tr("Corpus") << ":\t" << d_corpusReader->name().c_str() << "\n"
            << tr("Filter") << ":\t" << d_filter << "\n"
            << tr("Attribute") << ":\t" << d_ui->attributeComboBox->currentText() << "\n"
            << tr("Variants") << ":\t" << nlines << "\n"
            << tr("Total hits") << ":\t" << d_model->totalHits() << "\n"
            << tr("Date") << ":\t" << date << "\n\n";
    }

    if (html) {
        out.setRealNumberPrecision(1);
        out << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
            << "<html>\n"
            << "  <head>\n"
            << "    <title></title>\n"
            << "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
            << "  </head>\n"
            << "  <body>\n"
            << "    <table>\n"
            << "      <tr><td>" << HTMLescape(tr("Corpus"))  << ":</td><td>" << HTMLescape(d_corpusReader->name()) << "</td></tr>\n"
            << "      <tr><td>" << HTMLescape(tr("Filter"))  << ":</td><td>" << HTMLescape(d_filter) << "</td></tr>\n"
            << "      <tr><td>" << HTMLescape(tr("Attribute")) << ":</td><td>" << HTMLescape(d_ui->attributeComboBox->currentText()) << "</td></tr>\n"
            << "      <tr><td>" << HTMLescape(tr("Variants")) << ":</td><td>" << nlines << "</td></tr>\n"
            << "      <tr><td>" << HTMLescape(tr("Total hits")) << ":</td><td>" << d_model->totalHits() << "</td></tr>\n"
            << "      <tr><td>" << HTMLescape(tr("Date"))  << ":</td><td>" << HTMLescape(date) << "</td></tr>\n"
            << "    </table>\n"
            << "    <p>\n"
            << "    <table border=\"1\" cellspacing=\"0\" cellpadding=\"4\">\n";
    }

    if (xml)
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<?mso-application progid=\"Excel.Sheet\"?>\n"
            << "<Workbook\n"
            << "    xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n"
            << "    xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\">\n"
            << "  <Styles>\n"
            << "    <Style ss:ID=\"s01\">\n"
            << "      <NumberFormat ss:Format=\"0.0\"/>\n"
            << "    </Style>\n"
            << "  </Styles>\n"
            << "  <Worksheet ss:Name=\"Details\">\n"
            << "    <Table>\n";


    nlines = d_model->rowCount(QModelIndex()); // again, just in case there is more now
    for (int i = 0; i < nlines; i++) {
        lbl = d_model->data(d_model->index(i, 0)).toString();
        count = d_model->data(d_model->index(i, 1)).toInt();
        perc = d_model->data(d_model->index(i, 2)).toReal() * 100.0;
        if (txt)
            out << count << "\t" << perc << "%\t" << lbl << "\n";
        else if (html)
            out << "      <tr>\n"
                << "        <td align=\"right\">" << count << "</td>\n"
                << "        <td align=\"right\">" << perc << "%</td>\n"
                << "        <td>" << HTMLescape(lbl) << "</td>\n"
                << "      </tr>\n";
        else if (xml)
            out << "      <Row>\n"
                << "        <Cell><Data ss:Type=\"String\">" << XMLescape(lbl) << "</Data></Cell>\n"
                << "        <Cell><Data ss:Type=\"Number\">" << count << "</Data></Cell>\n"
                << "        <Cell ss:StyleID=\"s01\" ss:Formula=\"=RC[-1]/SUM(R[" << -i << "]C[-1]:R[" << nlines - 1 - i << "]C[-1])*100\"/>\n"
                << "      </Row>\n";
        else if (csv)
            out << "\"" << lbl.replace("\"", "\"\"")  << "\"," << count << "," << perc << "\n";
    }

    if (html)
        out << "    </table>\n"
            << "  </body>\n"
            << "</html>\n";

    if (xml)
        out << "      <Row>\n"
            << "        <Cell ss:Index=\"2\" ss:Formula=\"=SUM(R[" << -nlines << "]C:R[-1]C)\"/>\n"
            << "      </Row>\n"
            << "    </Table>\n"
            << "  </Worksheet>\n"
            << "  <Worksheet ss:Name=\"Overview\">\n"
            << "    <Table>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Corpus")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(d_corpusReader->name()) << "</Data></Cell>\n"
            << "      </Row>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Filter")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(d_filter) << "</Data></Cell>\n"
            << "      </Row>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Attribute")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(d_ui->attributeComboBox->currentText()) << "</Data></Cell>\n"
            << "      </Row>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Variants")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"Number\">" << nlines << "</Data></Cell>\n"
            << "      </Row>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Total hits")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"Number\">" << d_model->totalHits() << "</Data></Cell>\n"
            << "      </Row>\n"
            << "      <Row>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(tr("Date")) << ":</Data></Cell>\n"
            << "        <Cell><Data ss:Type=\"String\">" << XMLescape(date) << "</Data></Cell>\n"
            << "      </Row>\n"
            << "    </Table>\n"
            << "  </Worksheet>\n"
            << "</Workbook>\n";

    out.flush();
    data.close();

    QMessageBox::information(this,
                             tr("File saved"),
                             tr("File saved as %1").arg(filename),
                             QMessageBox::Ok);


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

    file.close();
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

    output.flush();
}

void StatisticsWindow::showPercentage(bool show)
{
   d_ui->resultsTable->setColumnHidden(2, !show);
   d_ui->percentageCheckBox->setChecked(show);
}

void StatisticsWindow::startQuery()
{
    setReady(1, false);

    setAggregateAttribute(d_ui->attributeComboBox->currentText());

    d_ui->resultsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    d_ui->resultsTable->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);

    d_ui->totalHitsLabel->clear();

    QString attrWithMissing = QString("%1/(@%2/string(), '%3')[1]")
        .arg(d_filter)
        .arg(d_ui->attributeComboBox->currentText())
        .arg(MISSING_ATTRIBUTE);

    if (d_model->validQuery(attrWithMissing))
        d_model->runQuery(attrWithMissing);
    else
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
    if (d_model.isNull())
        setReady(1, false);
    else
        setReady(1, d_model->rowCount(QModelIndex()) ? true : false);
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

    bool show = settings.value("query_show_percentage", true).toBool();
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

QString StatisticsWindow::XMLescape(QString s)
{
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;");
}

QString StatisticsWindow::XMLescape(std::string s)
{
    return XMLescape(QString(s.c_str()));
}

QString StatisticsWindow::HTMLescape(QString s)
{
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
}

QString StatisticsWindow::HTMLescape(std::string s)
{
    return HTMLescape(QString(s.c_str()));
}
