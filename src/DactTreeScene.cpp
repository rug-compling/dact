#include <algorithm>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QSettings>

#include "DactTreeScene.hh"
#include "TreeNode.hh"
#include "PopupItem.hh"

DactTreeScene::DactTreeScene(QObject *parent) :
    QGraphicsScene(parent),
    d_nodes()
{
    connect(this, SIGNAL(selectionChanged()),
        this, SLOT(emitSelectionChange()));
}

DactTreeScene::~DactTreeScene()
{
    if (rootNode())
        freeNodes();
}

void DactTreeScene::emitSelectionChange()
{
    emit selectionChanged(reinterpret_cast<TreeNode*>(focusItem()));
}

void DactTreeScene::parseTree(QString const &xml)
{
    // If there was a previous parse, clean up first.
    if (rootNode())
    {
        removeItem(rootNode());
        freeNodes();
    }
    
    parseXML(xml);
    
    // If parsing was successful enough to create a root node,
    // add it to the scene
    if (rootNode())
    {
        addItem(rootNode());
        rootNode()->layout();
    }
    else
    {
        qWarning() << "No root node was found";
    }
}

void DactTreeScene::parseXML(QString const &xml)
{
    QByteArray xmlData(xml.toUtf8());
    xmlTextReaderPtr reader = xmlReaderForMemory(xmlData.constData(),
        xmlData.size(), NULL, NULL, XML_PARSE_NOBLANKS | XML_PARSE_NOCDATA);
    
    // Process every node 
    QStack<TreeNode*> stack;
    while (true)
    {
        if (!xmlTextReaderRead(reader))
            break;
        
        processXMLNode(reader, d_nodes, stack);
    }
    
    xmlFreeTextReader(reader);
    
    if (stack.size() > 1)
        qWarning() << "Tree XML Read error: Stack not empty at the end";
}

void DactTreeScene::freeNodes()
{
    delete rootNode();
    d_nodes.clear();
    // because all have rootNode as parent/ancestor
    // they will have been deleted automatically.
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

void DactTreeScene::processXMLNode(xmlTextReaderPtr &reader, QList<TreeNode*> &list, QStack<TreeNode*> &stack)
{
    QString name = processXMLString(xmlTextReaderName(reader));
    QString value = processXMLString(xmlTextReaderValue(reader));
    
    int type = xmlTextReaderNodeType(reader);
    
    switch (type)
    {
        case XML_READER_TYPE_ELEMENT:
            if (name == "node")
            {
                TreeNode* node = new TreeNode();
                d_nodes.append(node);

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
                
                TreeNode *node = stack.pop();
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

void DactTreeScene::processXMLAttribute(xmlTextReaderPtr &reader, TreeNode* node)
{
    int type = xmlTextReaderNodeType(reader);
    
    if (type != XML_READER_TYPE_ATTRIBUTE)
        return;
    
    QString name = processXMLString(xmlTextReaderName(reader));
    QString value = processXMLString(xmlTextReaderValue(reader));
    
    // active is a special attribute, as it describes which node matched the
    // query. It is not an attribute of the original node in the parse tree and
    // therefore we do not treat it as an 'attribute'. We don't want this to
    // show up in the inspector now do we? Some poor soul might try to use it in
    // a query... >:)
    if (name == "active")
        node->setActive(!value.isEmpty());
    else
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