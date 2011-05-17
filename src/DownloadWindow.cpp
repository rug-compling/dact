#include <QChar>
#include <QByteArray>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QKeyEvent>
#include <QList>
#include <QMessageBox>
#include <QMetaEnum>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QtCore>

#include <QtDebug>

#include <DownloadWindow.hh>
#include <DactProgressDialog.hh>
#include <QtIOCompressor.hh>
#include <config.hh>

#include <ui_DownloadWindow.h>

DownloadWindow::DownloadWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DownloadWindow>(new Ui::DownloadWindow)),
    d_accessManager(new QNetworkAccessManager),
    d_corpusAccessManager(new QNetworkAccessManager),
    d_downloadProgressDialog(new QProgressDialog(this)),
    d_inflateProgressDialog(new QProgressDialog(this))
{
    d_ui->setupUi(this);
    
    // We only enable the download button when a corpus is selected.
    d_ui->downloadPushButton->setEnabled(false);

    d_downloadProgressDialog->setCancelButton(0);
    d_downloadProgressDialog->setWindowTitle("Downloading corpus");
    d_downloadProgressDialog->setRange(0, 100);
    
    d_inflateProgressDialog->setCancelButton(0);
    d_inflateProgressDialog->setWindowTitle("Decompressing corpus");
    d_inflateProgressDialog->setLabelText("Decompressing downloaded corpus");
    d_inflateProgressDialog->setRange(0, 100);
        
    connect(d_ui->archiveTreeWidget,
        SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
        SLOT(itemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(d_accessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(listReplyFinished(QNetworkReply*)));
    connect(d_corpusAccessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(corpusReplyFinished(QNetworkReply*)));
    connect(d_ui->refreshPushButton, SIGNAL(clicked()),
        SLOT(refreshCorpusList()));
    connect(d_ui->downloadPushButton, SIGNAL(clicked()),
        SLOT(download()));
    connect(this, SIGNAL(inflateProgressed(int)),
        d_inflateProgressDialog.data(), SLOT(setValue(int)));
    connect(this, SIGNAL(inflateError(QString)),
        SLOT(inflateHandleError(QString)));
    connect(this, SIGNAL(inflateFinished()),
        d_inflateProgressDialog.data(), SLOT(close()));
    
    refreshCorpusList();
}

DownloadWindow::~DownloadWindow()
{
}

void DownloadWindow::corpusReplyFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        QString errorValue(networkErrorToString(error));
        
        QMessageBox box(QMessageBox::Warning, "Failed to download corpus",
                        QString("Downloading of corpus failed with error: %1").arg(errorValue),
                        QMessageBox::Ok);
        
        d_downloadProgressDialog->close();
        
        box.exec();

	reply->deleteLater();

        return;
    }
    
    d_downloadProgressDialog->close();
    d_inflateProgressDialog->setValue(0);
    d_inflateProgressDialog->open();
    
    QtConcurrent::run(this, &DownloadWindow::inflate, reply);
}

void DownloadWindow::download()
{
    QTreeWidgetItem *item = d_ui->archiveTreeWidget->currentItem();
    
    // Safety check.
    if (item == 0)
        return;
    
    QString corpusName = item->text(0);
    QString finalCorpusName(corpusName);
    finalCorpusName.remove(QRegExp("\\.gz$"));
    
    QString filename(QFileDialog::getSaveFileName(this,
        "Download corpus", finalCorpusName, "*.dact"));
    
    if (filename.isNull())
        return;
    else {
        d_filename = filename;
        d_hash = item->text(3);        
    }
    
    d_downloadProgressDialog->setLabelText(QString("Downloading '%1'").arg(corpusName));
    d_downloadProgressDialog->reset();
    d_downloadProgressDialog->open();
    
    QString corpusUrl = QString("%1/%2").arg(d_baseUrl).arg(corpusName);
    QNetworkRequest request(corpusUrl);    
    QNetworkReply *reply = d_corpusAccessManager->get(request);
        
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
        SLOT(downloadProgress(qint64, qint64)));
}

void DownloadWindow::downloadProgress(qint64 progress, qint64 maximum)
{
    if (maximum == 0)
        return;
    
    d_downloadProgressDialog->setValue((progress * 100) / maximum);
}

