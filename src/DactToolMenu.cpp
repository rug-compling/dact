#include <QDebug>
#include <QList>
#include <QMenu>
#include <QProcess>

#include "DactToolMenu.hh"
#include "DactToolModel.hh"

void DactToolMenu::exec(QSharedPointer<DactToolModel> model, QString const &argument, QPoint const &position)
{
	QMenu menu;
	
	for (int row = 0, end = model->rowCount(QModelIndex()); row < end; ++row)
	{
		QModelIndex nameIndex(model->index(row, 0));
		
		// Create and label the menu item
		QAction *action = menu.addAction(nameIndex.data(Qt::DisplayRole).toString());
	
		// And remember the pattern in the data section, for easy use later on.
		action->setData(row);
	}

	QAction *action = menu.exec(position);
	// Who has ownership over the QAction*'s in menu? Should we delete them?

	if (action == 0)
		return;

	QModelIndex commandIndex = model->index(action->data().toInt(), DactToolModel::COLUMN_COMMAND);
	QString commandTemplate = commandIndex.data(Qt::DisplayRole).toString();

	QString command = commandTemplate.arg(argument);

	qDebug() << "Calling" << command;

	QProcess *process = new QProcess();
	process->start(command);

	// TODO: should we clean up process in some way?
}
