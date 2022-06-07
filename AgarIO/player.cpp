#include "player.h"
#include <QPainter>
#include <QDebug>
#include <QGraphicsScene>
#include "DEFINES.h"
#include "math.h"
#include <QRandomGenerator>


//klasa gracza do rysowania go na scenie (+ pare dodatkow)
Player::Player(int id, QObject* parent) : QObject(parent), QGraphicsEllipseItem()
{
    this->id = id;
    this->setZValue(1);
    timer.setSingleShot(true);
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

//wywolanie akcji na obiekcie gracza
void Player::action(bool spaceClicked, double movex, double movey)
{
    //predkosc to 3-wielkosc/1000, czyli w sumie spada liniowo (zaczynajac od troche ponizej 3),
    double speed = 3-(double)this->size/1000;

    //ale przy wielkosci 2333+ predkosc juz stoi w miejscu
    if(speed < 0.66) speed = 0.66;
    //jesli spacja to speed*2, ale moze sie obnizyc masa dodatkowo do normalnej obnizki
    if(spaceClicked)
    {
        speed*=2;
        if(rand()%100<2)
            this->minusSize();
    }

    //no i musze ustawic nowa pozycje
    QPointF move = mapToParent(2*speed*movex, 2*speed*movey);
    setPos(move);


    //mapToParent mapuje mi wzgledna moja pozycje na pozycje na scenie, a wiec mapToParent(0,0) da mi pozycje na scenie
    QPointF currPoint=mapToParent(0,0);
    if(currPoint.x() > RATIO*WIDTH+10) this->setX(RATIO*WIDTH+10);
    if(currPoint.y() > RATIO*HEIGHT) this->setY(RATIO*HEIGHT);
    if(currPoint.x() < RATIO*MINX-10) this->setX(RATIO*MINX-10);
    if(currPoint.y() < RATIO*MINY) this->setY(RATIO*MINY);
}

Player::~Player()
{

}

//dodaj wielkosc
void Player::addSize(double size)
{
    this->size+=size;
    sqrt_size = sqrt(this->size)*sqrt(sqrt(this->size));
}

//odejmij wielkosc(sqrt jest do rysowania, masa do wysylania i liczenia)
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

//a to nie wiem cos srednio dziala i probowalem wiele rzeczy aby nie crashowalo przy zjedzeniu
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

void Player::setId(int id)
{
    this->id = id;
}

int Player::getId()
{
    return id;
}

bool Player::timerActive()
{
    return timer.isActive();
}

void Player::startTimer()
{
    timer.start(10000);
}

bool Player::isLeader()
{
    return leader;
}

void Player::setLeader()
{
    this->leader = true;
}
