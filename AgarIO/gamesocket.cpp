#include "gamesocket.h"

GameSocket::GameSocket(int id, QTcpSocket* socket, QObject *parent) : QObject(parent)
{
    player = new Player(this);
    movex=0;
    movey=0;
    sprint=false;
    this->id = id;
    this->socket = socket;
    connect(socket, &QTcpSocket::readyRead, this, &GameSocket::read);
}

GameSocket::~GameSocket()
{
    //socket->disconnectFromHost();
    delete player;
}

Player *GameSocket::getPlayer()
{
    return player;
}

QTcpSocket *GameSocket::getSocket()
{
    return socket;
}

bool GameSocket::actionValid()
{
    double tempx = movex;
    double tempy = movey;
    return (tempx*tempx+tempy*tempy <= 1.1);
}

void GameSocket::doAction()
{
    double tempx = movex;
    double tempy = movey;
    bool tempspace = sprint;
    this->player->action(tempspace, tempx, tempy);
}

void GameSocket::sendState(QString gameState)
{
    socket->write(gameState.toUtf8());
}

int GameSocket::getId()
{
    return id;
}

void GameSocket::read()
{
    buffer += socket->readAll();
    if(buffer.contains("\r\n"))
    {
        QStringList list = buffer.split("\r\n");
        if(list.length() > 1)
            buffer=list.last();
        if(list.isEmpty()) return;
        QStringList actions = list[0].split(';');
        if(actions.length() < 3) return;
        movex = actions[0].toDouble();
        movey = actions[1].toDouble();
        sprint = (actions[2] == "1");
        player->update();
    }
}
