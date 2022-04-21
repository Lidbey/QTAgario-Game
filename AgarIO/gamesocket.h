#ifndef GAMESOCKET_H
#define GAMESOCKET_H

#include <QObject>
#include <QTcpSocket>
#include "player.h"
class GameSocket : public QObject
{
    Q_OBJECT
public:
    explicit GameSocket(int id, QTcpSocket* socket, QObject *parent = nullptr);
    ~GameSocket() override;
    Player* getPlayer();
    QTcpSocket* getSocket();
    bool actionValid();
    void doAction();
    void sendState(QString gameState);
    int getId();
    bool isOk();
private:
    QTcpSocket* socket;
    Player* player;
    QString buffer;
    int id;
    double movex=0;
    double movey=0;
    bool sprint=false;
    bool disconnected=false;

private slots:
    void read();
signals:

};

#endif // GAMESOCKET_H
