#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>

#include "DactToolModel.hh"

const QString DactToolModel::s_assignment_symbol("=");
const QString DactToolModel::s_start_replacement_symbol("\"\"\"");
const QString DactToolModel::s_end_replacement_symbol("\"\"\"");

DactToolModel::DactToolModel(QList<DactTool*> tools, QObject *parent)
:
    QAbstractItemModel(parent),
    d_tools(tools)
{
    //
}

DactToolModel::~DactToolModel()
{
    foreach (DactTool *tool, d_tools)
        delete tool;
}

QSharedPointer<DactToolModel> DactToolModel::s_sharedInstance;

QSharedPointer<DactToolModel> DactToolModel::sharedInstance()
{
    if (s_sharedInstance.isNull())
    {
        QSettings settings;
        QFile file(settings.value("ToolFile").toString());

        if (file.exists())
            s_sharedInstance = DactToolModel::loadFromFile(file);
        else
            s_sharedInstance = QSharedPointer<DactToolModel>(new DactToolModel());
    }

    return s_sharedInstance;
}
    
int DactToolModel::columnCount(const QModelIndex &parent) const
{
    return 2; // name and command
}

int DactToolModel::rowCount(const QModelIndex &parent) const
{
    // No index? You must be new here. A row for each file
    if (!parent.isValid())
        return d_tools.size();

    // Tool's themselves have no rows
    return 0;
}

QVariant DactToolModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) 
        return QVariant();
    
    if (orientation != Qt::Horizontal)
        return QVariant();
    
    switch (column)
    {
        case 0:
            return tr("Name");
    
        case 1:
            return tr("Command");
    
        default:
            return QVariant();   
    }
}

QVariant DactToolModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d_tools.size())
        return QVariant();
    
    DactTool *tool = d_tools[index.row()];
    
    if (role == Qt::DisplayRole)
        switch (index.column())
        {
            case COLUMN_NAME:
                return tool->name();
            
            case COLUMN_COMMAND:
                return tool->command();
            
            default:
                return QVariant();
        }

    else
        return QVariant();
}

QModelIndex DactToolModel::index(int row, int column, QModelIndex const &parent) const
{
    // Invalid parent -> coordinates point to a file
    if (!parent.isValid() && row < d_tools.size() && (column == 0 || column == 1))
        return createIndex(row, column);
    
    return QModelIndex();
}

QModelIndex DactToolModel::parent(QModelIndex const &index) const
{
    return QModelIndex();
}

QSharedPointer<DactToolModel> DactToolModel::loadFromFile(QFile &file)
{
    QList<DactTool*> tools;
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
        int assignment_symbol_pos = data.indexOf(s_assignment_symbol, cursor);

        if (assignment_symbol_pos == -1)
            break;

        // find the '"""' symbol which indicates the start of the replacement
        int opening_quotes_pos = data.indexOf(s_start_replacement_symbol,
            assignment_symbol_pos + s_assignment_symbol.size());

        if (opening_quotes_pos == -1)
            break;

        // find the second '"""' symbol which marks the end of the replacement
        int closing_quotes_pos = data.indexOf(s_end_replacement_symbol,
            opening_quotes_pos + s_start_replacement_symbol.size());

        if (closing_quotes_pos == -1)
            break;

        // and go get it!
        QString name = data.mid(cursor, assignment_symbol_pos - cursor).trimmed();
        QString command = data.mid(opening_quotes_pos + s_start_replacement_symbol.size(),
          closing_quotes_pos - (opening_quotes_pos + s_start_replacement_symbol.size())).trimmed();

        tools.append(new DactTool(name, command));

        cursor = closing_quotes_pos + s_end_replacement_symbol.size();
    }

    return QSharedPointer<DactToolModel>(new DactToolModel(tools));
}