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
    size_t bufferSize = d_reply->bytesAvailable();
    char *buffer = new char[bufferSize];
    d_reply->peek(buffer, bufferSize);

    // Convert the buffer to a string for easy access
    QString bufferString(buffer);
    QString bufferString(QString::fromUtf8(buffer, bytesPeeked));
    int bufferOffset = 0;

    // Search for complete sentences in the peeked buffer
    QRegExp sentencePattern("<alpino_ds([^>]*)>(.+?)</alpino_ds>");
    int pos;
    while ((pos = sentencePattern.indexIn(bufferString, bufferOffset)) != -1)
    {
        // If the match is not in front, read (skip) the data in front of it
        // till it is in front.
        if (pos != 0) {
            d_reply->read(pos);
            // FIXME pos is in characters, but utf8. Reading bytes, and that's why this is probably horribly broken!
            bufferOffset += pos;
        }

        // Read the sentence from the real stream, incrementing its internal pointer
        QString sentence(d_reply->read(sentencePattern.matchedLength()));
        bufferOffset += sentencePattern.matchedLength();

        // Deal with the sentence itself.
        receiveSentence(sentence);
    }

    delete[] buffer;
}

void WebserviceWindow::receiveSentence(QString const &sentence)
{
    // Insert the sentence into the corpuswriter
    QRegExp idPattern("<alpino_ds[^>]* id=\"([^\"]+)\"");

    // Find the name in the first element.
    if (idPattern.indexIn(sentence) == -1)
        return; // TODO big fat warning error (or pretty error recovery)

    QString name(idPattern.capturedTexts().at(1));

    d_corpus->write(name.toUtf8().constData(), sentence.toUtf8().constData());

    d_numberOfSentencesReceived++;

    updateProgressDialog();
}

void WebserviceWindow::updateProgressDialog()
{
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

    // Also close the corpus writer
    d_corpus.clear();

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
