#ifndef HISTORY_LINE_EDIT_HH
#define HISTORY_LINE_EDIT_HH

#include <QLineEdit>
#include <QString>
#include <QStringList>

class QWidget;
class QKeyEvent;

class HistoryLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    HistoryLineEdit(QWidget *parent = 0, QString settingsKey = QString());
    virtual ~HistoryLineEdit();
    
    void historyBack();
    void historyForward();
    void keyPressEvent(QKeyEvent *event);
    void readHistory(QString const &settingsKey);
    void revalidate();
    void writeHistory(QString const &settingsKey);

public slots:
    void addHistoryEntry();
    virtual void insert(QString const &newText);
    virtual void setText(QString const &newText);

private:

    QStringList d_history;
    QStringList::const_iterator d_currentIter;
};

#endif // HISTORY_LINE_EDIT_HH
