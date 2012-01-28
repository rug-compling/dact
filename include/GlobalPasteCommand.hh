#ifndef GLOBALPASTECOMMAND_H
#define GLOBALPASTECOMMAND_H

#include "GlobalEditCommand.hh"

class GlobalPasteCommand : public GlobalEditCommand
{
	Q_OBJECT

public:
	GlobalPasteCommand(QAction *action);

protected:
	bool supports(QWidget *);
};

#endif

