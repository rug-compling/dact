#include <QFont>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "PopupItem.hh"
#include "TreeNode.hh"

PopupItem::PopupItem(QGraphicsItem *parent, QList<QString> lines)
:
    QGraphicsItem(parent),
    d_lines(lines),
    d_padding(5.0)
{}

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
    return QRectF(QPointF(-popup.width() / 2.0, -34.0), popup);
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

    QRectF bRect(rect());
    QRectF textRect(bRect.x() + d_padding, bRect.y() + d_padding,
        bRect.width() - 2.0 * d_padding, bRect.height() - 2.0 * d_padding);

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
    painter->setFont(painterFont);
    painter->drawText(textRect, Qt::AlignCenter, lines);
    
    painter->restore();
}

QSizeF PopupItem::size() const
{
    QFontMetricsF metrics(font());
    QSizeF popupSize(10, 10);
    
    foreach (QString label, d_lines)
    {
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

qreal PopupItem::viewScale() const
{
    if (!scene() || scene()->views().isEmpty())
        return 1.0;
    
    return scene()->views().first()->transform().m11();
}
