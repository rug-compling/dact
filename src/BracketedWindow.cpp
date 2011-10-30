#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QVector>

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
#include "XSLTransformer.hh"
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
    //readSettings();
}

void BracketedWindow::cancelQuery()
{
    if (d_model)
        d_model->cancelQuery();
}

void BracketedWindow::queryFailed(QString error)
{
    QMessageBox::critical(this, tr("Error processing query"),
        tr("Could not process query: ") + error,
        QMessageBox::Ok);
}

void BracketedWindow::switchCorpus(QSharedPointer<ac::CorpusReader> corpusReader)
{
    d_corpusReader = corpusReader;    
}

void BracketedWindow::setFilter(QString const &filter)
{
    d_filter = filter;
    startQuery();
}

void BracketedWindow::setModel(FilterModel *model)
{
    d_model = QSharedPointer<FilterModel>(model);
    d_ui->resultsList->setModel(d_model.data());
    
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
    if (d_filter.trimmed().isEmpty())
        setModel(new FilterModel(QSharedPointer<ac::CorpusReader>()));
    else
        setModel(new FilterModel(d_corpusReader));

    // Reload the list delegate since they keep their results cached.
    // This will make sure no old cached data is used.
    reloadListDelegate();

    d_model->runQuery(generateQuery(d_filter, "(@cat or @root)"));

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
    
    QObject::connect(d_ui->resultsList, 
        // itemActivated is triggered by a single click on some systems
        // where this is the configured behavior: it can be annoying.
        // But it also enables using [enter] to raise the main window
        // which is the expected/preferred behavior.
        SIGNAL(activated(QModelIndex const &)),
        this,
        SLOT(entryActivated(QModelIndex const &)));

    QObject::connect(d_ui->listDelegateComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(listDelegateChanged(int)));
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
    emit entryActivated(index.data(Qt::UserRole).toString());
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
    
    QAbstractItemDelegate* prevItemDelegate = d_ui->resultsList->itemDelegate();
    d_ui->resultsList->setItemDelegate(d_listDelegateFactories[delegateIndex](d_corpusReader));
    delete prevItemDelegate;
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
}

void BracketedWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void BracketedWindow::readSettings()
{
    QSettings settings;
    
    // restore last selected display method
    int delegateIndex = settings.value("filter_list_delegate", 0).toInt();
    listDelegateChanged(delegateIndex);
    d_ui->listDelegateComboBox->setCurrentIndex(delegateIndex);
}

void BracketedWindow::writeSettings()
{
    QSettings settings;
    
    // display method
    settings.setValue("filter_list_delegate", d_ui->listDelegateComboBox->currentIndex());
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
