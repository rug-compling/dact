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
class QTreeWidgetItem;

class RemoteWindow : public QWidget {
    Q_OBJECT
public:
    RemoteWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~RemoteWindow();

signals:
    void openRemote(QString const &url);

private slots:
    void archiveNetworkError(QString error);
    void archiveProcessingError(QString error);
    void archiveRetrieved();
    void remoteCanceled();
    void remote();
    void refreshCorpusList();
    void rowChanged(QModelIndex const &current, QModelIndex const &previous);

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QString networkErrorToString(QNetworkReply::NetworkError error);

    QSharedPointer<Ui::RemoteWindow> d_ui;
    QSharedPointer<ArchiveModel> d_archiveModel;
    QSharedPointer<QNetworkAccessManager> d_corpusAccessManager;
    QString d_baseUrl;
    QString d_url;
    QNetworkReply *d_reply;
};

#endif // REMOTEWINDOW_H
