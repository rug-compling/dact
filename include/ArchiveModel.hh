#ifndef ARCHIVEMODEL_H
#define ARCHIVEMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>

struct ArchiveEntry {
    QString name;
    size_t sentences;
    double size;
    QString description;
    QString longDescription;
    QString checksum;

    QString filePath() const;
};

class ArchiveModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ArchiveModel(QObject *parent = 0);
    ArchiveModel(QUrl const &archiveUrl, QObject *parent = 0);
    QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    int columnCount(QModelIndex const &parent = QModelIndex()) const;
    ArchiveEntry const &entryAtRow(int row);
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
    QVector<ArchiveEntry> d_corpora;

};

inline ArchiveEntry const &ArchiveModel::entryAtRow(int row)
{
    return d_corpora.at(row);
}


#endif // DOWNLOADWINDOW_H
