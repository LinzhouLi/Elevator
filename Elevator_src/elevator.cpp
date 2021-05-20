#include "elevator.h"
#include "mainwindow.h"
#include "moc_elevator.cpp"
#include <QCoreApplication>
#include <QMessageBox>
#include <vector>

// Elevator

Elevator::Elevator()
{
    runStatus = STANDSTILL;//电梯运行状态
    nowFloor = 0;//当前楼层
    doorStatus = DOOR_CLOSE;//电梯门状态
    msgQueue = new std::vector<int>;
    msgQueueReverse = new std::vector<int>;
}

Elevator::~Elevator()
{
    delete lcd;
    delete eleStatus;
    delete closebtn;
    delete openbtn;
    delete msgQueue;
    delete msgQueueReverse;
    for(int i = 0; i < FLOOR_MAX; i++)
        delete btn[i];
    doorOpenThread.quit();
}

void Elevator::initUI(int pos, MainWindow* win)
{
    //显示楼层数码管
    lcd = new QLCDNumber(win);
    lcd->setGeometry(QRect(pos + 60, 120, 71, 51));
    lcd->setDigitCount(2);
    lcd->display(nowFloor + 1);
    //电梯状态显示
    eleStatus = new QLabel(win);
    eleStatus->setGeometry(QRect(pos, 120, 51, 51));
    eleStatus->setStyleSheet("QLabel{image: url(:/pic/stop.png);}");

    //按键
    int i, j,floor;
    for(i = 0; i < (FLOOR_MAX - 1) / 10 + 1; i++)
    {
        for(j = 0; j < 10; j++)
        {
            floor = j + i * 10;
            btn[floor] = new QPushButton(win);//楼层按键
            btn[floor]->setGeometry(QRect(pos + 80 * i, 200 + 70 * j, 51, 51));
            btn[floor]->setText(QString::number(floor + 1));
            //与槽函数链接
            connect(btn[floor], &QPushButton::clicked, this, [=](){clickResponse(floor);});
        }
    }

    //开门键关门键
    closebtn = new QPushButton(win);
    openbtn = new QPushButton(win);
    closebtn->setGeometry(QRect(pos, 900, 51, 51));
    openbtn->setGeometry(QRect(pos + 80, 900, 51, 51));
    closebtn->setStyleSheet(
                "QPushButton{image: url(:/pic/close.png); background: white;}");//开门按键
    openbtn->setStyleSheet(
                "QPushButton{image: url(:/pic/open.png); background: white;}");//关门按键

    connect(openbtn, &QPushButton::clicked, this, &Elevator::openDoor);
}

void Elevator::run(std::vector<int> * waitQueue)
{
    while(1)
    {
        if(runStatus == BROKENDOWN)
        {
            return;
        }
        if(doorStatus == DOOR_OPEN)
        {//如果门打开，则对此电梯不做操作，等待门关闭
            eleStatus->setStyleSheet(
                        "QLabel{image: url(:/pic/dooropen.png);}");
            continue;
        }

        //更新状态图标
        if(runStatus == STANDSTILL)
            eleStatus->setStyleSheet("QLabel{image: url(:/pic/stop.png);}");

        if(!msgQueue->empty())//如果电梯的消息队列非空
        {
            delNowfloor();

            //如果消息队列为空
            if(msgQueue->empty())
            {
                //恢复静止状态
                runStatus = STANDSTILL;

                if(!msgQueueReverse->empty())//反向消息队列不空
                {
                    //反向队列加入正向
                    *msgQueue = *msgQueueReverse;
                    msgQueueReverse->clear();
                    delNowfloor();
                    //qDebug()<<"msg "<<msgQueue->front()+1;

                    //阻塞队列不空
                    if(!waitQueue->empty())
                    {
                        *msgQueueReverse = *waitQueue;
                            waitQueue->clear();
                    }
                }
                else if(!waitQueue->empty())//反向消息队列为空,阻塞队列不空
                {
                    *msgQueue = *waitQueue;
                    waitQueue->clear();
                    delNowfloor();
                }
                else
                    return;

            }
            else//如果消息队列不空
            {
                if(runStatus == STANDSTILL)//电梯静止
                {
                    if(msgQueue->front() > nowFloor)//目标楼层在电梯之上
                        runStatus = GOING_UP;
                    else if(msgQueue->front() < nowFloor)//目标楼层在电梯之下
                        runStatus = GOING_DOWN;
                }

                if(runStatus != STANDSTILL)//电梯非静止
                {
                    //更新图标
                    switch(runStatus)
                    {
                    case GOING_UP: eleStatus->setStyleSheet(
                                "QLabel{image: url(:/pic/up.png);}");break;
                    case GOING_DOWN: eleStatus->setStyleSheet(
                                "QLabel{image: url(:/pic/down.png);}");break;
                    }

                    //电梯运行中禁用开门键
                    openbtn->setDisabled(true);

                    //电梯上升或下降
                    nowFloor = nowFloor + runStatus;
                    QThread::sleep(2);
                    lcd->display(nowFloor + 1);
                }
            }
        }
    }
}

