#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene=new QGraphicsScene();
    triangle=new Triangle();

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene->addItem(triangle);

    connect(this, &MainWindow::clicked, triangle, &Triangle::clickedSlot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
    if(event->type()==QEvent::KeyPress)
        emit clicked(static_cast<QKeyEvent*>(event)->key());
    return QMainWindow::eventFilter(target, event);
}

