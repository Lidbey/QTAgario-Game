#include "mainclass.h"
#include "ui_mainclass.h"
#include <QKeyEvent>
#include <QDebug>
#include "DEFINES.h"
#include "dot.h"
#include <QtMath>
#include <QHostAddress>
#include <QMessageBox>

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

    ui->graphicsView->setStyleSheet("background-image: url(\"./BG.jpg\")");

    //zalacz petle programu na 25ms
    timer.start(25);
    timerSpace.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &MainClass::sendData);
    connect(&socket, &QTcpSocket::disconnected, this, [=](){
        this->disconnect();
        QMessageBox::information(this, "Agar.IO", "Game lost!", QMessageBox::Ok);
        qApp->quit();
    });
}

MainClass::~MainClass()
{
    if(socket.isOpen())
        socket.disconnectFromHost();
    delete ui;
}

//zapisuj nacisniecie spacji
bool MainClass::eventFilter(QObject* target, QEvent* event)
{
    if(event->type()==QEvent::KeyPress)
    {
        if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Space)
        {
            if(!timerSpace.isActive()) space = true;
        }
        else if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_W)
        {
             if(!timerWClicked.isActive()) wClicked = true;
        }
    }
    else if(event->type()==QEvent::KeyRelease)
    {
        if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Space)
        {
        //    space = false;
        }
        else if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_W)
        {
      //      wClicked = false;
        }
    }
    else if(event->type() == QEvent::MouseButtonPress)
    {
        if(static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
        {
            mouseClick = true;
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        if(static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
        {
            mouseClick = false;
        }
    }
    return QWidget::eventFilter(target, event);
}

//wyslij dane(wywoluje sie co 25ms)
void MainClass::sendData()
{
    //szukam siebie w calej liscie graczy ktora mam
    int id = searchForPlayer(this->id);
    if(id==-1) return;
    Player* player = players[id];

    //a tutaj tworz wektor normalny, najpierw pozycje kursora, pozniej odejmij pozycje gracza
    QPoint pos = ui->graphicsView->mapFromGlobal(QCursor::pos());
    QPointF relativePos = ui->graphicsView->mapToScene(pos);
    int diffx = relativePos.x() - player->pos().x();
    int diffy = relativePos.y() - player->pos().y();

    botDiffx = relativePos.x();
    botDiffy = relativePos.y();

    //takie troche uproszczenie, aby dalo sie zatrzymac w miejscu gdy myszka jest naprawde blisko srodka
    if(abs(diffx)+abs(diffy) < 0.05)
    {
        sendState(mouseClick, space, 0, 0);
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
        sendState(mouseClick, space, (double)diffx/scaleFactor, (double)diffy/scaleFactor);
    //mimo wszystko, nie chce zawsze jechac na 'maxa',
    //wiec jezeli odleglosc od kursora do srodka kuli to mniej niz 100, to skaluj o wiecej (np jesli odleglosc to 50, to dziel przez 100 wiec poruszaj sie mimo wsystko 2 razy wolniej)
    else
        sendState(mouseClick, space, (double)diffx/100, (double)diffy/100);

}


//liczenie dystansu
qreal MainClass::calcDistance(QPointF a, QPointF b)
{
    return qSqrt((a.x()-b.x())*(a.x()-b.x())+(a.y()-b.y())*(a.y()-b.y()));
}


//pobranie danych z serwera
void MainClass::gatherData()
{
    qDebug() << buffer;
    //znowu sytuacja z buforem, czytaj serwer tam wytlumaczone
    buffer+=socket.readAll();
    qDebug() << buffer;
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


    //jesli bufor ma te PPPP i OOOO i DDD i KKKK to znaczy, ze przyszedl stan gry
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

    if(buffer.contains("C: "))
    {
        QStringList dataPlayerAndDot = buffer.split("\r\n");
        if(buffer.size()<3) return;
        dataPlayerAndDot[0].remove("C: ");
        QStringList data = dataPlayerAndDot[0].split("\t");
        buffer = "";
        for(QString& item: data)
        {
            QStringList itemData = item.split(";");
            if(itemData.length()!=2) continue;
            if(itemData[1]=="1")
            {
                dots[itemData[0].toInt()]->enable();
                scene->addItem(dots[itemData[0].toInt()]);
            }
            else
            {
                dots[itemData[0].toInt()]->disable();
                scene->removeItem(dots[itemData[0].toInt()]);
            }
        }

        dataPlayerAndDot[1].remove("PPPP").remove("OOOO");
        QString playerData = dataPlayerAndDot[1];
        analyzePlayerData(playerData);

    }
}

//wyslij stan w formie
//Vx;Vy;S;D\r\n (nie wiem czy wczesniej gdzies to pisalem, kazda wiadomosc koncze \r\n abym wiedzial jak je rozdzielic pozniej)
void MainClass::sendState(bool mouseClick, bool space, double diffx, double diffy)
{
    socket.write((QString::number(diffx) + ";"+QString::number(diffy) + ";" + (mouseClick?"1":"0")+";" + (space?"1":"0") + "\r\n").toUtf8());
    if(this->space) {
        this->space = false;
        timerSpace.start(3000);
    }
    if(this->wClicked) {
        this->wClicked = false;
        timerWClicked.start(3000);
        socket.write(("T;1;" + QString::number(this->botDiffx) + ";" + QString::number(this->botDiffy) + "\r\n").toUtf8());
    }
}

//analizuj dane z kropek (te z funkcji gatherData)
void MainClass::analyzeDotData(QString data)
{
    //kropki sa porozdzielane znakiem \t wiec stworz liste
    QStringList dotsData = data.split("\t");
    //i dla kazdego elementu listy:
    for(int i=0; i<dotsData.length(); i++)
    {
        //rozdziel teraz znakiem ';' bo tak sa rozdzielone dane kropki (x;y)
        QStringList splitted = dotsData[i].split(';');
        if(splitted.length() != 3) return;
        //stworz kropke, dodaj na scene, ustaw wspolrzedne)
        Dot* dot = new Dot();
        dot->setPos(splitted[0].toDouble(), splitted[1].toDouble());
        if(splitted[2]=="1")
        {
            dot->enable();
            scene->addItem(dot);
        }
        else
        {
            dot->disable();
            scene->removeItem(dot);
        }
        dots << dot;
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
    //i przeanalizuj dane ktore przyszly,
    //poszczegolni gracze rozdzieleni znakami \t, a dane rozdzielone znakiem ';' i znow to samo co z kropkami
    QStringList players = data.split("\t");
    for(int i=0; i<players.length(); i++)
    {
        QStringList splitted = players[i].split(';');
        if(splitted.length() != 5) continue;
        Player* player = new Player(splitted[0].toInt());
        this->players.append(player);
        scene->addItem(player);
        player->setPos(splitted[1].toDouble(), splitted[2].toDouble());
        player->setSize(splitted[3].toDouble());
        player->setId(splitted[4].toInt());
    }

    int id = searchForPlayer(this->id);
    if(id==-1) return;
    Player* player = this->players[id];
    QRectF rect(player->pos().x()-WIDTH/2-player->getSqrtSize()/3, player->pos().y()-HEIGHT/2-player->getSqrtSize()/3, WIDTH+player->getSqrtSize()*2/3, HEIGHT+player->getSqrtSize()*2/3);
    ui->graphicsView->setSceneRect(rect);
    ui->graphicsView->centerOn(rect.center());
    ui->graphicsView->fitInView(rect, Qt::AspectRatioMode::KeepAspectRatio);
}

int MainClass::searchForPlayer(int id)
{
    for(int i=0; i<players.length(); i++)
    {
        if(players[i]->getId()==id)
            return i;
    }
    return -1;
}
