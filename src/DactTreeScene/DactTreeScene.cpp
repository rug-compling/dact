#include <algorithm>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QHash>
#include <QPainter>
#include <QStack>
#include <QtDebug>

#include "DactTreeScene.h"

DactTreeScene::DactTreeScene(QObject *parent) :
	QGraphicsScene(parent),
	d_nodes()
{
}

void DactTreeScene::parseTree(QString const &xml)
{
	QByteArray xmlData(xml.toUtf8());
	xmlTextReaderPtr reader = xmlReaderForMemory(xmlData.constData(), xmlData.size(),
								NULL, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOCDATA);
	
	foreach (DactTreeNode* node, d_nodes)
		delete node;
	
	d_nodes.clear(); // memory leak? Who's gonna free the nodes?
	QStack<DactTreeNode*> stack;
	
	while (true)
	{
		if (!xmlTextReaderRead(reader))
			break;
		
		processXMLNode(reader, d_nodes, stack);
	}
	
	if (stack.size() > 1)
		qWarning() << "Tree XML Read error: Stack not empty at the end";
	
	xmlFreeTextReader(reader);
	
	//qWarning() << d_nodes[0]->asString();
	
	addItem(d_nodes[0]);
	
	d_nodes[0]->layout();
}

QList<DactTreeNode*> const &DactTreeScene::nodes()
{
	return d_nodes;
}

QList<DactTreeNode*> DactTreeScene::activeNodes()
{
    QList<DactTreeNode*> nodes;
    
    foreach (DactTreeNode* node, d_nodes)
    {
        if (node->isActive())
            nodes.append(node);
    }
    
    return nodes;
}

DactTreeNode* DactTreeScene::rootNode()
{
  if (d_nodes.size() > 0)
	  return d_nodes[0];
  else
    return 0;
}

void DactTreeScene::processXMLNode(xmlTextReaderPtr &reader, QList<DactTreeNode*> &list, QStack<DactTreeNode*> &stack)
{
	QString name = processXMLString(xmlTextReaderName(reader));
	QString value = processXMLString(xmlTextReaderValue(reader));
	
	int type = xmlTextReaderNodeType(reader);
	
	switch (type)
	{
		case XML_READER_TYPE_ELEMENT:
			if (name == "node")
			{
				DactTreeNode* node = new DactTreeNode();
				list.append(node);
				
				// When this isn't the root-node, append it to it's parent.
				if (!stack.isEmpty())
					stack.top()->appendChild(node);
				
				if (!xmlTextReaderIsEmptyElement(reader))
					stack.push(node);
				else
					qWarning() << "Tree XML Read error: encountered an empty <node> element. Most of the time they "
								  "contain child <node> elements or <line> elements if they are leaf nodes. Interesting.";
				
				for (int attrIdx = 0, attrCount = xmlTextReaderAttributeCount(reader); attrIdx < attrCount; ++attrIdx)
				{
					xmlTextReaderMoveToAttributeNo(reader, attrIdx);
					processXMLAttribute(reader, node);
				}
			}
			else if (name == "line" || name == "hoverLine")
			{
				if (stack.isEmpty())
				{
					qWarning() << "Tree XML Read error: encountered line element while the stack is empty."
								  "Is this <line> element outside a <node> element? Skipping";
					break;
				}
				
				if (xmlTextReaderIsEmptyElement(reader))
				{
					// Skip empty elements, no need for them.
					break;
				}
				
				QString label;
				while (xmlTextReaderRead(reader))
				{
					type = xmlTextReaderNodeType(reader);
					
					if (type == XML_READER_TYPE_END_ELEMENT)
						break;
					
					if (type == XML_READER_TYPE_TEXT)
					{
						value = processXMLString(xmlTextReaderValue(reader));
						label += value;
					}
				}
			
        if (name == "line")
				  stack.top()->appendLabel(label);
        else
          stack.top()->appendPopupLine(label);
			}
			break;
		case XML_READER_TYPE_END_ELEMENT:
			if (name == "node")
			{
				if (stack.isEmpty())
				{
					qWarning() << "Tree XML Read error: Trying to pop the stack while it is empty."
								  "Did I close a <node> element to many? This looks like a bug in the parser.";
					break;
				}
				
				DactTreeNode *node = stack.pop();
        if (node->popupLines().size() > 0) {
          PopupItem *popupItem = new PopupItem(0, node->popupLines());
          popupItem->setVisible(false);
          popupItem->setZValue(1.0);
          node->setPopupItem(popupItem);
          addItem(popupItem);
        }
			}
			break;
	}
}

