#include <QFileInfo>
#include <QKeyEvent>
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
#include <typeinfo>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "StatisticsWindow.hh"
#include "DactMacrosModel.hh"
#include "PercentageCellDelegate.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_StatisticsWindow.h"

#include <libxml/hash.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>

StatisticsWindow::StatisticsWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::StatisticsWindow>(new Ui::StatisticsWindow)),
    d_corpusReader(corpusReader),
    d_macrosModel(macrosModel),
    d_attrMap(NULL),
    d_xpathMapper(QSharedPointer<XPathMapper>(new XPathMapper)),
    d_xpathValidator(QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel)))
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
    stopMapper();
}

void StatisticsWindow::startMapper()
{
    if(d_xpathMapper->isRunning()) {
    	qWarning() << "StatisticsWindow::startMapper: Trying to start mapper while it is running? This should not happen. Terminating running mapper anyway.";
    	stopMapper();
    }
    
    d_attrMap = new AttributeMap(d_ui->attributeComboBox->currentText());
    	
    QObject::connect(d_attrMap, SIGNAL(attributeFound(QString)), this, SLOT(attributeFound(QString)));
    
    // When I am released, does d_xpathMapper crash because the pointer that points to d_attrMap points to released space?
    // And when so, would a destructor which makes sure this thread is destroyed before d_attrMap gets freed fix this?
    d_xpathMapper->start(d_corpusReader.data(), d_macrosModel->expand(d_ui->filterLineEdit->text()), d_attrMap);
}

void StatisticsWindow::stopMapper()
{
    if(d_xpathMapper->isRunning()) {
    	d_xpathMapper->cancel();
    	d_xpathMapper->wait();
    }
}

void StatisticsWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    stopMapper();
    
    d_corpusReader = corpusReader;
    
    updateResults();
}

void StatisticsWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
}

void StatisticsWindow::setAggregateAttribute(QString const &detail)
{
    // @TODO: update d_ui->attributeComboBox.currentIndex when changed from outside
    // to reflect the current (changed) state of the window.
}

void StatisticsWindow::updateResults()
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
    } catch (std::runtime_error const &e) {
    	QMessageBox::critical(this, QString("Error calculating"),
    	    QString("Could not start searching: %1").arg(e.what()));
    }
}

QSharedPointer<StatisticsWindowResultsRow> StatisticsWindow::createResultsRow(QString const &value)
{
    QSharedPointer<StatisticsWindowResultsRow> row = QSharedPointer<StatisticsWindowResultsRow>(new StatisticsWindowResultsRow());
    row->setText(value);
    row->insertIntoTable(d_ui->resultsTableWidget);
    d_resultsTable[value] = row;
    return row;
}

void StatisticsWindow::attributeFound(QString value)
{
    QSharedPointer<StatisticsWindowResultsRow> row;
    
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
    } catch (std::runtime_error const &e) {
    	qWarning() << QString("StatisticsWindow::attributeFound: Could not add result to table: %1").arg(e.what());
    }
}

void StatisticsWindow::updateResultsTotalCount()
{
    d_ui->totalHitsLabel->setText(QString("%1").arg(d_totalHits));
    
    //updateResultsPercentages();
}

void StatisticsWindow::updateResultsPercentages()
{
    for (QHash<QString,QSharedPointer<StatisticsWindowResultsRow> >::const_iterator iter = d_resultsTable.constBegin();
    	 iter != d_resultsTable.constEnd(); ++iter)
    {
    	iter.value()->setMax(d_totalHits);
    }
}

void StatisticsWindow::applyValidityColor(QString const &)
{
    QObject *senderp = this->sender();

    if (senderp) {
        try {
            QLineEdit &sender = dynamic_cast<QLineEdit &>(*senderp);

            if (sender.hasAcceptableInput())
                sender.setStyleSheet("");
            else
                sender.setStyleSheet("background-color: salmon");
        } catch (std::bad_cast const &) {
        }
    }
}

