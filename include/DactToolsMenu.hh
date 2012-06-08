#ifndef DACTTOOLSMENU_H
#define DACTTOOLSMENU_H

#include <QSharedPointer>

class DactToolsModel;

class DactToolsMenu
{
public:
	static void exec(QSharedPointer<DactToolsModel> model,
		QString const &argument,
		QPoint const &position,
		QList<QAction *> const &actions = QList<QAction*>());
};

#endif

