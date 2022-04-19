#ifndef MAINCLASS_H
#define MAINCLASS_H


#include <QGraphicsScene>
#include <QTimer>
#include "player.h"
#include <QWidget>
#include <QSet>
#include <QTcpSocket>

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
protected:
    bool eventFilter(QObject* target, QEvent* event);

private:
    Ui::MainClass *ui;
    QTimer timer;
    void sendData();
    QVector<QGraphicsItem*> items;
    qreal calcDistance(QPointF, QPointF);
    double diffx;
    double diffy;
    bool space;
    int id;
    QTcpSocket socket;

    void setGraphicsView();
    void gatherData();
    QString buffer="";
    void sendState(bool space, double diffx, double diffy);

    void analyzeDotData(QString);
    void analyzePlayerData(QString);

signals:
    void clicked(QSet<int> keys, double movex, double movey);
};

#endif // MAINCLASS_H