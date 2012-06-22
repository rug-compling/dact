#ifndef POPUPITEM_HH
#define POPUPITEM_HH

#include <QGraphicsItem>
#include <QList>
#include <QTextDocument>

extern int qt_defaultDpi();

class PopupItem : public QGraphicsItem
{
public:
    PopupItem(QGraphicsItem *parent = 0, QString const &html = QString());
    QRectF boundingRect() const;
    QFont font() const;
    void paint(QPainter *painter, QStyleOptionGraphicsItem const *option, QWidget *widget);
    QSizeF size() const;
    QRectF rect() const;
private:
    qreal viewScale() const;
    QTextDocument d_content;
    qreal d_padding;
};

#endif

