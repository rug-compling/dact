#include <algorithm>

#include <QtDebug>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

#include <SecEdge.hh>
#include <TreeNode.hh>

QRectF SecEdge::boundingRect() const
{
    QPointF from(d_from->scenePos() + QPointF(d_from->boundingRect().width() / 2, 0));
    QPointF to(d_to->scenePos() + QPointF(d_to->boundingRect().width() / 2, 0));

    QPointF topLeft(std::min(from.x(), to.x()), std::min(from.y(), to.y()));
    topLeft += QPointF(0, -30);

    QPointF bottomRight(std::max(from.x(), to.x()), std::max(from.y(), to.y()));


    return QRectF(topLeft, bottomRight);
}

void SecEdge::layout()
{
}

void SecEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen(Qt::darkMagenta, 3);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    QPointF from(d_from->scenePos() + QPointF(d_from->boundingRect().width() / 2, 0));
    QPointF to(d_to->scenePos() + QPointF(d_to->boundingRect().width() / 2, 0));

    qreal width = std::max(from.x(), to.x()) - std::min(from.x(), to.x());
    qreal controlXDist = width / 2.;

    int xOp = 1;
    if (from.x() > to.x())
        xOp = -1;

    QPointF control(from.x() + xOp * controlXDist, std::min(from.y(), to.y()) - 30);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.moveTo(from);
    path.quadTo(control, to);
    painter->drawPath(path);

    // Useful for debugging, to get control points
    //
    // painter->drawText(control, "c");
}
