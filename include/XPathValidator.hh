#ifndef XPATHVALIDATOR_HH
#define XPATHVALIDATOR_HH

#include <QObject>
#include <QSharedPointer>
#include <QValidator>

#include "DactMacrosModel.h"

class XPathValidator : public QValidator
{
    Q_OBJECT
public:
    XPathValidator(QObject *parent = 0, bool variables = false);
	XPathValidator(QSharedPointer<DactMacrosModel> macrosModel, QObject *parent = 0, bool variables = false);
    State validate(QString &exprStr, int &pos) const;
private:
    bool d_variables;
	QSharedPointer<DactMacrosModel> d_macrosModel;
};

#endif // XPATHVALIDATOR_HH
