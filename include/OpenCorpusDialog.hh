#ifndef OPENCORPUSDIALOG_H
#define OPENCORPUSDIALOG_H

#include <QAbstractTableModel>
#include <QDialog>
#include <QNetworkReply>
#include <QModelIndex>
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include <AlpinoCorpus/CorpusReader.hh>

namespace Ui {
    class OpenCorpusDialog;
}

class ArchiveModel;
class ArchiveEntry;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class QKeyEvent;
class QProgressDialog;
class QTreeWidgetItem;

class OpenCorpusDialog : public QDialog {
    Q_OBJECT
public:
    OpenCorpusDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~OpenCorpusDialog();

    static QString getCorpusFileName(QWidget *parent);
    static QSharedPointer<alpinocorpus::CorpusReader> getCorpusReader(QWidget *parent);

signals:
    void inflateCanceled();
    void inflateError(QString error);
    void inflateFinished();
    void inflateProgressed(int value);

private slots:
    void archiveNetworkError(QString error);
    void archiveProcessingError(QString error);
    void archiveRetrieved();
    void cancelInflate();
    void corpusReplyFinished(QNetworkReply *reply);
    void downloadCanceled();
    void inflate(QIODevice *dev);
    void inflateHandleError(QString error);
    void download(ArchiveEntry const &entry);
    void downloadProgress(qint64 progress, qint64 maximum);
    void refreshCorpusList();
    void rowChanged(QModelIndex const &current, QModelIndex const &previous);

    void openLocalFile();
    void openSelectedCorpus();
    void openSelectedCorpus(QModelIndex const &);

    void deleteSelectedCorpus();

protected:
    void keyPressEvent(QKeyEvent *event);
        
private:
    QString networkErrorToString(QNetworkReply::NetworkError error);
    
    QSharedPointer<Ui::OpenCorpusDialog> d_ui;
    QSharedPointer<ArchiveModel> d_archiveModel;
    QSharedPointer<QNetworkAccessManager> d_corpusAccessManager;
    QSharedPointer<QProgressDialog> d_downloadProgressDialog;
    QSharedPointer<QProgressDialog> d_inflateProgressDialog;
    QString d_baseUrl;
    QString d_filename;
    QString d_hash;
    QNetworkReply *d_reply;
    volatile bool d_cancelInflate;
};

#endif // OPENCORPUSDIALOG_H
