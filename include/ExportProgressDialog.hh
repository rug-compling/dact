#ifndef EXPORTPROGRESSDIALOG_HH
#define EXPORTPROGRESSDIALOG_HH

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
    class ExportProgressDialog;
}

class ExportProgressDialog : public QDialog {
    Q_OBJECT
public:
    ExportProgressDialog(QWidget *parent);
    ~ExportProgressDialog();
    void open();
public slots:
    void setMaximum(int maximum);
    void setProgress(int progress);
private:
    QSharedPointer<Ui::ExportProgressDialog> d_ui;
};

#endif // EXPORTPROGRESSDIALOG_HH