#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>

#include "TreeNode.hh"
#include "PopupItem.hh"

static const QPointF popupCursorOffset(0, -16);

TreeNode::TreeNode(QGraphicsItem *parent) :
    QGraphicsItem(parent),
    d_active(false),
    d_attributes(),
    d_parentNode(0),
    d_childNodes(),
    d_labels(),
    d_popupItem(0),
    d_spaceBetweenLayers(40),
    d_spaceBetweenNodes(10),
    d_leafMinimumWidth(30),
    d_leafMinimumHeight(30),
    d_leafPadding(10)
{
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
}

bool TreeNode::isLeaf() const
{
    return d_childNodes.length() == 0;
}

void TreeNode::appendChild(TreeNode *child)
{
    child->setParentNode(this);
    d_childNodes.append(child);
}

void TreeNode::appendLabel(QString const &label)
{
    d_labels.append(label);
}

TreeNode *TreeNode::parentNode()
{
    return d_parentNode;
}

void TreeNode::setParentNode(TreeNode *parent)
{
    d_parentNode = parent;
    setParentItem(parent);
}

QList<TreeNode*> TreeNode::children()
{
    return d_childNodes;
}

void TreeNode::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // This doesn't seem to work well, but is not necessary yet, since
    // we only have popup lines on leaf nodes:
    // 
    //QList<QGraphicsItem *> items(scene()->items(event->scenePos(),
    //  Qt::IntersectsItemBoundingRect, Qt::AscendingOrder));
    //if (items.contains(this) && d_popupItem) {
    if (d_popupItem)
    {
        d_popupItem->setPos(event->scenePos());
        d_popupItem->setVisible(true);
        event->accept();
    }
}

void TreeNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d_popupItem && d_popupItem->isVisible()) {

        d_popupItem->setVisible(false);
        event->accept();
    }
}

void TreeNode::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d_popupItem && d_popupItem->isVisible()) {
        if (event->screenPos() == event->lastScreenPos())
        {
            d_popupItem->setPos(event->scenePos() + popupCursorOffset / viewScale());
        }
        else
            d_popupItem->setPos(event->scenePos() + popupCursorOffset / viewScale());
        event->accept();
    }
}

void TreeNode::setAttribute(QString const &name, QString const &value)
{
    d_attributes[name] = value;
}

QString TreeNode::asString(QString const &indent) const
{
    QString dump = indent + "[node labels:";
    
    foreach (QString const &line, d_labels)
    {
        dump += " \"" + line + "\"";
    }
    
    dump += "\n" + indent + QString("      attributes: (%1)").arg(d_attributes.size());
    
    for (QHash<QString, QString>::const_iterator i = d_attributes.constBegin();
        i != d_attributes.constEnd(); ++i)
    {
        dump += " <" + i.key() + ":" + i.value() + ">";
    }
    
    dump += "\n" + indent + QString("      children: (%1)\n").arg(d_childNodes.length());
    
    foreach (TreeNode const *child, d_childNodes)
    {
        dump += child->asString(indent + "\t");
    }
    
    dump += indent + "]\n";
    
    return dump;
}

QRectF TreeNode::boundingRect() const
{
    /*QRectF branch = branchBoundingRect();
    QRectF leaf = QRectF(QPointF(), leafSize());
    
    leaf.translate((branch.width() - leaf.width()) / 2, 0);
    
    return leaf;*/
    return branchBoundingRect();
}

QPainterPath TreeNode::shape() const
{
    QPainterPath path;
    path.addEllipse(leafRect());
    return path;
}

QRectF TreeNode::branchBoundingRect() const
{
    return childrenBoundingRect() | leafBoundingRect();
}

QRectF TreeNode::leafBoundingRect() const
{
    QRectF leaf(leafRect());
    leaf.setWidth(leaf.width() + 10);
    leaf.setHeight(leaf.height() + 10);
    leaf.translate(-5, -5);
    return leaf;
}

QRectF TreeNode::leafRect() const
{
    QSizeF branch(branchSize());
    QRectF leaf(QPointF(0, 0), leafSize());
    leaf.translate(std::max((branch.width() - leaf.width()) / 2, qreal(0)), 0);
    return leaf;
}