void StatisticsWindow::createActions()
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
    
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)),
        this, SLOT(applyValidityColor(QString const &))); 
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this,
        SLOT(startQuery()));
    QObject::connect(d_ui->startPushButton, SIGNAL(clicked()),
        this, SLOT(startQuery()));
    QObject::connect(d_ui->resultsTableWidget, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(itemActivated(QTableWidgetItem*)));
    
    QObject::connect(d_ui->percentageCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPercentageChanged()));
}

void StatisticsWindow::keyPressEvent(QKeyEvent *event)
{
    // When pressing Esc, stop with what you where doing
    if (event->key() == Qt::Key_Escape)
    {
        stopMapper();
        event->accept();
    }
    // Cmd + w closes the window in OS X (and in some programs on Windows as well)
    else if (event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier)
    {
        hide();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

QString StatisticsWindow::generateQuery(QString const &base, QString const &attribute, QString const &value) const
{
    int subSelectionPos = base.lastIndexOf('/');
    
    if (!subSelectionPos)
        return QString();
    
    qWarning() << base.mid(subSelectionPos);
    
    int closingBracketPos = base.mid(subSelectionPos).lastIndexOf(']');
    
    QString condition = QString("@%1=\"%2\"").arg(attribute).arg(value);
    
    if (closingBracketPos == -1)
        return QString("%1[%2]").arg(base).arg(condition);
    else
        return QString("%1 and %2%3").arg(
            base.left(subSelectionPos + closingBracketPos),
            condition,
            base.mid(subSelectionPos + closingBracketPos));
}

QString StatisticsWindow::generateQuery(QTableWidgetItem *item) const
{
    return generateQuery(
        d_ui->filterLineEdit->text(),
        d_ui->attributeComboBox->currentText(),
        item->data(Qt::UserRole).toString());
}

void StatisticsWindow::itemActivated(QTableWidgetItem* item)
{
    emit entryActivated(
        item->text(),
        generateQuery(item));
}

void StatisticsWindow::showPercentage(bool show)
{
   //d_ui->resultsTableWidget->setColumnHidden(1, show);
   d_ui->resultsTableWidget->setColumnHidden(2, !show);
    
   d_ui->percentageCheckBox->setChecked(show);
}

void StatisticsWindow::startQuery()
{
    setFilter(d_ui->filterLineEdit->text());
    
    setAggregateAttribute(d_ui->attributeComboBox->currentText());
    
    updateResults();
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
    // @TODO maybe indicate some way when n != total that it's not busy anymore
    // but also not finished. (e.g. it was cancelled by pressing [esc], which isn't
    // even implemented yet.)
    d_ui->filterProgress->setVisible(false);

	updateResultsPercentages();
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

StatisticsWindowResultsRow::StatisticsWindowResultsRow() :
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

int StatisticsWindowResultsRow::insertIntoTable(QTableWidget *table)
{
    int row = table->rowCount();
    table->insertRow(row);
    
    table->setItem(row, 0, d_labelItem);
    table->setItem(row, 1, d_countItem);
    table->setItem(row, 2, d_percentageItem);
    
    return row;
}

StatisticsWindowResultsRow::~StatisticsWindowResultsRow()
{}

void StatisticsWindowResultsRow::setText(QString const &text)
{
    d_labelItem->setText(text);
    d_labelItem->setData(Qt::UserRole, text);
    d_percentageItem->setData(Qt::UserRole, text);
    d_countItem->setData(Qt::UserRole, text);
}

void StatisticsWindowResultsRow::setValue(int n)
{
    d_hits = n;
    d_countItem->setData(Qt::DisplayRole, n);
}

void StatisticsWindowResultsRow::setMax(int totalHits)
{
    d_percentageItem->setData(Qt::DisplayRole, ((float) d_hits / totalHits) * 100.0);
}
