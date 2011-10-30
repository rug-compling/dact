#include "RecentFilesMenu.hh"

#include <QFileIconProvider>
#include <QSettings>

RecentFilesMenu::RecentFilesMenu(QWidget *parent)
:
    QMenu(parent)
{    
    readSettings();

    updateMenu();
}

void RecentFilesMenu::pruneFileList()
{
    while (d_files.size() > maximum_files)
        d_files.removeLast();
}

void RecentFilesMenu::readSettings()
{
    // read all files from the settings into d_files.    
    d_files.clear();
    
    QSettings settings;

    int size = settings.beginReadArray("recent_files");
    
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        
        QFileInfo file(settings.value("path").toString());
        d_files.append(file);
    }

    settings.endArray();

    // make sure the menu is never longer than needed
    pruneFileList();
}

void RecentFilesMenu::writeSettings()
{
    // dump d_files into the settings
    QSettings settings;

    settings.beginWriteArray("recent_files", d_files.size());

    for (QList<QFileInfo>::const_iterator itr = d_files.begin(); itr != d_files.end(); itr++)
    {
        settings.setArrayIndex(itr - d_files.begin());
        settings.setValue("path", itr->filePath());
    }

    settings.endArray();
}

void RecentFilesMenu::addFile(QString const &file)
{
    // remove any occurrence of file already in the menu
    d_files.removeAll(file);

    // and add it as new on the top of the list
    d_files.prepend(file);
    
    // don't make the list too long
    pruneFileList();    

    updateMenu();
    
    writeSettings();
}

void RecentFilesMenu::updateMenu()
{
    // disable the complete Recent menu when there are no recent files
    setEnabled(d_files.size() > 0);
    
    clear();

    QFileIconProvider iconProvider;

    // Add actions to this menu, complete with data for the signal handler
    foreach (QFileInfo const &file, d_files)
    {
        QAction *openFileAction = new QAction(file.fileName(), this);
        openFileAction->setData(file.filePath());
        connect(openFileAction, SIGNAL(triggered()), SLOT(openFile()));
        
        // icons! Makes it easy to spot different types of corpora
        openFileAction->setIcon(iconProvider.icon(file));
        openFileAction->setIconVisibleInMenu(true);
        
        addAction(openFileAction);
    }

    addSeparator();

    // and add a Clear menu item for those that want to have a little privacy.
    QAction *clearAction = new QAction("Clear menu", this);
    connect(clearAction, SIGNAL(triggered()), SLOT(clearMenu()));
    addAction(clearAction);
}

void RecentFilesMenu::clearMenu()
{
    d_files.clear();
    
    updateMenu();

    writeSettings();
}

void RecentFilesMenu::openFile()
{
    QAction *openFileAction = reinterpret_cast<QAction*>(sender());
    
    emit fileSelected(openFileAction->data().toString());
}

