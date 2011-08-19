#include <QKeyEvent>

#include <HistoryLineEdit.hh>

HistoryLineEdit::HistoryLineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, SIGNAL(returnPressed()), SLOT(addHistoryEntry()));
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
    else {
        QLineEdit::keyPressEvent(event);
    }
}

void HistoryLineEdit::historyBack()
{
    if (d_history.empty() ||
            d_currentIter == d_history.begin())
        return;
    
    --d_currentIter;
    
    setText(*d_currentIter);
}

void HistoryLineEdit::historyForward()
{
    if (d_history.empty() ||
            d_currentIter + 1 == d_history.end())
        return;
    
    ++d_currentIter;
    
    setText(*d_currentIter);
}