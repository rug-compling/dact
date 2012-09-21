#include "DactApplicationEvent.hh"

const QEvent::Type DactApplicationEvent::CorpusOpen = (QEvent::Type) QEvent::registerEventType();
const QEvent::Type DactApplicationEvent::MacroOpen = (QEvent::Type) QEvent::registerEventType();
const QEvent::Type DactApplicationEvent::UrlOpen = (QEvent::Type) QEvent::registerEventType();

DactApplicationEvent::DactApplicationEvent(QEvent::Type type, QVariant const &data)
:
	QEvent(type),
	d_data(data)
{
	//
}

QVariant const &DactApplicationEvent::data() const
{
	return d_data;
}
