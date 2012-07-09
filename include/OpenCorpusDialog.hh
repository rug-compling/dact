#ifndef OPENCORPUSDIALOG_H
#define OPENCORPUSDIALOG_H

#include <QDialog>
#include <QSharedPointer>

#include <AlpinoCorpus/CorpusReader.hh>

namespace Ui {
    class OpenCorpusDialog;
}

class OpenCorpusDialog : public QDialog {
    Q_OBJECT

public:
    OpenCorpusDialog(QWidget *parent = 0);
    ~OpenCorpusDialog();

    static QString getCorpusFileName(QWidget *parent);
    static QSharedPointer<alpinocorpus::CorpusReader> getCorpusReader(QWidget *parent);

signals:
    void openError(QString const &error);

private slots:
    void openSelectedCorpus();
    void openLocalFile();

private:
    QSharedPointer<Ui::OpenCorpusDialog> d_ui;
    QString d_selectedFileName;
};

#endif // OPENCORPUSDIALOG_H