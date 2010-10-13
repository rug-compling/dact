#include <QDialog>
#include <QFont>
#include <QFontDialog>
#include <QSettings>
#include <QSharedPointer>
#include <QWidget>

#include <PreferencesWindow.hh>

#include <ui_PreferencesWindow.h>

#include <QtDebug>

PreferencesWindow::PreferencesWindow(QWidget *parent, Qt::WindowFlags f) :
        QWidget(parent, f),
        d_ui(QSharedPointer<Ui::PreferencesWindow>(new Ui::PreferencesWindow))
{
  d_ui->setupUi(this);

  applyAppFont();

  QObject::connect(d_ui->appFontPushButton, SIGNAL(clicked()), this,
    SLOT(selectAppFont()));
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

