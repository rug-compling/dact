#ifndef POPUPITEM_HH
#define POPUPITEM_HH

#include <QGraphicsItem>
#include <QList>

extern int qt_defaultDpi();

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

#endif