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
#include "DactMacrosModel.hh"
#include "FilterModel.hh"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "Query.hh"
#include "ValidityColor.hh"
#include "ui_BracketedWindow.h"

namespace ac = alpinocorpus;

BracketedWindow::BracketedWindow(QSharedPointer<ac::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::BracketedWindow>(new Ui::BracketedWindow)),
    d_macrosModel(macrosModel),
    d_xpathValidator(QSharedPointer<XPathValidator>(new XPathValidator(d_macrosModel, 0, corpusReader)))
{
    d_ui->setupUi(this);
    
    switchCorpus(corpusReader);
    
    initListDelegates();
    createActions();
    readSettings();
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
    
    d_xpathValidator->setCorpusReader(d_corpusReader);
    QString query = d_ui->filterLineEdit->text();
    d_ui->filterLineEdit->clear();
    d_ui->filterLineEdit->insert(query);

    setModel(new FilterModel(corpusReader));
}

void BracketedWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
    
    // Don't try to filter with an invalid xpath expression
    if (!d_ui->filterLineEdit->hasAcceptableInput())
        d_filter = QString();
    else
        d_filter = filter.trimmed();
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
}

void BracketedWindow::startQuery()
{
    setFilter(d_ui->filterLineEdit->text());
    
    // Reload the list delegate since they keep their results cached.
    // This will make sure no old cached data is used.
    reloadListDelegate();
    
    QString query = d_macrosModel->expand(d_filter);
    
    d_model->runQuery(generateQuery(query, "(@root or .//node[@root])"));
}

void BracketedWindow::stopQuery()
{
    d_model->cancelQuery();
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
    
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)),
        this, SLOT(applyValidityColor(QString const &)));
    
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()),
        this, SLOT(startQuery()));
    
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

void BracketedWindow::keyPressEvent(QKeyEvent *event)
{
    // When pressing Esc, stop with what you where doing
    if (event->key() == Qt::Key_Escape)
    {
        d_model->cancelQuery();
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

    // Window geometry.
    QPoint pos = settings.value("filter_pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("filter_size", QSize(350, 400)).toSize();
    resize(size);
    move(pos);
    
    // restore last selected display method
    int delegateIndex = settings.value("filter_list_delegate", 0).toInt();
    listDelegateChanged(delegateIndex);
    d_ui->listDelegateComboBox->setCurrentIndex(delegateIndex);
}

void BracketedWindow::writeSettings()
{
    QSettings settings;

    // Window geometry
    settings.setValue("filter_pos", pos());
    settings.setValue("filter_size", size());
    
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
