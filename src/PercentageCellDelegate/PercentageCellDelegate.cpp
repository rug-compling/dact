#include <QApplication>
#include "PercentageCellDelegate.h"

void PercentageCellDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    double progress = index.data().toDouble();

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = progress;
    progressBarOption.text = QString("%1%").arg(progress, 0, 'f', 1);
    progressBarOption.textVisible = true;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}
