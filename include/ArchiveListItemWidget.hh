#ifndef ARCHIVELISTITEMWIDGET_H
#define ARCHIVELISTITEMWIDGET_H

#include <QSharedPointer>
#include <QWidget>

namespace Ui {
    class ArchiveListItemWidget;
}

class ArchiveEntry;

class ArchiveListItemWidget : public QWidget
{
    Q_OBJECT
public:
    ArchiveListItemWidget(QWidget *parent = 0);
    ~ArchiveListItemWidget();

    void setArchiveEntry(ArchiveEntry const &entry);

private:
    QSharedPointer<Ui::ArchiveListItemWidget> d_ui;
};

#endif // ARCHIVELISTITEMWIDGET_H
