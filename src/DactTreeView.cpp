#include <QWheelEvent>

#include "DactTreeView.hh"
#include "DactTreeScene.hh"

DactTreeView::DactTreeView(QWidget* parent)
:
    QGraphicsView(parent)
{
    QFile stylesheet(":/stylesheets/tree.xsl");
    d_transformer = QSharedPointer<XSLTransformer>(new XSLTransformer(stylesheet));
}

void DactTreeView::showTree(QString const &xml)
{
    d_xml = xml;
    QString tree_xml = transformParseToTree(xml);
    
    DactTreeScene *scene = new DactTreeScene(this);
    scene->parseTree(tree_xml);
    setScene(scene);
}

QString DactTreeView::transformParseToTree(QString const &xml) const
{   
    QHash<QString, QString> params;
    params["expr"] = d_query.trimmed().isEmpty()
        ? "'/..'"
        : QString("'%1'").arg(d_query);
    
    return d_transformer->transform(xml, params);
}

void DactTreeView::setHighlightQuery(QString const &query)
{
    d_query = query;
    
    // Only update the tree if there is a tree already
    if (scene())
        showTree(d_xml);
}

QString const &DactTreeView::highlightQuery() const
{
    return d_query;
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
    if (!scene())
        return;
    
    if (scene()->rootNode() != 0)
        fitInView(scene()->rootNode()->boundingRect(), Qt::KeepAspectRatio);
}

void DactTreeView::focusTreeNode(int direction)
{
    if (scene())
    {
        QList<DactTreeNode*> nodes = scene()->nodes();
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
            DactTreeNode &node = *nodes[(nodeCount + offset + direction * i) % nodeCount];
        
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
    scale(ZOOM_IN_FACTOR, ZOOM_IN_FACTOR);
}

void DactTreeView::zoomOut()
{
    scale(ZOOM_OUT_FACTOR, ZOOM_OUT_FACTOR);
}

void DactTreeView::resetZoom()
{
    setMatrix(QMatrix());
}

void DactTreeView::focusNextTreeNode()
{
    focusTreeNode(1);
}

void DactTreeView::focusPreviousTreeNode()
{
    focusTreeNode(-1);
}

void DactTreeView::wheelEvent(QWheelEvent * event)
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
    scale(zoomFactor, zoomFactor);
    
    event->accept();
}
