#ifndef DACTTREESCENE_H
#define DACTTREESCENE_H

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

class DactTreeScene : public QGraphicsScene
{
	Q_OBJECT
public:	
	DactTreeScene(QObject *parent = 0);
	void parseTree(QString const &xml);
	QList<DactTreeNode*> const &nodes();
	DactTreeNode* rootNode();
private:
	void layout(QPointF pos, DactTreeNode* node);
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
	QPainterPath shape() const;
private:
	void paintLabels(QPainter *painter, QRectF const &leaf);
	void paintEdges(QPainter *painter, QRectF const &leaf);
	QFont font() const;
	QHash<QString,QString> d_attributes;
	QList<DactTreeNode*> d_childNodes;
	QList<QString> d_labels;
	qreal d_spaceBetweenNodes;
	qreal d_spaceBetweenLayers;
	qreal d_leafMinimumWidth;
	qreal d_leafMinimumHeight;
	qreal d_leafPadding;
};

#endif
