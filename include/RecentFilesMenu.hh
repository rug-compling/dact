#ifndef RECENTFILESMENU_HH
#define RECENTFILESMENU_HH

#include <QFileInfo>
#include <QList>
#include <QMenu>

class RecentFilesMenu : public QMenu
{
    Q_OBJECT

static const int maximum_files = 10;

public:
    RecentFilesMenu(QWidget *parent = 0);
    void addFile(QString const &file);

signals:
    void fileSelected(QString const &path);

protected slots:
    void updateMenu();
    void clearMenu();

private slots:
    void openFile();

private:
    void pruneFileList();
    void readSettings();
    void writeSettings();
    
    QList<QFileInfo> d_files;
};

#endif // RECENTFILESMENU_HH
