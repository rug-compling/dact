#ifndef SPARKLEAUTOUPDATER_HH
#define SPARKLEAUTOUPDATER_HH

#include <QString>

#include "AutoUpdater.hh"

class SparkleAutoUpdater : public AutoUpdater
{
public:
	SparkleAutoUpdater(QString const &url);
	virtual ~SparkleAutoUpdater();

	virtual void checkForUpdates();

private:
	class Private;
	Private* d;
};

#endif
