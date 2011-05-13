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

class TreeNode;
class PopupItem;

class DactTreeScene : public QGraphicsScene
{
    Q_OBJECT
public: 
    DactTreeScene(QObject *parent = 0);
    ~DactTreeScene();
    void parseTree(QString const &xml);
    QList<TreeNode*> const &nodes() const;
    QList<TreeNode*> activeNodes() const;
    TreeNode* rootNode();

private:
    void freeNodes();
    void layout(QPointF pos, TreeNode* node);
    void parseXML(QString const &xml);
    void processXMLNode(xmlTextReaderPtr &reader, QList<TreeNode*> &list, QStack<TreeNode*> &stack);
    void processXMLAttribute(xmlTextReaderPtr &reader, TreeNode* node);
    QString processXMLString(xmlChar* xmlValue) const;
    QList<TreeNode*> d_nodes;
};

#endif
