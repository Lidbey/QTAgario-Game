#ifndef DOT_H
#define DOT_H

#include <QGraphicsItem>

class Dot : public QGraphicsEllipseItem
{
public:
    Dot(QColor color);
    void disable();
    void enable();
protected:
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QPainterPath shape() const;

private:
    bool active;
    QColor color;

};

#endif // DOT_H
