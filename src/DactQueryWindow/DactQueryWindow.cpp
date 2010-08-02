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
#include <QVector>
#include <QtDebug>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "DactQueryWindow.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactQueryWindow.h"

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
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    createActions();
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

    if (d_xpathFilter.isNull())
        return;

    try {
        results = d_xpathFilter->aggregate(d_corpusReader.data(), d_ui->attributeComboBox->currentText());
    } catch (runtime_error &e) {
        QMessageBox::critical(this, QString("Error reading corpus"),
            QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
        return;
    }

    for (QHash<QString,int>::const_iterator iter = results.begin(); iter != results.end(); ++iter)
    {
        QTableWidgetItem *resultItem = new QTableWidgetItem(iter.key());
        // Set the flags to disable the default inline editing.
        resultItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        
        QTableWidgetItem *countItem = new QTableWidgetItem();
        // Using setData to force the value to be of the type int, or else it will sort sort alphabetically.
        countItem->setData(Qt::DisplayRole, iter.value());
        countItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        

        d_ui->resultsTableWidget->insertRow(row);
        d_ui->resultsTableWidget->setItem(row, 0, resultItem);
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
    
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
    
    QObject::connect(d_ui->attributeComboBox, SIGNAL(currentIndexChanged(int)), this,
        SLOT(filterChanged()));
}

void DactQueryWindow::filterChanged()
{
    setFilter(d_ui->filterLineEdit->text());
    
    setAggregateAttribute(d_ui->attributeComboBox->currentText());
    
    updateResults();
}

void DactQueryWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void DactQueryWindow::readSettings()
{
    QSettings settings("RUG", "Dact");

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

    // Window geometry
    settings.setValue("query_pos", pos());
    settings.setValue("query_size", size());
}
