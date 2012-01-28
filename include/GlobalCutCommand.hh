#ifndef GLOBALCUTCOMMAND_H
#define GLOBALCUTCOMMAND_H

#include "GlobalEditCommand.hh"

class GlobalCutCommand : public GlobalEditCommand
{
	Q_OBJECT

public:
	GlobalCutCommand(QAction *action);

protected:
	bool supports(QWidget *);
};

#endif

