#include "DactQueryHistory.hh"

#include <QSettings>

DactQueryHistory::DactQueryHistory()
:
    d_completer(new QCompleter())
{
    readHistory();
    d_completer->setModel(this);
}

QCompleter* DactQueryHistory::completer()
{
    return d_completer;
}

int DactQueryHistory::rowCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent);
    return d_history.size();
}

QVariant DactQueryHistory::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > d_history.size())
        return QVariant();
    
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return d_history[index.row()];
    
    return QVariant();
}

void DactQueryHistory::addToHistory(QString const &query)
{
    d_history.append(query);
    writeHistory();
}

void DactQueryHistory::readHistory()
{
    QSettings settings;
    
    int size = settings.beginReadArray("history");

    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        d_history.append(settings.value("query").toString());
    }
    
    settings.endArray();
}

void DactQueryHistory::writeHistory()
{
    QSettings settings;
    
    settings.beginWriteArray("history");
    
    for (int i = 0; i < d_history.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("query", d_history[i]);
    }
    
    settings.endArray();
}