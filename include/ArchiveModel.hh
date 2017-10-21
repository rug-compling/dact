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

enum class EntryType {
    Local,
    Downloaded,
    Downloadable
};

struct ArchiveEntry {
    QString name;
    QString path;
    QString url;
    size_t sentences;
    double size;
    QString description;
    QString longDescription;
    QString checksum;
    EntryType type;

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

    void deleteLocalFiles(QModelIndex const &index);

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
    void addRecentFiles();

    void writeLocalArchiveIndex(QByteArray const &data);
    QByteArray readLocalArchiveIndex() const;

    bool parseArchiveIndex(QByteArray const &xmlData, bool listLocalFilesOnly = false);

    QString d_archiveUrl;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    QVector<ArchiveEntry> d_corpora;

};

inline ArchiveEntry const &ArchiveModel::entryAtRow(int row) const
{
    Q_ASSERT(row >= 0 && row < d_corpora.size());
    
    return d_corpora.at(row);
}

#endif // DOWNLOADWINDOW_H