void DactTreeScene::processXMLAttribute(xmlTextReaderPtr &reader, DactTreeNode* node)
{
	int type = xmlTextReaderNodeType(reader);
	
	if (type != XML_READER_TYPE_ATTRIBUTE)
		return;
	
	QString name = processXMLString(xmlTextReaderName(reader));
	QString value = processXMLString(xmlTextReaderValue(reader));
		
	node->setAttribute(name, value);
}

QString DactTreeScene::processXMLString(xmlChar* xmlValue) const
{
	if (xmlValue == NULL)
		return QString();

  QString value(QString::fromUtf8(reinterpret_cast<char const *>(xmlValue)));
	xmlFree(xmlValue);
	return value;
}

DactTreeNode::DactTreeNode(QGraphicsItem *parent) :
	QGraphicsItem(parent),
	d_attributes(),
	d_childNodes(),
	d_labels(),
  d_popupItem(0),
	d_spaceBetweenLayers(40),
	d_spaceBetweenNodes(10),
	d_leafMinimumWidth(30),
	d_leafMinimumHeight(30),
	d_leafPadding(10)
{
	setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
  setAcceptHoverEvents(true);
}

bool DactTreeNode::isLeaf() const
{
	return d_childNodes.length() == 0;
}

bool DactTreeNode::isActive() const
{
	return d_attributes.contains("active") && !d_attributes["active"].isEmpty();
}

void DactTreeNode::appendChild(DactTreeNode *child)
{
	child->setParentItem(this);
	d_childNodes.append(child);
}

void DactTreeNode::appendLabel(QString const &label)
{
	d_labels.append(label);
}

QList<DactTreeNode*> DactTreeNode::children()
{
	return d_childNodes;
}

void DactTreeNode::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  // This doesn't seem to work well, but is not necessary yet, since
  // we only have popup lines on leaf nodes:
  // 
  //QList<QGraphicsItem *> items(scene()->items(event->scenePos(),
  //  Qt::IntersectsItemBoundingRect, Qt::AscendingOrder));
  //if (items.contains(this) && d_popupItem) {
  if (d_popupItem) {
    d_popupItem->setPos(event->scenePos());
    d_popupItem->setVisible(true);
  }
}

void DactTreeNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  if (d_popupItem && d_popupItem->isVisible())
    d_popupItem->setVisible(false);
}

void DactTreeNode::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
  if (d_popupItem && d_popupItem->isVisible())
    d_popupItem->setPos(event->scenePos());
}

void DactTreeNode::setAttribute(QString const &name, QString const &value)
{
	d_attributes[name] = value;
}

QString DactTreeNode::asString(QString const &indent) const
{
	QString dump = indent + "[node labels:";
	
	foreach (QString line, d_labels)
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
	
	foreach (DactTreeNode* child, d_childNodes)
	{
		dump += child->asString(indent + "\t");
	}
	
	dump += indent + "]\n";
	
	return dump;
}

QRectF DactTreeNode::boundingRect() const
{
	/*QRectF branch = branchBoundingRect();
	QRectF leaf = QRectF(QPointF(), leafSize());
	
	leaf.translate((branch.width() - leaf.width()) / 2, 0);
	
	return leaf;*/
	return branchBoundingRect();
}

QPainterPath DactTreeNode::shape() const
{
	QPainterPath path;
	path.addEllipse(leafRect());
	return path;
}

QRectF DactTreeNode::branchBoundingRect() const
{
	return childrenBoundingRect() | leafBoundingRect();
}

QRectF DactTreeNode::leafBoundingRect() const
{
	QRectF leaf(leafRect());
	leaf.setWidth(leaf.width() + 10);
	leaf.setHeight(leaf.height() + 10);
	leaf.translate(-5, -5);
	return leaf;
}

QRectF DactTreeNode::leafRect() const
{
	QSizeF branch(branchSize());
	QRectF leaf(QPointF(0, 0), leafSize());
	leaf.translate(std::max((branch.width() - leaf.width()) / 2, qreal(0)), 0);
	return leaf;
}

