#include <QApplication>
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

        QSettings settings;

#if defined(Q_WS_MAC)
        // Work around a bug (#56) where Qt 4.8.x on Mac OS X does not
        // invalidate the region of the popup when we hover as a result of
        // mouse scrolling.
        if (!settings.value("useNativeGraphicsSystem", false).toBool())
            QApplication::setGraphicsSystem("raster");
#endif


        QScopedPointer<DactApplication> a(new DactApplication(argc, argv));
        bool dactIsRunning = a->isRunning();
        if (dactIsRunning)
          a->activateWindow();
    
        QVariant fontValue = settings.value("appFont", qApp->font().toString());
        
#ifndef __APPLE__
        QFont appFont;
        appFont.fromString(fontValue.toString());
        a->setFont(appFont);
#endif

        a->init();
    
        QStringList corpusPaths, macroPaths;
        
        QStringList args = a->arguments();
        for (int i = 1; i < args.size(); ++i)
        {
            if (args[i] == "-m")
            {
                // please do follow with a path after -m switch.
                if (i + 1 >= args.size()) 
                    usage(argv[0]);
                
                macroPaths.append(args[++i]);
            }
            else
            {
                if (args[i].startsWith("dact:"))
                {
                    if (dactIsRunning)
                      a->sendMessage(args[i]);
                    else
                      a->openUrl(QUrl::fromUserInput(args[i]));
                }
                else
                    corpusPaths.append(args[i]);
            }
        }

        if (dactIsRunning)
          return 0;

        a->openMacros(macroPaths);

        if (corpusPaths.size() != 0)
          a->openCorpora(corpusPaths);
        
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