void Elevator::delNowfloor()
{
    //如果到达楼层，从消息队列中去除
    if(msgQueue->front() == nowFloor)
    {
        msgQueue->front() = msgQueue->back();
        msgQueue->pop_back();
        std::sort(msgQueue->begin(), msgQueue->end());

        //按键恢复
        btn[nowFloor]->setEnabled(true);
        openbtn->setEnabled(true);
        btn[nowFloor]->setStyleSheet("");

        //电梯开门
        openDoor(1);
    }
    if(msgQueue->back() == nowFloor)
    {
        msgQueue->pop_back();

        //按键恢复
        btn[nowFloor]->setEnabled(true);
        openbtn->setEnabled(true);
        btn[nowFloor]->setStyleSheet("");

        //电梯开门
        openDoor(1);
    }
}

void Elevator::clickResponse(int floor)
{
    qDebug()<<"clicked"<<Qt::endl;
    //若按键层数和电梯目前层数相等，直接返回
    if(floor == nowFloor)return;

    //将按键设为不可用
    btn[floor]->setDisabled(true);
    btn[floor]->setStyleSheet("background-color: rgb(255, 150, 3);");//设置颜色

    if(runStatus == GOING_UP)//电梯正在上行
    {
        if(floor > nowFloor)//按键层数大于电梯目前的层数，加入队列
            addMsg(floor);
        else//按键层数小于电梯目前的层数,加入反向队列
            addMsgReverse(floor);
    }
    else if(runStatus == GOING_DOWN)//电梯正在下行
    {
        if(floor > nowFloor)//按键层数大于电梯目前的层数，加入反向队列
            addMsgReverse(floor);
        else//按键层数小于电梯目前的层数,加入队列
            addMsg(floor);
    }
    else//电梯静止
        addMsg(floor);
}

void Elevator::addMsg(int floor)
{
    qDebug()<<"addmsg:"<<floor+1<<Qt::endl;
    //若按键层数和电梯目前层数相等，直接返回
    if(floor == nowFloor)return;

    //若此楼层已在消息队列中，直接返回
    std::vector<int>::iterator fd;
    fd = std::find(msgQueue->begin(), msgQueue->end(), floor);
    if(fd != msgQueue->end()) return;
    //否则加入队列
    msgQueue->push_back(floor);
    std::sort(msgQueue->begin(), msgQueue->end());
}

void Elevator::addMsgReverse(int floor)
{
    qDebug()<<"addRmsg:"<<floor+1<<Qt::endl;
    //若按键层数和电梯目前层数相等，直接返回
    if(floor == nowFloor)return;

    //若此楼层已在反向消息队列中，直接返回
    std::vector<int>::iterator fd;
    fd = std::find(msgQueueReverse->begin(), msgQueueReverse->end(), floor);
    if(fd != msgQueueReverse->end()) return;
    //否则加入反向消息队列
    msgQueueReverse->push_back(floor);
    std::sort(msgQueueReverse->begin(), msgQueueReverse->end());
}

