#ifndef DACTMACROSWINDOW_H
#define DACTMACROSWINDOW_H

#include <QCloseEvent>
#include <QSharedPointer>
#include <QWidget>

#include "DactMacrosModel.hh"

namespace Ui {
	class DactMacrosWindow;
}

class DactMacrosWindow : public QWidget {
    Q_OBJECT
public:
    DactMacrosWindow(QSharedPointer<DactMacrosModel> model, QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DactMacrosWindow();

private slots:
    void addButtonPressed();
    void removeButtonPressed();
    void macrosTableSelectionChanged();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.
    void keyPressEvent(QKeyEvent *event);

private:
    void createActions();
    void readSettings();
    void writeSettings();
    
    QSharedPointer<DactMacrosModel> d_model;
    QSharedPointer<Ui::DactMacrosWindow> d_ui;
};

#endif // DACTMACROSWINDOW_H
