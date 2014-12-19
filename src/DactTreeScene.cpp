#include <algorithm>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>

extern "C" {
#include <libxml/parser.h>
#include <libxml/tree.h>
}

#include "DactTreeScene.hh"
#include "Edge.hh"
#include "SecEdge.hh"
#include "TreeNode.hh"
#include "PopupItem.hh"
#include "XMLDeleters.hh"

DactTreeScene::DactTreeScene(QObject *parent) :
    QGraphicsScene(parent),
    d_nodes()
{
    connect(this, SIGNAL(selectionChanged()),
        this, SLOT(emitSelectionChange()));
}

DactTreeScene::~DactTreeScene()
{
    disconnect(this, SIGNAL(selectionChanged()));
    
    if (rootNode())
        freeNodes();
}

void DactTreeScene::checkElementAndAssign(xmlNodePtr node,
    char const *nodeName, xmlNodePtr *variable)
{
    if (node->type == XML_ELEMENT_NODE && nodeNameIs(node, nodeName))
    {
        {
            if (*variable == 0)
                *variable = node;
            else
                qWarning() << "More than one '" << nodeName << "', only using first!";
        }
    }
}

void DactTreeScene::emitSelectionChange()
{
    emit selectionChanged(dynamic_cast<TreeNode*>(focusItem()));
}

void DactTreeScene::parseTree(QString const &xml)
{
    // If there was a previous parse, clean up first.
    if (rootNode())
    {
        removeItem(rootNode());
        foreach (QGraphicsItem *edge, d_edges)
        {
            removeItem(edge);
        }

        freeNodes();
    }
    
    parseXML(xml);
    
    // If parsing was successful enough to create a root node,
    // add it to the scene
    if (rootNode())
    {
        rootNode()->layout();

        foreach (QGraphicsItem *edge, d_edges)
        {
            addItem(edge);
        }

        addItem(rootNode());
    }
    else
    {
        qWarning() << "No root node was found";
    }
}

void DactTreeScene::parseXML(QString const &xml)
{
    QByteArray xmlData(xml.toUtf8());

    QScopedPointer<xmlDoc, XmlDocDeleter> doc(
        xmlReadMemory(xmlData.constData(), xmlData.size(), NULL, NULL, 0));
    if (doc == 0) {
      qWarning() << "Could not parse tree!";
      return;
    }

    xmlNode *treeRoot = xmlDocGetRootElement(doc.data());
    if (treeRoot == NULL) {
      qWarning() << "Tree does not have a root?";
      return;
    }

    xmlNodePtr treeRootNode = 0;
    xmlNodePtr secEdges = 0;

    for (xmlNodePtr child = treeRoot->children; child; child = child->next)
    {
      checkElementAndAssign(child, "node", &treeRootNode);
      checkElementAndAssign(child, "secedges", &secEdges);
    }

    if (treeRootNode == 0) {
      qWarning() << "Tree does not have a root?";
      return;
    }
    processNode(treeRootNode);

    if (secEdges)
      processSecondaryEdges(secEdges);
}

