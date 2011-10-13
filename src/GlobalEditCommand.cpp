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
	QObject *focussedWidget = QApplication::focusWidget();

	while (QWidget *widget = qobject_cast<QWidget *>(focussedWidget))
	{
		if (supports(widget))
		{
			d_action->setEnabled(true);
			return;
		}

		focussedWidget = widget->parent();
	}

	d_action->setEnabled(false);
}

void GlobalEditCommand::trigger()
{
	QObject *focussedWidget = QApplication::focusWidget();

	while (QWidget *widget = qobject_cast<QWidget *>(focussedWidget))
	{
		if (supports(widget))
		{
			apply(widget);
			return;
		}
		
		focussedWidget = focussedWidget->parent();
	}
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