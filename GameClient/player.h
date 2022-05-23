#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsEllipseItem>

class Player : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    explicit Player(int id, QObject* parent=nullptr);
    ~Player();
    int getId();
    void setId(int id);
    void setSize(int size);
    int getSize();
    double getSqrtSize();

protected:
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
private:
    int size;
    double sqrt_size;
    int id;
};

#endif // PLAYER_H