TreeNode *DactTreeScene::processNode(xmlNodePtr xmlNode)
{
    TreeNode *node = new TreeNode;
    node->setZValue(100.);
    d_nodes.append(node);

    for (xmlNodePtr child = xmlNode->children; child; child = child->next)
    {
      if (child->type == XML_ELEMENT_NODE && nodeNameIs(child, "node"))
      {
        TreeNode *childNode = processNode(child);
        node->appendChild(childNode);
        Edge *edge = new Edge();
        edge->setParent(node);
        edge->setChild(childNode);
        d_edges.push_back(edge);
      }
      if (child->type == XML_ELEMENT_NODE &&
          (nodeNameIs(child, "label") || nodeNameIs(child, "tooltip")))
      {
        QScopedPointer<xmlBuffer, XmlBufferDeleter> buf(xmlBufferCreate());

        for (xmlNodePtr contentNode = child->children; contentNode;
                contentNode = contentNode->next) {
            scrubNamespace(contentNode);
            xmlNodeDump(buf.data(), 0, contentNode, 0, 0);
        }

        xmlChar const *value = xmlBufferContent(buf.data());

        if (nodeNameIs(child, "label"))
            node->setLabel(QString::fromUtf8(reinterpret_cast<char const *>(value)).trimmed());
        else
        {
            node->setTooltip(QString::fromUtf8(reinterpret_cast<char const *>(value)));

            PopupItem *popupItem = new PopupItem(0, node->tooltip());
            popupItem->setVisible(false);
            popupItem->setZValue(1.0);
            node->setPopupItem(popupItem);
            addItem(popupItem);
        }
      }
    }

    for (xmlAttrPtr attr = xmlNode->properties; attr; attr = attr->next)
    {
      QScopedPointer<xmlChar, XmlDeleter> value(
        xmlNodeGetContent(attr->children));
      if (value == 0)
        continue;

      // Do we have an active node marker?
      if (xmlStrEqual(attr->name, reinterpret_cast<xmlChar const *>("active")) &&
          xmlStrEqual(value.data(), reinterpret_cast<xmlChar const *>("1")))
        node->setActive(true);
      else
        node->setAttribute(
            QString::fromUtf8(reinterpret_cast<char const *>(attr->name)),
            QString::fromUtf8(reinterpret_cast<char const *>(value.data())));

      if (xmlStrEqual(attr->name, reinterpret_cast<xmlChar const *>("id")))
      {
        QString nodeID = QString::fromUtf8(reinterpret_cast<char const *>(value.data()));
        d_idNodes[nodeID] = node;
      }
    }

    return node;
}

void DactTreeScene::processSecondaryEdges(xmlNodePtr node)
{
    for (xmlNodePtr edge = node->children; edge; edge = edge->next)
    {
        if (!nodeNameIs(edge, "secedge"))
            continue;

        QScopedPointer<xmlChar, XmlDeleter> cat(xmlGetProp(edge,
              reinterpret_cast<xmlChar const *>("cat")));
        QScopedPointer<xmlChar, XmlDeleter> from(xmlGetProp(edge,
              reinterpret_cast<xmlChar const *>("from")));
        QScopedPointer<xmlChar, XmlDeleter> to(xmlGetProp(edge,
              reinterpret_cast<xmlChar const *>("to")));

        QString qFrom = QString::fromUtf8(reinterpret_cast<char const *>(from.data()));
        QMap<QString, TreeNode *>::const_iterator fromIter = d_idNodes.find(qFrom);
        if (fromIter == d_idNodes.end())
        {
          qWarning() << "Could not find from-node for secondary edge, skipping";
          continue;
        }

        QString qTo = QString::fromUtf8(reinterpret_cast<char const *>(to.data()));
        QMap<QString, TreeNode *>::const_iterator toIter = d_idNodes.find(qTo);
        if (toIter == d_idNodes.end())
        {
          qWarning() << "Could not find to-node for secondary edge, skipping";
          continue;
        }

        SecEdge *secEdge = new SecEdge;
        secEdge->setZValue(50);
        secEdge->setFrom(fromIter.value());
        secEdge->setTo(toIter.value());
        secEdge->setLabel(QString::fromUtf8(reinterpret_cast<char const *>(cat.data())));
        d_edges.push_back(secEdge);
    }
}

bool DactTreeScene::nodeNameIs(xmlNodePtr xmlNode, char const *name)
{
    return xmlStrEqual(xmlNode->name, reinterpret_cast<xmlChar const *>(name));
}

void DactTreeScene::scrubNamespace(xmlNodePtr xmlNode)
{
    xmlSetNs(xmlNode, 0);

    for (xmlNodePtr child = xmlNode->children; child; child = child->next)
        scrubNamespace(child);
}

void DactTreeScene::freeNodes()
{
    delete rootNode();
    d_nodes.clear();
    // because all have rootNode as parent/ancestor
    // they will have been deleted automatically.
    
    foreach (QGraphicsItem *edge, d_edges)
    {
        delete edge;
    }
    d_edges.clear();
}

QList<TreeNode*> const &DactTreeScene::nodes() const
{
    return d_nodes;
}

QList<TreeNode*> DactTreeScene::activeNodes() const
{
    QList<TreeNode*> nodes;
    
    foreach (TreeNode* node, d_nodes)
    {
        if (node->isActive())
            nodes.append(node);
    }
    
    return nodes;
}

TreeNode* DactTreeScene::rootNode()
{
    if (d_nodes.size() > 0)
        return d_nodes[0];
    else
        return 0;
}

