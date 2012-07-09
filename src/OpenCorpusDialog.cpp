#include <QDebug>

#include "OpenCorpusDialog.hh"
#include "ui_OpenCorpusDialog.h"

OpenCorpusDialog::OpenCorpusDialog(QWidget *parent)
:
	QDialog(parent),
	d_ui(QSharedPointer<Ui::OpenCorpusDialog>(new Ui::OpenCorpusDialog))
{
	d_ui->setupUi(this);
}

OpenCorpusDialog::~OpenCorpusDialog()
{
	// 
}

void OpenCorpusDialog::openSelectedCorpus()
{
	qDebug() << "Open the selected corpus";
}

void OpenCorpusDialog::openLocalFile()
{
	qDebug() << "Open a local file";
}