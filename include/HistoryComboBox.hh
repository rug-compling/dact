#ifndef HISTORY_COMBO_BOX_HH
#define HISTORY_COMBO_BOX_HH

#include <QComboBox>
#include <QString>
#include <QStringList>

class QWidget;
class Workspace;

class HistoryComboBox : public QComboBox
{
    Q_OBJECT
public:
    HistoryComboBox(QWidget *parent = 0, QString settingsKey = QString());
    virtual ~HistoryComboBox();
    
    void readHistory(Workspace *workspace);
    void writeHistory(Workspace *workspace);
    void clearHistory();

    void revalidate();
    QString text() const;

signals:
    void returnOrClick();

public slots:
    virtual void itemClicked();
    virtual void returnPressed();
    virtual void setText(QString const &newText);

private slots:
    void comboBoxActivated(QString const &);
    void listViewClicked(QModelIndex const &);

private:
    bool d_listViewWasClicked;
};

#endif // HISTORY_COMBO_BOX_HH
