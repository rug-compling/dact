#include <stdexcept>

#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QtDebug>

#include <Workspace.hh>

Workspace::Workspace() :
    d_settings(new QSettings)
{
    readWorkspace(true);
}

Workspace::Workspace(QString const &filename) :
    d_settings(new QSettings(filename, QSettings::IniFormat))
{
    if (!d_settings->isWritable()) {
        d_settings = QSharedPointer<QSettings>(new QSettings());

        throw std::runtime_error("Could not open workspace file for reading.");
    }

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

QStringList Workspace::macrosFilenames()
{
    return d_macrosFilenames;
}

void Workspace::readWorkspace(bool defaultWs)
{
    QVariant value = d_settings->value("corpus", QString());
    if (value.type() == QVariant::String) {
        d_corpus = value.toString();

        QFileInfo corpusInfo(d_corpus);
        if (defaultWs && !d_corpus.startsWith("http://") && !corpusInfo.exists())
          d_corpus = QString();
    }
    else
        qWarning() << "Read corpus name, but it is not a QString.";

    value = d_settings->value("macrosFilenames", QStringList());
    d_macrosFilenames = value.toStringList();

    value = d_settings->value("filterHistory", QStringList());
    d_history = value.toStringList();
}

void Workspace::save()
{
    if (!d_corpus.isNull())
        d_settings->setValue("corpus", d_corpus);
    
    d_settings->setValue("macrosFilenames", d_macrosFilenames);

    d_settings->setValue("filterHistory", d_history);
}

void Workspace::saveAs(QString const &filename)
{
    d_settings = QSharedPointer<QSettings>(
        new QSettings(filename, QSettings::IniFormat));

    if (!d_settings->isWritable()) {
        d_settings = QSharedPointer<QSettings>(new QSettings());

        throw std::runtime_error("Could not open workspace file for writing.");
    }

    save();
}

void Workspace::setCorpus(QString const &filename)
{
    d_corpus = filename;
}

void Workspace::setHistory(QStringList const &history)
{
    d_history = history;
}

void Workspace::setMacrosFilenames(QStringList const &filenames)
{
    d_macrosFilenames = filenames;
}
