#ifndef DACTQUERYHISTORY_H
#define DACTQUERYHISTORY_H

#include <QAbstractListModel>
#include <QCompleter>
#include <QList>;

class DactQueryHistory : public QAbstractListModel
{
	Q_OBJECT
	
	QCompleter* d_completer;
	QList<QString> d_history;
	
public:
	DactQueryHistory();
	int rowCount(QModelIndex const &parent = QModelIndex()) const;
	QCompleter* completer();
    QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    void addToHistory(QString const &query);

private:
	void readHistory();
	void writeHistory();
};

#endif
