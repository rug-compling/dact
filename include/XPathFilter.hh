#ifndef XPATH_FILTER_HH
#define XPATH_FILTER_HH

#include <QByteArray>
#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

class XPathFilter
{
public:
	XPathFilter(QString const &xpathQuery);
	QVector<QString> entries(alpinocorpus::CorpusReader *reader) const;
private:
	QByteArray d_xpathQuery;
};

#endif // XPATH_FILTER_HH
