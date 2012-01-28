#include "GlobalCopyCommand.hh"

#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

GlobalCopyCommand::GlobalCopyCommand(QAction *action)
:
	GlobalEditCommand(action, "copy()")
{}

bool GlobalCopyCommand::supports(QWidget *widget)
{
	if (!GlobalEditCommand::supports(widget))
		return false;
	
	// If it is a qlineedit, we can just ask whether it has
	// something to copy
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);
	if (lineEdit != 0)
		return lineEdit->hasSelectedText();

	// If it is a textfield, then we can ask the cursor if
	// it has a selection (which we can copy)
	QTextEdit *textField = qobject_cast<QTextEdit *>(widget);
	if (textField != 0)
		return textField->textCursor().hasSelection();
	
	// Otherwise, it probably supports copying
	return true;
}
