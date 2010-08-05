#ifndef DACTMACRO_H
#define DACTMACRO_H

#include <QString>

// Currently macros are just structs. This might become something decent once
// the macro-related code is abstracted out of DactMacrosModel.h which contains
// a lot of code just for the tableview.

struct DactMacro {
    QString pattern;
    QString replacement;
};

#endif
