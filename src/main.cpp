#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QFont>

#include <cstdlib>
#include <iostream>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include "DactApplication.h"
#include "DactMainWindow.h"

namespace {
    void usage(char const *progname)
    {
        std::cerr << "Usage: " << progname << " [file]" << std::endl;
        std::exit(1);
    }
}

int main(int argc, char *argv[])
{
	xmlInitMemory();
    xmlInitParser();

    // EXSLT extensions
    exsltRegisterAll();

    // XPath
    xmlXPathInit();

	DactApplication* a = new DactApplication(argc, argv);
    
	QSettings settings("RUG", "Dact");
    QVariant fontValue = settings.value("appFont", qApp->font().toString());
    QFont appFont;
    appFont.fromString(fontValue.toString());
    a->setFont(appFont);
	
	a->init();
    
	if (a->arguments().size() > 2)
        usage(argv[0]);
    
	if (a->arguments().size() == 2)
        a->openCorpus(a->arguments().at(1));

    int r = a->exec();

	// Delete the main window explicitly to force-stop all transformers and other
	// xml operations. Otherwise commencing xml and xslt cleanup while they are
	// still in use causes errors.
	delete a;
	
    xsltCleanupGlobals();
    xmlCleanupParser();
	
    return r;
}
