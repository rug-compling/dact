#ifndef DACT_CONFIG_HH
#define DACT_CONFIG_HH

#include <QString>

#define GIT_REVISION "${GIT_REVISION}"

#cmakedefine USE_REMOTE_CORPUS
#cmakedefine USE_WEBSERVICE
#cmakedefine USE_SPARKLE
#cmakedefine ENABLE_SANDBOXING

QString const ARCHIVE_BASEURL_KEY("archiveBaseUrl");
QString const DEFAULT_ARCHIVE_BASEURL("http://www.let.rug.nl/dekok/corpora");
QString const WEBSERVICE_BASEURL_KEY("webserviceBaseUrl");
QString const DEFAULT_WEBSERVICE_BASEURL("http://145.100.57.148/bin/alpino");

#define MISSING_ATTRIBUTE "[missing attribute]"
#define MISSING_ATTRIBUTE_SHORT "?"

#endif // DACT_CONFIG_HH
