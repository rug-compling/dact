#include <QtGui/QApplication>
#include "DactMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DactMainWindow w;
    w.show();
    return a.exec();
}
