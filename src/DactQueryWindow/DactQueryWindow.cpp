#include <QFileInfo>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVector>
#include <QtDebug>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "DactQueryWindow.h"
#include "PercentageCellDelegate.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactQueryWindow.h"

#include <libxml/hash.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>

using namespace alpinocorpus;
using namespace std;

DactQueryWindow::DactQueryWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DactQueryWindow>(new Ui::DactQueryWindow)),
    d_corpusReader(corpusReader),
    d_xpathValidator(new XPathValidator)
{
    d_ui->setupUi(this);
    createActions();
    readNodeAttributes();
    readSettings();
}

DactQueryWindow::~DactQueryWindow()
{
}

void DactQueryWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;
    
    updateResults();
}

void DactQueryWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
    
    // Don't try to filter with an invalid xpath expression
    if (filter.trimmed().isEmpty() || !d_ui->filterLineEdit->hasAcceptableInput())
        d_xpathFilter.clear();
    else
        d_xpathFilter = QSharedPointer<XPathFilter>(new XPathFilter(filter));
}

void DactQueryWindow::setAggregateAttribute(QString const &detail)
{
    // @TODO: update d_ui->attributeComboBox.currentIndex when changed from outside
    // to reflect the current (changed) state of the window.
}

void DactQueryWindow::updateResults()
{
    // @TODO: allow the user to copy and/or export this table.

    if (d_corpusReader.isNull())
        return;
        
    int row = 0;

    QHash<QString,int> results;

    d_ui->resultsTableWidget->setRowCount(0);
    //d_ui->resultsTableWidget->setColumnCount(3);

    if (d_xpathFilter.isNull())
        return;

    try {
        AggregateFun fun(d_ui->attributeComboBox->currentText());
        results = d_xpathFilter->fold(d_corpusReader.data(), &fun);
    } catch (runtime_error &e) {
        QMessageBox::critical(this, QString("Error reading corpus"),
            QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
        return;
    }
    
    float totalHits = 0;
    float percentage;
    
    for (QHash<QString,int>::const_iterator iter = results.begin(); iter != results.end(); ++iter)
    {
        totalHits += iter.value();
    }

    for (QHash<QString,int>::const_iterator iter = results.begin(); iter != results.end(); ++iter)
    {
        d_ui->resultsTableWidget->insertRow(row);
        
        QTableWidgetItem *resultItem = new QTableWidgetItem(iter.key());
        resultItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        d_ui->resultsTableWidget->setItem(row, 0, resultItem);
        
        percentage = ((float) iter.value() / totalHits) * 100.0;
        QTableWidgetItem *percentageItem = new QTableWidgetItem();
        percentageItem->setData(Qt::DisplayRole, percentage);
        percentageItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        percentageItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        
        d_ui->resultsTableWidget->setItem(row, 2, percentageItem);
        
        QTableWidgetItem *countItem = new QTableWidgetItem();
        countItem->setData(Qt::DisplayRole, iter.value());
        countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        
        d_ui->resultsTableWidget->setItem(row, 1, countItem);

        ++row;
    }
}

void DactQueryWindow::applyValidityColor(QString const &)
{
    // @TODO: maybe we can create a template, mixin or something else to allow
    // this function to be shared across most of the window classes.

    // Hmpf, unfortunately we get text, rather than a sender. Attempt
    // to determine the sender ourselvers.
    QObject *sender = this->sender();
    
    if (!sender)
        return;

    if (!sender->inherits("QLineEdit"))
        return;

    QLineEdit *widget = reinterpret_cast<QLineEdit *>(sender);

    if (widget->hasAcceptableInput())
        widget->setStyleSheet("");
    else
        widget->setStyleSheet("background-color: salmon");
}

void DactQueryWindow::createActions()
{
    // @TODO: move this non action related ui code to somewhere else. The .ui file preferably.
    d_ui->resultsTableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    d_ui->resultsTableWidget->verticalHeader()->hide();
    d_ui->resultsTableWidget->setShowGrid(false);
    d_ui->resultsTableWidget->setSortingEnabled(true);
    d_ui->resultsTableWidget->sortByColumn(1, Qt::DescendingOrder);
    d_ui->resultsTableWidget->setItemDelegateForColumn(2, new PercentageCellDelegate());
    // As long as copying etc. from the columns isn't supported, I think this is the most expected behavior.
    d_ui->resultsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    
    QObject::connect(d_ui->attributeComboBox, SIGNAL(currentIndexChanged(int)), this,
        SLOT(filterChanged()));
    
    QObject::connect(d_ui->percentageCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPercentageChanged()));
}

