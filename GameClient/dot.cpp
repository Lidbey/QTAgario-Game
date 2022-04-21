#include "dot.h"
#include <QPainter>
#define RADIUS 5
Dot::Dot() : QGraphicsEllipseItem()
{
}

QRectF Dot::boundingRect() const
{
    if(!active) return QRectF(0,0,0,0);
    return QRectF(-RADIUS,-RADIUS,RADIUS,RADIUS);
}

void Dot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(!active) return;
    painter->setBrush(Qt::red);
    painter->drawEllipse(QRect(-RADIUS,-RADIUS,RADIUS,RADIUS));
}

QPainterPath Dot::shape() const
{
    if(!active) return QPainterPath();
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
