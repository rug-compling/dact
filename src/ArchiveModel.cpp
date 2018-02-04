#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMetaEnum>
#include <QMetaObject>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QScopedPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include <stdexcept>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

#include <ArchiveModel.hh>
#include <HumanReadableSize.hh>
#include <XMLDeleters.hh>

QString const DOWNLOAD_EXTENSION(".dact.gz");

QString ArchiveEntry::filePath() const
{
    if (!path.isEmpty())
        return path;
    
    return QString("%1/%2.dact").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation), name);
}

bool ArchiveEntry::existsLocally() const
{
    return QFile(filePath()).exists();
}

ArchiveModel::ArchiveModel(QObject *parent) :
    QAbstractTableModel(parent),
    d_accessManager(new QNetworkAccessManager)
{
    init();
}

ArchiveModel::ArchiveModel(QString const &archiveUrl, QObject *parent) :
    QAbstractTableModel(parent),
    d_archiveUrl(archiveUrl),
    d_accessManager(new QNetworkAccessManager)
{
    init();
}

void ArchiveModel::init()
{
    connect(d_accessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(replyFinished(QNetworkReply*)));

    // Add recently-opened corpora.
    addRecentFiles();

    // Add downloaded files.
    addLocalFiles();

    // Then, if there is a local copy of the archive index, read it
    QByteArray localArchiveIndex(readLocalArchiveIndex());
    if (localArchiveIndex.size() > 0)
        parseArchiveIndex(localArchiveIndex, true);
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
                return humanReadableSize(corpus.size);
            case 2:
                return QString("%L1").arg(corpus.sentences);
            case 3:
                return corpus.description;
            default:
                return QVariant();
        }
    else if (role == Qt::TextAlignmentRole)
        // Left-align all but the corpus size and sentence count.
        switch (index.column())
        {
            case 1:
            case 2:
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
      return tr("Sentences");
    case 3:
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

namespace {
    
QString childValue(xmlDocPtr doc, xmlNodePtr children, xmlChar const *name) {
    for (xmlNodePtr child = children; child != 0; child = child->next) {
        if (xmlStrcmp(child->name, name) == 0) {
            // Retrieve value as a QString.
            QScopedPointer<xmlChar, XmlDeleter> str(
                xmlNodeListGetString(doc, child->children, 1));
            QString value = QString::fromUtf8(reinterpret_cast<char const *>(str.data()));

            return value;
        }
    }
    
    return QString();
}
    
}


void ArchiveModel::replyFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        QString errorValue(networkErrorToString(error));
        
        emit networkError(errorValue);
        
        reply->deleteLater();
        
        return;
    }
    
    QByteArray xmlData(reply->readAll());
    
    // Save a local copy of XML Data, for when the application is offline
    if (parseArchiveIndex(xmlData))
        writeLocalArchiveIndex(xmlData);

    emit retrievalFinished();

    reply->deleteLater();
}

