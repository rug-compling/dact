#include "GlobalEditCommand.hh"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
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
	if (prev != 0 && supportsSignal(supportedWidget(prev), "selectionChanged()"))
		disconnect(supportedWidget(prev), SIGNAL(selectionChanged()),
			this, SLOT(update()));

	if (current != 0 && supportsSignal(supportedWidget(current), "selectionChanged()"))
		connect(supportedWidget(current), SIGNAL(selectionChanged()),
			this, SLOT(update()));

	update();
}

void GlobalEditCommand::update()
{
	d_action->setEnabled(supportedFocusWidget() != 0);
}

void GlobalEditCommand::trigger()
{
	QWidget *current = supportedFocusWidget();

	if (current != 0)
		apply(current);
}

QWidget *GlobalEditCommand::supportedWidget(QWidget *widget)
{
	// If it is QComboBox, grab the QLineEdit inside
	QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
	if (comboBox != 0)
		widget = comboBox->lineEdit();

	return widget;
}

QWidget *GlobalEditCommand::supportedFocusWidget()
{
	QWidget *current = supportedWidget(QApplication::focusWidget());

	// Walk the tree, sometimes it is not the list-widget that
	// supports the copy operation, but the window that hosts it.
	while (current != 0 && !supports(current))
		current = current->parentWidget();

	return current;
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
