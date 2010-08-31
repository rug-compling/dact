#include <QDesktopServices>
#include <QGraphicsScene>
#include <QPixmap>
#include <QSharedPointer>
#include <QUrl>
#include <QWidget>

#include <AboutWindow.hh>

#include <ui_AboutWindow.h>

AboutWindow::AboutWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::AboutWindow>(new Ui::AboutWindow))
{
    d_ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(d_ui->aboutGraphicsView);
    QPixmap logo(":/dact-espresso.png");
    scene->addPixmap(logo);

    d_ui->aboutGraphicsView->setScene(scene);

    QObject::connect(d_ui->bugPushButton, SIGNAL(clicked()),
        this, SLOT(reportBug()));
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::reportBug()
{
    QDesktopServices::openUrl(QUrl("http://github.com/danieldk/dact/issues"));
}