bool ArchiveModel::parseArchiveIndex(QByteArray const &xmlData, bool listLocalFilesOnly)
{
    // Clear the list, but add the local files again.
    d_corpora.clear();
    addRecentFiles();
    addLocalFiles();
    
    QScopedPointer<xmlDoc, XmlDocDeleter> xmlDoc(
        xmlReadMemory(xmlData.constData(), xmlData.size(), 0, 0, 0));
    if (xmlDoc == 0) {
        emit processingError("could not parse the corpus archive index.");
        return false;
    }
    
    xmlNodePtr root = xmlDocGetRootElement(xmlDoc.data());
    if (!root) {
        emit processingError("could not get the root node of the corpus archive index.");
        return false;
    }


    if (QString::fromUtf8(reinterpret_cast<char const *>(root->name)) !=
        QString("corpusarchive")) {
        emit processingError("the corpus archive index has an incorrect root node.");
        return false;
    }
    
    for (xmlNodePtr child = root->children; child != 0; child = child->next)
    {
        if (QString::fromUtf8(reinterpret_cast<char const *>(child->name)) !=
            QString("corpus"))
            continue;
        
        QString name(childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("filename")));

        if (name.isNull())
            continue;
        
        // Chop the extension, we do not want to bother users.
        if (name.endsWith(DOWNLOAD_EXTENSION))
            name.chop(DOWNLOAD_EXTENSION.length());
        
        // Retrieve and verify file size.
        QString fileSizeStr = childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("filesize"));
        if (fileSizeStr.isNull())
            continue;

        // Attempt to convert the size to a number.
        bool ok = true;
        size_t fileSize = fileSizeStr.toULong(&ok);
        if (!ok)
            continue;
        
        ArchiveEntry *corpus(0);

        // Try to find if the file is already in the index (e.g. a local file)
        for (QVector<ArchiveEntry>::iterator it = d_corpora.begin(); it != d_corpora.end(); it++)
        {
            if (it->name == name)
            {
                corpus = it;
                corpus->type = DownloadedEntryType;
                break;
            }
        }

        // If there is no such corpus on the list already, add a new one.
        if (corpus == 0)
        {
            // .. unless we only want to list local corpora. If so, skip it.
            if (listLocalFilesOnly)
                continue;

            d_corpora.push_back(ArchiveEntry());
            corpus = &d_corpora.last();
            corpus->type = DownloadableEntryType;
        }

        corpus->name = name;
        corpus->url = QString("%1/%2").arg(d_archiveUrl).arg(name + DOWNLOAD_EXTENSION);
        corpus->sentences = childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("sentences")).toULong();
        corpus->size = fileSize;
        corpus->description = childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("shortdesc"));
        corpus->longDescription = childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("desc")).trimmed();
        corpus->checksum = childValue(xmlDoc.data(), child->children, reinterpret_cast<xmlChar const *>("sha1"));
    }

    emit layoutChanged();

    return true;
}

int ArchiveModel::rowCount(QModelIndex const &parent) const
{
    if (parent != QModelIndex())
        return 0;
    
    return d_corpora.size();
}

void ArchiveModel::addLocalFiles()
{
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

    if (locations.isEmpty()) {
      throw std::runtime_error("No standard data location.");
    }
    
    QDir localFiles(locations.at(0));

    QStringList extensions;
    extensions << "*.dact";

    foreach (QFileInfo const &entry, localFiles.entryInfoList(extensions))
    {
        ArchiveEntry corpus;
        corpus.name = entry.baseName();
        corpus.path = entry.absoluteFilePath();
        corpus.type = LocalEntryType;

        // Try to find if the file is already in the index (through recents)
        bool found = false;
        for (QVector<ArchiveEntry>::iterator it = d_corpora.begin(); it != d_corpora.end(); it++)
        {
            if (it->name == corpus.name)
            {
                found = true;
                break;
            }
        }

        if (!found)
        { 
            d_corpora.push_back(corpus);
        } 
    }

    emit layoutChanged();
}

void ArchiveModel::addRecentFiles()
{
    QSettings settings;

    int size = settings.beginReadArray("recent_files");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QFileInfo file(settings.value("path").toString());
        if (!file.exists())
        {
	  continue;
        }

        ArchiveEntry corpus;
        corpus.name = file.baseName();
        corpus.path = file.absoluteFilePath();
        corpus.size = file.size();
        corpus.type = LocalEntryType;
        d_corpora.push_back(corpus);
    }
    
    emit layoutChanged();
}

void ArchiveModel::setUrl(QString const &archiveUrl)
{
    d_archiveUrl = archiveUrl;
    refresh();
}

void ArchiveModel::refresh()
{
    emit retrieving();
    
    QNetworkRequest request(d_archiveUrl + "/index.xml");
    d_accessManager->get(request);
}

void ArchiveModel::writeLocalArchiveIndex(QByteArray const &data)
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/index.xml");

    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(data);
}

QByteArray ArchiveModel::readLocalArchiveIndex() const
{
    QString indexName(QStandardPaths::locate(QStandardPaths::DataLocation, "index.xml"));

    if (indexName.isEmpty())
        return QByteArray();

    QFile file(indexName);

    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return QByteArray();

    return file.readAll();
}

void ArchiveModel::deleteLocalFiles(QModelIndex const &index)
{
    ArchiveEntry const &entry(entryAtRow(index.row()));

    // If it indeed exists as a local file, delete it.
    if (entry.existsLocally())
    {
        QFile(entry.filePath()).remove();

        // If it only exists as a local file (i.e. not in the corpus index) also delete it from the index.
        if (entry.url.isEmpty())
            d_corpora.remove(index.row());

        emit layoutChanged();
    }
}
