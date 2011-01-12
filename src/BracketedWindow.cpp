#include <QFileInfo>
#include <QKeyEvent>
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

#include "BracketedDelegates.hh"
#include "BracketedWindow.hh"
#include "DactMacrosModel.h"
#include "XPathValidator.hh"
#include "XSLTransformer.hh"
#include "ui_BracketedWindow.h"

using namespace alpinocorpus;
using namespace std;

BracketedWindow::BracketedWindow(QSharedPointer<alpinocorpus::CorpusReader> corpusReader,
        QSharedPointer<DactMacrosModel> macrosModel, QWidget *parent, Qt::WindowFlags f) :
    QWidget(parent, f),
    d_ui(QSharedPointer<Ui::BracketedWindow>(new Ui::BracketedWindow)),
    d_corpusReader(corpusReader),
    d_entryMap(0),
    d_macrosModel(macrosModel),
    d_xpathMapper(QSharedPointer<XPathMapper>(new XPathMapper)),
    d_xpathValidator(new XPathValidator(d_macrosModel))
{
    d_ui->setupUi(this);
    d_ui->filterLineEdit->setValidator(d_xpathValidator.data());
	initListDelegates();
    initSentenceTransformer();
    createActions();
    readSettings();
}

BracketedWindow::~BracketedWindow()
{
    stopMapper();
}

void BracketedWindow::switchCorpus(QSharedPointer<alpinocorpus::CorpusReader> corpusReader)
{
    stopMapper();

    d_corpusReader = corpusReader;
    
    updateResults();
}

void BracketedWindow::setFilter(QString const &filter)
{
    d_ui->filterLineEdit->setText(filter);
    
    // Don't try to filter with an invalid xpath expression
    if (!d_ui->filterLineEdit->hasAcceptableInput())
        d_filter = QString();
    else
        d_filter = filter.trimmed();

    updateResults();
}

void BracketedWindow::stopMapper()
{
    	if(d_xpathMapper->isRunning()) {
    		d_xpathMapper->cancel();
    		d_xpathMapper->wait();
    	}
}

void BracketedWindow::updateResults()
{
    if(!d_corpusReader || d_filter.isEmpty())
    	return;
    
    try {
        stopMapper();
    	
    	d_ui->resultsListWidget->clear();
    	
    	if(d_entryMap)
    		delete d_entryMap;
    	
    	QString xpathExpression = d_macrosModel->expand(d_filter);
    	
    	d_entryMap = new EntryMapAndTransform(d_corpusReader, d_sentenceTransformer, xpathExpression);
    	QObject::connect(d_entryMap, SIGNAL(sentenceFound(QString, QString)), this, SLOT(sentenceFound(QString, QString)));
    	
    	d_xpathMapper->start(d_corpusReader.data(), xpathExpression, d_entryMap);
    } catch(runtime_error &e) {
    	QMessageBox::critical(this, QString("Error reading corpus"),
    	    QString("Could not read corpus: %1\n\nCorpus data is probably corrupt.").arg(e.what()));
    }
    
    return;
}

void BracketedWindow::sentenceFound(QString file, QString sentence)
{
    QListWidgetItem *item(new QListWidgetItem(sentence, d_ui->resultsListWidget));
    item->setData(Qt::UserRole, file);
}

void BracketedWindow::applyValidityColor(QString const &)
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

void BracketedWindow::createActions()
{
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
    
	QObject::connect(d_ui->listDelegateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(listDelegateChanged(int)));
}

void BracketedWindow::entrySelected(QListWidgetItem *current, QListWidgetItem *)
{
    if (current == 0)
        return;
    
    emit currentEntryChanged(current->data(Qt::UserRole).toString());
    
    // Raises this window again when using cursor keys after using\
    // [enter] to raise the main window.
    raise();
}

void BracketedWindow::entryActivated(QListWidgetItem *item)
{
    emit entryActivated();
}

void BracketedWindow::filterChanged()
{
    setFilter(d_ui->filterLineEdit->text());
}

void BracketedWindow::addListDelegate(QString const &name, QStyledItemDelegate*(*factory)())
{
	d_ui->listDelegateComboBox->addItem(name, d_listDelegateFactories.size());
	d_listDelegateFactories.append(factory);
}

