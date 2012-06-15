#include <QDebug>
#include <QProcess>
#include "DactTool.hh"

DactTool::DactTool()
{
	//
}

DactTool::DactTool(QString const &name, QString const &command, QString const &corpus)
:
	d_name(name),
	d_command(command),
	d_corpusNamePattern(corpus, Qt::CaseInsensitive, QRegExp::Wildcard)
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

bool DactTool::availableForCorpus(QString const &corpus) const
{
	return d_corpusNamePattern.isEmpty() || d_corpusNamePattern.exactMatch(corpus);
}