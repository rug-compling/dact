#include <QDebug>
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
    if (scene())
        fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
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
                clearSelection();
                focusNode(node);
                
                // Move the focus to the center of the focussed leaf
                centerOn(node.mapToScene(node.leafRect().center()));
                break;
            }
        }
    }
}

void DactTreeView::focusNode(TreeNode &node)
{
    // Move focus to new node
    node.setFocus();
                
    // Cause a selectionChanged signal, used to update the inspector
    node.setSelected(true);
    
    // reset the matrix to undo the scale operation done by fitTree.
    // I don't like this yet, because it always resets the zoom.
    //d_ui->treeGraphicsView->setMatrix(QMatrix());

    ensureVisible(node.mapToScene(node.leafRect()).boundingRect());
}

void DactTreeView::clearSelection()
{
    foreach (TreeNode *node, scene()->nodes())
        node->setSelected(false);
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
    // Scale back to 1:1
    QRectF unity = matrix().mapRect(QRectF(0, 0, 1, 1));
    scale(1 / unity.width(), 1 / unity.height());
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

TreeNode *DactTreeView::focussedNode()
{
    QList<TreeNode*> nodes(scene()->nodes());

    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes[i]->hasFocus())
            return nodes[i];
    }

    return 0;
}

void DactTreeView::keyPressEvent(QKeyEvent * event)
{
    // Don't do anything if there is no scene
    if (!scene())
        return;

    // Append nodes to the selection when shift-key is down. Otherwise, move
    // the selection with the focus.
    bool shiftKeyPressed = (event->modifiers() & Qt::ShiftModifier) != 0;

    switch (event->key())
    {
        case Qt::Key_Up:
            // Only move up when there is a parent
            if (focussedNode() && focussedNode()->parentNode() != 0)
            {
                if (!shiftKeyPressed)
                    clearSelection();

                focusNode(*focussedNode()->parentNode());
            }
            break;
        
        case Qt::Key_Down:
            // Only move down to the first child if there are children
            if (focussedNode() && !focussedNode()->isLeaf())
            {
                if (!shiftKeyPressed)
                    clearSelection();

                focusNode(*focussedNode()->children()[0]);
            }
            break;

        case Qt::Key_Left:
        case Qt::Key_Right:
            // The top node has left and right siblings
            if (focussedNode() && focussedNode()->parentNode() != 0)
            {
                // Find currently focussed sibling, and move focus to sibling +1 or -1.
                QList<TreeNode*> siblings(focussedNode()->parentNode()->children());
                int currentIndex = siblings.indexOf(focussedNode());
                int direction = event->key() == Qt::Key_Left ? -1 : 1;
                int nextIndex = currentIndex + direction;
                if (nextIndex >= 0 && nextIndex < siblings.size())
                {
                    if (!shiftKeyPressed)
                        clearSelection();

                    focusNode(*siblings[nextIndex]);
                }
            }
            break;
        default:
            QGraphicsView::keyPressEvent(event);
            break;
    }
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
