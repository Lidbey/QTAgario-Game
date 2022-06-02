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
#include <QProcess>

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

    QString port;

    qreal calcDistance(QPointF, QPointF);

    void gameLoop();
    QString prepareGameState();
    QString preparePlayerData();
    QString prepareChangeState(QList<Dot*> changed);

    QList<QProcess*> currentProcesses;
    GameSocket* findSocket(int id);
    QList<GameSocket*> findTeamMembers(int team);

private slots:
    void newConnection();
    void addBots();
    void playerDivide(int team, int player);
    void tactic(int team, int player, int tactic, QStringList args);
    void setTeam(int team);
};

#endif // MAINCLASS_H
