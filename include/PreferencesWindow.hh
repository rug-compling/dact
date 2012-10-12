#ifndef PREFERENCES_WINDOW_HH 
#define PREFERENCES_WINDOW_HH

#include <QSharedPointer>
#include <QWidget>

namespace Ui {
    class PreferencesWindow;
}

class PreferencesWindow : public QWidget
{
    Q_OBJECT
public:
    PreferencesWindow(QWidget *parent = 0, Qt::WindowFlags f = Qt::Window);
    ~PreferencesWindow();
protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void colorChanged(); // More fine-grained???
    void toolsChanged();

private slots:
    void restoreDefaultColors();
    void restoreDefaultNetwork();
    void restoreDefaultRemote();
    void restoreDefaultWebservice();
    void saveArchiveBaseUrl();
    void saveRemoteBaseUrl();
    void saveWebserviceBaseUrl();
    void saveColorsTab();
    void saveToolsTab();
    void saveAppearanceTab();
    void selectAppFont();
    void selectToolsFilePath();
    
private:
    void loadAppearanceTab();
    void loadColorsTab();
    void loadNetworkTab();
    void loadRemoteTab();
    void loadToolsTab();
    QSharedPointer<Ui::PreferencesWindow> d_ui;
};

#endif // PREFERENCES_WINDOW_HH
