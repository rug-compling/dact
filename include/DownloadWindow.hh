#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QNetworkReply>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

namespace Ui {
    class DownloadWindow;
}

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
    void inflateError(QString error);
    void inflateFinished();
    void inflateProgressed(int value);

private slots:
    void corpusReplyFinished(QNetworkReply *reply);
    void inflate(QIODevice *dev);
    void inflateHandleError(QString error);
    void download();
    void downloadProgress(qint64 progress, qint64 maximum);
    void itemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void listReplyFinished(QNetworkReply *reply);
    void refreshCorpusList();

protected:
    void keyPressEvent(QKeyEvent *event);
        
private:
    QString networkErrorToString(QNetworkReply::NetworkError error);
    
    QSharedPointer<Ui::DownloadWindow> d_ui;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    QSharedPointer<QNetworkAccessManager> d_corpusAccessManager;
    QSharedPointer<QProgressDialog> d_downloadProgressDialog;
    QSharedPointer<QProgressDialog> d_inflateProgressDialog;
    QString d_baseUrl;
    QString d_filename;
    QString d_hash;
};

#endif // DOWNLOADWINDOW_H
