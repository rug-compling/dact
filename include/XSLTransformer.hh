#ifndef XSLTTRANSFORMER_HH
#define XSLTTRANSFORMER_HH

#include <QScopedPointer>
#include <QString>
#include <QHash>
#include <QFile>

#include "XMLDeleters.hh"

extern "C" {
#include <libxslt/xsltInternals.h>
}

class XSLTransformer
{
public:
    typedef QHash<QString, QString> ParamHash;

    XSLTransformer(QFile &xslt);
    XSLTransformer(QString const &xslt);
    ~XSLTransformer();
    QString transform(QString const &xml,
    	QHash<QString, QString> const &params = ParamHash()) const;
private:
    XSLTransformer(XSLTransformer const &other);
    XSLTransformer &operator=(XSLTransformer const &other);
    void initWithStylesheet(QString const &xslt);

    QScopedPointer<xsltStylesheet, XsltStylesheetDeleter> d_xslPtr;
};

#endif // XSLTTRANSFORMER_HH
