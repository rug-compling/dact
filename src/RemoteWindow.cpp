#include <QChar>
#include <QByteArray>
#include <QKeyEvent>
#include <QList>
#include <QMessageBox>
#include <QMetaEnum>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QtCore>

#include <QtDebug>

#include <RemoteArchiveModel.hh>
#include <RemoteWindow.hh>
#include <config.hh>

#include <ui_RemoteWindow.h>

QString const REMOTE_EXTENSION(".dact.gz");

RemoteWindow::RemoteWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::RemoteWindow>(new Ui::RemoteWindow)),
    d_archiveModel(new RemoteArchiveModel()),
    d_corpusAccessManager(new QNetworkAccessManager),
    d_reply(0)
{
    d_ui->setupUi(this);

    d_ui->archiveTreeView->setModel(d_archiveModel.data());
    d_ui->archiveTreeView->hideColumn(1);

    // We only enable the remote button when a corpus is selected.
    d_ui->openPushButton->setEnabled(false);
    d_ui->informationGroupBox->setEnabled(false);

    connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
        SLOT(archiveNetworkError(QString)));
    connect(d_archiveModel.data(), SIGNAL(processingError(QString)),
        SLOT(archiveProcessingError(QString)));

    connect(d_ui->archiveTreeView->selectionModel(),
        SIGNAL(currentRowChanged(QModelIndex const &, QModelIndex const &)),
        SLOT(rowChanged(QModelIndex const &, QModelIndex const &)));
    connect(d_archiveModel.data(), SIGNAL(retrievalFinished()),
            SLOT(archiveRetrieved()));
    connect(d_ui->refreshPushButton, SIGNAL(clicked()),
        SLOT(refreshCorpusList()));
    connect(d_ui->openPushButton, SIGNAL(clicked()),
        SLOT(remote()));

    connect(d_archiveModel.data(), SIGNAL(retrieving()),
        d_ui->activityIndicator, SLOT(show()));
    connect(d_archiveModel.data(), SIGNAL(retrievalFinished()),
        d_ui->activityIndicator, SLOT(hide()));
    connect(d_archiveModel.data(), SIGNAL(networkError(QString)),
        d_ui->activityIndicator, SLOT(hide()));

    refreshCorpusList();
}

RemoteWindow::~RemoteWindow()
{
}

void RemoteWindow::archiveNetworkError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Failed to fetch corpus index",
        QString("Could not fetch the list of corpora, failed with error: %1").arg(error),
        QMessageBox::Ok);

    box.exec();
}

void RemoteWindow::archiveProcessingError(QString error)
{
    QMessageBox box(QMessageBox::Warning, "Could not process archive index",
                    QString("Could not process the index of the archive: %1").arg(error),
                    QMessageBox::Ok);

    box.exec();
}

void RemoteWindow::archiveRetrieved()
{
    d_ui->archiveTreeView->resizeColumnToContents(0);
    d_ui->archiveTreeView->resizeColumnToContents(2);
    d_ui->archiveTreeView->resizeColumnToContents(3);
}

void RemoteWindow::remote()
{
    QItemSelectionModel *selectionModel =
      d_ui->archiveTreeView->selectionModel();

    if (selectionModel->selectedRows().size() == 0)
      return;

    int row = selectionModel->selectedRows().at(0).row();

    RemoteArchiveEntry const &entry = d_archiveModel->entryAtRow(row);

    QSettings settings;
    d_url = settings.value(REMOTE_BASEURL_KEY, DEFAULT_REMOTE_BASEURL).toString() + "/" + entry.name;

    hide();

    emit openRemote(d_url);
}

void RemoteWindow::remoteCanceled()
{
    Q_ASSERT(d_reply != 0);
    d_reply->abort();
}

void RemoteWindow::keyPressEvent(QKeyEvent *event)
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

void RemoteWindow::refreshCorpusList()
{
    QSettings settings;
    d_baseUrl = settings.value(REMOTE_BASEURL_KEY, DEFAULT_REMOTE_BASEURL).toString();

    d_archiveModel->setUrl(QString("%1/corpora.xml").arg(d_baseUrl));
}

void RemoteWindow::rowChanged(QModelIndex const &current, QModelIndex const &previous)
{
    Q_UNUSED(previous);

    if (current.isValid()) {
        d_ui->openPushButton->setEnabled(true);
        d_ui->informationGroupBox->setEnabled(true);

        // Retrieve the active entry.
        int row = current.row();
        RemoteArchiveEntry const &entry(d_archiveModel->entryAtRow(row));

        d_ui->descriptionTextBrowser->setText(entry.longDescription);
    }
    else {
        d_ui->openPushButton->setEnabled(false);
        d_ui->informationGroupBox->setEnabled(false);

        d_ui->descriptionTextBrowser->clear();
    }
}

QString RemoteWindow::networkErrorToString(QNetworkReply::NetworkError error)
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
