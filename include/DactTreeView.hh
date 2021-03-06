#ifndef DACTTREEVIEW_H
#define DACTTREEVIEW_H

#include <QGraphicsView>
#include <QSharedPointer>
#include "XSLTransformer.hh"

double const ZOOM_OUT_FACTOR = 0.8;
double const ZOOM_IN_FACTOR = 1.0 / ZOOM_OUT_FACTOR;

class TreeNode;
class DactTreeScene;

class DactTreeView : public QGraphicsView
{
    Q_OBJECT
    
public:
    DactTreeView(QWidget *parent = 0);
    
    DactTreeScene* scene() const;
    void setScene(DactTreeScene *scene);
    void setStylesheet(QFile *stylesheet);
    
    void showTree(QString const &xml);
    void focusTreeNode(int direction);
    void fitInView(QRectF const &rect, Qt::AspectRatioMode aspectRatioMode);
    
public slots:
    void fitTree();
    void focusNextTreeNode();
    void focusPreviousTreeNode();
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void sceneChanged(DactTreeScene* scene);

protected:
    QString transformParseToTree(QString const &xml) const;
    void wheelEvent(QWheelEvent * event);
    void keyPressEvent(QKeyEvent * event);
    void focusNode(TreeNode &node);
    TreeNode *focussedNode();
    void clearSelection();

private:
    QString d_xml; // xml of the last transformed tree
    QSharedPointer<XSLTransformer> d_transformer;
};

#endif
