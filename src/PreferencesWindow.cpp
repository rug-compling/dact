#include <QFont>
#include <QFontDialog>
#include <QKeyEvent>
#include <QObject>
#include <QSettings>

#include <PreferencesWindow.hh>

#include <ui_PreferencesWindow.h>

PreferencesWindow::PreferencesWindow(QWidget *parent, Qt::WindowFlags f)
:
QWidget(parent, f),
d_ui(QSharedPointer<Ui::PreferencesWindow>(new Ui::PreferencesWindow))
{
    d_ui->setupUi(this);

#ifndef __APPLE__
    applyAppFont();
    
    QObject::connect(d_ui->appFontPushButton,
                     SIGNAL(clicked()), this, SLOT(selectAppFont()));
#else
    d_ui->tabWidget->removeTab(0);
#endif
    
    loadColors();    
    
    QObject::connect(d_ui->treeActiveNodeForegroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
    QObject::connect(d_ui->treeActiveNodeBackgroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
    
    QObject::connect(d_ui->keywordsInContextKeywordForegroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
    QObject::connect(d_ui->keywordsInContextKeywordBackgroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
    QObject::connect(d_ui->keywordsInContextContextForegroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
    QObject::connect(d_ui->keywordsInContextContextBackgroundColor, 
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
        
    QObject::connect(d_ui->completeSentencesBackgroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveColors()));
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

void PreferencesWindow::loadColors()
{
    QSettings settings;
    
    settings.beginGroup("Tree");
    
    d_ui->treeActiveNodeForegroundColor->setColor(
        settings.value("activeNodeForeground", QColor(Qt::white)).value<QColor>());
    
    d_ui->treeActiveNodeBackgroundColor->setColor(
        settings.value("activeNodeBackground", QColor(Qt::darkGreen)).value<QColor>());
    
    settings.endGroup();
    
    
    settings.beginGroup("KeywordsInContext");
    
    d_ui->keywordsInContextKeywordForegroundColor->setColor(
        settings.value("keywordForeground", QColor(Qt::black)).value<QColor>());
    
    d_ui->keywordsInContextKeywordBackgroundColor->setColor(
        settings.value("keywordBackground", QColor(Qt::white)).value<QColor>());
    
    d_ui->keywordsInContextContextForegroundColor->setColor(
        settings.value("contextForeground", QColor(Qt::darkGray)).value<QColor>());
    
    d_ui->keywordsInContextContextBackgroundColor->setColor(
        settings.value("contextBackground", QColor(Qt::white)).value<QColor>());
    
    settings.endGroup();
    
    
    settings.beginGroup("CompleteSentence");
    
    d_ui->completeSentencesBackgroundColor->setColor(
           settings.value("background", QColor(Qt::green)).value<QColor>());
    
    settings.endGroup();
}

void PreferencesWindow::saveColors()
{
    QSettings settings;
    
    settings.beginGroup("Tree");
    settings.setValue("activeNodeForeground", d_ui->treeActiveNodeForegroundColor->color());
    settings.setValue("activeNodeBackground", d_ui->treeActiveNodeBackgroundColor->color());
    settings.endGroup();
    
    settings.beginGroup("KeywordsInContext");
    settings.setValue("keywordForeground", d_ui->keywordsInContextKeywordForegroundColor->color());
    settings.setValue("keywordBackground", d_ui->keywordsInContextKeywordBackgroundColor->color());
    settings.setValue("contextForeground", d_ui->keywordsInContextContextForegroundColor->color());
    settings.setValue("contextBackground", d_ui->keywordsInContextContextBackgroundColor->color());
    settings.endGroup();
    
    settings.beginGroup("CompleteSentence");
    settings.setValue("background", d_ui->completeSentencesBackgroundColor->color());
    settings.endGroup();
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
