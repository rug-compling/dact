#include "ArchiveModel.hh"
#include "ArchiveListItemWidget.hh"
#include "ui_ArchiveListItemWidget.h"

QString humanReadableSize(float num)
{
	// Source: http://lists.qt.nokia.com/pipermail/qt-interest/2010-August/027043.html
     QStringList list;
     list << "KB" << "MB" << "GB" << "TB";

     QStringListIterator i(list);
     QString unit("bytes");

     while (num >= 1024.0 && i.hasNext())
     {
         unit = i.next();
         num /= 1024.0;
     }
     return QString().setNum(num,'f',2) + " " + unit;
}

ArchiveListItemWidget::ArchiveListItemWidget(QWidget *parent)
:
	QWidget(parent),
	d_ui(QSharedPointer<Ui::ArchiveListItemWidget>(new Ui::ArchiveListItemWidget()))
{
	d_ui->setupUi(this);
}

ArchiveListItemWidget::~ArchiveListItemWidget()
{
	// declared here to generate QSharedPointer destructor where Ui::ArchiveListItemWidget is known.
}

void ArchiveListItemWidget::setArchiveEntry(ArchiveEntry const &entry)
{
	d_ui->nameLabel->setText(entry.name);
	d_ui->descriptionLabel->setText(entry.description);
	d_ui->sizeLabel->setText(humanReadableSize(entry.size));
}
