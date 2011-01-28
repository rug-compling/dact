#include <QDialog>
#include <QSharedPointer>
#include <QWidget>

#include <ExportProgressDialog.hh>

#include <ui_ExportProgressDialog.h>

ExportProgressDialog::ExportProgressDialog(QWidget *parent) :
        QDialog(parent),
        d_ui(QSharedPointer<Ui::ExportProgressDialog>(new Ui::ExportProgressDialog))
{
    d_ui->setupUi(this);
}

ExportProgressDialog::~ExportProgressDialog() {}

void ExportProgressDialog::open()
{
    // This will reset the progress bar to its undetermined state
    setMaximum(0);
    setProgress(-1);
    
    QDialog::open();
}
 

void ExportProgressDialog::setMaximum(int max)
{
    d_ui->progressBar->setMaximum(max);
}

void ExportProgressDialog::setProgress(int progress)
{
    d_ui->progressBar->setValue(progress);
}