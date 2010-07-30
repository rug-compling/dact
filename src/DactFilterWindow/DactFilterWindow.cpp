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
#include "DactMainWindow.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_DactFilterWindow.h"

using namespace alpinocorpus;
using namespace std;

DactFilterWindow::DactFilterWindow(DactMainWindow *mainWindow, QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::DactFilterWindow>(new Ui::DactFilterWindow)),
    d_mainWindow(mainWindow),
    d_corpusReader(corpusReader),
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
    d_corpusReader = corpusReader;
    updateResults();
}

void DactFilterWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
    
    // Don't try to filter with an invalid xpath expression
    if (filter.trimmed().isEmpty() || !d_ui->filterLineEdit->hasAcceptableInput())
        d_xpathFilter.clear();
    else
        d_xpathFilter = QSharedPointer<XPathFilter>(new XPathFilter(filter));

    if (!d_corpusReader.isNull())
        updateResults();
}

void DactFilterWindow::updateResults()
{
    d_ui->resultsListWidget->clear();

    QVector<QString> entries;

    try {
        if (d_xpathFilter.isNull())
            entries = d_corpusReader->entries();
        else
            entries = d_xpathFilter->entries(d_corpusReader.data());
    } catch (runtime_error &e) {
        QMessageBox::critical(this, QString("Error reading corpus"),
            QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
    }

    for (QVector<QString>::const_iterator iter = entries.begin();
         iter != entries.end(); ++iter)
    {
		QFileInfo entryFi(*iter);
		QListWidgetItem *item(new QListWidgetItem(sentenceForFile(entryFi, d_ui->filterLineEdit->text()), d_ui->resultsListWidget));
		item->setData(Qt::UserRole, entryFi.fileName());
    }
}

QString DactFilterWindow::sentenceForFile(QFileInfo const &file, QString const &query)
{
	// Read XML data.
    if (d_corpusReader.isNull())
        throw runtime_error("DactMainWindow::sentenceForFile CorpusReader not available");

    QString xml = d_corpusReader->read(file.fileName());

    if (xml.size() == 0)
        throw runtime_error("DactMainWindow::sentenceForFile: empty XML data!");

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
    
    d_mainWindow->showFile(current->data(Qt::UserRole).toString());
    
    // Raises this window again when using cursor keys after using\
    // [enter] to raise the main window.
    raise();
}

void DactFilterWindow::entryActivated(QListWidgetItem *item)
{
    d_mainWindow->raise();
}

void DactFilterWindow::filterChanged()
{
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
