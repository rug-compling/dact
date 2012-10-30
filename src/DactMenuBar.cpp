#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include <DactMenuBar.hh>

#include <ui_DactMenuBar.h>

DactMenuBar::DactMenuBar(QWidget *parent, bool global) :
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

    if (global)
        disableLocalActions();

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
    connect(d_ui->menuRecentFiles, SIGNAL(fileSelected(QString)),
        qApp, SLOT(openCorpus(QString)));

}

DactMenuBar::~DactMenuBar()
{
}

void DactMenuBar::addRecentFile(QString const &filename)
{
    d_ui->menuRecentFiles->addFile(filename);
}

void DactMenuBar::disableLocalActions()
{
    d_ui->saveAsAction->setEnabled(false);
    d_ui->saveCorpus->setEnabled(false);
    d_ui->xmlExportAction->setEnabled(false);
    d_ui->pdfExportAction->setEnabled(false);
    d_ui->printAction->setEnabled(false);
    d_ui->focusFilterAction->setEnabled(false);
    d_ui->focusHighlightAction->setEnabled(false);
    d_ui->globalCopyAction->setEnabled(false);
    d_ui->globalCutAction->setEnabled(false);
    d_ui->globalPasteAction->setEnabled(false);
    d_ui->previousAction->setEnabled(false);
    d_ui->nextAction->setEnabled(false);
    d_ui->fitAction->setEnabled(false);
    d_ui->zoomInAction->setEnabled(false);
    d_ui->zoomOutAction->setEnabled(false);
    d_ui->previousTreeNodeAction->setEnabled(false);
    d_ui->nextTreeNodeAction->setEnabled(false);
    d_ui->inspectorAction->setEnabled(false);
    d_ui->toolbarAction->setEnabled(false);
    d_ui->toggleFullScreenAction->setEnabled(false);
    d_ui->clearHistoryAction->setEnabled(false);
    d_ui->loadMacrosAction->setEnabled(false);
    d_ui->minimizeAction->setEnabled(false);
    d_ui->closeAction->setEnabled(false);
}


void DactMenuBar::setMacrosModel(QSharedPointer<DactMacrosModel> model)
{
	d_ui->menuMacros->setModel(model);
}

QSharedPointer<Ui::DactMenuBar const> DactMenuBar::ui()
{
	return d_ui;
}