void Elevator::openDoor(int flag = 0)
{
    doorStatus = DOOR_OPEN;
    eleStatus->setStyleSheet("QLabel{image: url(:/pic/dooropen.png);}");
    //等待4秒

    if(flag)
        QThread::sleep(DOOR_OPEN_TIME);
    else
    {
        QTimer::singleShot(DOOR_OPEN_TIME * 1000, &doorOpenThread, SLOT(quit()));
        doorOpenThread.exec();
    }

    doorStatus = DOOR_CLOSE;
}



//ElevatorControl

ElevatorControl::ElevatorControl(MainWindow * win)
{
    initUI(win);//初始化外部控制界面UI
    initEle(win);//初始化5部电梯界面UI

    waitQueue = new std::vector<int>;
    //设置定时器
    timer = new QTimer(win);
    connect(timer, &QTimer::timeout, this, &ElevatorControl::updateEle);
    timer->start(100);
}

ElevatorControl::~ElevatorControl()
{
    delete label;
    delete comboBox;
    delete upButton;
    delete downButton;
    delete timer;
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        ele[i]->runStatus = BROKENDOWN;
        if(eleThread[i].isRunning())
            eleThread[i].waitForFinished();
        delete ele[i];
    }
    delete waitQueue;
}

void ElevatorControl::initUI(MainWindow * win)
{
    //设置选择楼层标签
    label = new QLabel(win);
    label->setGeometry(QRect(250, 30, 511, 71));
    QFont font;
    font.setPointSize(20);
    label->setFont(font);
    label->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\344\275\240\346\211\200\345\234\250\347\232\204\346\245\274\345\261\202\357\274\232", nullptr));

    //设置选择楼层菜单
    comboBox = new QComboBox(win);
    for(int i = 0; i < FLOOR_MAX; i++)
        comboBox->addItem(QString::number(i + 1));
    comboBox->setGeometry(QRect(740, 40, 121, 51));

    //设置上下行按键
    upButton = new QPushButton(win);
    upButton->setGeometry(QRect(890, 40, 51, 51));
    downButton = new QPushButton(win);
    downButton->setGeometry(QRect(960, 40, 51, 51));
    upButton->setStyleSheet(
                "QPushButton{image: url(:/pic/upbtn.png); background: white;}");
    downButton->setStyleSheet(
                "QPushButton{image: url(:/pic/downbtn.png); background: white;}");
    //与槽函数链接
    connect(upButton, &QPushButton::clicked, this, [=](){
        ElevatorControl::upDispatch(comboBox->currentIndex());});
    connect(downButton, &QPushButton::clicked, this, [=](){
        ElevatorControl::downDispatch(comboBox->currentIndex());});
}

void ElevatorControl::initEle(MainWindow * win)
{
    int pos = 0;
    for(int i = 0; i < ELEVATOR_NUM; i++)//设置电梯
    {
        pos = 70 + 240 * i;
        ele[i] = new Elevator;
        ele[i]->initUI(pos, win);

        //报警键
        alert[i] = new QPushButton(win);
        alert[i]->setGeometry(QRect(pos + 40, 980, 51, 51));
        alert[i]->setStyleSheet(
                    "QPushButton{image: url(:/pic/alert.png); background: white;}");
        connect(alert[i], &QPushButton::clicked, this, [=](){eleAlert(win,i);});
    }
}

