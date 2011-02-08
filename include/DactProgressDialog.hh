#ifndef DACTPROGRESSDIALOG_HH
#define DACTPROGRESSDIALOG_HH

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
    class DactProgressDialog;
}

class DactProgressDialog : public QDialog {
    Q_OBJECT
public:
    DactProgressDialog(QWidget *parent);
    ~DactProgressDialog();
    void open();
public slots:
    void setCancelable(bool cancelable);
    void setMaximum(int maximum);
    void setProgress(int progress);
    void setDescription(QString const &description);
private:
    QSharedPointer<Ui::DactProgressDialog> d_ui;
};

#endif // DACTPROGRESSDIALOG_HH
