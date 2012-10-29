#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include <GlobalMenuBar.hh>

#include <ui_GlobalMenuBar.h>

GlobalMenuBar::GlobalMenuBar(QWidget *parent) :
    QMenuBar(parent),
    d_ui(QSharedPointer<Ui::GlobalMenuBar>(new Ui::GlobalMenuBar))
{
    d_ui->setupUi(this);
}
