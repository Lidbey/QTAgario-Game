#include "mainclass.h"
#include "ui_mainclass.h"
#include <QKeyEvent>
#include <QDebug>
#include "DEFINES.h"
#include "dot.h"
#include <QtMath>
#include <QHostAddress>



//glowna klasa programu
MainClass::MainClass(QString address, int port, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainClass)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);

    //jesli przyjdzei info z socketa to wywolaj metode gatherdata
    connect(&socket, &QTcpSocket::readyRead, this, &MainClass::gatherData);

    //polacz sie z hostem
    socket.connectToHost(QHostAddress(address),port);


    //ustaw scene
    scene=new QGraphicsScene();
    scene->setSceneRect(0, 0, RATIO*WIDTH, RATIO*HEIGHT);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //zalacz petle programu na 25ms
    timer.start(25);
    connect(&timer, &QTimer::timeout, this, &MainClass::sendData);
}

MainClass::~MainClass()
{
    delete ui;
}

//zapisuj nacisniecie spacji
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


//wyslij dane(wywoluje sie co 25ms)
void MainClass::sendData()
{

    //szukam siebie w calej liscie graczy ktora mam
    Player* player = nullptr;
    for(int i=0; i<players.length(); i++)
    {
        if(players[i]->getId()==this->id)
        {
            player=players[i];
        }
    }

    //jesli nie znalazlem to koniec (czemu moze mnie nie byc? - zabezpieczenie)
    if(!player) return;

    //ustaw widok scentrowany na mnie
    QRectF rect(player->pos().x()-WIDTH/2-player->getSqrtSize()/3, player->pos().y()-HEIGHT/2-player->getSqrtSize()/3, WIDTH+player->getSqrtSize()*2/3, HEIGHT+player->getSqrtSize()*2/3);
    ui->graphicsView->setSceneRect(rect);
    ui->graphicsView->centerOn(rect.center());
    ui->graphicsView->fitInView(rect, Qt::AspectRatioMode::KeepAspectRatio);

    //a tutaj tworz wektor normalny, najpierw pozycje kursora, pozniej odejmij pozycje gracza
    QPoint pos = ui->graphicsView->mapFromGlobal(QCursor::pos());
    QPointF relativePos = ui->graphicsView->mapToScene(pos);
    int diffx = relativePos.x() - player->pos().x();
    int diffy = relativePos.y() - player->pos().y();

    //takie troche uproszczenie, aby dalo sie zatrzymac w miejscu gdy myszka jest naprawde blisko srodka
    if(abs(diffx)+abs(diffy) < 0.01)
    {
        sendState(space, 0, 0);
        return;
    }

    //i mam wektor np. 10,-20, ale musze go znormalizowac aby w sumie wektorowej dawal odleglosc 1
    //wiec z matmy, jesli mam x^2+y^2=r^2, a chce tak zmniejszyc x i y abym mial (x/k)^2+(y/k)^2=1, to
    // (x^2+y^2)/r^2 = 1
    // x^2/r^2 + y^2/r^2 = 1
    // (x/r)^2+(y/r)^2=1, wiec x i y musze podzielic przez dlugosc wektora
    //a to dlugosc tego wektora
    double scaleFactor = sqrt(diffx*diffx+diffy*diffy);
    if(scaleFactor > 100)
        sendState(space, (double)diffx/scaleFactor, (double)diffy/scaleFactor);
    //mimo wszystko, nie chce zawsze jechac na 'maxa',
    //wiec jezeli odleglosc od kursora do srodka kuli to mniej niz 100, to skaluj o wiecej (np jesli odleglosc to 50, to dziel przez 100 wiec poruszaj sie mimo wsystko 2 razy wolniej)
    else
        sendState(space, (double)diffx/100, (double)diffy/100);

}


//liczenie dystansu
qreal MainClass::calcDistance(QPointF a, QPointF b)
{
    return qSqrt((a.x()-b.x())*(a.x()-b.x())+(a.y()-b.y())*(a.y()-b.y()));
}


//pobranie danych z serwera
void MainClass::gatherData()
{
    //znowu sytuacja z buforem, czytaj serwer tam wytlumaczone
    buffer+=socket.readAll();
    //jesli jest "ID" to znaczy, ze to pierwsza dana ktora przyszla a wiec informacja o tym, ktorym ID jest klient(bardzo wazne!)
    if(buffer.contains("ID") && buffer.contains("\r\n"))
    {
        QStringList list = buffer.split("\r\n");
        for(int i=0; i<list.length(); i++)
        {
            if(list[i].contains("ID"))
            {
                list[i].remove("ID: ");
                //i zapisanie tego ID po sformatowaniu
                this->id = list[i].toInt();
                list.removeAt(i);
                qDebug() << "found myself\n";
                break;
            }
        }
        buffer = list.join("\r\n");
    }


    //jesli bufor ma te PPPP i OOOO i DDD i KKKK to znaczy, ze mamy jeden kompletny stan
    if(buffer.contains("PPPP") && buffer.contains("OOOO") && buffer.contains("DDDD") && buffer.contains("KKKK"))
    {
        //no i tutaj formatowanie tego wszystkiego ...
        int from = buffer.indexOf("PPPP");
        int to = buffer.indexOf("KKKK");
        QString data = buffer.mid(from+4, to);
        buffer ="";
        QStringList values = data.split("OOOODDDD");
        if(values.length() < 2) return;
        QString playerData = values[0];
        QString dotData = values[1];
        //playerData to juz oczyszczone dane player'a (czyli wszystko pomiedzy PPPP i OOOO)
        //dotData to juz oczyszczone dane dot'ow (czyli wszystko pomiedzy DDDD i KKKK)
        analyzePlayerData(playerData);
        analyzeDotData(dotData);
    }

}

//wyslij stan w formie
//Vx;Vy;S\r\n (nie wiem czy wczesniej gdzies to pisalem, kazda wiadomosc koncze \r\n abym wiedzial jak je rozdzielic pozniej)
void MainClass::sendState(bool space, double diffx, double diffy)
{
    socket.write((QString::number(diffx)+";"+QString::number(diffy)+";" + (space?"1":"0")+"\r\n").toUtf8());
}

//analizuj dane z kropek (te z funkcji gatherData)
void MainClass::analyzeDotData(QString data)
{
    //kropki sa porozdzielane znakiem \t wiec stworz liste
    QStringList dots = data.split("\t");
    //i dla kazdego elementu listy:
    for(int i=0; i<dots.length(); i++)
    {
        //rozdziel teraz znakiem ';' bo tak sa rozdzielone dane kropki (x;y)
        QStringList splitted = dots[i].split(';');
        if(splitted.length() != 2) return;
        //stworz kropke, dodaj na scene, ustaw wspolrzedne)
        Dot* dot = new Dot();
        scene->addItem(dot);
        dot->setPos(splitted[0].toDouble(), splitted[1].toDouble());
    }
    //odswiez scene (dopiero po pelnej analizie wszystkich danych!!)
    scene->update();
}

//analiza danych gracza
void MainClass::analyzePlayerData(QString data)
{

    //usun graczy z sceny
    for(int i=0; i<players.length(); i++)
    {
        players[i]->deleteLater();
    }
    //wyczysc graczy z kontenera
    players.clear();
    //wyczysc scene
    scene->clear();
    //i przeanalizuj dane ktore przyszly,
    //poszczegolni gracze rozdzieleni znakami \t, a dane rozdzielone znakiem ';' i znow to samo co z kropkami
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