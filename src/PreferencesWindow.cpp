#include <QFileDialog>
#include <QFont>
#include <QFontDialog>
#include <QKeyEvent>
#include <QObject>
#include <QSettings>

#include <DactSettings.hh>
#include <PreferencesWindow.hh>
#include <config.hh>

#include <ui_PreferencesWindow.h>

PreferencesWindow::PreferencesWindow(QWidget *parent, Qt::WindowFlags f)
:
QWidget(parent, f),
d_ui(QSharedPointer<Ui::PreferencesWindow>(new Ui::PreferencesWindow))
{
    d_ui->setupUi(this);

#ifndef USE_REMOTE_CORPUS
    d_ui->groupBox_x->hide();
#endif

#ifndef USE_WEBSERVICE
    d_ui->groupBoxWebservice->hide();
#endif


#ifndef __APPLE__
    applyAppFont();

    connect(d_ui->appFontPushButton, SIGNAL(clicked()),
        SLOT(selectAppFont()));
#else
    d_ui->tabWidget->removeTab(0);
#endif

    loadColorsTab();
    loadNetworkTab();
    loadRemoteTab();
    loadToolsTab();

    connect(d_ui->treeActiveNodeBorderColor, SIGNAL(colorSelected(QColor)),
        SLOT(saveColorsTab()));
    
    connect(d_ui->completeSentencesBackgroundColor, SIGNAL(colorSelected(QColor)),
        SLOT(saveColorsTab()));

    connect(d_ui->archiveBaseUrlLineEdit, SIGNAL(editingFinished()),
        SLOT(saveArchiveBaseUrl()));

    connect(d_ui->remoteBaseUrlLineEdit, SIGNAL(editingFinished()),
        SLOT(saveRemoteBaseUrl()));

    connect(d_ui->webserviceBaseUrlLineEdit, SIGNAL(editingFinished()),
        SLOT(saveWebserviceBaseUrl()));
    
    connect(d_ui->toolsFilePath, SIGNAL(editingFinished()),
        SLOT(saveToolsTab()));
    
    connect(d_ui->restoreDefaultColorsButton, SIGNAL(clicked()),
        SLOT(restoreDefaultColors()));

    connect(d_ui->restoreDefaultNetworkButton, SIGNAL(clicked()),
        SLOT(restoreDefaultNetwork()));

    connect(d_ui->restoreDefaultRemoteButton, SIGNAL(clicked()),
        SLOT(restoreDefaultRemote()));

    connect(d_ui->restoreDefaultWebserviceButton, SIGNAL(clicked()),
        SLOT(restoreDefaultWebservice()));

    connect(d_ui->selectToolsFilePath, SIGNAL(clicked()),
        SLOT(selectToolsFilePath()));
}

PreferencesWindow::~PreferencesWindow() {}

void PreferencesWindow::applyAppFont()
{
    d_ui->appFontLineEdit->setText(qApp->font().family());
}

void PreferencesWindow::selectAppFont()
{
    bool ok;
    QFont newFont(QFontDialog::getFont(&ok, qApp->font(), this));
    if (!ok)
        return;

    qApp->setFont(newFont);

    QSettings().setValue("appFont", newFont.toString());

    applyAppFont();
}

void PreferencesWindow::selectToolsFilePath()
{
    QString path(QFileDialog::getOpenFileName(this, "Select Tools configuration file"));

    if (path.isNull())
        return;

    d_ui->toolsFilePath->setText(path);
    saveToolsTab();
}

void PreferencesWindow::loadColorsTab()
{
    QSettings settings;

    settings.beginGroup("Tree");

    d_ui->treeActiveNodeBorderColor->setColor(
        settings.value("activeNodeBorder", QColor(Qt::black)).value<QColor>());

    settings.endGroup();

    settings.beginGroup("CompleteSentence");

    d_ui->completeSentencesBackgroundColor->setColor(
           settings.value("background", QColor(140, 50, 255)).value<QColor>());

    settings.endGroup();
}

void PreferencesWindow::loadNetworkTab()
{
    QSettings settings;
    d_ui->archiveBaseUrlLineEdit->setText(
        settings.value(ARCHIVE_BASEURL_KEY, DEFAULT_ARCHIVE_BASEURL).toString());

    d_ui->webserviceBaseUrlLineEdit->setText(
        settings.value(WEBSERVICE_BASEURL_KEY, DEFAULT_WEBSERVICE_BASEURL).toString());
}

void PreferencesWindow::loadRemoteTab()
{
    QSettings settings;
    d_ui->remoteBaseUrlLineEdit->setText(
        settings.value(REMOTE_BASEURL_KEY, DEFAULT_REMOTE_BASEURL).toString());
}

void PreferencesWindow::loadToolsTab()
{
    d_ui->toolsFilePath->setText(
        DactSettings::sharedInstance()->value("toolsFilePath", "").toString());
}

void PreferencesWindow::saveArchiveBaseUrl()
{
    QSettings settings;
    settings.setValue(ARCHIVE_BASEURL_KEY, d_ui->archiveBaseUrlLineEdit->text());
}

void PreferencesWindow::saveRemoteBaseUrl()
{
    QSettings settings;
    settings.setValue(REMOTE_BASEURL_KEY, d_ui->remoteBaseUrlLineEdit->text());
}

void PreferencesWindow::saveWebserviceBaseUrl()
{
    QSettings settings;
    settings.setValue(WEBSERVICE_BASEURL_KEY, d_ui->webserviceBaseUrlLineEdit->text());
}

void PreferencesWindow::saveColorsTab()
{
    QSettings settings;

    settings.beginGroup("Tree");
    settings.setValue("activeNodeBorder", d_ui->treeActiveNodeBorderColor->color());
    settings.endGroup();

    settings.beginGroup("CompleteSentence");
    settings.setValue("background", d_ui->completeSentencesBackgroundColor->color());
    settings.endGroup();

    emit colorChanged();
}

void PreferencesWindow::saveToolsTab()
{
    DactSettings::sharedInstance()->setValue("toolsFilePath", d_ui->toolsFilePath->text());
}

void PreferencesWindow::restoreDefaultColors()
{
    QSettings settings;

    settings.beginGroup("Tree");
    settings.remove("");
    settings.endGroup();

    settings.beginGroup("CompleteSentence");
    settings.remove("");
    settings.endGroup();

    loadColorsTab();
}

void PreferencesWindow::restoreDefaultNetwork()
{
    QSettings settings;

    settings.remove(ARCHIVE_BASEURL_KEY);

    loadNetworkTab();
}

void PreferencesWindow::restoreDefaultRemote()
{
    QSettings settings;

    settings.remove(REMOTE_BASEURL_KEY);

    loadRemoteTab();
}

void PreferencesWindow::restoreDefaultWebservice()
{
    QSettings settings;

    settings.remove(WEBSERVICE_BASEURL_KEY);

    loadNetworkTab();
}

void PreferencesWindow::keyPressEvent(QKeyEvent *event)
{
    // Cmd + w closes the window in OS X (and in some programs on Windows as well)
    // But closing preference windows with ESC isn't uncommon either.
    if (event->key() == Qt::Key_Escape
        || (event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier))
    {
        hide();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}
