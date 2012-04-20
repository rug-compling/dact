#ifndef WEBSERVICEWINDOW_H
#define WEBSERVICEWINDOW_H

#include <QWidget>
#include <QSharedPointer>
// #include <QNetworkReply>

namespace Ui {
    class WebserviceWindow;
}

class QNetworkAccessManager;
class QNetworkReply;

class WebserviceWindow : public QWidget {
    Q_OBJECT
public:
    WebserviceWindow(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~WebserviceWindow();

signals:
    void parseSentencesFinished(QString corpus);

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

private:
    QSharedPointer<Ui::WebserviceWindow> d_ui;
    QSharedPointer<QNetworkAccessManager> d_accessManager;
    // QNetworkReply *d_reply;
};

#endif
