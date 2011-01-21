#include <QDialog>
#include <QSharedPointer>
#include <QWidget>

#include <OpenProgressDialog.hh>

#include <ui_OpenProgressDialog.h>

OpenProgressDialog::OpenProgressDialog(QWidget *parent) :
        QDialog(parent),
        d_ui(QSharedPointer<Ui::OpenProgressDialog>(new Ui::OpenProgressDialog))
{
    d_ui->setupUi(this);
}

OpenProgressDialog::~OpenProgressDialog() {}