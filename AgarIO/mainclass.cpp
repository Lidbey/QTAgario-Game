#include "mainclass.h"
#include "ui_mainclass.h"
#include <QKeyEvent>
#include <QDebug>
#include "DEFINES.h"
#include "dot.h"
#include <QtMath>
#include <QTcpSocket>


MainClass::MainClass(int port, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainClass)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    qDebug() << server.listen(QHostAddress::Any, port);

    scene=new QGraphicsScene();
    scene->setSceneRect(0, 0, RATIO*WIDTH, RATIO*HEIGHT);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->scale(1.30/RATIO, 1.30/RATIO);

    for(int i=0; i<START_DOTS; i++)
    {
        Dot* dot=new Dot();
        scene->addItem(dot);
        dot->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
        dotsData << dot->pos();
    }

    timer.start(25);
    connect(&timer, &QTimer::timeout, this, &MainClass::gameLoop);
    connect(&server, &QTcpServer::newConnection, this, &MainClass::newConnection);
}

MainClass::~MainClass()
{
    delete ui;
}

bool MainClass::eventFilter(QObject* target, QEvent* event)
{
    if(event->type()==QEvent::KeyPress)
    {
        keysClicked.insert(static_cast<QKeyEvent*>(event)->key());
    }
    else if(event->type()==QEvent::KeyRelease)
        keysClicked.remove(static_cast<QKeyEvent*>(event)->key());

    return QWidget::eventFilter(target, event);
}

void MainClass::gameLoop()
{
    for(int i=connectedSockets.length()-1; i>=0; i--)
    {
        Player* player = connectedSockets[i]->getPlayer();
        if(player->dead())
        {
            connectedSockets[i]->getSocket()->disconnectFromHost();
            continue;
        }
        if(connectedSockets[i]->actionValid())
        {
            connectedSockets[i]->doAction();
        }
        if(rand()%200<2)
        {
            player->minusSize();
        }
        player->update();
    }

    for(int j=0; j<connectedSockets.length(); j++)
    {
        Player* player = connectedSockets[j]->getPlayer();
        if(!player || player->dead()) continue;
        QList<QGraphicsItem*> colliding=scene->collidingItems(player);
        for(int i=0; i<colliding.length(); i++)
        {
            if(dynamic_cast<Dot*>(colliding[i])!=nullptr)
            {
                dotsData.removeOne(colliding[i]->pos());
                scene->removeItem(colliding[i]);
                delete colliding[i];
                player->addSize();
            }
            if(dynamic_cast<Player*>(colliding[i])!=nullptr)
            {
                Player* player = dynamic_cast<Player*>(colliding[i]);
                Player* enemyPlayer = connectedSockets[i]->getPlayer();
                if(enemyPlayer->dead()) continue;
                QPointF enemyPos = enemyPlayer->pos();
                QPointF allyPos = player->pos();
                qreal distance = calcDistance(enemyPos, allyPos);
                double enemySize = enemyPlayer->getSqrtSize();
                double allySize = player->getSqrtSize();
                if(allySize > enemySize)
                {
                    if(allySize > distance)
                    {
                        qDebug() << "ally won";
                        player->addSize(enemyPlayer->getSize());
                        //scene->removeItem(enemyPlayer);
                        enemyPlayer->setDead();
                    }
                }
                else if (allySize < enemySize)
                {
                    if(enemySize > distance)
                    {
                        qDebug() << "enemy won";
                        enemyPlayer->addSize(player->getSize());
                        //scene->removeItem(player);
                        player->setDead();
                    }
                }
            }
        }
    }

    if(rand()%100<10)
    {
        if(scene->items().length()<5000)
        {
            Dot* dot=new Dot();
            scene->addItem(dot);
            dot->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
            dotsData << dot->pos();
            for(int i=0; i<connectedSockets.length(); i++)
            {
                if(connectedSockets[i]->getPlayer()->collidesWithItem(dot))
                {
                    dotsData.removeOne(dot->pos());
                    scene->removeItem(dot);
                    delete dot;
                    break;
                }
            }
        }
    }

    QString gameState = this->prepareGameState();
    for(GameSocket* socket: connectedSockets)
    {
        socket->sendState(gameState);
    }
}

qreal MainClass::calcDistance(QPointF a, QPointF b)
{
    return qSqrt((a.x()-b.x())*(a.x()-b.x())+(a.y()-b.y())*(a.y()-b.y()));
}

QString MainClass::prepareGameState()
{
    QString playerData = "PPPP";
    for(GameSocket* socket: connectedSockets)
    {
        QPointF pos = socket->getPlayer()->pos();
        playerData+=QString::number(socket->getId())+";"+QString::number(pos.x())+";"+QString::number(pos.y())+";"+QString::number(socket->getPlayer()->getSize())+"\t";
    }
    playerData += "OOOO";

    QString dotData = "DDDD";
    for(QPointF point: dotsData)
    {
        dotData+=QString::number(point.x())+";"+QString::number(point.y())+"\t";
    }
    dotData += "KKKK";
    return playerData+dotData;
}

void MainClass::newConnection()
{
    QTcpSocket* socket = server.nextPendingConnection();
    int id = this->playerId++;
    socket->write(("ID: "+QString::number(id)+"\r\n").toUtf8());
    GameSocket* gameSocket = new GameSocket(id, socket);
    connectedSockets.append(gameSocket);
    scene->addItem(gameSocket->getPlayer());
    gameSocket->getPlayer()->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
    connect(gameSocket->getSocket(), &QTcpSocket::disconnected, this, [=](){
        stopConnection(gameSocket);
    });
}

void MainClass::stopConnection(GameSocket* socket)
{
    qDebug() << "stop connection start";
    connectedSockets.removeAll(socket);
    scene->removeItem(socket->getPlayer());
    socket->deleteLater();
    qDebug() << "stop connection stop";
}
