#ifndef HISTORY_LINE_EDIT_HH
#define HISTORY_LINE_EDIT_HH

#include <QLineEdit>
#include <QList>

class QWidget;
class QKeyEvent;

class HistoryLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    HistoryLineEdit(QWidget *parent = 0);
    virtual ~HistoryLineEdit () {}
    
    void historyBack();
    void historyForward();
    void keyPressEvent(QKeyEvent *event);

public slots:
    void addHistoryEntry();
    virtual void insert(QString const &newText);
    virtual void setText(QString const &newText);

private:
    QList<QString> d_history;
    QList<QString>::const_iterator d_currentIter;
};

#endif // HISTORY_LINE_EDIT_HH
