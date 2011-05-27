#include "StatisticsTable.hh"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QStringList>

StatisticsTable::StatisticsTable(QWidget *parent)
:
    QTableView(parent),
    d_contextMenu(this)
{
    createContextMenu();
}

void StatisticsTable::contextMenuEvent(QContextMenuEvent *event)
{
    d_contextMenu.exec(event->globalPos());
}

void StatisticsTable::copy() const
{
    QString csv = selectionAsCSV("\t");

    if (!csv.isEmpty())
        QApplication::clipboard()->setText(csv);
}

void StatisticsTable::createContextMenu()
{
    QAction *copyAction = new QAction(tr("&Copy"), this);
    copyAction->setShortcuts(QKeySequence::Copy);
    connect(copyAction, SIGNAL(triggered()), SLOT(copy()));
    
    d_contextMenu.addAction(copyAction);
}

void StatisticsTable::keyPressEvent(QKeyEvent *event)
{
    if (event == QKeySequence::Copy)
    {
        copy();
        event->accept();
    }
    else
        QTableView::keyPressEvent(event);
}

QString StatisticsTable::selectionAsCSV(QString const &separator) const
{
    // If there is no model attached (e.g. no corpus loaded) do nothing
    if (!model())
        return QString();
    
    QModelIndexList rows = selectionModel()->selectedRows();
    
    // If there is nothing selected, do nothing
    if (rows.isEmpty())
        return QString();
    
    QStringList output;
    
    foreach (QModelIndex row, rows)
    {
        // This only works if the selection behavior is SelectRows
        output << model()->data(row).toString() // value
               << separator
               << model()->data(row.sibling(row.row(), 1)).toString() // count
               << "\n";
    }
    
    // Remove superfluous newline separator
    output.removeLast();
    
    return output.join(QString());
}