void ElevatorControl::updateEle()
{
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        //电梯正在运行或开门或已损坏，则直接跳过
        if(eleThread[i].isRunning() || ele[i]->doorStatus == DOOR_OPEN || ele[i]->runStatus == BROKENDOWN)
            continue;
        if(!ele[i]->msgQueue->empty())
            eleThread[i] = QtConcurrent::run(ele[i], &Elevator::run, waitQueue);
        else
            ele[i]->eleStatus->setStyleSheet("QLabel{image: url(:/pic/stop.png);}");
    }

    //不使用多线程的刷新代码

    /*
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        if(ele[i]->doorStatus == DOOR_OPEN)
        {//如果门打开，则对此电梯不做操作，等待门关闭
            ele[i]->eleStatus->setStyleSheet(
                        "QLabel{image: url(:/pic/dooropen.png);}");
            continue;
        }

        //更新状态图标
        if(ele[i]->runStatus == STANDSTILL)
            ele[i]->eleStatus->setStyleSheet("QLabel{image: url(:/pic/stop.png);}");

        if(!ele[i]->msgQueue.empty())//如果第i个电梯的消息队列非空
        {
            //如果到达楼层，从消息队列中去除
            if(ele[i]->msgQueue.front() == ele[i]->nowFloor)
            {
                ele[i]->msgQueue.front() = ele[i]->msgQueue.back();
                ele[i]->msgQueue.pop_back();
                std::sort(ele[i]->msgQueue.begin(), ele[i]->msgQueue.end());

                //按键恢复
                ele[i]->btn[ele[i]->nowFloor]->setEnabled(true);
                ele[i]->openbtn->setEnabled(true);
                ele[i]->btn[ele[i]->nowFloor]->setStyleSheet("");

                //电梯开门
                ele[i]->openDoor();
            }
            if(ele[i]->msgQueue.back() == ele[i]->nowFloor)
            {
                ele[i]->msgQueue.pop_back();

                //按键恢复
                ele[i]->btn[ele[i]->nowFloor]->setEnabled(true);
                ele[i]->openbtn->setEnabled(true);
                ele[i]->btn[ele[i]->nowFloor]->setStyleSheet("");

                //电梯开门
                ele[i]->openDoor();
            }

            //如果消息队列为空
            if(ele[i]->msgQueue.empty())
            {
                qDebug()<<"empty"<<i+1<<Qt::endl;
                if(!ele[i]->msgQueueReverse.empty())//反向消息队列不空
                    {
                        qDebug()<<"reverse"<<i+1<<Qt::endl;
                        //反向队列加入正向
                        ele[i]->msgQueue = ele[i]->msgQueueReverse;
                        ele[i]->msgQueueReverse.clear();

                        //阻塞队列不空
                        if(!waitQueue.empty())
                        {
                            ele[i]->msgQueueReverse = waitQueue;
                            waitQueue.clear();
                        }
                    }
                else if(!waitQueue.empty())//反向消息队列为空,阻塞队列不空
                    {
                        ele[i]->msgQueue = waitQueue;
                        waitQueue.clear();
                    }
                //恢复静止状态
                ele[i]->runStatus = STANDSTILL;

            }
            else
            {
                if(ele[i]->runStatus == STANDSTILL)//电梯静止
                {
                    if(ele[i]->msgQueue.front() > ele[i]->nowFloor)//目标楼层在电梯之上
                        ele[i]->runStatus = GOING_UP;
                    else if(ele[i]->msgQueue.front() < ele[i]->nowFloor)//目标楼层在电梯之下
                        ele[i]->runStatus = GOING_DOWN;
                }

                if(ele[i]->runStatus != STANDSTILL)//电梯非静止
                {
                    //更新图标
                    switch(ele[i]->runStatus)
                    {
                    case GOING_UP: ele[i]->eleStatus->setStyleSheet(
                                "QLabel{image: url(:/pic/up.png);}");break;
                    case GOING_DOWN: ele[i]->eleStatus->setStyleSheet(
                                "QLabel{image: url(:/pic/down.png);}");break;
                    }

                    //电梯运行中禁用开门键
                    ele[i]->openbtn->setDisabled(true);

                    //电梯上升或下降
                    ele[i]->nowFloor = ele[i]->nowFloor + ele[i]->runStatus;
                    ele[i]->lcd->display(ele[i]->nowFloor + 1);
                }
            }
        }
    }*/
}

