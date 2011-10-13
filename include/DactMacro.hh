#ifndef DACTMACRO_H
#define DACTMACRO_H

#include <QFile>
#include <QSharedPointer>
#include <QString>

// Currently macros are just structs. This might become something decent once
// the macro-related code is abstracted out of DactMacrosModel.h which contains
// a lot of code just for the tableview.

struct DactMacro {
    QString pattern;
    QString replacement;
    QSharedPointer<QString> source;
};

bool operator==(DactMacro const &left, DactMacro const &right);

Q_DECLARE_TYPEINFO(DactMacro, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(DactMacro);

#endif
