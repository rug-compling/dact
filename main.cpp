#include <QtGui/QApplication>

#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

#include "DactMainWindow.h"

XALAN_USING_XERCES(XMLPlatformUtils)
XALAN_USING_XALAN(XalanTransformer)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize Xalan.
    XMLPlatformUtils::Initialize();
    XalanTransformer::initialize();

    DactMainWindow w;
    w.show();
    int r = a.exec();

    // Deinitialize Xalan.
    XalanTransformer::terminate();
    XMLPlatformUtils::Terminate();
    XalanTransformer::ICUCleanUp();

    return r;
}
