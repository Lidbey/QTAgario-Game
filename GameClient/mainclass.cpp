#include "mainclass.h"
#include "ui_mainclass.h"
#include <QKeyEvent>
#include <QDebug>
#include "DEFINES.h"
#include "dot.h"
#include <QtMath>
#include <QHostAddress>


MainClass::MainClass(QString address, int port, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainClass)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    connect(&socket, &QTcpSocket::readyRead, this, &MainClass::gatherData);
    socket.connectToHost(QHostAddress(address),port);

    scene=new QGraphicsScene();
    scene->setSceneRect(0, 0, RATIO*WIDTH, RATIO*HEIGHT);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    timer.start(25);
    connect(&timer, &QTimer::timeout, this, &MainClass::sendData);
}

MainClass::~MainClass()
{
    delete ui;
}

bool MainClass::eventFilter(QObject* target, QEvent* event)
{
    if(event->type()==QEvent::KeyPress)
    {
        if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Space)
        {
            space = true;
        }
    }
    else if(event->type()==QEvent::KeyRelease)
    {
        if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Space)
        {
            space = false;
        }
    }
    return QWidget::eventFilter(target, event);
}

void MainClass::sendData()
{
    Player* player = nullptr;
    for(int i=0; i<players.length(); i++)
    {
        if(players[i]->getId()==this->id)
        {
            player=players[i];
        }
    }
    if(!player) return;
    QRectF rect(player->pos().x()-WIDTH/2-player->getSqrtSize()/3, player->pos().y()-HEIGHT/2-player->getSqrtSize()/3, WIDTH+player->getSqrtSize()*2/3, HEIGHT+player->getSqrtSize()*2/3);
    ui->graphicsView->setSceneRect(rect);
    ui->graphicsView->centerOn(rect.center());
    ui->graphicsView->fitInView(rect, Qt::AspectRatioMode::KeepAspectRatio);

    QPoint pos = ui->graphicsView->mapFromGlobal(QCursor::pos());
    QPointF relativePos = ui->graphicsView->mapToScene(pos);
    int diffx = relativePos.x() - player->pos().x();
    int diffy = relativePos.y() - player->pos().y();
    if(abs(diffx)+abs(diffy) < 0.01)
    {
        sendState(space, 0, 0);
        return;
    }
    double scaleFactor = sqrt(diffx*diffx+diffy*diffy);
    if(scaleFactor > 100)
        sendState(space, (double)diffx/scaleFactor, (double)diffy/scaleFactor);
    else
        sendState(space, (double)diffx/100, (double)diffy/100);

}

qreal MainClass::calcDistance(QPointF a, QPointF b)
{
    return qSqrt((a.x()-b.x())*(a.x()-b.x())+(a.y()-b.y())*(a.y()-b.y()));
}

void MainClass::gatherData()
{
    buffer+=socket.readAll();
    if(buffer.contains("ID") && buffer.contains("\r\n"))
    {
        QStringList list = buffer.split("\r\n");
        for(int i=0; i<list.length(); i++)
        {
            if(list[i].contains("ID"))
            {
                list[i].remove("ID: ");
                this->id = list[i].toInt();
                list.removeAt(i);
                qDebug() << "found myself\n";
                break;
            }
        }
        buffer = list.join("\r\n");
    }
    if(buffer.contains("PPPP") && buffer.contains("OOOO") && buffer.contains("DDDD") && buffer.contains("KKKK"))
    {
        int from = buffer.indexOf("PPPP");
        int to = buffer.indexOf("KKKK");
        QString data = buffer.mid(from+4, to);
        buffer ="";
        QStringList values = data.split("OOOODDDD");
        if(values.length() < 2) return;
        QString playerData = values[0];
        QString dotData = values[1];
        analyzePlayerData(playerData);
        analyzeDotData(dotData);
    }

}

void MainClass::sendState(bool space, double diffx, double diffy)
{
    socket.write((QString::number(diffx)+";"+QString::number(diffy)+";" + (space?"1":"0")+"\r\n").toUtf8());
}

void MainClass::analyzeDotData(QString data)
{
    QStringList dots = data.split("\t");
    for(int i=0; i<dots.length(); i++)
    {
        QStringList splitted = dots[i].split(';');
        if(splitted.length() != 2) return;
        Dot* dot = new Dot();
        scene->addItem(dot);
        dot->setPos(splitted[0].toDouble(), splitted[1].toDouble());
    }
    scene->update();
}

void MainClass::analyzePlayerData(QString data)
{
    for(int i=0; i<players.length(); i++)
    {
        players[i]->deleteLater();
    }
    players.clear();
    scene->clear();
    QStringList players = data.split("\t");
    for(int i=0; i<players.length(); i++)
    {
        QStringList splitted = players[i].split(';');
        if(splitted.length() != 4) return;
        Player* player = new Player(splitted[0].toInt());
        this->players.append(player);
        scene->addItem(player);
        player->setPos(splitted[1].toDouble(), splitted[2].toDouble());
        player->setSize(splitted[3].toDouble());
    }
}
