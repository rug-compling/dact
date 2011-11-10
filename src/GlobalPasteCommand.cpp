#include "GlobalPasteCommand.hh"

#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

GlobalPasteCommand::GlobalPasteCommand(QAction *action)
:
	GlobalEditCommand(action, "paste()")
{}

bool GlobalPasteCommand::supports(QWidget *widget)
{
	// if you can't call paste() anyway, why botter checking any further
	if (!GlobalEditCommand::supports(widget))
		return false;
	
	// It should be ok as long as the widget is not read-only.
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);
	if (lineEdit != 0)
		return !lineEdit->isReadOnly();

	QTextEdit *textField = qobject_cast<QTextEdit *>(widget);
	if (textField != 0)
		return !textField->isReadOnly();
	
	// Otherwise, it is probably ok to paste.
	return false;
}
