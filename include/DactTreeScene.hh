#ifndef DACTTREESCENE_H
#define DACTTREESCENE_H

#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QHash>
#include <QList>
#include <QStack>

extern "C" {
    #include <libxml/parser.h>
    #include <libxml/xmlreader.h>
};

extern int qt_defaultDpi();

class DactTreeNode;
class PopupItem;

class DactTreeScene : public QGraphicsScene
{
    Q_OBJECT
public: 
    DactTreeScene(QObject *parent = 0);
    ~DactTreeScene();
    void parseTree(QString const &xml);
    QList<DactTreeNode*> const &nodes() const;
    QList<DactTreeNode*> activeNodes() const;
    DactTreeNode* rootNode();

private:
    void freeNodes();
    void layout(QPointF pos, DactTreeNode* node);
    void parseXML(QString const &xml);
    void processXMLNode(xmlTextReaderPtr &reader, QList<DactTreeNode*> &list, QStack<DactTreeNode*> &stack);
    void processXMLAttribute(xmlTextReaderPtr &reader, DactTreeNode* node);
    QString processXMLString(xmlChar* xmlValue) const;
    QList<DactTreeNode*> d_nodes;
};

class DactTreeNode : public QGraphicsItem
{
public:
    DactTreeNode(QGraphicsItem *parent = 0);
    void setAttribute(QString const &name, QString const &value);
    void appendChild(DactTreeNode *node);
    void appendLabel(QString const &label);
    void appendPopupLine(QString const &line);
    QString asString(QString const &indent = "") const; // debugging purpuse
    QRectF boundingRect() const;
    QRectF leafBoundingRect() const;
    QRectF leafRect() const;
    QRectF branchBoundingRect() const;
    QSizeF leafSize() const;
    QSizeF branchSize() const;
    QList<DactTreeNode*> children();
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
    QList<DactTreeNode*> d_childNodes;
    QList<QString> d_labels;
    QList<QString> d_popupLines;
    PopupItem *d_popupItem;
    qreal d_spaceBetweenLayers;
    qreal d_spaceBetweenNodes;
    qreal d_leafMinimumWidth;
    qreal d_leafMinimumHeight;
    qreal d_leafPadding;
};

class PopupItem : public QGraphicsItem
{
public:
    PopupItem(QGraphicsItem *parent = 0,
    QList<QString> lines = QList<QString>());
    QRectF boundingRect() const;
    QFont font() const;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget);
    QSizeF size() const;
    QRectF rect() const;
private:
    qreal viewScale() const;
    QList<QString> d_lines;
    qreal d_padding;
};

inline QList<QString> const &DactTreeNode::popupLines() const
{
    return d_popupLines;
}

inline void DactTreeNode::appendPopupLine(QString const &line)
{
    d_popupLines.append(line);
}

inline void DactTreeNode::setPopupItem(PopupItem *item)
{
    d_popupItem = item;
}

#endif
