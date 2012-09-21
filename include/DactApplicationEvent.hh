#ifndef DACTAPPLICATIONEVENT_H
#define DACTAPPLICATIONEVENT_H

#include <QEvent>
#include <QVariant>

class DactApplicationEvent : public QEvent
{
public:
	static const QEvent::Type CorpusOpen;
	static const QEvent::Type MacroOpen;
	static const QEvent::Type UrlOpen;

	DactApplicationEvent(QEvent::Type type, QVariant const &data);
	QVariant const &data() const;

private:
	QVariant d_data;
};

#endif
