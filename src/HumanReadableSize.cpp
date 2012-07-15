#include "HumanReadableSize.hh"

#include <QStringList>

QString humanReadableSize(float num)
{
    // Source: http://lists.qt.nokia.com/pipermail/qt-interest/2010-August/027043.html
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while (num >= 1024.0 && i.hasNext())
    {
        unit = i.next();
        num /= 1024.0;
    }

    return QString().setNum(num,'f',2) + " " + unit;
}