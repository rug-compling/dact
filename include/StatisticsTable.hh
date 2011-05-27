#ifndef STATISTICSTABLE_HH
#define STATISTICSTABLE_HH

#include <QTableView>
#include <QMenu>
#include <QString>

class StatisticsTable : public QTableView
{
    Q_OBJECT

public:
    StatisticsTable(QWidget *parent = 0);
    QString selectionAsCSV(QString const &separator) const;
    
public slots:
    void copy() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    void createContextMenu();

    QMenu d_contextMenu;
};

#endif //STATISTICSTABLE_HH