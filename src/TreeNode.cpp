#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>
#include <QTextDocument>

#include <Edge.hh>
#include <TreeNode.hh>
#include <PopupItem.hh>

static const QPointF popupCursorOffset(0, -16);

TreeNode::TreeNode(QGraphicsItem *parent) :
    QGraphicsItem(parent),
    d_active(false),
    d_attributes(),
    d_parentNode(0),
    d_childNodes(),
    d_popupItem(0),
    d_spaceBetweenLayers(40),
    d_spaceBetweenNodes(10),
    d_leafMinimumWidth(30),
    d_leafMinimumHeight(30),
    d_leafPadding(10)
{
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    d_label.setDefaultFont(font());
    d_label.setDocumentMargin(d_leafPadding);
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

void TreeNode::setLabel(QString const &label)
{
    d_label.setHtml(label);
}

void TreeNode::setTooltip(QString const &tooltip)
{
    d_tooltip = tooltip;

    if (d_popupItem)
        d_popupItem->setContent(tooltip);
}

QString const &TreeNode::tooltip() const
{
    return d_tooltip;
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
    QString dump = indent + "[node label: \"" + d_label.toHtml() + "\"";
    
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
    return d_label.size();
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

    QColor activeNodeBorder(settings.value("activeNodeBorder",
        QColor(Qt::black)).value<QColor>());
    
    QRectF branch(branchBoundingRect());
    QRectF leaf(leafRect());
    
    // paint the edges first, so I can paint the leaf right over it
    // which is needed because the edges sometimes leave some anti-alias
    // pixels inside the leaf's box.
    
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor background(Qt::white);

    if (isActive())
        background = background.darker(120);
   
    QPen borderPen(isSelected() ?
        activeNodeBorder : QColor(Qt::black),
        isSelected() ? 3 : 1);

    //if (isActive())
    //  borderPen.setStyle(Qt::DotLine);
    //if (!isSelected() && !isActive())
    //    borderPen.setStyle(Qt::DashLine);
    
    painter->fillRect(leaf, background);
    painter->setPen(borderPen);
    painter->drawRect(leaf);
    
    paintLabel(painter, leaf);
}

void TreeNode::paintLabel(QPainter *painter, QRectF const &leaf)
{
    // Necessary for centred text.
    d_label.setTextWidth(leaf.width());
    
    painter->save();
    painter->translate(leaf.topLeft());
    d_label.drawContents(painter, QRectF(QPointF(0,0), leaf.size()));
    painter->restore();
}

qreal TreeNode::viewScale() const
{
    if (!scene() || scene()->views().isEmpty())
        return 1.0;
    
    return scene()->views().first()->transform().m11();
}
