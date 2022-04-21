#ifndef MAINCLASS_H
#define MAINCLASS_H


#include <QGraphicsScene>
#include <QTimer>
#include "player.h"
#include <QWidget>
#include <QSet>
#include <QTcpServer>
#include "gamesocket.h"
#include "dot.h"

namespace Ui {
class MainClass;
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainClass; }
QT_END_NAMESPACE

class MainClass : public QWidget
{
    Q_OBJECT

public:
    MainClass(int port, QWidget *parent = nullptr);
    ~MainClass();
    QGraphicsScene* scene;

private:
    Ui::MainClass *ui;
    QTimer timer;
    QTcpServer server;
    QVector<QGraphicsItem*> items;
    QList<GameSocket*> connectedSockets;
    QList<Dot*> dots;
    int playerId=0;


    void gameLoop();
    QString prepareGameState();
    QString prepareChangeState(QList<Dot*> changed);


    qreal calcDistance(QPointF, QPointF);

private slots:
    void newConnection();
};

#endif // MAINCLASS_H
