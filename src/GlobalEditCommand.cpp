#include "GlobalEditCommand.hh"
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMetaMethod>
#include <QWidget>

GlobalEditCommand::GlobalEditCommand(QAction *action, char const *method)
:
	QObject(action),
	d_action(action),
	d_method(QMetaObject::normalizedSignature(method))
{
	focusHasChanged(0, QApplication::focusWidget());

	connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)),
		SLOT(focusHasChanged(QWidget *, QWidget *)));
	
	connect(d_action, SIGNAL(triggered()),
		SLOT(trigger()));
}

void GlobalEditCommand::focusHasChanged(QWidget *prev, QWidget *current)
{
	if (prev != 0 && supportsSignal(prev, "selectionChanged()"))
		disconnect(prev, SIGNAL(selectionChanged()),
			this, SLOT(update()));

	if (current != 0 && supportsSignal(current, "selectionChanged()"))
		connect(current, SIGNAL(selectionChanged()),
			this, SLOT(update()));
	
	update();
}

void GlobalEditCommand::update()
{
	QWidget *current = QApplication::focusWidget();
	d_action->setEnabled(current != 0 && supports(current));
}

void GlobalEditCommand::trigger()
{
	QWidget *current = QApplication::focusWidget();

	if (current && supports(current))
		apply(current);
}

bool GlobalEditCommand::supportsSignal(QObject *object, char const *signal)
{
	return object->metaObject()->indexOfSignal(signal) != -1;
}

bool GlobalEditCommand::supports(QWidget *widget)
{
	return widget->metaObject()->indexOfMethod(d_method.data()) != -1;
}

void GlobalEditCommand::apply(QWidget *widget)
{
	int method = widget->metaObject()->indexOfMethod(d_method.data());
	widget->metaObject()->method(method).invoke(widget);
}