#ifndef HISTORY_COMBO_BOX_HH
#define HISTORY_COMBO_BOX_HH

#include <QComboBox>
#include <QString>
#include <QStringList>

class QWidget;

class HistoryComboBox : public QComboBox
{
    Q_OBJECT
public:
    HistoryComboBox(QWidget *parent = 0, QString settingsKey = QString());
    virtual ~HistoryComboBox();
    
    void readHistory(QString const &settingsKey);
    void writeHistory(QString const &settingsKey);

    void revalidate();
    QString text() const;

public slots:
    virtual void setText(QString const &newText);

};

#endif // HISTORY_COMBO_BOX_HH
