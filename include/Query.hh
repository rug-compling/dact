#ifndef QUERY_HH
#define QUERY_HH

#include <QString>

QString generateQuery(QString const &base, QString const &attribute, QString const &value);

QString generateQuery(QString const &base, QString const &condition);

#endif // QUERY_HH
