#ifndef DACTSETTINGS_H
#define DACTSETTINGS_H

#include <QSettings>
#include <QSharedPointer>

class DactSettings : public QObject
{
	Q_OBJECT

private:
	QSettings d_settings;
	static QSharedPointer<DactSettings> s_sharedInstance;

public:
	static QSharedPointer<DactSettings> sharedInstance();

	QVariant value(QString const &key, QVariant const &defaultValue = QVariant()) const;
	void setValue(QString const &key, QVariant const &value);

signals:
	void valueChanged(QString const &key, QVariant const &value);

private:
	DactSettings();
};

#endif
