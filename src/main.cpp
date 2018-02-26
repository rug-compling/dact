#include <QApplication>
#include <QFileInfo>
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
          "  -m filename\tMacro file, multiple files are separated by a colon (:)" << std::endl << std::endl;
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
#if defined(ENABLE_SANDBOXING)
        // Use different identifier for sandboxed version to avoid clashes
        // between application state (e.g. recent files).
        QCoreApplication::setOrganizationName("danieldk");
        QCoreApplication::setOrganizationDomain("danieldk.eu");
#else
        QCoreApplication::setOrganizationName("RUG");
        QCoreApplication::setOrganizationDomain("rug.nl");
#endif
        QCoreApplication::setApplicationName("Dact");

        QSettings settings;

#if defined(Q_WS_MAC)
        // Work around a bug (#56) where Qt 4.8.x on Mac OS X does not
        // invalidate the region of the popup when we hover as a result of
        // mouse scrolling.
        if (!settings.value("useNativeGraphicsSystem", false).toBool())
            QApplication::setGraphicsSystem("raster");

        // QTBUG-32789: GUI widgets use the wrong font on OS X Mavericks.
        if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
            QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
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

        QString uri;

        // Corpus paths and URIs.
        for (std::vector<std::string>::const_iterator iter = options.arguments().begin();
                iter != options.arguments().end(); ++iter)
        {
            QString arg = QString::fromStdString(*iter);

            if (arg.startsWith("dact:")) {
                uri = arg;
                continue;
            }

            // Dact may have been started in a different directory.
            QFileInfo corpusFileInfo(arg);
            corpusPaths.push_back(corpusFileInfo.absoluteFilePath());
        }

        if (dactIsRunning) {
            if (corpusPaths.size() != 0)
                a->sendMessage(QString("%1%2").arg(CORPUS_OPEN_MESSAGE,
                      corpusPaths.join(CORPUS_SEPARATOR)));

            if (!uri.isNull())
                a->sendMessage(uri);

          return 0;
        }

        a->openMacros(macroPaths);

        if (corpusPaths.size() != 0)
          a->openCorpora(corpusPaths);

        if (!uri.isNull())
            a->openUrl(QUrl::fromUserInput(uri));
        
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