void ElevatorControl::upDispatch(int floor)
{
    int score = INFINITE, bestEle = 0, bestScore = 200;//可调度性,上一个电梯的可调度性，最佳电梯索引, 最佳可调度性
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        score = floor - ele[i]->nowFloor;
        switch(ele[i]->runStatus)
        {
            case BROKENDOWN://电梯损坏
            {
                score = INFINITE;
            }break;
            case GOING_UP://第i个电梯上行中
            {
                if(score == 0 && ele[i]->doorStatus == DOOR_OPEN)//如果电梯在这一层并且开门中，则继续开门
                {
                    ele[i]->openDoor();
                    return;
                }
                else if(score <= 0)//如果电梯在这一层之上，则不顺路
                    score = OUTOFWAY;
            }break;

            case STANDSTILL://第i个电梯静止
            {
                if(score == 0)//电梯就在这一层原地不动
                {
                    ele[i]->openDoor();
                    return;
                }
                score = abs(score);
            }break;

            case GOING_DOWN: //第i个电梯下行中
            {
                if(ele[i]->msgQueue->front() < floor)//下行电梯须逆行,score设为负数
                    score = -(ele[i]->nowFloor + floor - 2 * ele[i]->msgQueue->front());
            }
        }
        if( abs(score) < abs(bestScore) )//如果可调度性更小，则将其赋给最佳电梯
        {
            bestEle = i;
            bestScore = score;
        }
    }
    if(bestScore == INFINITE)return;//电梯全损坏
    if(bestScore == OUTOFWAY)//没有一个顺路电梯
        waitQueue->push_back(floor);
    else if (bestScore < 0)//有逆行顺路电梯
        ele[bestEle]->addMsgReverse(floor);
    else//有顺路电梯
        ele[bestEle]->addMsg(floor);
}

void ElevatorControl::downDispatch(int floor)
{
    int score = INFINITE, bestEle = 0, bestScore = 200;//可调度性,上一个电梯的可调度性，最佳电梯索引，最佳可调度性
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        score = ele[i]->nowFloor - floor;
        switch(ele[i]->runStatus)
        {
            case BROKENDOWN://电梯损坏
            {
                score = INFINITE;
            }break;
            case GOING_DOWN://第i个电梯下行中
            {
                if(score == 0 && ele[i]->doorStatus == DOOR_OPEN)//如果电梯在这一层并且开门中，则继续开门
                {
                    ele[i]->openDoor();
                    return;
                }
                else if(score <= 0)//否则不顺路
                    score = INFINITE;
            }break;
            case STANDSTILL://第i个电梯静止
            {
                if(score == 0)//电梯就在这一层原地不动
                {
                    ele[i]->openDoor();
                    return;
                }
                score = abs(score);
            }break;
            case GOING_UP: //第i个电梯上行中
            {
                if(ele[i]->msgQueue->back() > floor)//上行电梯须逆行,score设为负数
                    score = ele[i]->nowFloor + floor - 2 * ele[i]->msgQueue->back();
            }
        }
        if( abs(score) < abs(bestScore) )//如果可调度性更小，则将其赋给最佳电梯
        {
            bestEle = i;
            bestScore = score;
        }
    }
    if(bestScore == INFINITE)return;//电梯全损坏
    if(bestScore == OUTOFWAY)//没有一个顺路电梯
        waitQueue->push_back(floor);
    else if (bestScore < 0)//有逆行顺路电梯
        ele[bestEle]->addMsgReverse(floor);
    else//有顺路电梯
        ele[bestEle]->addMsg(floor);
}

void ElevatorControl::eleAlert(MainWindow* win, int i)
{
    //状态设为损坏
    ele[i]->runStatus = BROKENDOWN;

    //结束进程
    eleThread[i].cancel();
    eleThread[i].waitForFinished();

    //禁用按钮
    for(int j = 0; j < FLOOR_MAX; j++)
        ele[i]->btn[j]->setDisabled(true);
    ele[i]->closebtn->setDisabled(true);
    ele[i]->openbtn->setDisabled(true);

    //改变UI
    ele[i]->eleStatus->setStyleSheet("QLabel{image: url(:/pic/disabled.png);}");
    QMessageBox::warning(win, "警告", "电梯"+QString::number(i+1)+"损坏",QMessageBox::Ok);
}
