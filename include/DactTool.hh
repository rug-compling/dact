#ifndef DACTTOOL_H
#define DACTTOOL_H

#include <QMetaType>
#include <QString>

// Currently macros are just structs. This might become something decent once
// the macro-related code is abstracted out of DactMacrosModel.h which contains
// a lot of code just for the tableview.

class DactTool : public QObject
{
	Q_OBJECT;

public:
	DactTool();
	DactTool(QString const &name, QString const &command);
	QString const &name() const;
	QString const &command() const;
	void run(QString const &identifier) const;

private:
    QString d_name;
    QString d_command;
};

#endif
