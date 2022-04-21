#include "player.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsScene>
#include "DEFINES.h"
#include "math.h"
#include <QRandomGenerator>

Player::Player(int id, QObject* parent) : QObject(parent), QGraphicsEllipseItem()
{
    this->setZValue(1);
    this->id = id;
}

QRectF Player::boundingRect() const
{
    QRectF polygon(-MAIN_WIDTH/2-sqrt_size/2,-MAIN_HEIGHT/2-sqrt_size/2,MAIN_WIDTH+sqrt_size,MAIN_HEIGHT+sqrt_size);
    return polygon;
}

QPainterPath Player::shape() const
{
    QPainterPath path;
    QRectF polygon(-MAIN_WIDTH/2-sqrt_size/2,-MAIN_HEIGHT/2-sqrt_size/2,MAIN_WIDTH+sqrt_size,MAIN_HEIGHT+sqrt_size);
    path.addEllipse(polygon);
    return path;
}

void Player::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRandomGenerator generator(id);
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QRectF polygon(-MAIN_WIDTH/2-sqrt_size/2,-MAIN_HEIGHT/2-sqrt_size/2,MAIN_WIDTH+sqrt_size,MAIN_HEIGHT+sqrt_size);
    painter->setBrush(QColor(generator.bounded(256),generator.bounded(256),generator.bounded(256)));
    painter->drawEllipse(polygon);
}

Player::~Player()
{

}

int Player::getId()
{
    return id;

}

void Player::setSize(int size)
{
    this->size = size;
    sqrt_size = sqrt(size)*sqrt(sqrt(size));

}

int Player::getSize()
{
    return size;
}

double Player::getSqrtSize()
{
    return sqrt_size;
}
