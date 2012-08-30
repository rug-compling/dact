#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>

#include "DactSettings.hh"
#include "DactToolsModel.hh"

const QString DactToolsModel::s_assignment_symbol("=");
const QString DactToolsModel::s_start_replacement_symbol("\"\"\"");
const QString DactToolsModel::s_end_replacement_symbol("\"\"\"");
const QString DactToolsModel::s_start_section_symbol("[");
const QString DactToolsModel::s_end_section_symbol("]");

DactToolsModel::DactToolsModel(QList<DactTool*> tools, QObject *parent)
:
    QAbstractItemModel(parent),
    d_tools(tools)
{
    connect(&d_watcher, SIGNAL(fileChanged(QString const &)),
        SLOT(reloadFromFile(QString const &)));
}

DactToolsModel::~DactToolsModel()
{
    clear();
}

QSharedPointer<DactToolsModel> DactToolsModel::s_sharedInstance;

QSharedPointer<DactToolsModel> DactToolsModel::sharedInstance()
{
    if (s_sharedInstance.isNull())
    {
        QSettings settings;
        QFile file(settings.value("toolsFilePath").toString());

        s_sharedInstance = QSharedPointer<DactToolsModel>(new DactToolsModel());

        if (file.exists())
            s_sharedInstance->watchFile(file);

        connect(DactSettings::sharedInstance().data(),
            SIGNAL(valueChanged(QString const &, QVariant const &)),
            s_sharedInstance.data(), SLOT(preferenceChanged(QString const &, QVariant const &)));
    }

    return s_sharedInstance;
}
    
int DactToolsModel::columnCount(const QModelIndex &parent) const
{
    return 2; // name, command (and corpus)
}

int DactToolsModel::rowCount(const QModelIndex &parent) const
{
    // No index? You must be new here. A row for each file
    if (!parent.isValid())
        return d_tools.size();

    // Tool's themselves have no rows
    return 0;
}

QVariant DactToolsModel::headerData(int column, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) 
        return QVariant();
    
    if (orientation != Qt::Horizontal)
        return QVariant();
    
    switch (column)
    {
        case COLUMN_NAME:
            return tr("Name");
    
        case COLUMN_COMMAND:
            return tr("Command");
    
        // case COLUMN_CORPUS:
        //     return tr("Corpus");

        default:
            return QVariant();   
    }
}

QVariant DactToolsModel::data(const QModelIndex &index, int role) const
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
            
            // case COLUMN_CORPUS:
            //     return tool->corpus();

            default:
                return QVariant();
        }

    else
        return QVariant();
}

QModelIndex DactToolsModel::index(int row, int column, QModelIndex const &parent) const
{
    // Invalid parent -> coordinates point to a file
    if (!parent.isValid() && row < d_tools.size() && (column >= 0 && column <= 2))
        return createIndex(row, column);
    
    return QModelIndex();
}

QModelIndex DactToolsModel::parent(QModelIndex const &index) const
{
    return QModelIndex();
}

void DactToolsModel::preferenceChanged(QString const &key, QVariant const &value)
{
    if (key == "toolsFilePath")
    {
        clear();
        
        QFile file(value.toString());
        
        if (file.exists())
            watchFile(file);
    }
}

void DactToolsModel::readFromFile(QFile &file)
{
    QString data;
    QString section;

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

        // Or find a '[' symbol, indicating a section name.
        int section_start_symbol_pos = data.indexOf(s_start_section_symbol, cursor);

        // Jeej, first we will parse the section header
        if (section_start_symbol_pos > -1 && section_start_symbol_pos < assignment_symbol_pos)
        {
            int section_end_symbol_pos = data.indexOf(s_end_section_symbol, section_start_symbol_pos);

            // There is no section end? Bail out!
            if (section_end_symbol_pos == -1)
                break;

            section = data.mid(section_start_symbol_pos + 1, section_end_symbol_pos - section_start_symbol_pos - 1);
            cursor = section_end_symbol_pos + 1;

            // Jump back to beginning of the parser
            continue;
        }

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

        d_tools.append(new DactTool(name, command, section));

        cursor = closing_quotes_pos + s_end_replacement_symbol.size();
    }
}

void DactToolsModel::watchFile(QFile &file)
{
    // Clear current queries (they get in the way if you want to watch a file)
    // and clear out when reloading
    clear();

    // read the file initially
    readFromFile(file);

    // and watch only this file.
    foreach (QString const &watchedFile, d_watcher.files())
        d_watcher.removePath(watchedFile);

    d_watcher.addPath(file.fileName());
}

void DactToolsModel::reloadFromFile(QString const &path)
{
    clear();

    QFile file(path);
    readFromFile(file);
}

void DactToolsModel::clear()
{
    foreach (DactTool const *tool, d_tools)
        delete tool;

    d_tools.clear();
}

QList<DactTool const *> DactToolsModel::tools(QString const &corpus) const
{
    QFileInfo fileInfo(corpus);
    QString corpusName(fileInfo.baseName());

    QList<DactTool const *> tools;

    // Select only the tools that have no corpus specified, or a corpus 
    // with the same name as the basename of the supplied corpus name.
    foreach (DactTool const *tool, d_tools)
        if (tool->availableForCorpus(corpus))
            tools << tool;

    return tools;
}

