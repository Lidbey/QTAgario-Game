#ifndef MAINCLASS_H
#define MAINCLASS_H


#include <QGraphicsScene>
#include <QTimer>
#include "player.h"
#include <QWidget>
#include <QSet>
#include <QTcpServer>
#include "gamesocket.h"

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
protected:
    bool eventFilter(QObject* target, QEvent* event);

private:
    Ui::MainClass *ui;
    QSet<int> keysClicked;
    QTimer timer;
    void gameLoop();
    QVector<QGraphicsItem*> items;
    qreal calcDistance(QPointF, QPointF);
    QTcpServer server;
    QList<GameSocket*> connectedSockets;
    QString prepareGameState();

    QList<QPointF> dotsData;
    int playerId=0;

private slots:
    void newConnection();
    void stopConnection(GameSocket* socket);
};

#endif // MAINCLASS_H
