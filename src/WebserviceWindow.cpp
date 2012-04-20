#include <QDebug>
#include <QNetworkAccessManager>

#include <WebserviceWindow.hh>
#include <ui_WebserviceWindow.h>

WebserviceWindow::WebserviceWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::WebserviceWindow>(new Ui::WebserviceWindow)),
    d_accessManager(new QNetworkAccessManager)
{
    d_ui->setupUi(this);
}

WebserviceWindow::~WebserviceWindow()
{
}

void WebserviceWindow::openSentencesFile()
{
    qDebug() << "Open sentences file!";
}

void WebserviceWindow::parseSentences()
{
    qDebug() << "Parse sentences!";
}
