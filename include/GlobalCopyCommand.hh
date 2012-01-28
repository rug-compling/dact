#ifndef GLOBALCOPYCOMMAND_H
#define GLOBALCOPYCOMMAND_H

#include "GlobalEditCommand.hh"

class GlobalCopyCommand : public GlobalEditCommand
{
	Q_OBJECT

public:
	GlobalCopyCommand(QAction *action);

protected:
	bool supports(QWidget *);
};

#endif

