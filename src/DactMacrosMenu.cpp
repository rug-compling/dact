#include <QApplication>
#include <QDebug>
#include <QLineEdit>

#include "DactMacrosMenu.hh"

DactMacrosMenu::DactMacrosMenu(QWidget *parent)
:
	QMenu(parent)
{
}

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
	QList<QAction*> staticActions;
	foreach (QAction *action, actions())
	{
		removeAction(action);

		if (!action->data().canConvert<DactMacro>())
			staticActions.append(action);
		else
			delete action;
	}

	for (int row = 0, end = d_model->rowCount(d_model->index(0, 0)); row < end; ++row)
	{
		QModelIndex index(d_model->index(row, 0));

		// Use the pattern as label
		QAction *action = addAction(index.data(Qt::DisplayRole).toString());
		
		// And show the replacement in the tooltip
		action->setToolTip(index.sibling(row, 1).data(Qt::DisplayRole).toString());
		
		// And remember the macro in the data section, for easy use later on.
		action->setData(index.data(Qt::UserRole));

		// and connect the action to the method that inserts the pattern in the focussed widget.
		connect(action, SIGNAL(triggered()), SLOT(macroTriggered()));
	}
	
	foreach (QAction *action, staticActions)
		addAction(action);
}

void DactMacrosMenu::macroTriggered()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (!action)
	{
		qDebug() << "Could not cast the sender of the macroTriggered event to a QAction pointer";
		return;	
	}
	
	if (!action->data().canConvert<DactMacro>())
	{
		qDebug() << "Could not convert the QAction's data to a DactMacro";
		return;
	}

	if (QLineEdit *focussedLineEdit = qobject_cast<QLineEdit *>(QApplication::focusWidget()))
	{
		DactMacro macro = action->data().value<DactMacro>();
		focussedLineEdit->insert(QString("%%1%").arg(macro.pattern));
	}
}