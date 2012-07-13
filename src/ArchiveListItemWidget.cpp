#include <QFile>

#include "ArchiveModel.hh"
#include "ArchiveListItemWidget.hh"
#include "ui_ArchiveListItemWidget.h"

#include "HumanReadableSize.hh"

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

    d_ui->downloadedLabel->setVisible(QFile(entry.filePath()).exists());
}
