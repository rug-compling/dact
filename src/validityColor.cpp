#include <QLineEdit>
#include <stdexcept>

#include "ValidityColor.hh"

void applyValidityColor(QObject *sendero)
{
    QLineEdit *sender = qobject_cast<QLineEdit *>(sendero);

    if (!sender)
        throw std::logic_error("applyValidityColor called on non-QLineEdit");

    sender->setStyleSheet(sender->hasAcceptableInput()
                          ? ""
                          : "background-color: salmon");
}
