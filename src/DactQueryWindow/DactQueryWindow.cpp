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
    d_xpathValidator(new XPathValidator),
	d_attrMap(NULL),
	d_xpathMapper(QSharedPointer<XPathMapper>(new XPathMapper))
{
    d_ui->setupUi(this);
	
    createActions();
    readNodeAttributes();
    readSettings();
}

DactQueryWindow::~DactQueryWindow()
{
}

void DactQueryWindow::startMapper()
{
	if(d_xpathMapper->isRunning()) {
		qWarning() << "DactQueryWindow::startMapper: Trying to start mapper while it is running? This should not happen. Terminating running mapper anyway.";
		stopMapper();
	}
	
	d_attrMap = new AttributeMap(d_ui->attributeComboBox->currentText());
		
	QObject::connect(d_attrMap, SIGNAL(attributeFound(QString)), this, SLOT(attributeFound(QString)));
	
	// When I am released, does d_xpathMapper crash because the pointer that points to d_attrMap points to released space?
	// And when so, would a destructor which makes sure this thread is destroyed before d_attrMap gets freed fix this?
	d_xpathMapper->start(d_corpusReader.data(), d_ui->filterLineEdit->text(), d_attrMap);
}

void DactQueryWindow::stopMapper()
{
	if(d_xpathMapper->isRunning()) {
		d_xpathMapper->cancel();
		d_xpathMapper->wait();
	}
}

void DactQueryWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
	stopMapper();
	
    d_corpusReader = corpusReader;
    
    updateResults();
}

void DactQueryWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
	
	// Don't try to filter with an invalid xpath expression
    if (!filter.trimmed().isEmpty() && d_ui->filterLineEdit->hasAcceptableInput())
        updateResults();
}

void DactQueryWindow::setAggregateAttribute(QString const &detail)
{
    // @TODO: update d_ui->attributeComboBox.currentIndex when changed from outside
    // to reflect the current (changed) state of the window.
}

void DactQueryWindow::updateResults()
{
    // @TODO: allow the user to copy and/or export this table.
	try {
		if (d_corpusReader.isNull())
			return;
		
		if (d_ui->filterLineEdit->text().trimmed().isEmpty() || !d_ui->filterLineEdit->hasAcceptableInput())
			return;
		
		stopMapper();
		
		d_results.clear();
		d_totalHits = 0;
		
		d_ui->resultsTableWidget->setRowCount(0);
		d_resultsTable.clear(); // @TODO should I release each row? Since they are dynamically allocated...
		updateResultsTotalCount();
		
		startMapper();
		
		return;
	} catch (runtime_error &e) {
		QMessageBox::critical(this, QString("Error calculating"),
		    QString("Could not start searching: %1").arg(e.what()));
	}
}

QSharedPointer<DactQueryWindowResultsRow> DactQueryWindow::createResultsRow(QString const &value)
{
	QSharedPointer<DactQueryWindowResultsRow> row = QSharedPointer<DactQueryWindowResultsRow>(new DactQueryWindowResultsRow());
	row->setText(value);
	row->insertIntoTable(d_ui->resultsTableWidget);
	d_resultsTable[value] = row;
	return row;
}

void DactQueryWindow::attributeFound(QString value)
{
	QSharedPointer<DactQueryWindowResultsRow> row;
	
	int hits = ++d_results[value];
	++d_totalHits;
	
	try {
		if (d_resultsTable.contains(value)) {
			row = d_resultsTable[value];
		} else {
			row = createResultsRow(value);
		}

		row->setValue(hits);
		
		updateResultsTotalCount();		
	} catch (runtime_error &e) {
		qWarning() << QString("DactQueryWindow::attributeFound: Could not add result to table: %1").arg(e.what());
	}
}

void DactQueryWindow::updateResultsTotalCount()
{
	d_ui->totalHitsLabel->setText(QString("%1").arg(d_totalHits));
	
	updateResultsPercentages();
}

void DactQueryWindow::updateResultsPercentages()
{
	for (QHash<QString,QSharedPointer<DactQueryWindowResultsRow> >::const_iterator iter = d_resultsTable.constBegin();
		 iter != d_resultsTable.constEnd(); ++iter)
	{
		iter.value()->setMax(d_totalHits);
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
	
	QObject::connect(d_xpathMapper.data(), SIGNAL(progress(int,int)), this, SLOT(progressChanged(int,int)));
    
	QObject::connect(d_xpathMapper.data(), SIGNAL(started(int)), this, SLOT(progressStarted(int)));
	QObject::connect(d_xpathMapper.data(), SIGNAL(stopped(int,int)), this, SLOT(progressStopped(int,int)));
	
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

void DactQueryWindow::progressStarted(int total)
{
	d_ui->filterProgress->setMaximum(total);
	d_ui->filterProgress->setDisabled(false);
	d_ui->filterProgress->setVisible(true);
}

void DactQueryWindow::progressChanged(int n, int total)
{
	d_ui->filterProgress->setValue(n);
}

void DactQueryWindow::progressStopped(int n, int total)
{
	// @TODO maybe indicate some way when n != total that it's not busy anymore
	// but also not finished. (e.g. it was cancelled by pressing [esc], which isn't
	// even implemented yet.)
	d_ui->filterProgress->setVisible(false);
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

DactQueryWindowResultsRow::DactQueryWindowResultsRow() :
	d_hits(0),
	d_labelItem(new QTableWidgetItem()),
	d_countItem(new QTableWidgetItem()),
	d_percentageItem(new QTableWidgetItem())
{
	d_labelItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	d_percentageItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	d_percentageItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        
	
	d_countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	d_countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        
}

int DactQueryWindowResultsRow::insertIntoTable(QTableWidget *table)
{
	int row = table->rowCount();
	table->insertRow(row);
	
	table->setItem(row, 0, d_labelItem);
	table->setItem(row, 1, d_countItem);
	table->setItem(row, 2, d_percentageItem);
	
	return row;
}

DactQueryWindowResultsRow::~DactQueryWindowResultsRow()
{
	// I expected that I needed to delete d_labelItem etc. manually, but
	// that just tells me I can't access that piece of memory. Aparently
	// they are already released?
	
	//delete d_labelItem;
	//delete d_countItem;
	//delete d_percentageItem;
}

void DactQueryWindowResultsRow::setText(QString const &text)
{
	d_labelItem->setText(text);
}

void DactQueryWindowResultsRow::setValue(int n)
{
	d_hits = n;
	d_countItem->setData(Qt::DisplayRole, n);
}

void DactQueryWindowResultsRow::setMax(int totalHits)
{
	d_percentageItem->setData(Qt::DisplayRole, ((float) d_hits / totalHits) * 100.0);
}