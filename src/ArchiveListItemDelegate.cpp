#include <QPainter>

#include "ArchiveModel.hh"
#include "ArchiveListItemDelegate.hh"
#include "ArchiveListItemWidget.hh"

ArchiveListItemDelegate::ArchiveListItemDelegate(QObject *parent)
:
    QStyledItemDelegate(parent),
    d_view(QSharedPointer<ArchiveListItemWidget>(new ArchiveListItemWidget()))
{
    
}

ArchiveListItemDelegate::~ArchiveListItemDelegate()
{
    //
}

void ArchiveListItemDelegate::setEntry(QModelIndex const &index) const
{
    d_view->setArchiveEntry(dynamic_cast<ArchiveModel const *>(index.model())->entryAtRow(index.row()));
}

QSize ArchiveListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    setEntry(index);
    d_view->adjustSize();

    return d_view->sizeHint();
}

void ArchiveListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
    {
        // Paint background
        painter->fillRect(option.rect, option.palette.highlight());

        // Set text color
        painter->setBrush(option.palette.highlightedText());
    }

    setEntry(index);

    d_view->resize(option.rect.size());
    painter->save();
    painter->translate(option.rect.topLeft());
    d_view->render(painter, QPoint(), QRegion(), QWidget::DrawChildren);
    painter->restore();
}
