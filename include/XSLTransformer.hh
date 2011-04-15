#ifndef XSLTTRANSFORMER_HH
#define XSLTTRANSFORMER_HH

#include <QString>
#include <QHash>
#include <QFile>

extern "C" {
#include <libxslt/xsltInternals.h>
}

class XSLTransformer
{
public:
    XSLTransformer(QFile &xslt);
    XSLTransformer(QString const &xslt);
    ~XSLTransformer();
    QString transform(QString const &xml, QHash<QString, QString> const &params) const;
private:
    XSLTransformer(XSLTransformer const &other);
    XSLTransformer &operator=(XSLTransformer const &other);
    void initWithStylesheet(QString const &xslt);

    xsltStylesheetPtr d_xslPtr;
};

#endif // XSLTTRANSFORMER_HH
