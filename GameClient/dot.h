#ifndef DOT_H
#define DOT_H

#include <QGraphicsItem>

class Dot : public QGraphicsEllipseItem
{
public:
    Dot();
protected:
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QPainterPath shape() const;

};

#endif // DOT_H
