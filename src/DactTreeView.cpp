#include <QEvent>
#include <QScrollBar>
#include <QTouchEvent>
#include <QWheelEvent>

#include "DactTreeView.hh"
#include "DactTreeScene.hh"
#include "TreeNode.hh"

DactTreeView::DactTreeView(QWidget* parent)
:
    QGraphicsView(parent)
{
    QFile stylesheet(":/stylesheets/tree.xsl");
    d_transformer = QSharedPointer<XSLTransformer>(new XSLTransformer(stylesheet));
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    setDragMode(ScrollHandDrag);
}

void DactTreeView::showTree(QString const &xml)
{
    d_xml = xml;
    QString tree_xml = transformParseToTree(xml);
    
    DactTreeScene *scene = new DactTreeScene(this);
    scene->parseTree(tree_xml);
    setScene(scene);
    d_scaleFactor = transform().m11();
}

QString DactTreeView::transformParseToTree(QString const &xml) const
{   
    QHash<QString, QString> params;
    return d_transformer->transform(xml, params);
}

void DactTreeView::setScene(DactTreeScene *scene)
{
    QGraphicsView::setScene(scene);
    emit sceneChanged(scene);
}

DactTreeScene* DactTreeView::scene() const
{
    return reinterpret_cast<DactTreeScene*>(QGraphicsView::scene());
}

void DactTreeView::fitTree()
{
    if (scene()) {
        fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        d_scaleFactor = transform().m11();
    }
}

void DactTreeView::focusTreeNode(int direction)
{
    if (scene())
    {
        QList<TreeNode*> nodes = scene()->nodes();
        int offset = 0;
        int nodeCount = nodes.length();
        
        // Find the currently focussed node
        for (int i = 0; i < nodeCount; ++i)
        {
            if (nodes[i]->hasFocus())
            {
                offset = i + direction;
                break;
            }
        }
    
        // then find the next node that's active
        for (int i = 0; i < nodeCount; ++i)
        {
            // nodeCount + offset + direction * i is positive for [0..nodeCount]
            // whichever the direction, and modulo nodeCount makes shure it wraps around.
            TreeNode &node = *nodes[(nodeCount + offset + direction * i) % nodeCount];
        
            if (node.isActive())
            {
                node.setFocus();
                
                // cause a selectionChanged signal
                node.setSelected(false);
                node.setSelected(true);
                
                // reset the matrix to undo the scale operation done by fitTree.
                // I don't like this yet, because it always resets the zoom.
                //d_ui->treeGraphicsView->setMatrix(QMatrix());
            
                // move the focus to the center of the focussed leaf
                centerOn(node.mapToScene(node.leafRect().center()));
                break;
            }
        }
    }
}

void DactTreeView::zoomIn()
{
    d_scaleFactor *= ZOOM_IN_FACTOR;
    setTransform(QTransform().scale(d_scaleFactor, d_scaleFactor));
}

void DactTreeView::zoomOut()
{
    d_scaleFactor *= ZOOM_OUT_FACTOR;
    setTransform(QTransform().scale(d_scaleFactor, d_scaleFactor));
}

void DactTreeView::resetZoom()
{
    // Scale back to 1:1
    QRectF unity = matrix().mapRect(QRectF(0, 0, 1, 1));
    scale(1 / unity.width(), 1 / unity.height());
    d_scaleFactor = transform().m11();
}

void DactTreeView::focusNextTreeNode()
{
    focusTreeNode(1);
}

void DactTreeView::focusPreviousTreeNode()
{
    focusTreeNode(-1);
}

void DactTreeView::fitInView(QRectF const &rect, Qt::AspectRatioMode aspectRatioMode)
{
    QGraphicsView::fitInView(rect, aspectRatioMode);
    
    // Yeah, it fits, but that doesn't mean in needs to blow up in your face.
    // So, if it is scaled beyond its original size, reset the scale to 1.0
    if (matrix().m11() > 1.0 || matrix().m22() > 1.0)
        resetZoom();
}

bool DactTreeView::viewportEvent(QEvent *event)
{
    // Based on touch-pinchzoom example.
    switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        {
            QTouchEvent *touchEvent = reinterpret_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> points = touchEvent-> touchPoints();
            
            if (points.count() == 2) {
                QTouchEvent::TouchPoint &touchPoint0 = points.first();
                QTouchEvent::TouchPoint &touchPoint1 = points.last();
                qreal curScaleFactor = QLineF(touchPoint0.pos(), touchPoint1.pos()).length() /
                    QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
                if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                    // if one of the fingers is released, remember the current scale
                    // factor so that adding another finger later will continue zooming
                    // by adding new scale factor to the existing remembered value.
                    d_scaleFactor *= curScaleFactor;
                    curScaleFactor = 1;
                }
                
                setTransform(QTransform().scale(d_scaleFactor * curScaleFactor,
                                                d_scaleFactor * curScaleFactor));
            }
            return true;
        }
        default:
            break;
    }
    
    return QGraphicsView::viewportEvent(event);
}

void DactTreeView::wheelEvent(QWheelEvent *event)
{
    // If the control modifier key isn't pressed, handle this wheel
    // even as any other event: pan the scene... probably.
    if (event->modifiers() != Qt::ControlModifier)
        return QGraphicsView::wheelEvent(event);
    
    // if it is not vertical scrolling, we ignore it.
    if (event->orientation() != Qt::Vertical)
        return;
    
    // according to the QWheelEvent docs
    qreal degree = event->delta() / 8; 
    
    // zoomFactor has to be something around 1.
    // NEVER BE ZERO OR YOU WILL RUIN THE MATRIX
    // Todo: adjust this formula. It works ok on my magic trackpad, but
    // I haven't tested it with a real mouse or anything.
    // Todo: maybe use the ZOOM_IN_FACTOR and ZOOM_OUT_FACTOR constants
    //qreal zoomFactor = 1.0 + degree / 360;
    
    qreal zoomFactor = 1.0 + degree / 180;
    
    if (zoomFactor > 2.0)
        zoomFactor = 2.0;
    else if (zoomFactor <= 0.0)
        zoomFactor = 0.1;
    
    // TODO: some kind of limit which prevents zooming in or out too far.
    // because being able to do that is silly.
    
    // Zoom in on position of the mouse (like Google Maps)
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    d_scaleFactor *= zoomFactor;
    setTransform(QTransform().scale(d_scaleFactor, d_scaleFactor));
    
    event->accept();
}
