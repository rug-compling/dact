#include <QDialog>
#include <QSharedPointer>
#include <QWidget>

#include <DactProgressDialog.hh>

#include <ui_DactProgressDialog.h>

DactProgressDialog::DactProgressDialog(QWidget *parent) :
        QDialog(parent),
        d_ui(QSharedPointer<Ui::DactProgressDialog>(new Ui::DactProgressDialog))
{
    d_ui->setupUi(this);
}

DactProgressDialog::~DactProgressDialog() {}

void DactProgressDialog::open()
{
    // This will reset the progress bar to its undetermined state
    setMaximum(0);
    setProgress(-1);
    
    QDialog::open();
}

void DactProgressDialog::setCancelable(bool cancelable)
{
    d_ui->buttonBox->setEnabled(cancelable);
}

void DactProgressDialog::setMaximum(int max)
{
    d_ui->progressBar->setMaximum(max);
}

void DactProgressDialog::setProgress(int progress)
{
    d_ui->progressBar->setValue(progress);
}