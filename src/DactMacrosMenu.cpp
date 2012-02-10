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
	foreach (QMenu *menu, d_menus)
	{
		// remove the action from the menu so it doesn't show up in staticActions.
		removeAction(menu->menuAction());

		// release our hand-added actions.
		foreach (QAction *action, menu->actions())
		{
			menu->removeAction(action);
			delete action;
		}

		// release our home-built menu.
		delete menu;
	}

	// all menus have been removed, their pointers have been released. Now forget them.
	d_menus.clear();

	QList<QAction *> menuActions;

	// For each macro file:
	for (int m_row = 0, m_end = d_model->rowCount(QModelIndex()); m_row < m_end; ++m_row)
	{
		QModelIndex macroIndex(d_model->index(m_row, 0));

		// Use the pattern as label
		QMenu *menu = new QMenu(macroIndex.data(Qt::DisplayRole).toString());

		menuActions.append(menu->menuAction());
		d_menus.append(menu);
		
		for (int row = 0, end = d_model->rowCount(macroIndex); row < end; ++row)
		{
			QModelIndex patternIndex(macroIndex.child(row, 0)),
				replacementIndex(macroIndex.child(row, 1));
			
			// Label the menu item, yeah!
			QAction *action = menu->addAction(patternIndex.data(Qt::DisplayRole).toString());
		
			// And show the replacement in the tooltip
			action->setToolTip(replacementIndex.data(Qt::DisplayRole).toString());
		
			// And remember the pattern in the data section, for easy use later on.
			action->setData(patternIndex.data(Qt::UserRole).toString());

			// and connect the action to the method that inserts the pattern in the focussed widget.
			connect(action, SIGNAL(triggered()), SLOT(macroActionTriggered()));
		}

		menu->addSeparator();

		// A force-reload menu action, which reloads the macro file.
		QAction *reloadAction = menu->addAction("Reload file");
		reloadAction->setData(macroIndex.data(Qt::UserRole).toString());
		connect(reloadAction, SIGNAL(triggered()), SLOT(reloadActionTriggered()));
	}

	// Insert all the menus we collected at the top of this menu.
	insertActions(
		actions().size() ? actions()[0] : 0,
		menuActions);
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
	QAction *action = qobject_cast<QAction *>(sender());

	if (!action)
	{
		qDebug() << "Could not cast the sender of the reloadActionTriggered event to a QAction pointer";
		return;	
	}

	d_model->loadFile(action->data().toString());
}
