#include <QByteArray>
#include <QFile>
#include <QSharedPointer>
#include <QString>
#include <QWidget>

#include <DactHelpWindow.h>

#include "ui_DactHelpWindow.h"

#include <QtDebug>

DactHelpWindow::DactHelpWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DactHelpWindow>(new Ui::DactHelpWindow))
{
    d_ui->setupUi(this);
    QFile helpFile(":/doc/index.html");
    helpFile.open(QFile::ReadOnly);
    QByteArray helpData(helpFile.readAll());
    d_ui->helpTextBrowser->setText(QString(helpData));
}

DactHelpWindow::~DactHelpWindow()
{
}
