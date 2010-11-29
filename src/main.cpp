#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QFont>

#include <memory>

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
    QSettings settings("RUG", "Dact");
    QVariant fontValue = settings.value("appFont", qApp->font().toString());
    QFont appFont;
    appFont.fromString(fontValue.toString());
    qApp->setFont(appFont);

    xmlInitMemory();
    xmlInitParser();

    // EXSLT extensions
    exsltRegisterAll();

    // XPath
    xmlXPathInit();

    std::auto_ptr<DactMainWindow> w(new DactMainWindow);
    w->show();

    if (qApp->arguments().size() == 2)
        w->readCorpus(qApp->arguments().at(1));

    int r = a.exec();

    xsltCleanupGlobals();
    xmlCleanupParser();

    return r;
}
