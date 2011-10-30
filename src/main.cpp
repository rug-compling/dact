#include <QFont>
#include <QSettings>
#include <QVariant>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include "DactApplication.hh"

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

    int r = 0;

    try {
        QCoreApplication::setOrganizationName("RUG");
        QCoreApplication::setOrganizationDomain("rug.nl");
        QCoreApplication::setApplicationName("Dact");

        QScopedPointer<DactApplication> a(new DactApplication(argc, argv));
    
        QSettings settings;
        QVariant fontValue = settings.value("appFont", qApp->font().toString());
        
#ifndef __APPLE__
        QFont appFont;
        appFont.fromString(fontValue.toString());
        a->setFont(appFont);
#endif

        a->init();
    
        if (a->arguments().size() > 2)
            usage(argv[0]);
    
        if (a->arguments().size() == 2)
            a->openCorpus(a->arguments().at(1));

        r = a->exec();
    } catch (std::logic_error const &e) {
        std::cerr << "dact: internal logic error: please report at\n"
                     "      https://github.com/rug-compling/dact/issues, citing"
                  << e.what()
                  << std::endl;
        r = 1;
    }

    // must be called after the DactApplication is deleted
    xsltCleanupGlobals();
    xmlCleanupParser();

    return r;
}
