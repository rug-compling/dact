#include <QByteArray>
#include <QHash>
#include <QString>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

extern "C" {
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
}

#include "XSLTransformer.hh"

XSLTransformer::XSLTransformer(QString const &xsl)
{
	QByteArray xslData(xsl.toUtf8());
	xmlDocPtr xslDoc = xmlReadMemory(xslData.constData(), xslData.size(), 0, 0, 0);
    d_xslPtr = xsltParseStylesheetDoc(xslDoc);
}

XSLTransformer::~XSLTransformer()
{
	xsltFreeStylesheet(d_xslPtr);
}

QString XSLTransformer::transform(const QString &xml, QHash<QString, QString> const &params)
{
	// Read XML data intro an xmlDoc.
    QByteArray xmlData(xml.toUtf8());
    xmlDocPtr xmlDoc = xmlReadMemory(xmlData.constData(), xmlData.size(), 0, 0, 0);

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

	// Transform...
    xmlDocPtr res = xsltApplyStylesheet(d_xslPtr, xmlDoc, cParams);
    xmlChar *output = 0;
	int outputLen = -1;
	xsltSaveResultToString(&output, &outputLen, res, d_xslPtr);

	if (!output)
		throw std::runtime_error("Could not apply stylesheet!");

	QString result(QString::fromUtf8(reinterpret_cast<char const *>(output)));

	// Deallocate parameter memory
	for (int i = 0; i < params.size() * 2; ++i)
        free(const_cast<char *>(cParams[i]));
	delete[] cParams;

	// Deallocate memory used for libxml2/libxslt.
	xmlFree(output);
	xmlFreeDoc(xmlDoc);

	return result;
}
