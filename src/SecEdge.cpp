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
    QPainterPath edgePath = paintEdge(painter, option, widget);
    paintLabel(painter, option, widget, edgePath);

}

QPainterPath SecEdge::paintEdge(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

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

    painter->restore();

    return path;
}

void SecEdge::paintLabel(QPainter *painter, QStyleOptionGraphicsItem const *option,
    QWidget *widget, QPainterPath edgePath)
{
    painter->save();
    QPen labelPen(Qt::darkMagenta, 2);
    painter->setPen(labelPen);

    QFontMetricsF fontMetrics = QFontMetricsF(painter->fontMetrics());
    QRectF labelRect = fontMetrics.boundingRect(d_label);
    QRectF boxRect = labelRect;
    boxRect.setWidth(boxRect.width() + 6);
    boxRect.setHeight(boxRect.height() + 6);

    labelRect.moveCenter(edgePath.pointAtPercent(0.5));
    boxRect.moveCenter(edgePath.pointAtPercent(0.5));

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(boxRect, Qt::white);
    painter->drawRoundedRect(boxRect, 4, 2);
    painter->drawText(labelRect, d_label);

    painter->restore();
}
