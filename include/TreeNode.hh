#ifndef TREENODE_HH
#define TREENODE_HH

#include <QGraphicsItem>
#include <QList>
#include <QTextDocument>

class PopupItem;

class TreeNode : public QGraphicsItem
{
public:
    TreeNode(QGraphicsItem *parent = 0);
    QHash<QString, QString> const &attributes() const;
    void setAttribute(QString const &name, QString const &value);
    void appendChild(TreeNode *node);
    void setLabel(QString const &label);
    void setTooltip(QString const &tooltip);
    QString const &tooltip() const;
    QString asString(QString const &indent = "") const; // debugging purpuse
    QRectF boundingRect() const;
    QRectF leafBoundingRect() const;
    QRectF leafRect() const;
    QRectF branchBoundingRect() const;
    QSizeF leafSize() const;
    QSizeF branchSize() const;
    TreeNode *parentNode();
    QList<TreeNode*> children();
    bool isLeaf() const;
    bool isActive() const;
    void layout();
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget);
    QList<QString> const &popupLines() const;
    void setActive(bool active);
    void setPopupItem(PopupItem *item);
    QPainterPath shape() const;
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
    void setParentNode(TreeNode *node);
private:
    void paintLabel(QPainter *painter, QRectF const &leaf);
    QFont font() const;
    qreal viewScale() const;
    bool d_active;
    QHash<QString,QString> d_attributes;
    TreeNode *d_parentNode;
    QList<TreeNode*> d_childNodes;
    QTextDocument d_label;
    QString d_tooltip;
    PopupItem *d_popupItem;
    qreal d_spaceBetweenLayers;
    qreal d_spaceBetweenNodes;
    qreal d_leafMinimumWidth;
    qreal d_leafMinimumHeight;
    qreal d_leafPadding;
};

inline QHash<QString, QString> const &TreeNode::attributes() const
{
    return d_attributes;
}

inline void TreeNode::setActive(bool active)
{
    d_active = active;
}

inline bool TreeNode::isActive() const
{
    return d_active;
}

inline void TreeNode::setPopupItem(PopupItem *popupItem)
{
    d_popupItem = popupItem;
}

#endif


