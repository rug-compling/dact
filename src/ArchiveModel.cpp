// TODO: remove this line
#include <iostream>

#include <QByteArray>
#include <QIODevice>
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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

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
                return QString("%1").arg(corpus.sentences);
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
            //TODO: restore this?
            //case 3:
            //    return corpus.checksum;
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
            xmlChar *str = xmlNodeListGetString(doc, child->children, 1);
            QString value = QString::fromUtf8(reinterpret_cast<char const *>(str));
            xmlFree(str);

            return value;
        }
    }
    
    return QString();
}
    
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
    
    QByteArray xmlData(reply->readAll());
    xmlDocPtr xmlDoc = xmlReadMemory(xmlData.constData(), xmlData.size(), 0, 0, 0);
    if (xmlDoc == 0) {
        emit processingError("could not parse the corpus archive index.");
        return;
    }
    
    xmlNodePtr root = xmlDocGetRootElement(xmlDoc);
    if (QString::fromUtf8(reinterpret_cast<char const *>(root->name)) !=
        QString("corpusarchive")) {
        xmlFreeDoc(xmlDoc);
        emit processingError("the corpus archive index has an incorrect root node.");
        return;
    }
    
    for (xmlNodePtr child = root->children; child != 0; child = child->next)
    {
        if (QString::fromUtf8(reinterpret_cast<char const *>(child->name)) !=
            QString("corpus"))
            continue;
        
        QString name(childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("filename")));

        if (name.isNull())
            continue;
        
        // Chop the extension, we do not want to bother users.
        if (name.endsWith(DOWNLOAD_EXTENSION))
            name.chop(DOWNLOAD_EXTENSION.length());
        
        // Retrieve and verify file size.
        QString fileSizeStr = childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("filesize"));
        if (fileSizeStr.isNull())
            continue;

        // Attempt to convert the size to a number.
        bool ok = true;
        size_t fileSize = fileSizeStr.toULong(&ok);
        if (!ok)
            continue;
        
        double fileSizeMB = fileSize / (1024 * 1024);
        
        ArchiveEntry corpus;
        corpus.name = name;
        corpus.sentences = childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("sentences")).toULong();
        corpus.size = fileSizeMB;
        corpus.description = childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("shortdesc"));
        corpus.longDescription = childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("desc")).trimmed();
        corpus.checksum = childValue(xmlDoc, child->children, reinterpret_cast<xmlChar const *>("sha1"));
        
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
    emit retrieving();
    
    QNetworkRequest request(d_archiveUrl);
    d_accessManager->get(request);
}