void DownloadWindow::inflate(QIODevice *dev)
{
    qint64 initAvailable = dev->bytesAvailable();
        
    QtIOCompressor data(dev);
    data.setStreamFormat(QtIOCompressor::GzipFormat);
    if (!data.open(QIODevice::ReadOnly))
    {
        dev->deleteLater();
        emit inflateError("could not compressed data stream.");
        return;
    }
    
    QFile out(d_filename);
    if (!out.open(QIODevice::WriteOnly))
    {
        dev->deleteLater();
        emit inflateError("could not open output file for writing.");
        return;
    }
    
    // We'll check whether the uncompressed data matches the given hash.
    QCryptographicHash sha1(QCryptographicHash::Sha1);
    
    while (!data.atEnd()) {
        emit inflateProgressed(static_cast<int>(
            ((initAvailable - dev->bytesAvailable()) * 100) / initAvailable));
        QByteArray newData = data.read(65535);
        sha1.addData(newData);
        out.write(newData);
    }
    
    /*
    Decryption state checking should be covered by the SHA-1 check that follows.
     
    if (!data.errorString().isNull()) {
        delete dev;
        emit inflateError(data.errorString());
        return;
    }
    */
    
    QString hash(sha1.result().toHex());

    dev->deleteLater();
    
    if (hash != d_hash) {
        out.remove();
        emit inflateError("invalid checksum, data was corrupted.");
    }
    
    emit inflateFinished();
    
}

void DownloadWindow::inflateHandleError(QString error)
{
    d_inflateProgressDialog->close();
    
    QMessageBox box(QMessageBox::Warning, "Failed to decompress corpus",
                    QString("Could not decompress corpus: %1").arg(error),
                    QMessageBox::Ok);
    
    box.exec();
}

void DownloadWindow::itemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current == 0)
        d_ui->downloadPushButton->setEnabled(false);
    else
        d_ui->downloadPushButton->setEnabled(true);
}


void DownloadWindow::keyPressEvent(QKeyEvent *event)
{
    // Close window on ESC and CMD + W.
    if (event->key() == Qt::Key_Escape
        || (event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier))
    {
        hide();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

void DownloadWindow::refreshCorpusList()
{
    QSettings settings;
    d_baseUrl = settings.value(ARCHIVE_BASEURL_KEY, DEFAULT_ARCHIVE_BASEURL).toString();

    QString indexUrl = QString("%1/index").arg(d_baseUrl);
    QNetworkRequest request(indexUrl);    
    d_accessManager->get(request);
}

void DownloadWindow::listReplyFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        QString errorValue(networkErrorToString(error));
        
        QMessageBox box(QMessageBox::Warning, "Failed to fetch corpus index",
                        QString("Could not fetch the list of corpora, failed with error: %1").arg(errorValue),
                        QMessageBox::Ok);
        
        box.exec();

	reply->deleteLater();
        
        return;
    }

    
    QTextStream replyStream(reply);

    d_ui->archiveTreeWidget->clear();
    
    QList<QTreeWidgetItem *> items;
    QString line;
    while (true) {
        line = replyStream.readLine();
        if (line.isNull())
            break;
        
        QStringList lineParts = line.trimmed().split(QChar('|'),
            QString::SkipEmptyParts);
        
        size_t fileSize = lineParts[2].toULong();
        double fileSizeMB = fileSize / (1024 * 1024);
        
        QStringList cols;
        cols.push_back(lineParts[0]);
        cols.push_back(QString("%1 MB").arg(QString::number(fileSizeMB, 'f', 1)));
        cols.push_back(lineParts[1]);
        cols.push_back(lineParts[3]);

        items.append(new QTreeWidgetItem(cols));        
    }
    
    d_ui->archiveTreeWidget->addTopLevelItems(items);
    
    d_ui->archiveTreeWidget->resizeColumnToContents(0);
    d_ui->archiveTreeWidget->resizeColumnToContents(1);

    reply->deleteLater();
}

QString DownloadWindow::networkErrorToString(QNetworkReply::NetworkError error)
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
