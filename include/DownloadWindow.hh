#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QAbstractTableModel>
#include <QNetworkReply>
#include <QModelIndex>
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QWidget>

namespace Ui {
    class DownloadWindow;
}

class ArchiveModel;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class QKeyEvent;
class QProgressDialog;
class QTreeWidgetItem;

class DownloadWindow : public QWidget {
    Q_OBJECT
public:
    DownloadWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~DownloadWindow();

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
    void download();
    void downloadProgress(qint64 progress, qint64 maximum);
    void refreshCorpusList();
    void rowChanged(QModelIndex const &current, QModelIndex const &previous);

protected:
    void keyPressEvent(QKeyEvent *event);
        
private:
    QString networkErrorToString(QNetworkReply::NetworkError error);
    
    QSharedPointer<Ui::DownloadWindow> d_ui;
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

#endif // DOWNLOADWINDOW_H
