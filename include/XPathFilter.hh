#ifndef XPATH_FILTER_HH
#define XPATH_FILTER_HH

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>
#include <libxml/tree.h>

class XPathFilter
{
public:
	XPathFilter(QString const &xpathQuery);
	QVector<QString> entries(alpinocorpus::CorpusReader *reader) const;
	QHash<QString,int> aggregate(alpinocorpus::CorpusReader *reader, QString const &attribute) const;
private:
    QString getAttribute(xmlNode* node, QString const &attribute) const;
	QByteArray d_xpathQuery;
};

#endif // XPATH_FILTER_HH
