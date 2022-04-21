#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsEllipseItem>

class Player : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    explicit Player(int id, QObject* parent=nullptr);
    ~Player();
    void addSize(double size=8);
    void minusSize();
    int getSize();
    double getSqrtSize();
    void setDead();
    bool dead();

public slots:
    void action(bool, double, double);

protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    qreal angle;
    int size=0;
    double sqrt_size=0;
    bool isDead=false;
    int id;
};

#endif // PLAYER_H
