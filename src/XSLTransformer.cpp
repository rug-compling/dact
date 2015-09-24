#include <QByteArray>
#include <QScopedPointer>
#include <QTextStream>

#include <stdexcept>

extern "C" {
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

#include <XMLDeleters.hh>
#include <XSLTransformer.hh>
#include <QtDebug>

XSLTransformer::XSLTransformer(QFile *file)
{
    file->open(QIODevice::ReadOnly);
    QTextStream xslStream(file);
    initWithStylesheet(xslStream.readAll());
}

XSLTransformer::XSLTransformer(QString const &xsl)
{
    initWithStylesheet(xsl);
}

XSLTransformer::~XSLTransformer()
{
}

void XSLTransformer::initWithStylesheet(QString const &xsl)
{
    QByteArray xslData(xsl.toUtf8());
    xmlDocPtr xslDoc = xmlReadMemory(xslData.constData(), xslData.size(), 0, 0,
        XSLT_PARSE_OPTIONS);

    if (xslDoc == 0)
        throw std::runtime_error("XSLTransformer::initWithStylesheet: Could note parse stylesheet");

    d_xslPtr.reset(xsltParseStylesheetDoc(xslDoc));
}

QString XSLTransformer::transform(const QString &xml, QHash<QString, QString> const &params) const
{
    // Read XML data intro an xmlDoc.
    QByteArray xmlData(xml.toUtf8());
    QScopedPointer<xmlDoc, XmlDocDeleter> doc(
        xmlReadMemory(xmlData.constData(), xmlData.size(), 0, 0, 0));

    if (!doc)
        throw std::runtime_error("XSLTransformer::transform: Could not open XML data");

    // Hmpf, data conversions.
    char const **cParams = new char const *[params.size() * 2 + 1];
    int i = 0;
    for (QHash<QString, QString>::const_iterator iter = params.constBegin();
        iter != params.constEnd(); ++iter)
    {
        QByteArray keyData(iter.key().toUtf8());
        QByteArray valueData(iter.value().toUtf8());

        char const *cKey = strdup(keyData.constData());
        char const *cValue = strdup(valueData.constData());

        cParams[i] = cKey;
        cParams[i + 1] = cValue;

        i += 2;
    }

    cParams[params.size() * 2] = 0; // Terminator

    QScopedPointer<xsltTransformContext, XsltTransformContextDeleter> ctx(
        xsltNewTransformContext(d_xslPtr.data(), doc.data()));
    xsltSetCtxtParseOptions(ctx.data(), XSLT_PARSE_OPTIONS);

    // Transform...
    QScopedPointer<xmlDoc, XmlDocDeleter> res(
        xsltApplyStylesheetUser(d_xslPtr.data(), doc.data(), cParams, NULL, NULL, ctx.data()));

    if (!res)
        throw std::runtime_error("XSLTransformer::transform: Could not apply transformation!");
    else if (ctx->state != XSLT_STATE_OK)
        throw std::runtime_error("XSLTransformer::transform: Transformation error, check your query!");

    xmlChar *outputBare = 0;
    int outputLen = -1;
    xsltSaveResultToString(&outputBare, &outputLen, res.data(), d_xslPtr.data());
    QScopedPointer<xmlChar, XmlDeleter> output(outputBare);

    if (!output)
        throw std::runtime_error("Could not apply stylesheet!");

    QString result(QString::fromUtf8(reinterpret_cast<char const *>(output.data())));

    // Deallocate parameter memory
    for (int i = 0; i < params.size() * 2; ++i)
        free(const_cast<char *>(cParams[i]));
    delete[] cParams;

    return result;
}
