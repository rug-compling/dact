#ifndef DACTHELPWINDOW_H
#define DACTHELPWINDOW_H

#include <QSharedPointer>
#include <QWidget>
#include <Qt>

namespace Ui {
	class DactHelpWindow;
}

class DactHelpWindow : public QWidget {
    Q_OBJECT
public:
    DactHelpWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DactHelpWindow();

private:
    QSharedPointer<Ui::DactHelpWindow> d_ui;
};

#endif // DACTHELPWINDOW_H
