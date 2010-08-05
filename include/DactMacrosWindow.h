#ifndef DACTMACROSWINDOW_H
#define DACTMACROSWINDOW_H

#include <QCloseEvent>
#include <QList>
#include <QPair>
#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include <Qt>

#include "DactMacrosModel.h"

namespace Ui {
	class DactMacrosWindow;
}

class DactMacrosWindow : public QWidget {
    Q_OBJECT
public:
    DactMacrosWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DactMacrosWindow();

private slots:
    void addButtonPressed();
    void removeButtonPressed();
    void macrosTableSelectionChanged();

protected:
    void closeEvent(QCloseEvent *event); // save window dimensions on close.

private:
    void createActions();
    void readSettings();
    void writeSettings();
    
    DactMacrosModel d_model;
    QSharedPointer<Ui::DactMacrosWindow> d_ui;
};

#endif // DACTMACROSWINDOW_H
