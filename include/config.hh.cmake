#ifndef DACT_CONFIG_HH
#define DACT_CONFIG_HH

#include <QString>

#define GIT_REVISION "${GIT_REVISION}"

#cmakedefine USE_REMOTE_CORPUS
#cmakedefine USE_WEBSERVICE

QString const ARCHIVE_BASEURL_KEY("archiveBaseUrl");
QString const DEFAULT_ARCHIVE_BASEURL("http://www.let.rug.nl/dekok/corpora");
QString const REMOTE_BASEURL_KEY("remoteBaseUrl");
QString const DEFAULT_REMOTE_BASEURL("http://localhost:8000");
QString const WEBSERVICE_BASEURL_KEY("webserviceBaseUrl");
QString const DEFAULT_WEBSERVICE_BASEURL("http://145.100.57.148/bin/alpino");

#endif // DACT_CONFIG_HH
