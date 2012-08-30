#include <QAbstractItemView>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QSettings>
#include <QVariant>
#include <QtDebug>

#include <HistoryComboBox.hh>
#include <Workspace.hh>

HistoryComboBox::HistoryComboBox(QWidget *parent, QString settingsKey) :
  QComboBox(parent),
  d_listViewWasClicked(false)
{
  setEditable(true);
  setDuplicatesEnabled(true);
  setInsertPolicy(InsertAtTop);
  //setMaxCount(64);

  connect(lineEdit(), SIGNAL(returnPressed()),
    SLOT(returnPressed()));

  // Listen for activation. If an entry was selected from the list view,
  // and then the activation event fired, it was probably caused by the click.
  // Handle this case as if the user pressed the enter key.
  connect(this, SIGNAL(activated(QString const &)),
    SLOT(comboBoxActivated(QString const &)));

  connect(view(), SIGNAL(pressed(QModelIndex const &)),
    SLOT(listViewClicked(QModelIndex const &)));
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

void HistoryComboBox::readHistory(Workspace *workspace)
{
  clear();
  QStringList history = workspace->history();
  insertItems(count(), history);
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

void HistoryComboBox::writeHistory(Workspace *workspace)
{
    QStringList history;

    for (int i = 0; i < count(); ++i)
      history << itemText(i);

    workspace->setHistory(history);
}

void HistoryComboBox::comboBoxActivated(QString const &text)
{
  if (d_listViewWasClicked)
  {
    d_listViewWasClicked = false;
    returnPressed();
  }
}

void HistoryComboBox::listViewClicked(QModelIndex const &index)
{
  d_listViewWasClicked = true;
}
