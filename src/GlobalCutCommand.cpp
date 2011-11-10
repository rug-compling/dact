#include "GlobalCutCommand.hh"

#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

GlobalCutCommand::GlobalCutCommand(QAction *action)
:
	GlobalEditCommand(action, "cut()")
{}

bool GlobalCutCommand::supports(QWidget *widget)
{
	// if you can't call cut() anyway, why botter checking any further
	if (!GlobalEditCommand::supports(widget))
		return false;
	
	// If it is a qlineedit, we can just ask whether it has
	// something to cut
	QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget);
	if (lineEdit != 0)
		return !lineEdit->isReadOnly() && lineEdit->hasSelectedText();

	// If it is a textfield, then we can ask the cursor if
	// it has a selection (which we can cut)
	QTextEdit *textField = qobject_cast<QTextEdit *>(widget);
	if (textField != 0)
		return !textField->isReadOnly() && textField->textCursor().hasSelection();
	
	// Otherwise, it probably does not support cutting
	return false;
}
