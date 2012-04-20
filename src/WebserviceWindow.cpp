#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QStringList>
#include <QTextStream>

#include <AlpinoCorpus/CorpusWriter.hh>

#include <WebserviceWindow.hh>
#include <ui_WebserviceWindow.h>

namespace ac = alpinocorpus;

WebserviceWindow::WebserviceWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::WebserviceWindow>(new Ui::WebserviceWindow)),
    d_accessManager(new QNetworkAccessManager),
    d_progressDialog(new QProgressDialog(parent))
{
    d_ui->setupUi(this);

    connect(d_progressDialog,
        SIGNAL(canceled()), SLOT(cancelResponse()));
}

WebserviceWindow::~WebserviceWindow()
{
    delete d_progressDialog;
}

void WebserviceWindow::openSentencesFile()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Load sentences"),
        QString(), tr("Text files (*.txt)"));

    clearSentencesField();

    foreach (QString const &filename, files)
        loadSentencesFile(filename);
}

void WebserviceWindow::clearSentencesField()
{
    d_ui->sentencesField->clear();
}

void WebserviceWindow::loadSentencesFile(QString const &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QTextStream instream(&file);
    d_ui->sentencesField->appendPlainText(instream.readAll());
}

void WebserviceWindow::parseSentences()
{
    // Ask the user where he wants to save the parsed sentences
    d_corpusFilename = QFileDialog::getSaveFileName(this, tr("Save parsed sentences"),
        QString(), tr("Dact corpus (*.dact)"));

    // User cancelled choosing a save destination
    if (d_corpusFilename.isNull())
        return;

    // Open corpus
    d_corpus = QSharedPointer<ac::CorpusWriter>(
        ac::CorpusWriter::open(
            d_corpusFilename.toUtf8().constData(), true,
            ac::CorpusWriter::DBXML_CORPUS_WRITER));

    // Get sentences and count them for progress updates
    QString sentences(d_ui->sentencesField->toPlainText());
    d_numberOfSentences = countSentences(sentences);
    d_numberOfSentencesReceived = 0;

    // Hide the sentences dialog
    hide();

    // Show the progress dialog
    d_progressDialog->setWindowTitle(tr("Parsing sentences"));
    d_progressDialog->setLabelText("Sending sentences to webservice");
    d_progressDialog->open();

    // Send the request
    QNetworkRequest request(QString("http://145.100.57.148/bin/alpino"));
    d_reply = d_accessManager->post(request, sentences.toUtf8());

    // Connect all the event handlers to the response
    connect(d_reply, SIGNAL(readyRead()),
        SLOT(readResponse()));

    connect(d_reply, SIGNAL(finished()),
        SLOT(finishResponse()));

    connect(d_reply, SIGNAL(error(QNetworkReply::NetworkError)),
        SLOT(errorResponse(QNetworkReply::NetworkError)));
}

int WebserviceWindow::countSentences(QString const &sentences)
{
    // TODO maybe this is a bit too optimistic, and should we prune empty lines.
    return sentences.count('\n');
}

void WebserviceWindow::readResponse()
{
    // Peek to see if there is a complete sentence to be read

    // If so, read it and insert it into the corpuswriter

    // Update progress dialog.
    d_progressDialog->setLabelText(tr("Parsing sentence %1 of %2")
        .arg(d_numberOfSentencesReceived)
        .arg(d_numberOfSentences));
}

void WebserviceWindow::finishResponse()
{
    qDebug() << "finishResponse";

    // Reset the reply pointer, since the request is no longer active.
    d_reply->deleteLater();
    d_reply = 0;

    // Hide the dialog
    d_progressDialog->accept();

    // Clear the sentences in the window
    clearSentencesField();

    // Open the corpus
    emit parseSentencesFinished(d_corpusFilename);
}

void WebserviceWindow::errorResponse(QNetworkReply::NetworkError error)
{
    // TODO show human readable error.
    QMessageBox box(QMessageBox::Warning,
        tr("Failed to receive sentences"),
        tr("Could not receive sentences: %1")
            .arg("Something something"),
        QMessageBox::Ok);
}

void WebserviceWindow::cancelResponse()
{
    qDebug() << "cancelResponse";

    if (d_reply)
        d_reply->abort();
}
