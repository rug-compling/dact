#ifndef DACT_XML_DELETERS
#define DACT_XML_DELETERS

#include <libxml/tree.h>
#include <libxslt/transform.h>


struct XmlDeleter {
	static inline void cleanup(xmlChar *str)
	{
		xmlFree(str);
	}
};

struct XmlBufferDeleter {
	static inline void cleanup(xmlBuffer *buf)
	{
		xmlBufferFree(buf);
	}
};

struct XmlDocDeleter {
	static inline void cleanup(xmlDoc *doc)
	{
		xmlFreeDoc(doc);
	}
};

struct XmlDtdDeleter {
	static inline void cleanup(xmlDtd *dtd)
	{
		xmlFreeDtd(dtd);
	}
};

struct XsltStylesheetDeleter {
	static inline void cleanup(xsltStylesheet *stylesheet)
	{
		xsltFreeStylesheet(stylesheet);
	}
};

struct XsltTransformContextDeleter {
	static inline void cleanup(xsltTransformContext *ctx)
	{
		xsltFreeTransformContext(ctx);
	}
};


#endif // DACT_XML_DELETERS
