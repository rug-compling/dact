#ifndef OPENPROGRESSDIALOG_HH
#define OPENPROGRESSDIALOG_HH

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
    class OpenProgressDialog;
}

class OpenProgressDialog : public QDialog {
    Q_OBJECT
public:
    OpenProgressDialog(QWidget *parent);
    ~OpenProgressDialog();
    void setProgress(size_t progress);
private:
    QSharedPointer<Ui::OpenProgressDialog> d_ui;
};

#endif // OPENPROGRESSDIALOG_HH
