#ifndef XSLTTRANSFORMER_HH
#define XSLTTRANSFORMER_HH

#include <QByteArray>
#include <QString>
#include <QHash>

extern "C" {
#include <libxslt/xsltInternals.h>
}

class XSLTransformer
{
public:
    XSLTransformer(QString const &xslt);
    ~XSLTransformer();
    QString transform(QString const &xml, QHash<QString, QString> const &params);
private:
    XSLTransformer(XSLTransformer const &other);
    XSLTransformer &operator=(XSLTransformer const &other);

    xsltStylesheetPtr d_xslPtr;
};

#endif // XSLTTRANSFORMER_HH
