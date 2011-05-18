#include <QMetaEnum>
#include <QMetaObject>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include <QtDebug>

#include <ArchiveModel.hh>

QString const DOWNLOAD_EXTENSION(".dact.gz");

ArchiveModel::ArchiveModel(QObject *parent) :
    QAbstractTableModel(parent),
    d_accessManager(new QNetworkAccessManager)
{
    connect(d_accessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(replyFinished(QNetworkReply*)));
}

ArchiveModel::ArchiveModel(QUrl const &archiveUrl, QObject *parent) :
    QAbstractTableModel(parent),
    d_accessManager(new QNetworkAccessManager)
{
    connect(d_accessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(replyFinished(QNetworkReply*)));
}

int ArchiveModel::columnCount(QModelIndex const &parent) const
{
    if (parent != QModelIndex())
        return 0;

    return 4;
}

QVariant ArchiveModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid() ||
        index.row() >= d_corpora.size() ||
        index.row() < 0)
      return QVariant();

    ArchiveEntry const &corpus = d_corpora.at(index.row());
   
    if (role == Qt::DisplayRole)
        switch (index.column())
        {
            case 0:
                return corpus.name;
            case 1:
                return QString("%1 MB").arg(corpus.size);
            case 2:
                return corpus.description;
            default:
                return QVariant();
        }
    else if (role == Qt::TextAlignmentRole)
        // Left-align all but the corpus size.
        switch (index.column())
        {
            case 1:
                return Qt::AlignRight;
            default:
                return Qt::AlignLeft;
        }
    else if (role == Qt::UserRole)
        switch (index.column())
        {
            case 3:
                return corpus.checksum;
        }

    return QVariant();
}

QVariant ArchiveModel::headerData(int column, Qt::Orientation orientation,
  int role) const
{
  if (orientation != Qt::Horizontal ||
      role != Qt::DisplayRole)
      return QVariant();

  switch (column)
  {
    case 0:
      return tr("Name");
    case 1:
      return tr("Size");
    case 2:
      return tr("Description");
    default:
      return QVariant();
  }
}

QString ArchiveModel::networkErrorToString(QNetworkReply::NetworkError error)
{
    QString errorValue;
    QMetaObject meta = QNetworkReply::staticMetaObject;
    for (int i = 0; i < meta.enumeratorCount(); ++i) {
        QMetaEnum m = meta.enumerator(i);
        if (m.name() == QLatin1String("NetworkError"))
        {
            errorValue = QLatin1String(m.valueToKey(error));
            break;
        }
    }
    
    return errorValue;
}

void ArchiveModel::replyFinished(QNetworkReply *reply)
{
    d_corpora.clear();
    emit layoutChanged();
    
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        QString errorValue(networkErrorToString(error));
        
        emit networkError(errorValue);
        
        reply->deleteLater();
        
        return;
    }
    
    QTextStream replyStream(reply);
    
    QString line;
    while (true) {
        line = replyStream.readLine();
        if (line.isNull())
            break;
        
        QStringList lineParts = line.trimmed().split(QChar('|'),
             QString::SkipEmptyParts);
        
        // Only accept dact.gz entries.
        QString name(lineParts[0]);
        if (!name.endsWith(DOWNLOAD_EXTENSION))
            continue;
        
        // Chop the extension, we do not want to bother users.
        name.chop(DOWNLOAD_EXTENSION.length());
        
        size_t fileSize = lineParts[2].toULong();
        double fileSizeMB = fileSize / (1024 * 1024);
    
        ArchiveEntry corpus;
        corpus.name = name;
        corpus.size = fileSizeMB;
        corpus.description = lineParts[1];
        corpus.checksum = lineParts[3];

        d_corpora.push_back(corpus);
    }

    emit layoutChanged();
    
    emit retrievalFinished();

    reply->deleteLater();
}

int ArchiveModel::rowCount(QModelIndex const &parent) const
{
    if (parent != QModelIndex())
        return 0;
    
    return d_corpora.size();
}

void ArchiveModel::setUrl(QUrl const &archiveUrl)
{
    d_archiveUrl = archiveUrl;
    refresh();
}

void ArchiveModel::refresh()
{
    QNetworkRequest request(d_archiveUrl);
    d_accessManager->get(request);
}

