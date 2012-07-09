#include <QChar>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDesktopServices>
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
#include <QtIOCompressor.hh>

#include "OpenCorpusDialog.hh"
#include "ArchiveModel.hh"

#include <config.hh>

#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Error.hh>

#include "ui_OpenCorpusDialog.h"

namespace ac = alpinocorpus;

QString const DOWNLOAD_EXTENSION(".dact.gz");

OpenCorpusDialog::OpenCorpusDialog(QWidget *parent)
:
    QDialog(parent),
    d_ui(QSharedPointer<Ui::OpenCorpusDialog>(new Ui::OpenCorpusDialog)),
    d_archiveModel(new ArchiveModel),
    d_corpusAccessManager(new QNetworkAccessManager),
    d_downloadProgressDialog(new QProgressDialog()),
    d_inflateProgressDialog(new QProgressDialog()),
    d_reply(0),
    d_cancelInflate(false)
{
    d_ui->setupUi(this);

    d_ui->corpusListView->setModel(d_archiveModel.data());
    // d_ui->corpusListView->hideColumn(2);
    
    // We only enable the download button when a corpus is selected.
    d_ui->openButton->setEnabled(false);
    // d_ui->informationGroupBox->setEnabled(false);

    d_downloadProgressDialog->setWindowTitle("Downloading corpus");
    d_downloadProgressDialog->setRange(0, 100);
    
    d_inflateProgressDialog->setWindowTitle("Decompressing corpus");
    d_inflateProgressDialog->setLabelText("Decompressing downloaded corpus");
    d_inflateProgressDialog->setRange(0, 100);
    
    connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
        SLOT(archiveNetworkError(QString)));
    connect(d_archiveModel.data(), SIGNAL(processingError(QString)),
        SLOT(archiveProcessingError(QString)));
        
    connect(d_ui->corpusListView->selectionModel(),
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
    // connect(d_ui->refreshPushButton, SIGNAL(clicked()),
    //     SLOT(refreshCorpusList()));
    connect(this, SIGNAL(inflateProgressed(int)),
        d_inflateProgressDialog.data(), SLOT(setValue(int)));
    connect(this, SIGNAL(inflateError(QString)),
        SLOT(inflateHandleError(QString)));
    connect(this, SIGNAL(inflateFinished()),
        d_inflateProgressDialog.data(), SLOT(accept()));
    
    // connect(d_archiveModel.data(), SIGNAL(retrieving()),
    //     d_ui->activityIndicator, SLOT(show()));
    // connect(d_archiveModel.data(), SIGNAL(retrievalFinished()),
    //     d_ui->activityIndicator, SLOT(hide()));
    // connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
    //     d_ui->activityIndicator, SLOT(hide()));
    
    refreshCorpusList();
}

OpenCorpusDialog::~OpenCorpusDialog()
{
    // 
}

void OpenCorpusDialog::archiveNetworkError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Failed to fetch corpus index",
        QString("Could not fetch the list of corpora, failed with error: %1").arg(error),
        QMessageBox::Ok);
    
    box.exec();
}

void OpenCorpusDialog::archiveProcessingError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Could not process archive index",
                    QString("Could not process the index of the archive: %1").arg(error),
                    QMessageBox::Ok);
    
    box.exec();
}

void OpenCorpusDialog::archiveRetrieved()
{
    //
}

void OpenCorpusDialog::corpusReplyFinished(QNetworkReply *reply)
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
    
    QtConcurrent::run(this, &OpenCorpusDialog::inflate, reply);
}

void OpenCorpusDialog::download(ArchiveEntry const &entry)
{
    QString name = entry.name;
    QString hash = entry.checksum;
    
    QString corpusName = name + DOWNLOAD_EXTENSION;
    QString filename = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + name + ".dact";
    
    d_filename = filename;
    d_hash = hash;

    d_downloadProgressDialog->setLabelText(QString("Downloading '%1'").arg(corpusName));
    d_downloadProgressDialog->reset();
    d_downloadProgressDialog->open();
    
    QString corpusUrl = QString("%1/%2").arg(d_baseUrl).arg(corpusName);
    
    QNetworkRequest request(corpusUrl);
    d_reply = d_corpusAccessManager->get(request);
    
    connect(d_reply, SIGNAL(downloadProgress(qint64, qint64)),
        SLOT(downloadProgress(qint64, qint64)));
}