QSizeF DactTreeNode::leafSize() const
{
	QFontMetricsF metrics(font());
	QSizeF leaf(d_leafMinimumWidth, d_leafMinimumHeight);
	
	foreach (QString label, d_labels)
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

QSizeF DactTreeNode::branchSize() const
{
	return childrenBoundingRect().size();
}

void DactTreeNode::layout()
{
	qreal left = 0;
	QSizeF leaf = leafSize();

  if (d_popupItem)
    d_popupItem->setPos(scenePos());
	
	foreach (DactTreeNode* child, d_childNodes)
	{
		child->setPos(left, leaf.height() + d_spaceBetweenLayers);
		child->layout();
		
		left += child->branchBoundingRect().width() + d_spaceBetweenNodes;
	}
}

QFont DactTreeNode::font() const
{
	return QFont("verdana", 12);
}

void DactTreeNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QRectF branch(branchBoundingRect());
	QRectF leaf(leafRect());
	
	// paint the edges first, so I can paint the leaf right over it
	// which is needed because the edges sometimes leave some anti-alias
	// pixels inside the leaf's box.
	
	painter->setRenderHint(QPainter::Antialiasing, true);

	paintEdges(painter, leaf);
	
	painter->setRenderHint(QPainter::Antialiasing, true);
	
	QColor background(isActive() ? Qt::darkGreen : Qt::white);
	
	if (isSelected())
		background = background.darker(125);
	
	painter->fillRect(leaf, background);
	
	QPen borderPen(Qt::black, hasFocus() ? 3 : 1);
	painter->setPen(borderPen);
	painter->drawRect(leaf);
	
	painter->setPen(QColor(isActive() ? Qt::white : Qt::black));
	paintLabels(painter, leaf);
}

void DactTreeNode::paintLabels(QPainter *painter, QRectF const &leaf)
{
	// @TODO currently I draw all the labels by just concatenating them together
	// but if I could implement drawing each line separately, it would be easy as pie
	// to implement colors, weights etc for each line separately.
	QString labels;
	foreach (QString label, d_labels)
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

void DactTreeNode::paintEdges(QPainter *painter, QRectF const &leaf)
{
  QPen edgePen(Qt::black, 1);
	painter->setPen(edgePen);

	QPointF origin(leaf.x() + leaf.width() / 2, leaf.y() + leaf.height());
	
	foreach (DactTreeNode* child, d_childNodes)
	{
		// child's top center position, where the leaf should be found
		QPointF target(child->pos() + QPointF(child->branchBoundingRect().width() / 2, 0));
		
		painter->drawLine(origin, target);
	}
}

PopupItem::PopupItem(QGraphicsItem *parent, QList<QString> lines) :
  QGraphicsItem(parent), d_lines(lines), d_padding(5.0)
{
}

QRectF PopupItem::boundingRect() const
{
  // XXX - map to real screen pixels.
  return QRectF(QPointF(-size().width() / 2.0, -32.0), size());
}

QFont PopupItem::font() const
{
  return QFont("verdana", 12);
}


void PopupItem::paint(QPainter *painter, QStyleOptionGraphicsItem const *option,
    QWidget *widget)
{
  QString lines;
  foreach (QString line, d_lines)
    lines += QString("%1\n").arg(line);

  // Get the font size.
  double appDpi = qt_defaultDpi();
  double ratio = appDpi / painter->device()->logicalDpiY();
  QFont painterFont(font());
  painterFont.setPointSizeF(painterFont.pointSize() * ratio);

  QRectF bRect(boundingRect());
  QRectF textRect(bRect.x() + d_padding, bRect.y() + d_padding,
      bRect.width() - 2.0 * d_padding, bRect.height() - 2.0 * d_padding);

  // Popup box
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setOpacity(0.9);
  painter->setBrush(QBrush(Qt::gray));
  painter->setPen(QPen(QBrush(), 0.0));
  painter->drawRoundedRect(boundingRect(), 5.0, 5.0);

  // Popup text
  painter->setOpacity(1.0);
  painter->setBrush(QBrush());
  painter->setPen(QPen());
  painter->setFont(painterFont);
  painter->drawText(textRect, Qt::AlignCenter, lines);
}

QSizeF PopupItem::size() const
{
  QFontMetricsF metrics(font());
  QSizeF popupSize(10, 10);

  foreach (QString label, d_lines) {
    qreal labelWidth = metrics.width(label);
    if (labelWidth > popupSize.width())
      popupSize.setWidth(labelWidth);
    }

    qreal labelsHeight = d_lines.size() * metrics.lineSpacing();
    if (labelsHeight > popupSize.height())
      popupSize.setHeight(labelsHeight);

  popupSize.setWidth(popupSize.width() + 2.0 * d_padding);
  popupSize.setHeight(popupSize.height() + 2.0 * d_padding);

  return popupSize;
  
}

