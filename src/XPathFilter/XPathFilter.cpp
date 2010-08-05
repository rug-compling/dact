#include <QByteArray>
#include <QString>
#include <QVector>
#include <QtDebug>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "XPathFilter.hh"

using namespace std;
using namespace alpinocorpus;

XPathFilter::XPathFilter(QString const &xpathQuery)
{
    d_xpathQuery = xpathQuery.toUtf8();
}

EntryFun::~EntryFun() {}

void EntryFun::operator()(QVector<QString> *acc, QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
        acc->push_back(entry);
}

AggregateFun::~AggregateFun() {}

void AggregateFun::operator()(QHash<QString, int> *acc, QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0) {
        for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
            QString value = getAttribute(xpathObj->nodesetval->nodeTab[i], d_attribute);
            if(value != 0)
                ++(*acc)[value];
        }
    }
}

QString AggregateFun::getAttribute(xmlNode* node, QString const &attribute) const
{
    for (xmlAttrPtr attr = node->properties; attr; attr = attr->next)
    {
        // @TODO: it's probably faster if this uses strcmp and const char* instead of QString
        // but I coudn't get it to work correctly with Qstring.constData().
        if (QString(reinterpret_cast<const char *>(attr->name)).compare(attribute) == 0) {
            for (xmlNode* child = attr->children; child; child = child->next) {
                return QString::fromUtf8(reinterpret_cast<char const *>(child->content));
            }
        }
    }

    return 0;
}
