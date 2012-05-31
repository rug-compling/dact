#ifndef DACTTOOLMENU_H
#define DACTTOOLMENU_H

#include <QSharedPointer>

class DactToolModel;

class DactToolMenu
{
public:
	static void exec(QSharedPointer<DactToolModel> model, QString const &argument, QPoint const &position);
};

#endif

