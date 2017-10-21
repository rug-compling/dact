#include <stdexcept>

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QSettings>
#include <QStringList>
#include <QTextStream>

#include <config.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

#include <WebserviceWindow.hh>
#include <ui_WebserviceWindow.h>

namespace ac = alpinocorpus;

WebserviceWindow::WebserviceWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::WebserviceWindow>(new Ui::WebserviceWindow)),
    d_accessManager(new QNetworkAccessManager),
    d_progressDialog(new QProgressDialog(this))
{
    d_ui->setupUi(this);

    d_progressDialog->setWindowTitle("Parsing text");
    d_progressDialog->reset();

    connect(d_progressDialog.data(),
        SIGNAL(canceled()), SLOT(cancelResponse()));

    connect(this, SIGNAL(progress()), SLOT(updateProgressDialog()),
        Qt::QueuedConnection);
}

WebserviceWindow::~WebserviceWindow()
{
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
        QString("untitled"), tr("Dact corpus (*.dact)"));

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
    d_progressDialog->setRange(0, d_numberOfSentences);
    d_numberOfSentencesReceived = 0;

    // Hide the sentences dialog
    hide();

    // Show the progress dialog
    d_progressDialog->setWindowTitle(tr("Parsing sentences"));
    d_progressDialog->setLabelText("Sending sentences to webservice");
    d_progressDialog->reset();
    d_progressDialog->open();

    d_buffer = QByteArray();

    // Send the request
    QSettings settings;
    QNetworkRequest request(settings.value(WEBSERVICE_BASEURL_KEY, DEFAULT_WEBSERVICE_BASEURL).toString());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain;charset=UTF-8");
    request.setHeader(QNetworkRequest::ContentLengthHeader, sentences.toUtf8().size());
    d_reply = d_accessManager->post(request, sentences.toUtf8());

    // Connect all the event handlers to the response
    connect(d_reply, SIGNAL(readyRead()),
        SLOT(readResponse()));

    connect(d_reply, SIGNAL(error(QNetworkReply::NetworkError)),
        SLOT(errorResponse(QNetworkReply::NetworkError)));
}

int WebserviceWindow::countSentences(QString const &sentences)
{
    return sentences.split('\n', QString::SkipEmptyParts).size();
}

void WebserviceWindow::readResponse()
{
    QByteArray newData = d_reply->readAll();

    // Meh, nothing to read.
    if (newData.size() == 0)
        return;

    // Add data to our internal buffer.
    d_buffer.append(newData);

    // Detect treebank wrapper.
    {
        int treebankIdx = d_buffer.indexOf("<treebank");
        if (treebankIdx != -1)
        {
            // Find the end of the tag.
            int end = d_buffer.indexOf(">", treebankIdx);

            // Convert to a string, so that we can easily pry out the number
            // of sentences.
            QString treebankStr = QString::fromUtf8(
                d_buffer.data() + treebankIdx, end - treebankIdx + 1);
            
            QRegExp treebankPattern("<treebank[^>]+sentences=\"([0-9]+)\"[^>]*>");
            treebankPattern.setMinimal(true);
            if (treebankPattern.indexIn(treebankStr) != -1)
            {
                bool ok;
                int nSents = treebankPattern.cap(1).toInt(&ok);

                if (ok)
                {
                    d_numberOfSentences = nSents;
                    d_progressDialog->setRange(0, d_numberOfSentences);
                }
            }
        }
    }

    int dsIdx;
    while ((dsIdx = d_buffer.indexOf("<alpino_ds")) != -1)
    {
      int endDsIdx = d_buffer.indexOf("</alpino_ds>", dsIdx);
      if (endDsIdx == -1) // Not enough data for this ds.
        return; // Definitely not the end of the corpus...
      
      // Decode as a UTF-8 string, and handle it.
      int dsLen = endDsIdx - dsIdx + QString("</alpino_ds>").size();
      QString ds = QString::fromUtf8(d_buffer.data() + dsIdx, dsLen);
      receiveSentence(ds);

      // Purge data from the buffer. This may not be very efficient, but
      // we are not in a tight loop. So, we KISS.
      d_buffer.remove(0, dsIdx + dsLen);
    }

    // Are we done?
    if (d_buffer.indexOf("</treebank>") != -1)
      finishResponse();
}

void WebserviceWindow::receiveSentence(QString const &sentence)
{
    // Insert the sentence into the corpuswriter
    QRegExp idPattern("<alpino_ds[^>]* id=\"([^\"]+)\"");

    // Find the name in the first element.
    if (idPattern.indexIn(sentence) == -1)
        return; // TODO big fat warning error (or pretty error recovery)

    QString name(idPattern.capturedTexts().at(1));

    try {
      d_corpus->write(name.toUtf8().constData(), sentence.toUtf8().constData());
    } catch (std::runtime_error &e) {
      qDebug() << e.what();
    }

    d_numberOfSentencesReceived++;

    emit progress();
}

void WebserviceWindow::updateProgressDialog()
{
    // Update progress dialog.
    d_progressDialog->setLabelText(tr("Parsing sentence %1 of %2")
        .arg(d_numberOfSentencesReceived)
        .arg(d_numberOfSentences));

    // For some reason, qt crashes on the last update. Race condition maybe? Skip it for now.
    if (d_numberOfSentencesReceived < d_numberOfSentences)
        d_progressDialog->setValue(d_numberOfSentencesReceived);
}

void WebserviceWindow::finishResponse()
{
    // Reset the reply pointer, since the request is no longer active.
    d_reply->disconnect();
    //d_reply = 0;

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
    d_progressDialog->reset();

    QMessageBox box(QMessageBox::Warning,
        tr("Failed to parse sentences"),
        tr("Could not parse sentences: %1")
            .arg(d_reply->errorString()),
        QMessageBox::Ok);

    box.exec();
}

void WebserviceWindow::cancelResponse()
{
    if (d_reply)
        d_reply->abort();

    d_progressDialog->reset();
}