void DactQueryWindow::showPercentage(bool show)
{
   //d_ui->resultsTableWidget->setColumnHidden(1, show);
   d_ui->resultsTableWidget->setColumnHidden(2, !show);
    
   d_ui->percentageCheckBox->setChecked(show);
}

void DactQueryWindow::filterChanged()
{
    setFilter(d_ui->filterLineEdit->text());
    
    setAggregateAttribute(d_ui->attributeComboBox->currentText());
    
    updateResults();
}

void DactQueryWindow::showPercentageChanged()
{
    showPercentage(d_ui->percentageCheckBox->isChecked());
}

void DactQueryWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void DactQueryWindow::readNodeAttributes()
{
    QFile dtdFile(":/dtd/alpino_ds.dtd"); // XXX - hardcode?
    if (!dtdFile.open(QFile::ReadOnly)) {
        qWarning() << "DactQueryWindow::readNodeAttributes(): Could not read DTD.";
        return;
    }
    QByteArray dtdData(dtdFile.readAll());

    xmlParserInputBufferPtr input = xmlParserInputBufferCreateMem(dtdData.constData(),
        dtdData.size(), XML_CHAR_ENCODING_8859_1);
    // Note: xmlFreeParserInputBuffer() seems to segfault in input. It's probably because
    // xmlIOParseDTD takes (some?) ownership.

    xmlDtdPtr dtd = xmlIOParseDTD(NULL, input, XML_CHAR_ENCODING_8859_1);
    if (dtd == NULL) {
        qWarning() << "DactQueryWindow::readNodeAttributes(): Could not parse DTD.";
        return;
    }

    if (dtd->elements == NULL) {
        qWarning() << "DactQueryWindow::readNodeAttributes(): DTD hashtable contains no elements.";
        xmlFreeDtd(dtd);
        return;
    }

    xmlNode *elem = reinterpret_cast<xmlNode *>(xmlHashLookup(
        reinterpret_cast<xmlHashTablePtr>(dtd->elements),
        reinterpret_cast<xmlChar const *>("node")));
    if (elem == NULL) {
        qWarning() << "DactQueryWindow::readNodeAttributes(): could not finde 'node' element.";
        xmlFreeDtd(dtd);
        return;
    }

    // Should be safe to clear items now...
    d_ui->attributeComboBox->clear();

    QStringList attrs;
    for (xmlAttr *attr = elem->properties; attr != NULL; attr = attr->next)
          if (attr->type == XML_ATTRIBUTE_DECL)
              attrs.push_back(reinterpret_cast<char const *>(attr->name));

    sort(attrs.begin(), attrs.end());

    d_ui->attributeComboBox->addItems(attrs);

    xmlFreeDtd(dtd);
}

void DactQueryWindow::readSettings()
{
    QSettings settings("RUG", "Dact");

    bool show = settings.value("query_show_percentage", false).toBool();
    showPercentage(show);

    // Window geometry.
    QPoint pos = settings.value("query_pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("query_size", QSize(350, 400)).toSize();
    resize(size);

    // Move.
    move(pos);
}

void DactQueryWindow::writeSettings()
{
    QSettings settings("RUG", "Dact");

    settings.setValue("query_show_percentage", d_ui->percentageCheckBox->isChecked());

    // Window geometry
    settings.setValue("query_pos", pos());
    settings.setValue("query_size", size());
}
