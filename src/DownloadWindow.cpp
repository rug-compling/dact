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
#include <QTextStream>
#include <QUrl>
#include <QtCore>

#include <QtDebug>

#include <ArchiveModel.hh>
#include <DownloadWindow.hh>
#include <QtIOCompressor.hh>
#include <config.hh>

#include <ui_DownloadWindow.h>

QString const DOWNLOAD_EXTENSION(".dact.gz");

DownloadWindow::DownloadWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DownloadWindow>(new Ui::DownloadWindow)),
    d_archiveModel(new ArchiveModel),
    d_corpusAccessManager(new QNetworkAccessManager),
    d_downloadProgressDialog(new QProgressDialog(this)),
    d_inflateProgressDialog(new QProgressDialog(this)),
    d_reply(0),
    d_cancelInflate(false)
{
    d_ui->setupUi(this);

    d_ui->archiveTreeView->setModel(d_archiveModel.data());
    d_ui->archiveTreeView->hideColumn(2);
    
    // We only enable the download button when a corpus is selected.
    d_ui->downloadPushButton->setEnabled(false);
    d_ui->informationGroupBox->setEnabled(false);

    d_downloadProgressDialog->setWindowTitle("Downloading corpus");
    d_downloadProgressDialog->setRange(0, 100);
    
    d_inflateProgressDialog->setWindowTitle("Decompressing corpus");
    d_inflateProgressDialog->setLabelText("Decompressing downloaded corpus");
    d_inflateProgressDialog->setRange(0, 100);
    
    connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
        SLOT(archiveNetworkError(QString)));
    connect(d_archiveModel.data(), SIGNAL(processingError(QString)),
        SLOT(archiveProcessingError(QString)));
        
    connect(d_ui->archiveTreeView->selectionModel(),
        SIGNAL(currentRowChanged(QModelIndex const &, QModelIndex const &)),
        SLOT(rowChanged(QModelIndex const &, QModelIndex const &)));
    connect(d_archiveModel.data(), SIGNAL(retrievalFinished()),
            SLOT(archiveRetrieved()));
    connect(d_corpusAccessManager.data(), SIGNAL(finished(QNetworkReply *)),
        SLOT(corpusReplyFinished(QNetworkReply*)));
    connect(d_downloadProgressDialog.data(), SIGNAL(canceled()),
        SLOT(downloadCanceled()));
    connect(d_inflateProgressDialog.data(), SIGNAL(canceled()),
            SLOT(cancelInflate()));
    connect(d_ui->refreshPushButton, SIGNAL(clicked()),
        SLOT(refreshCorpusList()));
    connect(d_ui->downloadPushButton, SIGNAL(clicked()),
        SLOT(download()));
    connect(this, SIGNAL(inflateProgressed(int)),
        d_inflateProgressDialog.data(), SLOT(setValue(int)));
    connect(this, SIGNAL(inflateError(QString)),
        SLOT(inflateHandleError(QString)));
    connect(this, SIGNAL(inflateFinished()),
        d_inflateProgressDialog.data(), SLOT(accept()));
    
    connect(d_archiveModel.data(), SIGNAL(retrieving()),
        d_ui->activityIndicator, SLOT(show()));
    connect(d_archiveModel.data(), SIGNAL(retrievalFinished()),
        d_ui->activityIndicator, SLOT(hide()));
    connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
        d_ui->activityIndicator, SLOT(hide()));
    
    refreshCorpusList();
}

DownloadWindow::~DownloadWindow()
{
}

void DownloadWindow::archiveNetworkError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Failed to fetch corpus index",
        QString("Could not fetch the list of corpora, failed with error: %1").arg(error),
        QMessageBox::Ok);
    
    box.exec();
}

void DownloadWindow::archiveProcessingError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Could not process archive index",
                    QString("Could not process the index of the archive: %1").arg(error),
                    QMessageBox::Ok);
    
    box.exec();
}

void DownloadWindow::archiveRetrieved()
{
    d_ui->archiveTreeView->resizeColumnToContents(0);
    d_ui->archiveTreeView->resizeColumnToContents(1);
    d_ui->archiveTreeView->resizeColumnToContents(3);
}

