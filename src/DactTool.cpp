#include <QDebug>
#include <QProcess>
#include "DactTool.hh"

DactTool::DactTool()
{
	//
}

DactTool::DactTool(QString const &name, QString const &command)
:
	d_name(name),
	d_command(command)
{
	//
}

QString const &DactTool::name() const
{
	return d_name;
}

QString const &DactTool::command() const
{
	return d_command;
}

void DactTool::run(QString const &identifier) const
{
	QString command = d_command.arg(identifier);

	qDebug() << "Calling" << command;

	QProcess process;
	process.start(command);
}