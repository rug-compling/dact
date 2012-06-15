#ifndef DACTTOOL_H
#define DACTTOOL_H

#include <QMetaType>
#include <QString>
#include <QRegExp>

// Currently macros are just structs. This might become something decent once
// the macro-related code is abstracted out of DactMacrosModel.h which contains
// a lot of code just for the tableview.

class DactTool : public QObject
{
	Q_OBJECT;

public:
	DactTool();
	DactTool(QString const &name, QString const &command, QString const &corpus = QString());
	QString const &name() const;
	QString const &command() const;
	QString const &corpus() const;

	bool availableForCorpus(QString const &corpus) const;

private:
    QString d_name;
    QString d_command;
    QRegExp d_corpusNamePattern;
};

#endif
