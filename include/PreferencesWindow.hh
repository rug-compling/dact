#ifndef PREFERENCES_WINDOW_HH 
#define PREFERENCES_WINDOW_HH

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
private slots:
    void saveKeywordsInContextColors();
    void selectAppFont();
    
private:
    void applyAppFont();
    void loadKeywordsInContextColors();
    QSharedPointer<Ui::PreferencesWindow> d_ui;
};

#endif // PREFERENCES_WINDOW_HH
