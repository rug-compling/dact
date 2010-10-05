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

    xmlInitMemory();
    xmlInitParser();

    // EXSLT extensions
    exsltRegisterAll();

    // XPath
    xmlXPathInit();

    DactMainWindow *w;
    w = new DactMainWindow;
    w->show();

    if (qApp->arguments().size() == 2)
        w->readCorpus(qApp->arguments().at(1));

    int r = a.exec();
    delete w;

    xsltCleanupGlobals();
    xmlCleanupParser();

    return r;
}
