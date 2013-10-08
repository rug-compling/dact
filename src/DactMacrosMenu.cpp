#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>

#include "DactMacrosMenu.hh"

DactMacrosMenu::DactMacrosMenu(QWidget *parent)
:
    QMenu(parent)
{}

void DactMacrosMenu::setModel(QSharedPointer<DactMacrosModel> model)
{
    if (d_model)
        disconnect(d_model.data(), 0, this, 0);

    d_model = model;

    connect(d_model.data(), SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)), SLOT(reload()));

    reload();
}

void DactMacrosMenu::reload()
{
    // Remove all our dynamically added menu items.
    foreach (QAction *action, d_macroActions)
        removeAction(action);
    
    // All menus have been removed, now forget them.
    d_macroActions.clear();

    // For each macro:
    for (int m_row = 0, m_end = d_model->rowCount(QModelIndex()); m_row < m_end; ++m_row)
    {
        QModelIndex patternIndex(d_model->index(m_row, 0));
        QModelIndex replacementIndex(d_model->index(m_row, 1));

        // Label the menu item.
        QAction *action = addAction(patternIndex.data(Qt::DisplayRole).toString());
        d_macroActions.append(action);
    
        // And show the replacement in the tooltip.
        action->setToolTip(replacementIndex.data(Qt::DisplayRole).toString());
    
        // And remember the pattern in the data section, for easy use later on.
        action->setData(patternIndex.data(Qt::UserRole).toString());

        // And connect the action to the method that inserts the pattern in the focussed widget.
        connect(action, SIGNAL(triggered()), SLOT(macroActionTriggered()));
    }

    addSeparator();

    // A force-reload menu action, which reloads the macro file.
    QAction *reloadAction = addAction("Reload file");
    d_macroActions.append(reloadAction);
    connect(reloadAction, SIGNAL(triggered()), SLOT(reloadActionTriggered()));
}

void DactMacrosMenu::macroActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (!action)
    {
        qDebug() << "Could not cast the sender of the macroActionTriggered event to a QAction pointer";
        return;    
    }
    
    if (QLineEdit *focussedLineEdit = qobject_cast<QLineEdit *>(QApplication::focusWidget()))
        focussedLineEdit->insert(action->data().toString());
    
    else if (QComboBox *focussedComboBox = qobject_cast<QComboBox *>(QApplication::focusWidget()))
        focussedComboBox->lineEdit()->insert(action->data().toString());
}

void DactMacrosMenu::reloadActionTriggered()
{
    d_model->reloadFile();
}
