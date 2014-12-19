#include <algorithm>

#include <QtDebug>
#include <QGraphicsScene>
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

bool SecEdge::collidesWithTreeNode(QRectF rect)
{
    QList<QGraphicsItem *> collidingItems = scene()->items(rect);
    foreach (QGraphicsItem *item, collidingItems)
    {
        TreeNode *node = dynamic_cast<TreeNode *>(item);
        if (node == 0)
            continue;

        QRectF leaf = node->leafRect();
        leaf.translate(node->scenePos());

        if (rect.intersects(leaf))
            return true;
    }

    return false;
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

    // Search left and right on the path until the label does not collide
    // with a tree node.
    bool success = false;
    for (qreal offset = 0.; offset < 0.5; offset += 0.1)
    {
        if (tryLabelOffset(edgePath, -offset, &labelRect, &boxRect)) {
            success = true;
            break;
        }

        if (tryLabelOffset(edgePath, offset, &labelRect, &boxRect)) {
            success = true;
            break;
        }
    }

    // If not successful, prefer the middle.
    if (!success) {
      qWarning() << "Cannot draw the label without collisions";
      tryLabelOffset(edgePath, 0., &labelRect, &boxRect);
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->fillRect(boxRect, Qt::white);
    painter->drawRoundedRect(boxRect, 4, 2);
    painter->drawText(labelRect, d_label);

    painter->restore();
}


bool SecEdge::tryLabelOffset(QPainterPath path, qreal offset,
    QRectF *labelRect, QRectF *boxRect)
{
    qreal percent = 0.5 + offset;

    labelRect->moveCenter(path.pointAtPercent(percent));
    boxRect->moveCenter(path.pointAtPercent(percent));

    return !collidesWithTreeNode(*boxRect);
}
