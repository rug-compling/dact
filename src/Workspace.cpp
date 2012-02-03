#include <stdexcept>

#include <QSettings>
#include <QStringList>
#include <QtDebug>

#include <Workspace.hh>

Workspace::Workspace() :
    d_settings(new QSettings)
{
    readWorkspace();
}

Workspace::Workspace(QString const &filename) :
    d_settings(new QSettings(filename, QSettings::IniFormat))
{
    readWorkspace();
}

Workspace::~Workspace()
{
}

QString Workspace::corpus()
{
    return d_corpus;
}

QStringList Workspace::history()
{
    return d_history;
}

QString Workspace::macrosFilename()
{
    return d_macrosFilename;
}

void Workspace::readWorkspace()
{
    QVariant value = d_settings->value("corpus", QString());
    if (value.type() == QVariant::String)
        d_corpus = value.toString();
    else
        qWarning() << "Read corpus name, but it is not a QString.";

    value = d_settings->value("macrosFilename", QString());
    if (value.type() == QVariant::String)
        d_macrosFilename = value.toString();
    else
        qWarning() << "Read macro filename, but it is not a QString.";

    value = d_settings->value("filterHistory", QStringList());
    if (value.type() == QVariant::StringList)
        d_history = value.toStringList();
    else
        qWarning() << "Read history, but it is not a QStringList.";
}

void Workspace::save()
{
    if (!d_corpus.isNull())
        d_settings->setValue("corpus", d_corpus);
    
    if (!d_macrosFilename.isNull())
        d_settings->setValue("macrosFilename", d_macrosFilename);

    d_settings->setValue("filterHistory", d_history);
}

void Workspace::saveAs(QString const &filename)
{
    d_settings = QSharedPointer<QSettings>(
        new QSettings(filename, QSettings::IniFormat));


    if (!d_settings->isWritable()) {
        d_settings = QSharedPointer<QSettings>(
            new QSettings());

        throw std::runtime_error("Could not open workspace file for writing.");
    }

    save();
}


QString Workspace::setCorpus(QString const &filename)
{
    d_corpus = filename;
}

void Workspace::setHistory(QStringList const &history)
{
    d_history = history;
}

void Workspace::setMacrosFilename(QString const &filename)
{
    d_macrosFilename = filename;
}
