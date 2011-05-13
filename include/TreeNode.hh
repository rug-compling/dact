#ifndef TREENODE_HH
#define TREENODE_HH

#include <QGraphicsItem>
#include <QList>

class PopupItem;

class TreeNode : public QGraphicsItem
{
public:
    TreeNode(QGraphicsItem *parent = 0);
    void setAttribute(QString const &name, QString const &value);
    void appendChild(TreeNode *node);
    void appendLabel(QString const &label);
    void appendPopupLine(QString const &line);
    QString asString(QString const &indent = "") const; // debugging purpuse
    QRectF boundingRect() const;
    QRectF leafBoundingRect() const;
    QRectF leafRect() const;
    QRectF branchBoundingRect() const;
    QSizeF leafSize() const;
    QSizeF branchSize() const;
    QList<TreeNode*> children();
    bool isLeaf() const;
    bool isActive() const;
    void layout();
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget);
    QList<QString> const &popupLines() const;
    void setPopupItem(PopupItem *item);
    QPainterPath shape() const;
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
private:
    void paintLabels(QPainter *painter, QRectF const &leaf);
    void paintEdges(QPainter *painter, QRectF const &leaf);
    QFont font() const;
    QHash<QString,QString> d_attributes;
    QList<TreeNode*> d_childNodes;
    QList<QString> d_labels;
    QList<QString> d_popupLines;
    PopupItem *d_popupItem;
    qreal d_spaceBetweenLayers;
    qreal d_spaceBetweenNodes;
    qreal d_leafMinimumWidth;
    qreal d_leafMinimumHeight;
    qreal d_leafPadding;
};

inline QList<QString> const &TreeNode::popupLines() const
{
    return d_popupLines;
}

inline void TreeNode::appendPopupLine(QString const &line)
{
    d_popupLines.append(line);
}

inline void TreeNode::setPopupItem(PopupItem *item)
{
    d_popupItem = item;
}


#endif