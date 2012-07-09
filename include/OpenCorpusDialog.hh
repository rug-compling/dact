#ifndef OPENCORPUSDIALOG_H
#define OPENCORPUSDIALOG_H

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
    class OpenCorpusDialog;
}

class OpenCorpusDialog : public QDialog {
    Q_OBJECT

public:
    OpenCorpusDialog(QWidget *parent = 0);
    ~OpenCorpusDialog();

signals:
    void openError(QString const &error);

private slots:
    void openSelectedCorpus();
    void openLocalFile();

private:
    QSharedPointer<Ui::OpenCorpusDialog> d_ui;
};

#endif // OPENCORPUSDIALOG_H