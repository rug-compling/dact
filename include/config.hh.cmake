#ifndef DACT_CONFIG_HH
#define DACT_CONFIG_HH

#include <QString>

#define GIT_REVISION "${GIT_REVISION}"
#define DACT_VERSION "${VERSION}"

#cmakedefine USE_REMOTE_CORPUS
#cmakedefine USE_WEBSERVICE

#if defined Q_OS_MAC || defined Q_OS_WIN
#cmakedefine USE_AUTO_UPDATER
#endif

QString const ARCHIVE_BASEURL_KEY("archiveBaseUrl");
QString const DEFAULT_ARCHIVE_BASEURL("http://www.let.rug.nl/dekok/corpora");
QString const REMOTE_BASEURL_KEY("remoteBaseUrl");
QString const DEFAULT_REMOTE_BASEURL("http://localhost:8000");
QString const WEBSERVICE_BASEURL_KEY("webserviceBaseUrl");
QString const DEFAULT_WEBSERVICE_BASEURL("http://145.100.57.148/bin/alpino");

#ifdef Q_OS_MAC
QString const AUTO_UPDATER_FEED_URL("http://rug-compling.github.com/dact/appcast-mac.xml");
#endif
#ifdef Q_OS_WIN
QString const AUTO_UPDATER_FEED_URL("http://rug-compling.github.com/dact/appcast-win.xml");
#endif

#endif // DACT_CONFIG_HH
