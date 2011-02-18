#include <QSettings>
#include <QVector>

#include "DactMacrosWindow.hh"
#include "DactMacrosModel.hh"
#include "ui_DactMacrosWindow.h"

DactMacrosWindow::DactMacrosWindow(QSharedPointer<DactMacrosModel> model, QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DactMacrosWindow>(new Ui::DactMacrosWindow)),
    d_model(model)
{
    d_ui->setupUi(this);
    createActions();
    readSettings();
}

DactMacrosWindow::~DactMacrosWindow()
{
}

void DactMacrosWindow::createActions()
{
    d_ui->macrosTable->setModel(d_model.data());
    d_ui->macrosTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    d_ui->macrosTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    d_ui->macrosTable->verticalHeader()->hide();
    d_ui->macrosTable->setShowGrid(false);
       
    // update [remove] button state
    macrosTableSelectionChanged();
    
    // watch the table's selection to update the [remove] button's state.
    QObject::connect(d_ui->macrosTable->selectionModel(), 
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this, SLOT(macrosTableSelectionChanged()));
    
    QObject::connect(d_ui->addButton, SIGNAL(clicked(bool)),
        this, SLOT(addButtonPressed()));
    QObject::connect(d_ui->removeButton, SIGNAL(clicked(bool)),
        this, SLOT(removeButtonPressed()));
}

void DactMacrosWindow::readSettings()
{
    QSettings settings;
    
    settings.beginGroup("macroswindow");

    QPoint pos = settings.value("position", QPoint(200, 200)).toPoint();
    move(pos);
    
    QSize size = settings.value("size", QSize(350, 400)).toSize();
    resize(size);

    settings.endGroup();
}

void DactMacrosWindow::writeSettings()
{
    QSettings settings;
    
    settings.beginGroup("macroswindow");
    
    settings.setValue("position", pos());
    settings.setValue("size", size());
    
    settings.endGroup();
}

void DactMacrosWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void DactMacrosWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier)
    {
        hide();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

void DactMacrosWindow::addButtonPressed()
{
    int row = d_model->rowCount(QModelIndex());
    
    d_model->insertRows(row, 1, QModelIndex());
    
    // move focus to newly added cell and start editing.
    QModelIndex index = d_model->index(row, 0, QModelIndex());
    
    d_ui->macrosTable->setCurrentIndex(index);
    d_ui->macrosTable->edit(index);
}

void DactMacrosWindow::removeButtonPressed()
{
    QModelIndexList selectedIndexes = d_ui->macrosTable->selectionModel()->selectedRows();
    QModelIndex index;
    
    // I want to remove multiple rows, but each time I remove a row, the indexes change.
    // Therefore, let's be creative and recalculate the indexes ourselves. Simply by
    // counting which rows before a certain row where removed, and using that count to
    // lower the index of the current row.
    
    // d_model.rowCount is the maximum rows we can remove, so this should be enough.
    // It whould have been prettier if I could get the length of selectedIndexes.
    QVector<int> removedRows(d_model->rowCount(QModelIndex()));
    int i = 0;
    int n;
    int rowsBeforeRowDeleted;
    
    foreach(index, selectedIndexes)
    {
        for(n = 0, rowsBeforeRowDeleted = 0; n < i; ++n)
        {
            if(removedRows[n] < index.row())
                ++rowsBeforeRowDeleted;
        }
        
        d_model->removeRows(index.row() - rowsBeforeRowDeleted, 1, QModelIndex());
        
        removedRows[i++] = index.row();
    }
}

void DactMacrosWindow::macrosTableSelectionChanged()
{
    d_ui->removeButton->setEnabled(
        d_ui->macrosTable->selectionModel()->hasSelection());
}
