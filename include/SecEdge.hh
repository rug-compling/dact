#ifndef SECEDGE_HH
#define SECEDGE_HH

#include <QGraphicsItem>

class TreeNode;

class SecEdge : public QGraphicsItem
{
public:
    SecEdge(QGraphicsItem *parent = 0) :
      QGraphicsItem(parent), d_from(0), d_to(0) {}
    QRectF boundingRect() const;
    void layout();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setFrom(TreeNode const *from);
    void setLabel(QString label);
    void setTo(TreeNode const *to);
private:
    /**
     * Paint the secondary edge.
     */
    QPainterPath paintEdge(QPainter *painter,
        QStyleOptionGraphicsItem const *option,
        QWidget *widget);

    /**
     * Paint the label.
     */
    void paintLabel(QPainter *painter,
        QStyleOptionGraphicsItem const *option,
        QWidget *widget, QPainterPath edgePath);

    TreeNode const *d_from;
    TreeNode const *d_to;
    QString d_label;
};

inline void SecEdge::setFrom(TreeNode const *from)
{
    d_from = from;
}

inline void SecEdge::setLabel(QString label)
{
    d_label = label;
}

inline void SecEdge::setTo(TreeNode const *to)
{
    d_to = to;
}

#endif // SECEDGE_HH
