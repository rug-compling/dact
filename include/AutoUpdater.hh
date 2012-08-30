#ifndef AUTOUPDATER_HH
#define AUTOUPDATER_HH

class AutoUpdater
{
public:
	virtual ~AutoUpdater() {};
	virtual void checkForUpdates() = 0;
};

#endif