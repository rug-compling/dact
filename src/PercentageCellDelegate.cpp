#include <QApplication>
#include "PercentageCellDelegate.hh"

void PercentageCellDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Data supplied to this type of column should be in the range of 0..1
    double progress = index.data().toDouble() * 100;

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = progress;
    progressBarOption.text = QString("%1%").arg(progress, 0, 'f', 1);
    progressBarOption.textVisible = true;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
