#include <libxml/xpath.h>

#include "XPathValidator.hh"

void ignoreStructuredError(void *userdata, xmlErrorPtr err)
{
}

XPathValidator::XPathValidator(QObject *parent, bool variables) :
	QValidator(parent),
	d_variables(variables)
{
}

XPathValidator::XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent, bool variables) :
	QValidator(parent),
	d_variables(variables),
	d_macrosModel(macrosModel)
{
}


XPathValidator::State XPathValidator::validate(QString &exprStr, int &pos) const
{
    if (exprStr.trimmed().isEmpty())
        return XPathValidator::Acceptable;

    // Consistent quoting
    exprStr.replace('\'', '"');
	
	QByteArray expr(d_macrosModel.isNull()
		? exprStr.toUtf8()
		: (d_macrosModel->expand(exprStr)).toUtf8());

    // Prepare context
    xmlXPathContextPtr ctx = xmlXPathNewContext(0);
    if (!d_variables)
        ctx->flags = XML_XPATH_NOVAR;
    xmlSetStructuredErrorFunc(ctx, &ignoreStructuredError);

    // Compile expression
    xmlXPathCompExprPtr r = xmlXPathCtxtCompile(ctx,
        reinterpret_cast<xmlChar const *>(expr.constData()));

    if (!r) {
        xmlXPathFreeContext(ctx);
        return XPathValidator::Intermediate;
    }

    xmlXPathFreeCompExpr(r);
    xmlXPathFreeContext(ctx);

    return XPathValidator::Acceptable;
}
