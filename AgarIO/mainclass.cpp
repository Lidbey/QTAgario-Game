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

    //startuje serwer(zwraca true/false, ale narazie to ignoruje, pozniej mozna ogarnac aby wyskoczyl box gdy false)
    server.listen(QHostAddress::Any, port);

    //tworze scene
    scene=new QGraphicsScene();

    //wielkosc sceny
    scene->setSceneRect(0, 0, RATIO*WIDTH, RATIO*HEIGHT);

    //ustawianie parametrow sceny
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->scale(1.30/RATIO, 1.30/RATIO);

    //tworzenie krop poczatkowych i dodawanie na scene
    for(int i=0; i<START_DOTS; i++)
    {
        Dot* dot=new Dot();
        scene->addItem(dot);
        dot->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
        dotsData << dot->pos();
    }

    //czas trwania petli programu (25ms aktualnie)
    timer.start(25);
    //i sygnal aby GameLoop byl wywolywany co 25ms
    connect(&timer, &QTimer::timeout, this, &MainClass::gameLoop);
    //sygnal ktory wywoluje newConnection gdy ktos sie podlaczy do serwera
    connect(&server, &QTcpServer::newConnection, this, &MainClass::newConnection);
}

MainClass::~MainClass()
{
    delete ui;
}


//petla programu(wywolywana co n ms)
void MainClass::gameLoop()
{

    //dla kazdego polaczonego socketa:
    for(int i=connectedSockets.length()-1; i>=0; i--)
    {
        //pobierz gracza od polaczonego socketa
        Player* player = connectedSockets[i]->getPlayer();
        if(player->dead())
        {
            connectedSockets[i]->getSocket()->disconnectFromHost();
            continue;
        }
        //jesli akcja jest poprawna to ja wykonaj
        if(connectedSockets[i]->actionValid())
        {
            connectedSockets[i]->doAction();
        }

        //2 razy na 200 iteracji wywolaj player->minusSize() czyli po prostu obniz mu wielkosc
        if(rand()%200<2)
        {
            player->minusSize();
        }

        //zupdatuj gracza na scenie
        player->update();
    }

    //kolizje
    for(int j=0; j<connectedSockets.length(); j++)
    {
        Player* player = connectedSockets[j]->getPlayer();
        if(!player || player->dead()) continue;
        QList<QGraphicsItem*> colliding=scene->collidingItems(player);

        for(int i=0; i<colliding.length(); i++)
        {
            //kolizja z kropkami
            if(dynamic_cast<Dot*>(colliding[i])!=nullptr)
            {
                dotsData.removeOne(colliding[i]->pos());
                scene->removeItem(colliding[i]);
                delete colliding[i];
                player->addSize();
            }

            //kolizja z graczami
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

    // tworzenie nowej kropki
    if(rand()%100<10)
    {
        if(scene->items().length()<5000)
        {
            Dot* dot=new Dot();
            scene->addItem(dot);
            dot->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
            dotsData << dot->pos();
            //sprawdzanie czy kropka koliduje z graczami, jesli tak to usun ja
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

    //wysylanie stanu gry do kazdego gracza
    QString gameState = this->prepareGameState();
    for(GameSocket* socket: connectedSockets)
    {
        socket->sendState(gameState);
    }
}

//liczenie dystansu miedzy graczami
qreal MainClass::calcDistance(QPointF a, QPointF b)
{
    return qSqrt((a.x()-b.x())*(a.x()-b.x())+(a.y()-b.y())*(a.y()-b.y()));
}


//kazdy gracz dostaje:
//Gracze: ID;X;Y;SIZE
//Kule: X;Y
//PPPP<-GRACZE->OOOO
//DDDD<-KULE->KKKK
//PPPP1;12.23;34.54;30  2;34.34;50.50;40    ...OOOO
//DDDD2.5;4.5   3.8;4.9 ...KKKK
//8B
QString MainClass::prepareGameState()
{
    //przygotowanie danych graczy
    QString playerData = "PPPP";
    for(GameSocket* socket: connectedSockets)
    {
        QPointF pos = socket->getPlayer()->pos();
        playerData+=QString::number(socket->getId())+";"+QString::number(pos.x())+";"+QString::number(pos.y())+";"+QString::number(socket->getPlayer()->getSize())+"\t";
    }
    playerData += "OOOO";

    //przygotowanie danych kropek
    QString dotData = "DDDD";
    for(QPointF point: dotsData)
    {
        dotData+=QString::number(point.x())+";"+QString::number(point.y())+"\t";
    }
    dotData += "KKKK";
    return playerData+dotData;
}

//obsluga nowego polaczenia do serwera
void MainClass::newConnection()
{
    QTcpSocket* socket = server.nextPendingConnection();
    int id = this->playerId++;
    socket->write(("ID: "+QString::number(id)+"\r\n").toUtf8()); // "ID: 3\r\n"
    GameSocket* gameSocket = new GameSocket(id, socket);
    connectedSockets.append(gameSocket);
    scene->addItem(gameSocket->getPlayer());
    gameSocket->getPlayer()->setPos(rand()%(RATIO*WIDTH), rand()%(RATIO*HEIGHT));
    connect(gameSocket->getSocket(), &QTcpSocket::disconnected, this, [=](){
        stopConnection(gameSocket);
    });
}

//gdy ktos sie rozlaczy z serwera (tutaj jest wyciek pamieci gdzies)
void MainClass::stopConnection(GameSocket* socket)
{
    qDebug() << "stop connection start";
    connectedSockets.removeAll(socket);
    scene->removeItem(socket->getPlayer());
    socket->deleteLater();
    qDebug() << "stop connection stop";
}