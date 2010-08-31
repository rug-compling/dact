#ifndef XPATHVALIDATOR_HH
#define XPATHVALIDATOR_HH

#include <QObject>
#include <QValidator>

class XPathValidator : public QValidator
{
    Q_OBJECT
public:
    XPathValidator(QObject *parent = 0, bool variables = false);
    State validate(QString &exprStr, int &pos) const;
private:
    bool d_variables;
};

#endif // XPATHVALIDATOR_HH
