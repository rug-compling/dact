#include <QtGui/QApplication>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libexslt/exslt.h>
}

#include "DactMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // EXSLT extensions
    exsltCommonRegister();
    exsltSaxonRegister();
    exsltSetsRegister();

    DactMainWindow w;
    w.show();
    int r = a.exec();

    xsltCleanupGlobals();
    xmlCleanupParser();

    return r;
}
