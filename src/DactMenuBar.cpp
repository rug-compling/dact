#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include <DactMenuBar.hh>

#include <ui_DactMenuBar.h>

DactMenuBar::DactMenuBar(QWidget *parent) :
    QMenuBar(parent),
    d_ui(QSharedPointer<Ui::DactMenuBar>(new Ui::DactMenuBar))
{
    d_ui->setupUi(this);

#ifndef USE_WEBSERVICE
    d_ui->menuTools->removeAction(d_ui->webserviceAction);
#endif

#ifndef USE_REMOTE_CORPUS
    d_ui->menuFile->removeAction(d_ui->remoteAction);
#endif

    connect(d_ui->quitAction, SIGNAL(triggered(bool)),
        qApp, SLOT(quit()));
    connect(d_ui->aboutAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showAboutWindow()));
    connect(d_ui->preferencesAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showPreferencesWindow()));
    connect(d_ui->cookbookAction, SIGNAL(triggered(bool)),
        qApp, SLOT(openCookbook()));
    connect(d_ui->openAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showOpenCorpus()));
}

DactMenuBar::~DactMenuBar()
{
}

void DactMenuBar::addRecentFile(QString const &filename)
{
    d_ui->menuRecentFiles->addFile(filename);
}

void DactMenuBar::setMacrosModel(QSharedPointer<DactMacrosModel> model)
{
	d_ui->menuMacros->setModel(model);
}

QSharedPointer<Ui::DactMenuBar const> DactMenuBar::ui()
{
	return d_ui;
}
