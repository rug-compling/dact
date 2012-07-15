#ifndef ARCHIVELISTITEMDELEGATE_H
#define ARCHIVELISTITEMDELEGATE_H

#include <QSharedPointer>
#include <QStyledItemDelegate>

class ArchiveListItemWidget;

class ArchiveListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ArchiveListItemDelegate(QObject *parent = 0);
    ~ArchiveListItemDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    void setEntry(QModelIndex const &index) const;
    QSharedPointer<ArchiveListItemWidget> d_view;
};

#endif // ARCHIVELISTITEMDELEGATE_H
