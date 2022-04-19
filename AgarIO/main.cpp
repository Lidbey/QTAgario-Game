#include "mainclass.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(argc<=1)
    {
        MainClass w(1099);
        w.show();
        return a.exec();
    }
    else
    {
        MainClass w(QString(argv[1]).toInt());
        w.show();
        return a.exec();
    }
}
