#ifndef DACTMACROSMENU_H
#define DACTMACROSMENU_H

#include <QMenu>
#include <QSharedPointer>

#include "DactMacrosModel.hh"

class DactMacrosMenu : public QMenu
{
	Q_OBJECT

public:
	DactMacrosMenu(QWidget *parent);
	void setModel(QSharedPointer<DactMacrosModel> model);

signals:
	void triggered(DactMacro*);

private slots:
	void reload();
	void macroActionTriggered();
	void reloadActionTriggered();
	void unloadActionTriggered();

private:
	QSharedPointer<DactMacrosModel> d_model;
	QList<QMenu *> d_menus;
};

#endif

