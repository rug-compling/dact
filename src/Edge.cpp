#include <QPainter>
#include <QPen>

#include <Edge.hh>
#include <TreeNode.hh>

QRectF Edge::boundingRect() const
{
    if (d_parent == 0 || d_child == 0)
        return QRectF();

    //QPointF origin(leaf.x() + leaf.width() / 2, leaf.y() + leaf.height());
    QRectF leaf = d_parent->leafRect();
    QPointF parentPos = d_parent->scenePos();
    //QPointF origin(leaf.x() + leaf.width() / 2, leaf.y() + leaf.height());
    QPointF origin(d_parent->scenePos() + QPointF(d_parent->branchBoundingRect().width() / 2, leaf.height()));
    QPointF target(d_child->scenePos() + QPointF(d_child->branchBoundingRect().width() / 2, 0));

    QPointF topLeft(std::min(origin.x(), target.x()), std::min(origin.y(), target.y()));
    QPointF bottomRight(std::max(origin.x(), target.x()), std::max(origin.y(), target.y()));

    return QRectF(topLeft, bottomRight);
}

void Edge::paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget)
{
    if (d_parent == 0 || d_child == 0)
        return;

    QRectF leaf = d_parent->leafRect();
    //QPointF origin(leaf.x() + leaf.width() / 2, leaf.y() + leaf.height());
    QPointF origin(d_parent->scenePos() + QPointF(d_parent->branchBoundingRect().width() / 2, leaf.height()));
    QPointF target(d_child->scenePos() + QPointF(d_child->branchBoundingRect().width() / 2, 0));

    painter->save();

    QPen edgePen(Qt::black, 1);
    painter->setPen(edgePen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawLine(origin, target);

    painter->restore();
}
