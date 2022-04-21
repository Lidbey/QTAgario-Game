#ifndef DOT_H
#define DOT_H

#include <QGraphicsItem>

class Dot : public QGraphicsEllipseItem
{
public:
    Dot();
    QString mapToData();
    QString mapToChanges();
    void disable();
    void enable();
    bool enabled();
protected:
    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QPainterPath shape() const;
private:
    bool active;
};

#endif // DOT_H
