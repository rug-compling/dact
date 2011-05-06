#ifndef DACTTREEVIEW_H
#define DACTTREEVIEW_H

#include <QGraphicsView>
#include <QSharedPointer>
#include "XSLTransformer.hh"

class DactTreeScene;

class DactTreeView : public QGraphicsView
{
    Q_OBJECT
    
public:
    DactTreeView(QWidget *parent = 0);
    
    QString const &highlightQuery() const;
    void setHighlightQuery(QString const &xml);
    
    DactTreeScene* scene() const;
    void setScene(DactTreeScene *scene);
    
    void showTree(QString const &xml);
    void fitTree();

signals:
    void sceneChanged(DactTreeScene* scene);

protected:
    QString transformParseToTree(QString const &xml) const;
    void wheelEvent(QWheelEvent * event);

private:
    QString d_xml; // xml of the last transformed tree
    QString d_query;
    QSharedPointer<XSLTransformer> d_transformer;
};

#endif