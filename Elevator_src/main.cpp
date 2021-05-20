#include "elevator.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Elevator Simulator");
    w.setWindowIcon(QIcon(":/pic/logo.png"));

    ElevatorControl *eleCtrl = new ElevatorControl(&w);
    w.show();
    int quit = a.exec();
    delete eleCtrl;
    return quit;
}
