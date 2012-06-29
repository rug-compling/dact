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
    #include <libxml/tree.h>
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

signals:
    void selectionChanged(TreeNode const *);

private slots:
    void emitSelectionChange();

private:
    bool nodeNameIs(xmlNodePtr xmlNode, char const *name);
    void freeNodes();
    void layout(QPointF pos, TreeNode* node);
    void parseXML(QString const &xml);
    TreeNode *processNode(xmlNodePtr node);
    void scrubNamespace(xmlNodePtr xmlNode);
    QList<TreeNode*> d_nodes;
};

#endif
