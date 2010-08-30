#include <QSharedPointer>
#include <QWidget>

#include <AboutWindow.hh>

#include <ui_AboutWindow.h>

AboutWindow::AboutWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::AboutWindow>(new Ui::AboutWindow))
{
    d_ui->setupUi(this);
}

AboutWindow::~AboutWindow()
{
}

