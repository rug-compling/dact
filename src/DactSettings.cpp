#include "DactSettings.hh"

QSharedPointer<DactSettings> DactSettings::s_sharedInstance;

QSharedPointer<DactSettings> DactSettings::sharedInstance()
{
	if (s_sharedInstance.isNull())
		s_sharedInstance = QSharedPointer<DactSettings>(new DactSettings());
	
	return s_sharedInstance;
}

DactSettings::DactSettings()
{
	//
}

QVariant DactSettings::value(QString const &key, QVariant const &defaultValue) const
{
	return d_settings.value(key, defaultValue);
}

void DactSettings::setValue(QString const &key, QVariant const &value)
{
	d_settings.setValue(key, value);
	emit valueChanged(key, value);
}
