#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenuBar>
#include <QSharedPointer>
#include <QWidget>

#include <config.hh>
#include <DactMenuBar.hh>
#include <MainWindow.hh>

#include <ui_DactMenuBar.h>

#ifdef Q_WS_MAC
extern void qt_mac_set_dock_menu(QMenu *);
#endif

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

    if (global) {
        disableLocalActions();

#ifdef Q_WS_MAC
        QMenu *appleDockMenu = new QMenu(this);
        appleDockMenu->addAction(d_ui->openAction);
#ifdef USE_REMOTE_CORPUS
        appleDockMenu->addAction(d_ui->remoteAction);
#endif
        qt_mac_set_dock_menu(appleDockMenu);
#endif
    }

    connect(d_ui->quitAction, SIGNAL(triggered(bool)),
        qApp, SLOT(quit()));
    connect(d_ui->aboutAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showAboutWindow()));
    connect(d_ui->preferencesAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showPreferencesWindow()));
    connect(d_ui->helpAction, SIGNAL(triggered(bool)),
        qApp, SLOT(openHelp()));
    connect(d_ui->cookbookAction, SIGNAL(triggered(bool)),
        qApp, SLOT(openCookbook()));
    connect(d_ui->openAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showOpenCorpus()));
    connect(d_ui->menuRecentFiles, SIGNAL(fileSelected(QString)),
        qApp, SLOT(openCorpus(QString)));
#ifdef USE_REMOTE_CORPUS
    connect(d_ui->remoteAction, SIGNAL(triggered(bool)),
        qApp, SLOT(showRemoteWindow()));
#endif // USE_REMOTE_CORPUS

#ifdef ENABLE_SANDBOXING
    d_ui->menuTools->removeAction(d_ui->convertCorpusMenu->menuAction());
#else
    connect(d_ui->convertCompactCorpusAction, SIGNAL(triggered()),
        qApp, SLOT(convertCompactCorpus()));
    connect(d_ui->convertDirectoryCorpusAction, SIGNAL(triggered()),
        qApp, SLOT(convertDirectoryCorpus()));
#endif // ENABLE_SANDBOXING

#ifdef USE_WEBSERVICE
    connect(d_ui->webserviceAction, SIGNAL(triggered()),
        qApp, SLOT(showWebserviceWindow()));
#endif // USE_WEBSERVICE

    connect(d_ui->clearHistoryAction, SIGNAL(triggered()),
        qApp, SLOT(clearHistory()));

    connect(d_ui->menuWindow, SIGNAL(aboutToShow()),
        SLOT(updateWindowMenu()));
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

void DactMenuBar::updateWindowMenu()
{
    // Remove old menu items
    foreach (QAction *action, d_windowActions)
    {
        d_ui->menuWindow->removeAction(action);
        delete action;
    }

    d_windowActions.clear();

    // Separator
    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    d_windowActions.prepend(separator);

    // Create a new window entry for each MainWindow
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        if (qobject_cast<MainWindow *>(widget) == 0)
            continue;

        QAction *action = new QAction(widget->windowTitle(), this);

        // When clicked, signal the window to move to the foreground
        connect(action, SIGNAL(triggered()), widget, SLOT(makeActiveWindow()));

        // Mark active window with a check mark
        action->setCheckable(true);
        action->setChecked(widget->isActiveWindow());
        
        d_windowActions.append(action);
    }

    // Add them to the menu
    d_ui->menuWindow->addActions(d_windowActions);
}
