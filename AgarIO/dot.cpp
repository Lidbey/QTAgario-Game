#include "dot.h"
#include <QPainter>
#define RADIUS 10
Dot::Dot() : QGraphicsEllipseItem()
{
}

QString Dot::mapToData()
{
    return  QString::number(this->pos().x())+";"+
            QString::number(this->pos().y())+";"+
            (active?"1":"0");
}

void Dot::disable()
{
    active = false;
}

void Dot::enable()
{
    active = true;
}

bool Dot::enabled()
{
    return active;
}

QRectF Dot::boundingRect() const
{
    return QRectF(-RADIUS,-RADIUS,RADIUS,RADIUS);
}

void Dot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::red);
    painter->drawEllipse(QRect(-RADIUS,-RADIUS,RADIUS,RADIUS));
}

QPainterPath Dot::shape() const
{
    QPainterPath path;
    path.addEllipse(this->boundingRect());
    return path;
}
