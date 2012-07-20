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
    QString url;
    size_t sentences;
    double size;
    QString description;
    QString longDescription;
    QString checksum;

    QString filePath() const;
    bool existsLocally() const;
};

class ArchiveModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ArchiveModel(QObject *parent = 0);
    ArchiveModel(QString const &archiveUrl, QObject *parent = 0);
    QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    int columnCount(QModelIndex const &parent = QModelIndex()) const;
    ArchiveEntry const &entryAtRow(int row) const;
    QVariant headerData(int column, Qt::Orientation orientation,
        int role) const;
    void refresh();
    int rowCount(QModelIndex const &parent = QModelIndex()) const;
    void setUrl(QString const &archiveUrl);
    private slots:
    void replyFinished(QNetworkReply *reply);

signals:
    void networkError(QString error);
    void processingError(QString error);
    void retrieving();
    void retrievalFinished();
    
private:
    void init();

    QString networkErrorToString(QNetworkReply::NetworkError error);
    void addLocalFiles();

    void writeLocalArchiveIndex(QByteArray const &data);
    QByteArray readLocalArchiveIndex() const;

    bool parseArchiveIndex(QByteArray const &xmlData);

    QString d_archiveUrl;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    QVector<ArchiveEntry> d_corpora;

};

inline ArchiveEntry const &ArchiveModel::entryAtRow(int row) const
{
    return d_corpora.at(row);
}

#endif // DOWNLOADWINDOW_H
