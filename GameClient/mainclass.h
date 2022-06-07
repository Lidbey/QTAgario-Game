#ifndef MAINCLASS_H
#define MAINCLASS_H


#include <QGraphicsScene>
#include <QTimer>
#include "player.h"
#include <QWidget>
#include <QSet>
#include <QTcpSocket>
#include "dot.h"

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
    QTimer timer;
    QTimer timerSpace;
    QTimer timerWClicked;
    void sendData();
    QVector<QGraphicsItem*> items;
    QList<Dot*> dots;
    double diffx;
    double diffy;
    double botDiffx;
    double botDiffy;
    bool wClicked;
    bool space;
    bool mouseClick;
    int id;
    QTcpSocket socket;
    QString buffer="";
    bool end=false;

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
