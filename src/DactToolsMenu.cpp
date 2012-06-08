#include <QDebug>
#include <QList>
#include <QMenu>
#include <QProcess>

#include "DactToolsMenu.hh"
#include "DactToolsModel.hh"

void DactToolsMenu::exec(QSharedPointer<DactToolsModel> model, QList<QString> const &selectedFiles, QPoint const &position, QList<QAction*> const &widgetActions)
{
	QMenu menu;

	// If the widget already has some actions, show them first
	if (widgetActions.size())
	{
		foreach (QAction *action, widgetActions)
			menu.addAction(action);

		// and separate the actions from the tools with a separator.
		menu.addSeparator();
	}

	QMap<QAction*,QModelIndex> actionMap;
	
	// Fetch all the tools from the model and add actions to the menu for them.
	for (int row = 0, end = model->rowCount(QModelIndex()); row < end; ++row)
	{
		QModelIndex nameIndex(model->index(row, 0));
		
		// Create and label the menu item
		QAction *action = menu.addAction(nameIndex.data(Qt::DisplayRole).toString());
	
		// And remember the pattern in the data section, for easy use later on.
		actionMap[action] = model->index(row, DactToolsModel::COLUMN_COMMAND);
	}

	QAction *action = menu.exec(position);
	// Who has ownership over the QAction*'s in menu? Should we delete them?

	if (action == 0 || !actionMap.contains(action))
		return;

	QString commandTemplate = actionMap[action].data(Qt::DisplayRole).toString();

	foreach (QString const &selectedFile, selectedFiles)
	{
		// TODO: Should we test if there is a placeholder in the template? If not, Qt will throw a notice
		// to stdout, and it would be weird to have a command without the file as an argument.
		// TODO: Escape spaces etc. argument? Workaround: use quotes in your template.
		QString command = commandTemplate.arg(selectedFile);

		qDebug() << "Calling" << command;

		QProcess::startDetached(command);
	}

	// TODO: should we clean up process in some way?
}
