#ifndef DACTTOOLSMENU_H
#define DACTTOOLSMENU_H

#include <QSharedPointer>

class DactTool;

class DactToolsMenu
{
public:
	static void exec(
		QList<DactTool const *> const &tools,
		QList<QString> const &selectedFiles,
		QPoint const &position,
		QList<QAction *> const &actions = QList<QAction*>());
};

#endif

