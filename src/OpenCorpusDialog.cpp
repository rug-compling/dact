#include <QDebug>
#include <QFileDialog>

#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/Error.hh>

#include "OpenCorpusDialog.hh"
#include "ui_OpenCorpusDialog.h"

namespace ac = alpinocorpus;

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

QString OpenCorpusDialog::getCorpusFileName(QWidget *parent)
{
	OpenCorpusDialog dialog(parent);

	return dialog.exec() == QDialog::Accepted
		? dialog.d_selectedFileName
		: QString();
}

QSharedPointer<ac::CorpusReader> OpenCorpusDialog::getCorpusReader(QWidget *parent)
{
	// In the most ideal case, the OpenCorpusDialog would return just a corpus reader
	// which could be reading a local file, or a webservice, or anything else Dact
	// can open. All code to open a file and create a reader would be moved to
	// OpenCorpusDialog. One problem remains: where should the code live that is used
	// to open files passed as arguments on the command line?
	return QSharedPointer<ac::CorpusReader>(0);
}

void OpenCorpusDialog::openSelectedCorpus()
{
	qDebug() << "Open the selected corpus";
}

void OpenCorpusDialog::openLocalFile()
{
	d_selectedFileName = QFileDialog::getOpenFileName(this,
		"Open corpus", QString(), "Dact corpora (*.dact)");

	if (!d_selectedFileName.isNull())
		accept();
}