#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSettings>
#include <QVariant>
#include <QtDebug>

#include <HistoryComboBox.hh>
#include <Workspace.hh>

HistoryComboBox::HistoryComboBox(QWidget *parent, QString settingsKey) :
  QComboBox(parent)
{
  setEditable(true);
  setDuplicatesEnabled(true);
  setInsertPolicy(InsertAtTop);
  //setMaxCount(64);
}

HistoryComboBox::~HistoryComboBox()
{
}

void HistoryComboBox::readHistory(Workspace *workspace)
{
  QStringList history = workspace->history();
  insertItems(count(), history);
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
