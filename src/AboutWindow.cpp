#include <QDesktopServices>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QPixmap>
#include <QUrl>

#include <config.hh>

#include <AboutWindow.hh>

#include <ui_AboutWindow.h>

AboutWindow::AboutWindow(QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::AboutWindow>(new Ui::AboutWindow))
{
    d_ui->setupUi(this);

    d_ui->buildLabel->setText(GIT_REVISION);

    QGraphicsScene *scene = new QGraphicsScene(d_ui->aboutGraphicsView);
    QPixmap logo(":/dact-espresso.png");
    scene->addPixmap(logo);

    d_ui->aboutGraphicsView->setScene(scene);

    connect(d_ui->bugPushButton, SIGNAL(clicked()),
        SLOT(reportBug()));
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::keyPressEvent(QKeyEvent *event)
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

void AboutWindow::reportBug()
{
    static QUrl const issues("https://github.com/rug-compling/dact/issues");
    QDesktopServices::openUrl(issues);
}

