#include <QFile>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QList>
#include <QListWidgetItem>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>

#include <string>
#include <sstream>
#include <vector>

#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>
#include <xalanc/XSLT/XSLTInputSource.hpp>
#include <xalanc/XSLT/XSLTResultTarget.hpp>

#include <IndexedCorpus/ActCorpusReader.hh>

using namespace std;

XALAN_USING_XERCES(XMLPlatformUtils)
XALAN_USING_XALAN(XalanTransformer)
XALAN_USING_XALAN(XSLTInputSource)
XALAN_USING_XALAN(XSLTResultTarget)

#include "DactMainWindow.h"
#include "ui_DactMainWindow.h"

DactMainWindow::DactMainWindow(QWidget *parent) :
    QMainWindow(parent),
    d_ui(new Ui::DactMainWindow)
{
    XMLPlatformUtils::Initialize();
    XalanTransformer::initialize();

    d_ui->setupUi(this);


    if (qApp->arguments().size() == 2)
        addFiles();

    QObject::connect(d_ui->fileListWidget,
                     SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
                     this,
                     SLOT(showTree(QListWidgetItem *, QListWidgetItem *)));
    QObject::connect(d_ui->zoomInAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomIn(bool)));
    QObject::connect(d_ui->zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(treeZoomOut(bool)));
}

DactMainWindow::~DactMainWindow()
{
    delete d_ui;
}

void DactMainWindow::addFiles()
{
    indexedcorpus::ActCorpusReader corpusReader;
    vector<string> entries = corpusReader.entries(qApp->arguments().at(1).toUtf8().constData());

    for (vector<string>::const_iterator iter = entries.begin();
         iter !=entries.end(); ++iter)
    {
        new QListWidgetItem(QString(iter->c_str()), d_ui->fileListWidget);
    }
}

void DactMainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DactMainWindow::showTree(QListWidgetItem *current, QListWidgetItem *)
{
    QString xmlFilename = current->text();

    // Read stylesheet.
    QFile xslFile(":/stylesheets/dt2tree.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QByteArray xslData(xslFile.readAll());
    std::istringstream xslStream(xslData.constData());
    XSLTInputSource xslIn(xslStream);

    // Read XML data.
    indexedcorpus::ActCorpusReader corpusReader;
    vector<unsigned char> xmlData = corpusReader.getData(xmlFilename.toUtf8().constData());
    xmlData.push_back(0);
    istringstream xmlStream(reinterpret_cast<char const *>(&xmlData[0]));
    XSLTInputSource xmlIn(xmlStream);
    std::ostringstream svgStream;
    XSLTResultTarget svgOut(svgStream);

    // Transform to SVG.
    XalanTransformer transformer;
    int r = transformer.transform(xmlIn, xslIn, svgStream);
    QByteArray svg(svgStream.str().c_str());

    // Render SVG.
    QSvgRenderer *renderer = new QSvgRenderer(svg);
    QGraphicsScene *scene = new QGraphicsScene(d_ui->treeGraphicsView);
    QGraphicsSvgItem *item = new QGraphicsSvgItem;
    item->setSharedRenderer(renderer);
    scene->addItem(item);
    d_ui->treeGraphicsView->setScene(scene);
    //d_ui->treeGraphicsView->fitInView(item, Qt::KeepAspectRatio);
    //d_ui->treeGraphicsView->fitInView(scene->sceneRect());
}

void DactMainWindow::treeZoomIn(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_IN_FACTOR, ZOOM_IN_FACTOR);
}

void DactMainWindow::treeZoomOut(bool)
{
    d_ui->treeGraphicsView->scale(ZOOM_OUT_FACTOR, ZOOM_OUT_FACTOR);
}
