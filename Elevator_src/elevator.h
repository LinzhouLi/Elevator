#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "mainwindow.h"
#include <vector>

#include <QObject>
#include <QtConcurrent>
#include <QFuture>
#include <QPushButton>
#include <QLCDNumber>
#include <QComboBox>
#include <QLabel>
#include <QTimer>

#define OUTOFWAY 100 //定义不顺路
#define INFINITE 200 //定义无穷大

#define FLOOR_MAX 20
#define ELEVATOR_NUM 5
#define DOOR_OPEN_TIME 3

#define STANDSTILL 0
#define GOING_UP 1
#define GOING_DOWN -1
#define BROKENDOWN 200

#define DOOR_OPEN 0
#define DOOR_CLOSE 1

class Elevator : public QObject
{
    Q_OBJECT
    friend class ElevatorControl;
private:
    int runStatus, nowFloor, doorStatus;//上下行状态、当前楼层、电梯门状态
    std::vector<int> * msgQueue,* msgQueueReverse;//消息队列与逆向消息队列

    QPushButton *btn[FLOOR_MAX],* closebtn,* openbtn,* alert;//开门键,关门键,报警键
    QLCDNumber * lcd;
    QLabel * eleStatus;
    QEventLoop doorOpenThread;//开门线程
    QFuture<void> eleThread;//电梯运行线程
    void addMsg(int);//将楼层信息加入消息队列
    void addMsgReverse(int);//将楼层信息加入反向消息队列
    void delNowfloor();//将楼层信息从消息队列中去除
public:
    //explicit Elevator(QObject *parent = nullptr);
    Elevator();
    ~Elevator();
    void initUI(int, int, MainWindow*);//绘制一个电梯的图形界面
    void run(std::vector<int> *);//电梯运行线程函数
private slots:
    void clickResponse(int);//响应电梯内按钮
    void openDoor(int);//打开电梯门
    void eleAlert(MainWindow*, int);//响应报警按钮
};

class ElevatorControl : public QObject
{
    Q_OBJECT
    friend class ElevatorRun;
private:
    Elevator * ele[ELEVATOR_NUM];//5部电梯
    QLabel *label;//说明文字
    QComboBox *comboBox;//选择楼层combobox
    QPushButton *upButton, *downButton;//上行键、下行键
    QTimer *timer;//定时器
    std::vector<int> * waitQueue;
public:
    ElevatorControl(MainWindow *);
    ~ElevatorControl();
    void initUI(MainWindow *);//绘制外部界面UI
    void initEle(MainWindow *);//绘制每个电梯UI
private slots:
    void upDispatch(int);//响应上行按钮，调度电梯
    void downDispatch(int);//响应下行按钮，调度电梯
    void updateEle();//刷新，唤醒电梯运行线程
};

#endif // ELEVATOR_H
