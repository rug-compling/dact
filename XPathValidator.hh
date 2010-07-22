#ifndef XPATHVALIDATOR_HH
#define XPATHVALIDATOR_HH

#include <QValidator>

class XPathValidator : public QValidator
{
    Q_OBJECT
public:
    XPathValidator();
    State validate(QString &exprStr, int &pos) const;
};

#endif // XPATHVALIDATOR_HH
