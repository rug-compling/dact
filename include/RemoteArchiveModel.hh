#ifndef REMOTEARCHIVEMODEL_H
#define REMOTEARCHIVEMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>

struct RemoteArchiveEntry {
    QString name;
    size_t sentences;
    double size;
    QString description;
    QString longDescription;
    QString checksum;
};

class RemoteArchiveModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RemoteArchiveModel(QObject *parent = 0);
    RemoteArchiveModel(QUrl const &archiveUrl, QObject *parent = 0);
    QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    int columnCount(QModelIndex const &parent = QModelIndex()) const;
    RemoteArchiveEntry const &entryAtRow(int row);
    QVariant headerData(int column, Qt::Orientation orientation,
        int role) const;
    void refresh();
    int rowCount(QModelIndex const &parent = QModelIndex()) const;
    void setUrl(QUrl const &archiveUrl);
    private slots:
    void replyFinished(QNetworkReply *reply);

signals:
    void networkError(QString error);
    void processingError(QString error);
    void retrieving();
    void retrievalFinished();
    
private:    
    QString networkErrorToString(QNetworkReply::NetworkError error);
    
    QUrl d_archiveUrl;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    QVector<RemoteArchiveEntry> d_corpora;

};

inline RemoteArchiveEntry const &RemoteArchiveModel::entryAtRow(int row)
{
    return d_corpora.at(row);
}


#endif // REMOTEARCHIVEMODEL_H
