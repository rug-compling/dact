#include <QObject>
#include <QValidator>

#include <libxml/xpath.h>

#include "XPathValidator.hh"

void ignoreStructuredError(void *userdata, xmlErrorPtr err)
{
}

XPathValidator::XPathValidator(QObject *parent) : QValidator(parent)
{
}

XPathValidator::State XPathValidator::validate(QString &exprStr, int &pos) const
{
    if (exprStr.trimmed().isEmpty())
        return XPathValidator::Acceptable;

    exprStr.replace('\'', '"');
    QByteArray expr(exprStr.toUtf8());

    xmlXPathContextPtr ctx;
    ctx = xmlXPathNewContext(0);
    xmlSetStructuredErrorFunc(ctx, &ignoreStructuredError);

    xmlXPathCompExprPtr r = xmlXPathCtxtCompile(ctx,
        reinterpret_cast<xmlChar const *>(expr.constData()));

    if (!r)
    {
        xmlXPathFreeContext(ctx);
        return XPathValidator::Intermediate;
    }

    xmlXPathFreeCompExpr(r);
    xmlXPathFreeContext(ctx);

    return XPathValidator::Acceptable;
}
