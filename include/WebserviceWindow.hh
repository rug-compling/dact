#ifndef WEBSERVICEWINDOW_H
#define WEBSERVICEWINDOW_H

#include <QNetworkReply>
#include <QWidget>
#include <QSharedPointer>
#include <QString>

namespace Ui {
    class WebserviceWindow;
}

namespace alpinocorpus {
    class CorpusWriter;
}

class QNetworkAccessManager;
class QProgressDialog;

class WebserviceWindow : public QWidget {
    Q_OBJECT
public:
    WebserviceWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~WebserviceWindow();

signals:
    void parseSentencesFinished(QString corpus);

    /*!
     * This signal is emitted when there is progress (one or more sentences
     * were parse.
     */
    void progress();

private slots:
    /*!
     * Ask the user which plain text file(s) to load into the sentences text field.
     */
    void openSentencesFile();

    /*!
     * Ask the user where to save the parsed sentences and send the sentences
     * to the webservice.
     */
    void parseSentences();

    /*!
     * Read available response data from the request reply (i.e. the xml of a parsed sentence)
     */
    void readResponse();

    /*!
     * Called when receiving of response is finished (or errored or aborted).
     */
    void finishResponse();

    /*!
     * Display a message that an error occurred during downloading of the response from the webservice.
     */
    void errorResponse(QNetworkReply::NetworkError error);

    /*!
     * Called when the parse sentences progress was cancelled by the user.
     */
    void cancelResponse();

    /*!
     * Called when there is progress to report.
     */
    void updateProgressDialog();


private:
    QSharedPointer<Ui::WebserviceWindow> d_ui;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    QProgressDialog *d_progressDialog;

    QNetworkReply *d_reply;
    QByteArray d_buffer;

    int d_numberOfSentences;
    int d_numberOfSentencesReceived;
    
    QSharedPointer<alpinocorpus::CorpusWriter> d_corpus;
    QString d_corpusFilename;

    void clearSentencesField();
    void loadSentencesFile(QString const &filename);

    int countSentences(QString const &);
    void receiveSentence(QString const &);
};

#endif
