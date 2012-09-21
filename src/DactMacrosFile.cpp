#include <string>

#include <QStringList>
#include <QTextStream>
#include <QtDebug>

#include <AlpinoCorpus/macros.hh>

#include "DactMacrosFile.hh"


const QString DactMacrosFile::d_assignment_symbol("=");
const QString DactMacrosFile::d_start_replacement_symbol("\"\"\"");
const QString DactMacrosFile::d_end_replacement_symbol("\"\"\"");

DactMacrosFile::DactMacrosFile(QString const &fileName)
:
    d_file(fileName)
{
    reload();
}

void DactMacrosFile::reload()
{
    d_macros = read(d_file);
}

QFile const &DactMacrosFile::file() const
{
	return d_file;
}

QList<DactMacro> const &DactMacrosFile::macros() const
{
    return d_macros;
}

QList<DactMacro> DactMacrosFile::read(QFile &file) const
{   
    QList<DactMacro> dactMacros;

    std::string fn = file.fileName().toUtf8().constData();
    
    alpinocorpus::Macros macros;

    macros = alpinocorpus::loadMacros(fn);

    for (alpinocorpus::Macros::const_iterator iter = macros.begin();
        iter != macros.end(); ++iter)
    {
        DactMacro macro = { QString::fromUtf8(iter->first.c_str()),
            QString::fromUtf8(iter->second.c_str()) };
        dactMacros.push_back(macro);
    }


    return dactMacros;
}

void DactMacrosFile::write(QFile &file, QList<DactMacro> const &macros) const
{
    file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    QTextStream macro_data(&file);

    foreach (DactMacro const &macro, macros)
    {
        macro_data << macro.pattern
            << " " << d_assignment_symbol << " "
            << d_start_replacement_symbol << macro.replacement << d_end_replacement_symbol
            << '\n';
    }

    file.close();
}
