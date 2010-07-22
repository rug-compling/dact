#include <QtGui/QApplication>

extern "C" {
#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>
}

#include "DactMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // EXSLT extensions
    exsltCommonRegister();
    exsltDynRegister();
    exsltSaxonRegister();
    exsltSetsRegister();
    exsltStrRegister();

    // XPath
    xmlXPathInit();

    DactMainWindow *w;
    if (qApp->arguments().size() == 2)
        w = new DactMainWindow(qApp->arguments().at(1));
    else
        w = new DactMainWindow;
    w->show();

    int r = a.exec();
    delete w;

    xsltCleanupGlobals();
    xmlCleanupParser();

    return r;
}
