#include <QDialog>
#include <QFont>
#include <QFontDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QSharedPointer>
#include <QWidget>

#include <PreferencesWindow.hh>

#include <ui_PreferencesWindow.h>

#include <QtDebug>

PreferencesWindow::PreferencesWindow(QWidget *parent, Qt::WindowFlags f)
:
QWidget(parent, f),
d_ui(QSharedPointer<Ui::PreferencesWindow>(new Ui::PreferencesWindow))
{
    d_ui->setupUi(this);

    applyAppFont();
    loadKeywordsInContextColors();

    QObject::connect(d_ui->appFontPushButton,
        SIGNAL(clicked()), this, SLOT(selectAppFont()));
    
    QObject::connect(d_ui->keywordsInContextKeywordForegroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveKeywordsInContextColors()));
    QObject::connect(d_ui->keywordsInContextKeywordBackgroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveKeywordsInContextColors()));
    QObject::connect(d_ui->keywordsInContextContextForegroundColor,
        SIGNAL(colorSelected(QColor)), this, SLOT(saveKeywordsInContextColors()));
    QObject::connect(d_ui->keywordsInContextContextBackgroundColor, 
        SIGNAL(colorSelected(QColor)), this, SLOT(saveKeywordsInContextColors()));
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

    QSettings settings("RUG", "Dact");
    settings.setValue("appFont", newFont.toString());

    applyAppFont();
}

void PreferencesWindow::loadKeywordsInContextColors()
{
    QSettings settings("RUG", "Dact");
    settings.beginGroup("KeywordsInContext");
    
    d_ui->keywordsInContextKeywordForegroundColor->setColor(
        settings.value("keywordForeground", QColor(Qt::black)).value<QColor>());
    
    d_ui->keywordsInContextKeywordBackgroundColor->setColor(
        settings.value("keywordBackground", QColor(Qt::white)).value<QColor>());
    
    d_ui->keywordsInContextContextForegroundColor->setColor(
        settings.value("contextForeground", QColor(Qt::darkGray)).value<QColor>());
    
    d_ui->keywordsInContextContextBackgroundColor->setColor(
        settings.value("contextBackground", QColor(Qt::white)).value<QColor>());
}

void PreferencesWindow::saveKeywordsInContextColors()
{
    QSettings settings("RUG", "Dact");
    settings.beginGroup("KeywordsInContext");
    
    settings.setValue("keywordForeground", d_ui->keywordsInContextKeywordForegroundColor->color());
    settings.setValue("keywordBackground", d_ui->keywordsInContextKeywordBackgroundColor->color());
    settings.setValue("contextForeground", d_ui->keywordsInContextContextForegroundColor->color());
    settings.setValue("contextBackground", d_ui->keywordsInContextContextBackgroundColor->color());
}

void PreferencesWindow::keyPressEvent(QKeyEvent *event)
{
    // Cmd + w closes the window in OS X (and in some programs on Windows as well)
    // But closing preference windows with ESC isn't uncommon either.
    if (event->key() == Qt::Key_Escape
        || event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier)
    {
        hide();
        event->accept();
    }
    else
        QWidget::keyPressEvent(event);
}
