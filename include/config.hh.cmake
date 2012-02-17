#ifndef DACT_CONFIG_HH
#define DACT_CONFIG_HH

#include <QString>

#define GIT_REVISION "${GIT_REVISION}"

#cmakedefine USE_REMOTE_CORPUS

QString const ARCHIVE_BASEURL_KEY("archiveBaseUrl");
QString const DEFAULT_ARCHIVE_BASEURL("http://www.let.rug.nl/dekok/corpora");
QString const REMOTE_BASEURL_KEY("remoteBaseUrl");
QString const DEFAULT_REMOTE_BASEURL("http://localhost:8000");

#endif // DACT_CONFIG_HH
