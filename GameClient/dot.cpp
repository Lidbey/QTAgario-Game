#include "dot.h"
#include <QPainter>
#include <QTime>
#define RADIUS 10
Dot::Dot(QColor color) : QGraphicsEllipseItem()
{
    this->color = color;
}

QRectF Dot::boundingRect() const
{
    return QRectF(-RADIUS,-RADIUS,RADIUS,RADIUS);
}

void Dot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(color);
    painter->drawEllipse(QRect(-RADIUS,-RADIUS,RADIUS,RADIUS));
}

QPainterPath Dot::shape() const
{
    QPainterPath path;
    path.addEllipse(this->boundingRect());
    return path;
}

void Dot::enable()
{
    active = true;
}

void Dot::disable()
{
    active = false;
}
