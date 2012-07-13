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
	d_view->setArchiveEntry(reinterpret_cast<ArchiveModel const *>(index.model())->entryAtRow(index.row()));
}

QSize ArchiveListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	setEntry(index);
	// d_view->adjustSize();

	// return QSize(d_view->size().width(), 75);
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
    d_view->render(painter, option.rect.topLeft(), QRegion(), QWidget::DrawChildren);
}
