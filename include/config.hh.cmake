#ifndef DACT_CONFIG_HH
#define DACT_CONFIG_HH

#include <QString>

#define GIT_REVISION "${GIT_REVISION}"

QString const ARCHIVE_BASEURL_KEY("archiveBaseUrl");
QString const DEFAULT_ARCHIVE_BASEURL("http://www.let.rug.nl/dekok/corpora");

QString const SERVER_BASEURL_KEY("serverBaseUrl");
QString const DEFAULT_SERVER_BASEURL("http://localhost:8080");

#endif // DACT_CONFIG_HH
