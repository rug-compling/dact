#include <QFileInfo>
#include <QLineEdit>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QVector>
#include <QtDebug>

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include <AlpinoCorpus/CorpusReader.hh>

#include "DactFilterWindow.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactFilterWindow.h"

using namespace alpinocorpus;
using namespace std;

DactFilterWindow::DactFilterWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DactFilterWindow>(new Ui::DactFilterWindow)),
    d_corpusReader(corpusReader),
	d_xpathMapper(QSharedPointer<XPathMapper>(new XPathMapper)),
    d_xpathValidator(new XPathValidator)
{
    d_ui->setupUi(this);
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
    initSentenceTransformer();
    createActions();
    readSettings();
}

DactFilterWindow::~DactFilterWindow()
{
}

void DactFilterWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
	if(d_xpathMapper->isRunning()) {
        d_xpathMapper->terminate();
		d_xpathMapper->wait();
	}
	
    d_corpusReader = corpusReader;
	
    updateResults();
}

void DactFilterWindow::setFilter(QString const &filter)
{
	d_ui->filterLineEdit->setText(filter);
    
	// Don't try to filter with an invalid xpath expression
    if (!d_ui->filterLineEdit->hasAcceptableInput())
        d_filter = QString();
    else
        d_filter = filter.trimmed();

	updateResults();
}

void DactFilterWindow::updateResults()
{
	if(!d_corpusReader || d_filter.isEmpty()) {
		qWarning() << "Not running, empty reader or filter";
		return;
	}
	
    try {
		if(d_xpathMapper->isRunning()) {
			d_xpathMapper->cancel();
			d_xpathMapper->wait();
		}
		
		d_ui->resultsListWidget->clear();
		
		d_xpathMapper->start(d_corpusReader.data(), d_filter, &d_entryMap);
	} catch(runtime_error &e) {
		QMessageBox::critical(this, QString("Error reading corpus"),
		    QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
	}
	
	return;
}

void DactFilterWindow::entryFound(QString file)
{
	QString label = sentenceForFile(file, d_filter);
	QListWidgetItem *item(new QListWidgetItem(label, d_ui->resultsListWidget));
	item->setData(Qt::UserRole, file);
}

QString DactFilterWindow::sentenceForFile(QString const &file, QString const &query)
{
	// Read XML data.
    if (d_corpusReader.isNull())
        throw runtime_error("DactFilterWindow::sentenceForFile CorpusReader not available");

    QString xml = d_corpusReader->read(file);

    if (xml.size() == 0)
        throw runtime_error("DactFilterWindow::sentenceForFile: empty XML data!");

    // Parameters
    QString valStr = query.trimmed().isEmpty() ? "'/..'" :
                     QString("'") + query + QString("'");
    QHash<QString, QString> params;
    params["expr"] = valStr;

	return d_sentenceTransformer->transform(xml, params).trimmed();
}

void DactFilterWindow::applyValidityColor(QString const &)
{
    // Hmpf, unfortunately we get text, rather than a sender. Attempt
    // to determine the sender ourselvers.
    QObject *sender = this->sender();
    
    if (!sender)
        return;

    if (!sender->inherits("QLineEdit"))
        return;

    QLineEdit *widget = reinterpret_cast<QLineEdit *>(sender);

    if (widget->hasAcceptableInput())
        widget->setStyleSheet("");
    else
        widget->setStyleSheet("background-color: salmon");
}

void DactFilterWindow::createActions()
{
	QObject::connect(&d_entryMap, SIGNAL(entryFound(QString)), this, SLOT(entryFound(QString)));
	
	QObject::connect(d_xpathMapper.data(), SIGNAL(started(int)), this, SLOT(mapperStarted(int)));
	QObject::connect(d_xpathMapper.data(), SIGNAL(stopped(int, int)), this, SLOT(mapperStopped(int, int)));
	QObject::connect(d_xpathMapper.data(), SIGNAL(progress(int, int)), this, SLOT(mapperProgressed(int,int)));
	
    QObject::connect(d_ui->resultsListWidget,
        SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
        this,
        SLOT(entrySelected(QListWidgetItem*,QListWidgetItem*)));
    QObject::connect(d_ui->resultsListWidget, 
        // itemActivated is triggered by a single click on some systems
        // where this is the configured behavior: it can be annoying.
        // But it also enables using [enter] to raise the main window
        // which is the expected/preferred behavior.
        SIGNAL(itemActivated(QListWidgetItem*)),
        this,
        SLOT(entryActivated(QListWidgetItem*)));

    QObject::connect(d_ui->filterLineEdit, SIGNAL(textChanged(QString const &)), this,
        SLOT(applyValidityColor(QString const &)));
	
    QObject::connect(d_ui->filterLineEdit, SIGNAL(returnPressed()), this, SLOT(filterChanged()));
}

void DactFilterWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0)
        return;
    
    emit currentEntryChanged(current->data(Qt::UserRole).toString());
    
    // Raises this window again when using cursor keys after using\
    // [enter] to raise the main window.
    raise();
}

void DactFilterWindow::entryActivated(QListWidgetItem *item)
{
    emit entryActivated();
}

void DactFilterWindow::filterChanged()
{
	qWarning() << "Filter Changed";
	
    setFilter(d_ui->filterLineEdit->text());
}

void DactFilterWindow::initSentenceTransformer()
{
    // Read stylesheet.
    QFile xslFile(":/stylesheets/bracketed-sentence.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QTextStream xslStream(&xslFile);
    QString xsl(xslStream.readAll());
    d_sentenceTransformer = QSharedPointer<XSLTransformer>(new XSLTransformer(xsl));
}

void DactFilterWindow::mapperStarted(int totalEntries)
{
	d_ui->filterProgressBar->setMinimum(0);
	d_ui->filterProgressBar->setMaximum(totalEntries);
	d_ui->filterProgressBar->setValue(0);
	d_ui->filterProgressBar->setVisible(true);
}

void DactFilterWindow::mapperStopped(int processedEntries, int totalEntries)
{
	d_ui->filterProgressBar->setVisible(false);
}

void DactFilterWindow::mapperProgressed(int processedEntries, int totalEntries)
{
	d_ui->filterProgressBar->setValue(processedEntries);
}


void DactFilterWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void DactFilterWindow::readSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry.
    QPoint pos = settings.value("filter_pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("filter_size", QSize(350, 400)).toSize();
    resize(size);

    // Move.
    move(pos);
}

void DactFilterWindow::writeSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry
    settings.setValue("filter_pos", pos());
    settings.setValue("filter_size", size());
}
