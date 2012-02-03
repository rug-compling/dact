#ifndef WORKSPACE_HH
#define WORKSPACE_HH

#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QStringList>

class Workspace
{
public:
    /**
     * Open a default workspace. Writes settings to the global Dact
     * settings.
     */
    Workspace();

    /**
     * Open a workspace.
     *
     * @filename The workspace to open.
     */
    Workspace(QString const &filename);

    ~Workspace();

    QString corpus();

    QStringList history();

    QString macrosFilename();

    void save();

    void saveAs(QString const &filename);

    QString setCorpus(QString const &filename);
    
    void setHistory(QStringList const &history);

    void setMacrosFilename(QString const &filename);


private:
    void readWorkspace();

    QSharedPointer<QSettings> d_settings;

    QString d_corpus;
    QString d_macrosFilename;
    QStringList d_history;
};

#endif // WORKSPACE_HH

