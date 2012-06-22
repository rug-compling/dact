#include <QFont>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "PopupItem.hh"
#include "TreeNode.hh"

PopupItem::PopupItem(QGraphicsItem *parent, QString const &html)
:
    QGraphicsItem(parent),
    d_content(),
    d_padding(5.0)
{
    d_content.setDocumentMargin(d_padding);
    d_content.setDefaultFont(font());
    d_content.setHtml(html);
}

QRectF PopupItem::boundingRect() const
{
    QRectF popup = rect();
    qreal scale = viewScale();
    return QRectF(popup.topLeft() / scale,
                  popup.size() / scale);
}

QRectF PopupItem::rect() const
{
    QSizeF popup = size();
    return QRectF(QPointF(-popup.width() / 2.0, -popup.height() / 2.0), popup);
}

QFont PopupItem::font() const
{
    return QFont("verdana", 12);
}


void PopupItem::paint(QPainter *painter, QStyleOptionGraphicsItem const *option,
    QWidget *widget)
{
    QString lines;
    
    /*
    // Get the font size.
    double appDpi = qt_defaultDpi();
    double ratio = appDpi / painter->device()->logicalDpiY();
    QFont painterFont(font());
    painterFont.setPointSizeF(painterFont.pointSize() * ratio);
    
    QRectF bRect(rect());
    QRectF textRect(bRect.x() + d_padding, bRect.y() + d_padding,
        bRect.width() - 2.0 * d_padding, bRect.height() - 2.0 * d_padding);
    */
    
    painter->save();
    
    qreal scale = 1 / viewScale();
    painter->scale(scale, scale);
    
    // Popup box
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(0.9);
    painter->setBrush(QBrush(Qt::gray));
    painter->setPen(QPen(QBrush(), 0.0));
    painter->drawRoundedRect(rect(), 5.0, 5.0);

    // Popup text
    painter->setOpacity(1.0);
    painter->setBrush(QBrush());
    painter->setPen(QPen());

    painter->translate(rect().topLeft());
    d_content.drawContents(painter, QRectF(QPointF(0,0), size()));
    
    painter->restore();
}

QSizeF PopupItem::size() const
{
    return d_content.size();
}

qreal PopupItem::viewScale() const
{
    if (!scene() || scene()->views().isEmpty())
        return 1.0;
    
    return scene()->views().first()->transform().m11();
}
