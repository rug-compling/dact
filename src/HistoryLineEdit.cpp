#include <QKeyEvent>
#include <QSettings>
#include <QVariant>
#include <QtDebug>

#include <HistoryLineEdit.hh>

HistoryLineEdit::HistoryLineEdit(QWidget *parent, QString settingsKey) :
  QLineEdit(parent)
{
    connect(this, SIGNAL(returnPressed()), SLOT(addHistoryEntry()));
}

HistoryLineEdit::~HistoryLineEdit()
{
}

void HistoryLineEdit::addHistoryEntry()
{
    d_history.push_back(text());
    d_currentIter = d_history.end() - 1;
}

void HistoryLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        historyBack();
        event->accept();
    }
    else if (event->key() == Qt::Key_Down) {
        historyForward();
        event->accept();
    }
    else
        QLineEdit::keyPressEvent(event);
}

void HistoryLineEdit::historyBack()
{
    if (d_history.empty() ||
            d_currentIter == d_history.begin())
        return;
    
    --d_currentIter;
    
    QLineEdit::setText(*d_currentIter);
}

void HistoryLineEdit::historyForward()
{
    if (d_history.empty() ||
            d_currentIter + 1 == d_history.end())
        return;
    
    ++d_currentIter;
    
    QLineEdit::setText(*d_currentIter);
}

void HistoryLineEdit::insert(QString const &newText)
{
  QLineEdit::insert(newText);

  addHistoryEntry();
}

void HistoryLineEdit::readHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QSettings settings;
    QVariant history = settings.value(settingsKey, QStringList());

    if (history.type() == QVariant::StringList) {
      d_history = history.toStringList();

      if (d_history.size() > 0) {
          d_currentIter = d_history.end() - 1;
          QLineEdit::setText(*d_currentIter);
      }
    }
    else
      qWarning() << "Read history, but it is not a QStringList.";
  }
}

void HistoryLineEdit::revalidate()
{
    QString t(text());
    QLineEdit::clear();
    QLineEdit::insert(t);
}

void HistoryLineEdit::setText(QString const &newText)
{
  QLineEdit::setText(newText);

  addHistoryEntry();
}

void HistoryLineEdit::writeHistory(QString const &settingsKey)
{
  if (!settingsKey.isEmpty()) {
    QSettings settings;
    settings.setValue(settingsKey, d_history);
  }
}
