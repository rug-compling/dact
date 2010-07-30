#ifndef XPATHVALIDATOR_HH
#define XPATHVALIDATOR_HH

#include <QObject>
#include <QValidator>

class XPathValidator : public QValidator
{
    Q_OBJECT
public:
    XPathValidator(QObject *parent = 0);
    State validate(QString &exprStr, int &pos) const;
};

#endif // XPATHVALIDATOR_HH
