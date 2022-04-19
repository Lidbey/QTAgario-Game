#include "player.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsScene>
#include "DEFINES.h"
#include "math.h"

Player::Player(QObject* parent) : QObject(parent), QGraphicsEllipseItem()
{
    this->setZValue(1);
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
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QRectF polygon(-MAIN_WIDTH/2-sqrt_size/2,-MAIN_HEIGHT/2-sqrt_size/2,MAIN_WIDTH+sqrt_size,MAIN_HEIGHT+sqrt_size);
    painter->setBrush(Qt::blue);
    painter->drawEllipse(polygon);
}

void Player::action(bool spaceClicked, double movex, double movey)
{
    double speed = 3-(double)this->size/1000;
    if(speed < 0.66) speed = 0.66;
    if(spaceClicked)
    {
        speed*=2;
        if(rand()%100<2)
            this->minusSize();
    }

    QPointF move = mapToParent(2*speed*movex, 2*speed*movey);
    setPos(move);


    QPointF currPoint=mapToParent(0,0);
    if(currPoint.x() > RATIO*WIDTH+10) this->setX(RATIO*WIDTH+10);
    if(currPoint.y() > RATIO*HEIGHT) this->setY(RATIO*HEIGHT);
    if(currPoint.x() < RATIO*MINX-10) this->setX(RATIO*MINX-10);
    if(currPoint.y() < RATIO*MINY) this->setY(RATIO*MINY);
}

Player::~Player()
{

}

void Player::addSize(double size)
{
    this->size+=size;
    sqrt_size = sqrt(this->size)*sqrt(sqrt(this->size));
}

void Player::minusSize()
{
    if(size>0)
    {
        if(size>50)
            size=0.98*size;
        else
            size-=2;
    }
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

void Player::setDead()
{
    isDead=true;
    this->size=0;
    this->sqrt_size=0;
}

bool Player::dead()
{
    return isDead;
}
