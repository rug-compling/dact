#ifndef DACT_EDGE_HH
#define DACT_EDGE_HH

#include <QGraphicsItem>

class TreeNode;

class Edge : public QGraphicsItem
{
public:
    Edge(QGraphicsItem *parent = 0) : QGraphicsItem(parent) {}
    QRectF boundingRect() const;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget);
    void setChild(TreeNode *parent);
    void setParent(TreeNode *parent);
private:
    Edge(Edge const &other);

    TreeNode *d_parent;
    TreeNode *d_child;
};

inline void Edge::setChild(TreeNode *child)
{
    d_child = child;
}

inline void Edge::setParent(TreeNode *parent)
{
    d_parent = parent;
}

#endif // DACT_EDGE_HH
