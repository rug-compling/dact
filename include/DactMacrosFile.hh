#ifndef DACTMACROSFILE_H
#define DACTMACROSFILE_H

#include <QFile>
#include <QList>
#include <QString>

#include "DactMacro.hh"

class DactMacrosFile : public QObject
{
	Q_OBJECT

public:
	DactMacrosFile(QString const &fileName);
	
	QFile const &file() const;
	QList<DactMacro> const &macros() const;


public slots:
	void reload();

protected:
	virtual QList<DactMacro> read(QFile &file) const;
	virtual void write(QFile &file, QList<DactMacro> const &macros) const;

private:
	QFile d_file;
	QList<DactMacro> d_macros;

    static const QString d_assignment_symbol;
    static const QString d_start_replacement_symbol;
    static const QString d_end_replacement_symbol;
};

#endif


