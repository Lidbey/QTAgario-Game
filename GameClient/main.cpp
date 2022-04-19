#include "mainclass.h"

#include <QApplication>
#include <QLineEdit>
#include <QObject>
#include <QInputDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString address = QInputDialog::getText(nullptr, ("IP and port"),
                                            ("IP:port"), QLineEdit::Normal);
    QStringList list = address.split(":");
    qDebug() << list[0] << list[1].toInt();
    MainClass w(list[0], list[1].toInt());
    w.show();//FullScreen();
    return a.exec();
}
