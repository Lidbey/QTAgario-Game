#ifndef MAINCLASS_H
#define MAINCLASS_H


#include <QGraphicsScene>
#include <QTimer>
#include "player.h"
#include <QWidget>
#include <QSet>
#include <QTcpSocket>
#include "dot.h"
#include <QRandomGenerator>

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
    MainClass(QString address, int port, QWidget *parent = nullptr);
    ~MainClass();
    QGraphicsScene* scene;
    QList<Player*> players;
    int searchForPlayer(int id);
protected:
    bool eventFilter(QObject* target, QEvent* event);

private:
    Ui::MainClass *ui;
    QRandomGenerator rnd;
    QTimer timer;
    QTimer timerSpace;
    QTimer timerWClicked;
    QTimer timerQClicked;
    QVector<QGraphicsItem*> items;
    QList<Dot*> dots;
    double diffx;
    double diffy;
    double botDiffx;
    double botDiffy;
    bool wClicked=false;
    bool space=false;
    bool mouseClick=false;
    bool qClicked;
    int id;
    QTcpSocket socket;
    QString buffer="";
    bool end=false;

    void sendData();
    void sendState(bool mouseClick, bool space, double diffx, double diffy);
    void analyzeDotData(QString);
    void analyzePlayerData(QString);

    void setGraphicsView();
    void gatherData();

    qreal calcDistance(QPointF, QPointF);

signals:
    void clicked(QSet<int> keys, double movex, double movey);
};

#endif // MAINCLASS_H