void OpenCorpusDialog::downloadCanceled()
{
    Q_ASSERT(d_reply != 0);
    d_reply->abort();
}

void OpenCorpusDialog::downloadProgress(qint64 progress, qint64 maximum)
{
    if (maximum == 0)
        return;
    
    d_downloadProgressDialog->setValue((progress * 100) / maximum);
}

QString OpenCorpusDialog::getCorpusFileName(QWidget *parent)
{
    OpenCorpusDialog dialog(parent);

    return dialog.exec() == QDialog::Accepted
        ? dialog.d_filename
        : QString();
}

QSharedPointer<ac::CorpusReader> OpenCorpusDialog::getCorpusReader(QWidget *parent)
{
    // In the most ideal case, the OpenCorpusDialog would return just a corpus reader
    // which could be reading a local file, or a webservice, or anything else Dact
    // can open. All code to open a file and create a reader would be moved to
    // OpenCorpusDialog. One problem remains: where should the code live that is used
    // to open files passed as arguments on the command line?
    return QSharedPointer<ac::CorpusReader>(0);
}

void OpenCorpusDialog::inflate(QIODevice *dev)
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
    
    // Make sure the directory exists in which we want to store the result
    if (!QDir::current().mkpath(QFileInfo(d_filename).path()))
    {
        dev->deleteLater();
        emit inflateError("could not create output directory");
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

void OpenCorpusDialog::cancelInflate()
{
    d_cancelInflate = true;
}

void OpenCorpusDialog::inflateHandleError(QString error)
{
    d_inflateProgressDialog->accept();
    
    QMessageBox box(QMessageBox::Warning, "Failed to decompress corpus",
                    QString("Could not decompress corpus: %1").arg(error),
                    QMessageBox::Ok);
    
    box.exec();
}

void OpenCorpusDialog::keyPressEvent(QKeyEvent *event)
{
    // Close window on ESC and CMD + W.
    if (event->key() == Qt::Key_Escape
        || (event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier))
    {
        reject();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}

void OpenCorpusDialog::openLocalFile()
{
    d_filename = QFileDialog::getOpenFileName(this,
        "Open corpus", QString(), "Dact corpora (*.dact)");

    if (!d_filename.isNull())
        accept();
}

void OpenCorpusDialog::openSelectedCorpus()
{
    QItemSelectionModel *selectionModel =
      d_ui->corpusListView->selectionModel();

    if (selectionModel->selectedIndexes().size() == 0)
      return;

    int row = selectionModel->selectedIndexes().at(0).row();

    ArchiveEntry const &entry = d_archiveModel->entryAtRow(row);
    
    QFile localCorpus(QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + entry.name + ".dact");

    if (localCorpus.exists())
    {
        d_filename = localCorpus.fileName();
        accept();
    }
    else
    {
        download(entry);
    }
}

void OpenCorpusDialog::refreshCorpusList()
{
    QSettings settings;
    d_baseUrl = settings.value(ARCHIVE_BASEURL_KEY, DEFAULT_ARCHIVE_BASEURL).toString();
    
    d_archiveModel->setUrl(QUrl(QString("%1/index.xml").arg(d_baseUrl)));
}

void OpenCorpusDialog::rowChanged(QModelIndex const &current, QModelIndex const &previous)
{
    Q_UNUSED(previous);
    
    if (current.isValid()) {
        d_ui->openButton->setEnabled(true);
        // d_ui->informationGroupBox->setEnabled(true);
        
        // Retrieve the active entry.
        int row = current.row();
        ArchiveEntry const &entry(d_archiveModel->entryAtRow(row));
        
        // Show detailed information.
        // if (entry.sentences == 0)
        //     d_ui->sentenceCountLabel->setText("unknown");
        // else
        //     d_ui->sentenceCountLabel->setText(QString("%L1").arg(entry.sentences));
        // d_ui->descriptionTextBrowser->setText(entry.longDescription);
    }
    else {
        d_ui->openButton->setEnabled(false);
        // d_ui->informationGroupBox->setEnabled(false);
        
        // d_ui->sentenceCountLabel->clear();
        // d_ui->descriptionTextBrowser->clear();
    }
}

QString OpenCorpusDialog::networkErrorToString(QNetworkReply::NetworkError error)
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