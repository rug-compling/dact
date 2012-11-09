#include <QLineEdit>
#include <stdexcept>

#include "ValidityColor.hh"
#include "XPathValidator.hh"

void applyValidityColor(QObject *sendero)
{
    QLineEdit *sender = qobject_cast<QLineEdit *>(sendero);

    if (!sender)
        throw std::logic_error("applyValidityColor called on non-QLineEdit");

    QString style;

    if (!sender->hasAcceptableInput())
    	style = "background-color: salmon";
    else
    {
    	XPathValidator const *validator = qobject_cast<XPathValidator const *>(sender->validator());
    	if (validator && sender->text() != "" && !validator->validateAgainstDTD(sender->text()))
    		style = "background-color: yellow";
    	else
    		style = "";
    }
    
	sender->setStyleSheet(style);
}
