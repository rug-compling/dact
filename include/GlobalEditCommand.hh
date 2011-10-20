#ifndef GLOBALEDITCOMMAND_H
#define GLOBALEDITCOMMAND_H

#include <QObject>

class QAction;
class QWidget;

class GlobalEditCommand : public QObject
{
	Q_OBJECT

public:
	GlobalEditCommand(QAction *action, char const *method);

public slots:
	/**
	 * When triggered, it will ook at the currently focussed
	 * widget, and see whether it supports the method. If it
	 * doesn't, it will walk up the widget-tree till it runs
	 * out of QWidget parents.
	 */
	void trigger();

private slots:
	/**
	 * Triggered as the focus just from widget to widget.
	 * Updates the action's enabled status.
	 */
	void focusHasChanged(QWidget *, QWidget *);

protected:
	/**
	 * Tests whether the object supports this method.
	 */
	virtual bool supports(QWidget *);

	/**
	 * If it supports the method, it can be called
	 * with apply().
	 */
	virtual void apply(QWidget *);

private:
	/**
	 * The QAction widget which it tries to keep in sync with
	 * the capabilities of the currently focussed widget.
	 */
	QAction *d_action;

	/**
	 * The method which is tested for, and which is called when
	 * the QAction is triggered. At least, if a widget supports
	 * it.
	 */
	QByteArray d_method;
};

#endif