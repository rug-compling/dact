#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>

#include "DactMacrosModel.hh"

const QChar DactMacrosModel::d_symbol('%');
const QString DactMacrosModel::d_assignment_symbol("=");
const QString DactMacrosModel::d_start_replacement_symbol("\"\"\"");
const QString DactMacrosModel::d_end_replacement_symbol("\"\"\"");

DactMacrosModel::DactMacrosModel(QObject *parent)
:
    QAbstractTableModel(parent)
{
    connect(&d_watcher, SIGNAL(fileChanged(QString const &)),
        SLOT(fileChanged(QString const &)));
}
    
int DactMacrosModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d_macros.size();
}

int DactMacrosModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant DactMacrosModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) 
        return QVariant();
    
    if (orientation != Qt::Horizontal)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("Pattern");
    
        case 1:
            return tr("Replacement");
    
        case 2:
            return tr("Source file");
    
        default:
            return QVariant();   
    }
}

QVariant DactMacrosModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    if (index.row() >= d_macros.size() || index.row() < 0)
        return QVariant();
        
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        DactMacro macro = d_macros.at(index.row());
        
        switch (index.column())
        {
            case 0:
                return macro.pattern;
            
            case 1:
                return macro.replacement;
            
            case 2:
                return *macro.source;
        }
    }
    
    return QVariant();
}

Qt::ItemFlags DactMacrosModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QList<DactMacro> DactMacrosModel::readMacros(QFile &file) const
{   
    QList<DactMacro> macros;
    QSharedPointer<QString> source(new QString(file.fileName()));

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
        macro.source = source;
        macros.append(macro);

        cursor = closing_quotes_pos + d_end_replacement_symbol.size();
    }

    return macros;
}

void DactMacrosModel::writeMacros(QList<DactMacro> const &macros, QFile &file) const
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

void DactMacrosModel::watchFile(QString const &path)
{
    // TODO maybe be able to load (and watch) multiple files?
    // but then we need an interface to stop watching them.
    d_watcher.removePaths(d_watcher.files());

    d_watcher.addPath(path);
    fileChanged(path);
}

void DactMacrosModel::fileChanged(QString const &file_name)
{
    // TODO only delete macros with file_name as source
    // this way we can load and watch multiple macro files :D
    QFile file(file_name);
    d_macros = readMacros(file);
}

QString DactMacrosModel::expand(QString const &expression)
{
    QString query(expression);
    
    foreach (DactMacro const &macro, d_macros)
        query = query.replace(d_symbol + macro.pattern + d_symbol, macro.replacement);
    
    return query;
}