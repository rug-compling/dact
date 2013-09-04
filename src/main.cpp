#include <QApplication>
#include <QFont>
#include <QSettings>
#include <QVariant>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include <DactApplication.hh>
#include <ProgramOptions.hh>

namespace {
    void usage(char const *progname)
    {
        std::cerr << "Usage: " << progname << " [OPTION] corpus ..." <<
          std::endl << std::endl <<
          "  -m filename\tMacro file, multiple files are separated by a colon (:)" << std::endl <<
          "  -u URI\tURI, for instance a query URI" << std::endl << std::endl;
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
    
        QVariant fontValue = settings.value("appFont", qApp->font().toString());
        
#ifndef __APPLE__
        QFont appFont;
        appFont.fromString(fontValue.toString());
        a->setFont(appFont);
#endif

        a->init();
    
        QStringList corpusPaths, macroPaths;

        ProgramOptions options(argc, const_cast<char const **>(argv), "m:u:");

        // Process macros
        if (options.option('m'))
        {
            QString macros = QString::fromStdString(options.optionValue('m'));
            macroPaths = macros.split(':', QString::SkipEmptyParts);
        }

        // Corpus paths
        for (std::vector<std::string>::const_iterator iter = options.arguments().begin();
                iter != options.arguments().end(); ++iter)
            corpusPaths.push_back(QString::fromStdString(*iter));

        // URI
        if (options.option('u'))
        {
            QString uri = QString::fromStdString(options.optionValue('u'));

            if (uri.startsWith("dact:"))
            {
                if (dactIsRunning)
                    a->sendMessage(uri);
                else
                    a->openUrl(QUrl::fromUserInput(uri));
            }
        }

        if (dactIsRunning)
          return 0;

        a->openMacros(macroPaths);

        if (corpusPaths.size() != 0)
          a->openCorpora(corpusPaths);
        
        r = a->exec();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl << std::endl;
        usage(argv[0]);
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
