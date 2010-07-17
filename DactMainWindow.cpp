#include <QFile>
#include <QGraphicsSvgItem>
#include <QGraphicsScene>
#include <QList>
#include <QListWidgetItem>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>

#include <sstream>

#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>
#include <xalanc/XSLT/XSLTInputSource.hpp>
#include <xalanc/XSLT/XSLTResultTarget.hpp>

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


    if (qApp->arguments().size() > 1)
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
    QStringList args(qApp->arguments());

    for (QStringList::const_iterator iter = args.constBegin() + 1;
            iter != args.constEnd(); ++iter)
        new QListWidgetItem(*iter, d_ui->fileListWidget);
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

    XalanTransformer transformer;
    QFile xslFile(":/stylesheets/dt2tree.xsl");
    xslFile.open(QIODevice::ReadOnly);
    QByteArray xslData(xslFile.readAll());
    XSLTInputSource xslIn("dt2tree.xsl");
    XSLTInputSource xmlIn(xmlFilename.toUtf8().constData());
    std::ostringstream svgStream;
    XSLTResultTarget svgOut(svgStream);
    int r = transformer.transform(xmlIn, xslIn, svgStream);
    QByteArray svg(svgStream.str().c_str());

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