QSizeF TreeNode::leafSize() const
{
    QFontMetricsF metrics(font());
    QSizeF leaf(d_leafMinimumWidth, d_leafMinimumHeight);
    
    foreach (QString const &label, d_labels)
    {
        qreal labelWidth = metrics.width(label) + 2 * d_leafPadding;
        if (labelWidth > leaf.width())
            leaf.setWidth(labelWidth);
    }
    
    qreal labelsHeight = d_labels.size() * metrics.lineSpacing() + 2 * d_leafPadding;
    if (labelsHeight > leaf.height())
        leaf.setHeight(labelsHeight);
    
    return leaf;
}

QSizeF TreeNode::branchSize() const
{
    return childrenBoundingRect().size();
}

void TreeNode::layout()
{
    qreal left = 0;
    QSizeF leaf = leafSize();

    if (d_popupItem)
        d_popupItem->setPos(scenePos());
    
    foreach (TreeNode *child, d_childNodes)
    {
        child->setPos(left, leaf.height() + d_spaceBetweenLayers);
        child->layout();
        
        left += child->branchBoundingRect().width() + d_spaceBetweenNodes;
    }
}

QFont TreeNode::font() const
{
    return QFont("verdana", 12);
}

void TreeNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QSettings settings;
    settings.beginGroup("Tree");

    QColor activeNodeForeground(settings.value("activeNodeForeground",
        QColor(Qt::white)).value<QColor>());
    QColor activeNodeBackground(settings.value("activeNodeBackground",
        QColor(Qt::darkGreen)).value<QColor>());
    
    QRectF branch(branchBoundingRect());
    QRectF leaf(leafRect());
    
    // paint the edges first, so I can paint the leaf right over it
    // which is needed because the edges sometimes leave some anti-alias
    // pixels inside the leaf's box.
    
    painter->setRenderHint(QPainter::Antialiasing, true);

    paintEdges(painter, leaf);
    
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor background = isActive() ? activeNodeBackground : QColor(Qt::white);
    
    if (isSelected())
        background = background.darker(125);
    
    painter->fillRect(leaf, background);
    
    QPen borderPen(Qt::black, hasFocus() ? 3 : 1);
    painter->setPen(borderPen);
    painter->drawRect(leaf);
    
    painter->setPen(isActive() ? activeNodeForeground : QColor(Qt::black));
    paintLabels(painter, leaf);
}

void TreeNode::paintLabels(QPainter *painter, QRectF const &leaf)
{
    // @TODO currently I draw all the labels by just concatenating them together
    // but if I could implement drawing each line separately, it would be easy as pie
    // to implement colors, weights etc for each line separately.
    QString labels;
    foreach (QString const &label, d_labels)
        labels += QString("%1\n").arg(label);
    
    QRectF textBox(leaf);
    
    textBox.setWidth(leaf.width() - 2*d_leafPadding);
    textBox.setHeight(leaf.height() - 2*d_leafPadding);
    textBox.translate(d_leafPadding, d_leafPadding);

    // You can't be serious... Yes you can.
    double appDpi = qt_defaultDpi();
    double ratio = appDpi / painter->device()->logicalDpiY();
    QFont painterFont(font());
    painterFont.setPointSizeF(painterFont.pointSize() * ratio);
    painter->setFont(painterFont);
    painter->drawText(textBox, Qt::AlignCenter, labels);
}

void TreeNode::paintEdges(QPainter *painter, QRectF const &leaf)
{
    QPen edgePen(Qt::black, 1);
    painter->setPen(edgePen);

    QPointF origin(leaf.x() + leaf.width() / 2, leaf.y() + leaf.height());
    
    foreach (TreeNode const *child, d_childNodes)
    {
        // child's top center position, where the leaf should be found
        QPointF target(child->pos() + QPointF(child->branchBoundingRect().width() / 2, 0));
        
        painter->drawLine(origin, target);
    }
}

qreal TreeNode::viewScale() const
{
    if (!scene() || scene()->views().isEmpty())
        return 1.0;
    
    return scene()->views().first()->transform().m11();
}
