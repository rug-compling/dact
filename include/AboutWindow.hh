#ifndef ABOUTWINDOW_H
#define DACTWINDOW_H

#include <QSharedPointer>
#include <QWidget>

namespace Ui {
  class AboutWindow;
}

class AboutWindow : public QWidget {
    Q_OBJECT
public:
    AboutWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~AboutWindow();

private slots:
    void reportBug();

private:
    QSharedPointer<Ui::AboutWindow> d_ui;
};

#endif // ABOUTWINDOW_H
