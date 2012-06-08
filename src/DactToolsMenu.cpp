#include <QDebug>
#include <QList>
#include <QMenu>
#include <QProcess>

#include "DactToolsMenu.hh"
#include "DactToolsModel.hh"

void DactToolsMenu::exec(QList<DactTool const *> const &tools, QList<QString> const &selectedFiles, QPoint const &position, QList<QAction*> const &widgetActions)
{
	QMenu menu;

	// If the widget already has some actions, show them first
	foreach (QAction *action, widgetActions)
		menu.addAction(action);
		
	// and separate the actions from the tools with a separator.
	menu.addSeparator();

	QMap<QAction*,DactTool const *> actionMap;
	
	// Fetch all the tools from the model and add actions to the menu for them.
	foreach (DactTool const *tool, tools)
	{
		// Create and label the menu item
		QAction *action = menu.addAction(tool->name());
	
		// And remember the pattern in the data section, for easy use later on.
		actionMap[action] = tool;
	}

	QAction *action = menu.exec(position);
	// Who has ownership over the QAction*'s in menu? Should we delete them?

	// If no action was selected, or an action that is not a tool, we are done.
	if (action == 0 || !actionMap.contains(action))
		return;

	QString commandTemplate = actionMap[action]->command();

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