void BracketedWindow::listDelegateChanged(int index)
{
	int delegateIndex = d_ui->listDelegateComboBox->itemData(index, Qt::UserRole).toInt();
	
	if (delegateIndex >= d_listDelegateFactories.size())
	{
		qWarning() << QString("Trying to select a list delegate (%1) beyond the boundary "
							  "of the d_listDelegateFactories list (%2)")
							 .arg(delegateIndex).arg(d_listDelegateFactories.size());
		return;
	}
	
	QAbstractItemDelegate* prevItemDelegate = d_ui->resultsListWidget->itemDelegate();
	d_ui->resultsListWidget->setItemDelegate(d_listDelegateFactories[delegateIndex]());
	delete prevItemDelegate;
}

void BracketedWindow::initListDelegates()
{
    addListDelegate("Complete sentence", &BracketedWindow::colorDelegateFactory);
	addListDelegate("Only matches", &BracketedWindow::visibilityDelegateFactory);
	addListDelegate("Keyword in Context", &BracketedWindow::keywordInContextDelegateFactory);	
}

void BracketedWindow::initSentenceTransformer()
{
    // Read stylesheet.
    QFile xslFile(":/stylesheets/bracketed-sentence.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QTextStream xslStream(&xslFile);
    QString xsl(xslStream.readAll());
    d_sentenceTransformer = QSharedPointer<XSLTransformer>(new XSLTransformer(xsl));
}

void BracketedWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
      stopMapper();
    else
      QWidget::keyPressEvent(event);
}

void BracketedWindow::mapperStarted(int totalEntries)
{
    d_ui->filterProgressBar->setMinimum(0);
    d_ui->filterProgressBar->setMaximum(totalEntries);
    d_ui->filterProgressBar->setValue(0);
    d_ui->filterProgressBar->setVisible(true);
}

void BracketedWindow::mapperStopped(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setVisible(false);
}

void BracketedWindow::mapperProgressed(int processedEntries, int totalEntries)
{
    d_ui->filterProgressBar->setValue(processedEntries);
}


void BracketedWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void BracketedWindow::readSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry.
    QPoint pos = settings.value("filter_pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("filter_size", QSize(350, 400)).toSize();
	resize(size);
	move(pos);
	
	// restore last selected display method
	int delegateIndex = settings.value("filter_list_delegate", 0).toInt();
	listDelegateChanged(delegateIndex);
	d_ui->listDelegateComboBox->setCurrentIndex(delegateIndex);
}

void BracketedWindow::writeSettings()
{
    QSettings settings("RUG", "Dact");

    // Window geometry
    settings.setValue("filter_pos", pos());
    settings.setValue("filter_size", size());
	
	// display method
	settings.setValue("filter_list_delegate", d_ui->listDelegateComboBox->currentIndex());
}

QStyledItemDelegate* BracketedWindow::colorDelegateFactory()
{
	return new BracketedColorDelegate();
}

QStyledItemDelegate* BracketedWindow::visibilityDelegateFactory()
{
	return new BracketedVisibilityDelegate();
}

QStyledItemDelegate* BracketedWindow::keywordInContextDelegateFactory()
{
	return new BracketedKeywordInContextDelegate();
}

EntryMapAndTransform::EntryMapAndTransform(QSharedPointer<alpinocorpus::CorpusReader> corpusReader, QSharedPointer<XSLTransformer> xslTransformer, QString const &query) :
    d_corpusReader(corpusReader),
    d_xslTransformer(xslTransformer),
    d_query(query)
{}

void EntryMapAndTransform::operator()(QString const &entry, xmlXPathObjectPtr xpathObj)
{
    if (xpathObj->nodesetval && xpathObj->nodesetval->nodeNr > 0)
    {
    	emit sentenceFound(entry, transform(entry));
    }
}

QString EntryMapAndTransform::transform(QString const &file)
{
    // Read XML data.
    if (d_corpusReader.isNull())
        throw runtime_error("EntryMapAndTransform::transform CorpusReader not available");
    
    QString xml = d_corpusReader->read(file);
    
    if (xml.size() == 0)
        throw runtime_error("EntryMapAndTransform::transform: empty XML data!");
    
    // Parameters
    QString valStr = d_query.trimmed().isEmpty()
    	? "'/..'"
    	: QString("'") + d_query + QString("'");
    
    QHash<QString, QString> params;
    params["expr"] = valStr;
    
    return d_xslTransformer->transform(xml, params).trimmed();	
}
