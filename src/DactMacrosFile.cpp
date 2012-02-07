#include "DactMacrosFile.hh"

#include <QStringList>
#include <QTextStream>

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
    QList<DactMacro> macros;
    QString data;
    
    // XXX - a nice parser would parse directly from the QTextStream
    {
        file.open(QIODevice::ReadOnly);
        QTextStream macro_data(&file);
        data = macro_data.readAll();
        file.close();
    }

    int cursor = 0;

    while (cursor < data.size())
    {
        // find '=' symbol, which indicates the end of the name of the macro
        int assignment_symbol_pos = data.indexOf(d_assignment_symbol, cursor);

        if (assignment_symbol_pos == -1)
            break;

        // find the '"""' symbol which indicates the start of the replacement
        int opening_quotes_pos = data.indexOf(d_start_replacement_symbol,
            assignment_symbol_pos + d_assignment_symbol.size());

        if (opening_quotes_pos == -1)
            break;

        // find the second '"""' symbol which marks the end of the replacement
        int closing_quotes_pos = data.indexOf(d_end_replacement_symbol,
            opening_quotes_pos + d_start_replacement_symbol.size());

        if (closing_quotes_pos == -1)
            break;

        // and go get it!
        DactMacro macro;
        macro.pattern = data.mid(cursor, assignment_symbol_pos - cursor).trimmed();
        macro.replacement = data.mid(opening_quotes_pos + d_start_replacement_symbol.size(),
          closing_quotes_pos - (opening_quotes_pos + d_start_replacement_symbol.size())).trimmed();

        // Apply substitutions
        for (QList<DactMacro>::const_iterator iter = macros.begin();
            iter != macros.end(); ++iter)
          macro.replacement = macro.replacement.replace("%" + iter->pattern + "%", iter->replacement);

        macros.append(macro);

        cursor = closing_quotes_pos + d_end_replacement_symbol.size();
    }

    return macros;
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