void DownloadWindow::corpusReplyFinished(QNetworkReply *reply)
{
    d_reply = 0;
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        reply->deleteLater();

        if (error == QNetworkReply::OperationCanceledError) 
            return;
        
        QString errorValue(networkErrorToString(error));
        
        QMessageBox box(QMessageBox::Warning, "Failed to download corpus",
                        QString("Downloading of corpus failed with error: %1").arg(errorValue),
                        QMessageBox::Ok);
        
        d_downloadProgressDialog->accept();
        
        box.exec();

        return;
    }
    
    d_downloadProgressDialog->accept();
    d_inflateProgressDialog->setValue(0);
    d_inflateProgressDialog->open();
    
    QtConcurrent::run(this, &DownloadWindow::inflate, reply);
}

void DownloadWindow::download()
{
    QItemSelectionModel *selectionModel =
      d_ui->archiveTreeView->selectionModel();

    if (selectionModel->selectedRows().size() == 0)
      return;

    int row = selectionModel->selectedRows().at(0).row();

    ArchiveEntry const &entry = d_archiveModel->entryAtRow(row);
    
    QString name = entry.name;
    QString hash = entry.checksum;
    
    QString corpusName = name + DOWNLOAD_EXTENSION;
    QString finalCorpusName = name + ".dact";
    
    QString filename(QFileDialog::getSaveFileName(this,
        "Download corpus", finalCorpusName, "*.dact"));
    
    if (filename.isNull())
        return;
    else {
        d_filename = filename;
        d_hash = hash;
    }
    
    d_downloadProgressDialog->setLabelText(QString("Downloading '%1'").arg(corpusName));
    d_downloadProgressDialog->reset();
    d_downloadProgressDialog->open();
    
    QString corpusUrl = QString("%1/%2").arg(d_baseUrl).arg(corpusName);
    QNetworkRequest request(corpusUrl);    
    d_reply = d_corpusAccessManager->get(request);
        
    connect(d_reply, SIGNAL(downloadProgress(qint64, qint64)),
        SLOT(downloadProgress(qint64, qint64)));
}

void DownloadWindow::downloadCanceled()
{
    Q_ASSERT(d_reply != 0);
    d_reply->abort();
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
    
    while (!data.atEnd() && !d_cancelInflate) {
        emit inflateProgressed(static_cast<int>(
            ((initAvailable - dev->bytesAvailable()) * 100) / initAvailable));
        QByteArray newData = data.read(65535);
        sha1.addData(newData);
        out.write(newData);
    }
    
    dev->deleteLater();

    if (d_cancelInflate) {
        d_cancelInflate = false;
        out.remove();
        emit inflateCanceled();
        return;
    }
    
    QString hash(sha1.result().toHex());
    
    if (hash != d_hash) {
        out.remove();
        emit inflateError("invalid checksum, data was corrupted.");
    }
    
    emit inflateFinished();
    
}

void DownloadWindow::cancelInflate()
{
    d_cancelInflate = true;
}

void DownloadWindow::inflateHandleError(QString error)
{
    d_inflateProgressDialog->accept();
    
    QMessageBox box(QMessageBox::Warning, "Failed to decompress corpus",
                    QString("Could not decompress corpus: %1").arg(error),
                    QMessageBox::Ok);
    
    box.exec();
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
    
    d_archiveModel->setUrl(QUrl(QString("%1/index.xml").arg(d_baseUrl)));
}

void DownloadWindow::rowChanged(QModelIndex const &current, QModelIndex const &previous)
{
    Q_UNUSED(previous);
    
    if (current.isValid()) {
        d_ui->downloadPushButton->setEnabled(true);
        d_ui->informationGroupBox->setEnabled(true);
        
        // Retrieve the active entry.
        int row = current.row();
        ArchiveEntry const &entry(d_archiveModel->entryAtRow(row));
        
        // Show detailed information.
        if (entry.sentences == 0)
            d_ui->sentenceCountLabel->setText("unknown");
        else
            d_ui->sentenceCountLabel->setText(QString("%L1").arg(entry.sentences));
        d_ui->descriptionTextBrowser->setText(entry.longDescription);
    }
    else {
        d_ui->downloadPushButton->setEnabled(false);
        d_ui->informationGroupBox->setEnabled(false);
        
        d_ui->sentenceCountLabel->clear();
        d_ui->descriptionTextBrowser->clear();
    }
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
