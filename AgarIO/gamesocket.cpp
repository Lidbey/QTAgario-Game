#include "gamesocket.h"


//obiekt gracza posiadajacy socketa i item gracza ktory jest na scenie,
//ten obiekt jest po stronie serwera, ale nalezy do gracza (taki byl zamysl)
GameSocket::GameSocket(int id, QTcpSocket* socket, QObject *parent) : QObject(parent)
{
    player = new Player(id, this);
    movex=0;
    movey=0;
    sprint=false;
    this->id = id;
    this->socket = socket;
    connect(socket, &QTcpSocket::readyRead, this, &GameSocket::read);
    connect(socket, &QTcpSocket::disconnected, this, [=](){
        disconnected = true;
    });
}

GameSocket::~GameSocket()
{
    if(socket->isOpen())
        socket->disconnectFromHost();
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

//sprawdzanie czy akcja jest poprawna
//(movex i movey musza tworzyc wektor normalny tj taki ktorego dlugosc nie jest wieksza niz 1)
//dlatego z pitagorasa sqrt(movex^2+movey^2)<1, ale po co pierwiastek jak i tak wyjdzie bez
//dalem 1.1 aby pozwolic na lekkie zawieruszenia
bool GameSocket::actionValid()
{
    return (movex*movex+movey*movey <= 1.1);
}

//wykonaj akcje na itemie gracza na scenie
void GameSocket::doAction()
{
    this->player->action(sprint, movex, movey);
}

//wyslij stringa gameState do socketa
void GameSocket::sendState(QString gameState)
{
    socket->write(gameState.toUtf8());
}

int GameSocket::getId()
{
    return id;
}

bool GameSocket::isOk()
{
    return !player->dead() && socket->isOpen() && !disconnected;
}

//czytaj przychodzace dane, musi byc bufor bo tcp czasem ucina pakiety
//tj jakby przyszlo np. ...\r\nABC...DEF...GHI\r\nEJK
//to wiem, ze ABCEFGHI to jedna wiadomosc (bo jest pomiedzy kolejnymi \r\n)
//oraz wiem, ze EJK jest nowym elementem bufora do ktorego bede dalej doklejal jak bedzie cos przychodzic
void GameSocket::read()
{
    buffer += socket->readAll();
    qDebug() << buffer;
    if(buffer.contains("\r\n"))
    {
        QStringList list = buffer.split("\r\n");
        //ignoruje jesli dostalem 3 rzeczy na raz (np. abc def ghi e... to biore tylko ghi jako 'najswiezszy komunikat')
        //bo w sumie nie potrzebuje tego abc ani def, bo bede robil ghi (a nie pozwalam na zrobienie 3 rzeczy w jednej 'turze')
        if(list.length() > 1)
            buffer=list.last();
        //jesli nie ma nic
        if(list.isEmpty()) return;
        //splittuje po ';', bo dane przychodza w formie Vx;Vy;S
        //gdzie Vx to skladowa X wektora ruchu,
        //Vy skladowa Y wektora ruchu
        //S to 1 albo 0 w zaleznosci czy mam sprinta czy nie
        QStringList actions = list[0].split(';');
        if(actions.length() < 4) return;
        movex = actions[0].toDouble();
        movey = actions[1].toDouble();
        sprint = (actions[2] == "1");
        divide = (actions[3] == "1"); // ADDED - 1 jesli klinieta spacja, 0 jesli nie


        //w sumie tego chyba nie trzeba robic bo nic tu nie zmieniam
        player->update();
    }
}
