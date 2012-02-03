#ifndef REMOTEWINDOW_H
#define REMOTEWINDOW_H

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
    class RemoteWindow;
}

class ArchiveModel;
class QIODevice;
class QNetworkAccessManager;
class QNetworkReply;
class QKeyEvent;
class QProgressDialog;
class QTreeWidgetItem;

class RemoteWindow : public QWidget {
    Q_OBJECT
public:
    RemoteWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~RemoteWindow();

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
    void remoteCanceled();
    void inflate(QIODevice *dev);
    void inflateHandleError(QString error);
    void remote();
    void remoteProgress(qint64 progress, qint64 maximum);
    void refreshCorpusList();
    void rowChanged(QModelIndex const &current, QModelIndex const &previous);

protected:
    void keyPressEvent(QKeyEvent *event);
        
private:
    QString networkErrorToString(QNetworkReply::NetworkError error);
    
    QSharedPointer<Ui::RemoteWindow> d_ui;
    QSharedPointer<ArchiveModel> d_archiveModel;
    QSharedPointer<QNetworkAccessManager> d_corpusAccessManager;
    QSharedPointer<QProgressDialog> d_remoteProgressDialog;
    QSharedPointer<QProgressDialog> d_inflateProgressDialog;
    QString d_baseUrl;
    QString d_filename;
    QString d_hash;
    QNetworkReply *d_reply;
    volatile bool d_cancelInflate;
};

#endif // REMOTEWINDOW_H
