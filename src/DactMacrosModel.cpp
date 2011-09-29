#include <QDebug>
#include <QFileInfo>
#include <QSettings>

#include "DactMacrosModel.hh"

const QChar DactMacrosModel::d_symbol('%');
const QString DactMacrosModel::d_assignment_symbol("=");
const QString DactMacrosModel::d_start_replacement_symbol("\"\"\"");
const QString DactMacrosModel::d_end_replacement_symbol("\"\"\"");

DactMacrosModel::DactMacrosModel(QObject *parent)
:
    QAbstractTableModel(parent),
    d_macros(readMacros())
{
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
    
    if (column == 0)
        return tr("Pattern");
    else if (column == 1)
        return tr("Replacement");
    else
        return QVariant();
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
        
        if(index.column() == 0)
            return macro.pattern;
        else if(index.column() == 1)
            return macro.replacement;
    }
    
    return QVariant();
}

bool DactMacrosModel::insertRows(int position, int rows, const QModelIndex &index)
{
    beginInsertRows(index, position, position + rows - 1);
    
    for (int row = 0; row < rows; ++row)
    {
        DactMacro macro;
        macro.pattern = QString();
        macro.replacement = QString();
        d_macros.insert(position, macro);
    }
    
    endInsertRows();
    
    return true;
}

bool DactMacrosModel::removeRows(int position, int rows, const QModelIndex &index)
{
    beginRemoveRows(index, position, position + rows - 1);
    
    for (int row = 0; row < rows; ++row)
        d_macros.removeAt(position);
    
    endRemoveRows();
    
    writeMacros(d_macros);
    
    return true;
}

bool DactMacrosModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    
    DactMacro macro = d_macros.at(index.row());
    
    if (index.column() == 0)
        macro.pattern = value.toString();
    else if (index.column() == 1)
        macro.replacement = value.toString();
    
    d_macros.replace(index.row(), macro);
    
    emit dataChanged(index,index);
    
    writeMacros(d_macros);
    
    return true;
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

        qDebug() << assignment_symbol_pos
                 << opening_quotes_pos
                 << closing_quotes_pos;

        // and go get it!
        DactMacro macro;
        macro.pattern = data.mid(cursor, assignment_symbol_pos - cursor).trimmed();
        macro.replacement = data.mid(opening_quotes_pos + d_start_replacement_symbol.size(),
            closing_quotes_pos - (opening_quotes_pos + d_start_replacement_symbol.size())).trimmed();
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

QList<DactMacro> DactMacrosModel::readMacros() const
{
    QFileInfo old_path(QSettings().fileName());
    QFileInfo new_path(old_path.path() + "/macros");

    if (!new_path.exists())
        return QList<DactMacro>();
    
    QFile new_file(new_path.absoluteFilePath());
    return readMacros(new_file);
}

void DactMacrosModel::writeMacros(QList<DactMacro> const &macros) const
{
    QFileInfo old_path(QSettings().fileName());
    QFileInfo new_path(old_path.path() + "/macros");

    QFile new_file(new_path.absoluteFilePath());
    writeMacros(macros, new_file);
}

QString DactMacrosModel::expand(QString const &expression)
{
    QString query(expression);
    
    foreach (DactMacro const &macro, d_macros)
        query = query.replace(d_symbol + macro.pattern + d_symbol, macro.replacement);
    
    return query;
}