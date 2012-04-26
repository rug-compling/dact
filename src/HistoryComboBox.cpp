#include <QAbstractItemView>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QSettings>
#include <QVariant>
#include <QtDebug>

#include <HistoryComboBox.hh>

HistoryComboBox::HistoryComboBox(QWidget *parent, QString settingsKey) :
  QComboBox(parent)
{
  setEditable(true);
  setDuplicatesEnabled(true);
  setInsertPolicy(InsertAtTop);
  //setMaxCount(64);


  connect(lineEdit(), SIGNAL(returnPressed()),
      SLOT(returnPressed()));

  // Does not work, weirdly enough...
  //connect(view(),
  //    SIGNAL(activated(QModelIndex const &)),
  //    SLOT(itemClicked()));
}

HistoryComboBox::~HistoryComboBox()
{
}

void HistoryComboBox::clearHistory()
{
  clear();
}

void HistoryComboBox::itemClicked()
{
  emit returnOrClick();
}

void HistoryComboBox::readHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QSettings settings;
    QVariant value = settings.value(settingsKey, QStringList());

    if (value.type() == QVariant::StringList) {
      QStringList history(value.toStringList());
      insertItems(count(), history);
    }
    else
      qWarning() << "Read history, but it is not a QStringList.";
  }
}

void HistoryComboBox::returnPressed()
{
  QString compact = text().trimmed();

  if (compact.isEmpty())
    emit activated(QString(""));

  emit returnOrClick();
}

void HistoryComboBox::revalidate()
{
    QString t(lineEdit()->text());
    lineEdit()->clear();
    lineEdit()->insert(t);
}

QString HistoryComboBox::text() const
{
  return currentText();
}

void HistoryComboBox::setText(QString const &newText)
{
  lineEdit()->setText(newText);
  insertItem(0, newText);
}

void HistoryComboBox::writeHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QStringList history;

    for (int i = 0; i < count(); ++i)
      history << itemText(i);

    QSettings settings;
    settings.setValue(settingsKey, history);
  }
}
