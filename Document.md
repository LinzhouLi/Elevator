# 电梯调度

## 目录

[toc]

## 项目需求

某栋大楼有20层，内有五部互联的电梯。基于线程思想，编写一个电梯调度程序。

## 功能描述

1. 每个电梯里设置必要的功能键，比如数字楼层键、关门键、开门键、报警键等，以及显示当前电梯的楼层数、上升下降状态以及开关门状态。
2. 每层楼每部电梯门口，有上行和下行按钮。
3. 每部电梯门口的按钮是互联的，即当一个电梯的按钮按下去时，其他按钮也会同时点亮，表示也按下去。
4. 所有电梯的初始状态都在一层，每个电梯如果在它的上层或者下层没有相应请求时，在原地保持不动。

## 开发环境

- **开发环境**：Windows10
- **开发软件**：
  1. Qt  *v5.15.2*
  2. Qt Creator *v4.15.0* (Community)
- **开发语言**：C++
- **主要引用模块**：
  1. Qt核心组件: QObject, QMainWindow, QCoreApplication
  2. QtWidgets: QMessageBox, QLabel, QComboBox, QLCDNumber, QPushButton
  3. Qt多线程: QtConcurrent, QFuture, QTimer
  4. STL标准库: vector

## 项目结构

|   Elevator.pro
|   
+---Headers
|   |   elevator.h
|   |   mainwindow.h
|   |   
+---Sources
|   |    elevator.cpp
|   |    main.cpp
|   |    mainwindow.cpp
|   |   
+---Forms
|   |   mainwindow.ui
|   |   
\\---Resources
      \\---release
            \\---image.qrc
                   \\---pic
                             alert.png
                             close.png
                             disabled.png
            				 dooropen.png
            				 down.png
            			 	downbtn.png
            				 logo.png
           				  open.png
      				       stop.png
   				          up.png
        				     upbtn.png

## 操作说明

- 双击运行`Elevator.exe`，进入电梯模拟系统。

<img src="./img\test1.png" width = "600" alt="test1" align=center />

- 点击每部电梯的功能键（开/关门键，报警按钮，楼层按钮），模拟电梯内部命令。

<img src="./img\test2.png" width = "600" alt="test2" align=center />

<img src="./img\test3.png" width = "600" alt="test3" align=center />

- 在上方下拉框中选择楼层，并点击旁边的上/下行按钮，模拟电梯外部命令。

<img src="./img\test4.png" width = "600" alt="test4" align=center />

## 系统分析

### 界面设计

<img src="./img\show.png" width = "600" alt="show" align=center />

### 状态设计

1. **电梯状态：**
   - `#define STANDSTILL 0` //停
   - `#define GOING_UP 1`//上行
   - `#define GOING_DOWN -1`//下行
   - `#define BROKENDOWN 200`//损坏
2. **电梯门状态：**
   - `#define DOOR_OPEN 0`//正在开门
   - `#define DOOR_CLOSE 1`//关门
3. **电梯信息：**
   - `#define FLOOR_MAX 20`//楼层数
   - `#define ELEVATOR_NUM 5`//电梯数
   - `#define DOOR_OPEN_TIME 3`//开门时间（秒）
4. **响应外部命令时可调度性：**
   - `#define INFINITE 200`//无穷大
   - `#define OUTOFWAY 100`//不顺路

### 类设计

<img src="./img\class.svg" width = "600" alt="class" align=center />

1. **MainWindow:** 界面显示

2. **Elevator:** 电梯运行，处理内部命令，电梯UI绘制

   ```c++
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
       void openDoor();//打开电梯门
       void eleAlert(MainWindow*, int);//响应报警按钮
   };
   ```

3. **ElevatorContral:** 电梯控制，处理外部命令及电梯报警，外部UI绘制

   ```c++
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
   ```

### 调度控制方式

- 五部电梯共有等待队列`waitQueue`存放五部电梯都不顺路的外部命令的楼层。
- 每部电梯有两个队列，
  - 正向队列`msgQueue`存放此电梯一趟运行的目标楼层（一趟运行指不改变方向运行结束）。
  - 反向队列`msgQueueReverse`存放下一趟运行（即下一个方向）的目标楼层。

## 电梯运行控制

### 报警

1. 将电梯运行状态`runStatus`设为`BROKENDOWN`。
2. 如果电梯线程正在运行，则等待其完成。
3. 禁用电梯内部的所有按钮。
4. 改变电梯运行状态标识图标，并发出警告信号。

```c++
void Elevator::eleAlert(MainWindow* win, int whichEle)
{
    //状态设为损坏
    runStatus = BROKENDOWN;

    //结束进程
    if(eleThread.isRunning())
        eleThread.waitForFinished();

    //禁用按钮
    for(int j = 0; j < FLOOR_MAX; j++)
        btn[j]->setDisabled(true);
    closebtn->setDisabled(true);
    openbtn->setDisabled(true);
    alert->setDisabled(true);

    //改变UI
    eleStatus->setStyleSheet("QLabel{image: url(:/pic/disabled.png);}");
    QMessageBox::warning(win, "警告", "电梯"+QString::number(whichEle+1)+"损坏",QMessageBox::Ok);
}
```

### 开门

1. 将电梯门状态`doorStatus`设为`DOOR_OPEN`，并改变电梯运行标识按钮。
2. 等待3秒
   - 如果`flag`为1，说明是电梯到达需要开门，则直接将**电梯线程**`eleThread`暂停3秒。
   - 如果`flag`为0，说明是响应开门按钮，则将单独的**开门线程**`doorOpenThread`运行3秒。
3. 将电梯门状态`doorStatus`设为`DOOR_CLOSE`。

```
void Elevator::openDoor(int flag = 0)
{
    doorStatus = DOOR_OPEN;
    eleStatus->setStyleSheet("QLabel{image: url(:/pic/dooropen.png);}");

    //等待3秒
    if(flag)
        QThread::sleep(DOOR_OPEN_TIME);
    else
    {
        QTimer::singleShot(DOOR_OPEN_TIME * 1000, &doorOpenThread, SLOT(quit()));
        doorOpenThread.exec();
    }

    doorStatus = DOOR_CLOSE;
}
```

### 响应内部按键

1. 如果按键层数和电梯目前层数相等，直接返回。
2. 将此按键设为**不可用**。
3. 判断电梯状态，
   - 如果电梯**上行**，
     - 如果按键楼层数**大于**电梯当前楼层，则将此楼层加入**正向队列**`msgQueue`。
     - 如果按键楼层数**小于**电梯当前楼层，则将此楼层加入**反向队列**`msgQueueReverse`。
   - 如果电梯**下行**，
     - 如果按键楼层数**小于**电梯当前楼层，则将此楼层加入**正向队列**`msgQueue`。
     - 如果按键楼层数**大于**电梯当前楼层，则将此楼层加入**反向队列**`msgQueueReverse`。
   - 如果电梯**静止**，将此楼层加入**正向队列**`msgQueue`。

```c++
void Elevator::clickResponse(int floor)
{
    //若按键层数和电梯目前层数相等，直接返回
    if(floor == nowFloor)return;

    //将按键设为不可用
    btn[floor]->setDisabled(true);
    btn[floor]->setStyleSheet("background-color: rgb(255, 150, 3);");//设置颜色

    switch(runStatus)//电梯正在上行
    {
        case GOING_UP:
        {
            if(floor > nowFloor)//按键层数大于电梯目前的层数，加入队列
                addMsg(floor);
            else//按键层数小于电梯目前的层数,加入反向队列
                addMsgReverse(floor);
        }break;
        case GOING_DOWN://电梯正在下行
        {
            if(floor > nowFloor)//按键层数大于电梯目前的层数，加入反向队列
                addMsgReverse(floor);
            else//按键层数小于电梯目前的层数,加入队列
                addMsg(floor);
        }break;
        case STANDSTILL://电梯静止
            addMsg(floor);
    }
}
```

### 响应外部按键（外部调度）

**以响应上行按键为例（响应下行按键同理）：**

1. 如果在最高层，直接返回。

2. 计算每部电梯的“可调度性”`score`，其定义如下：
   $$
   score=\left\{\begin{array}\
   INFINITE&  &电梯损坏\\
   |floor-nowfloor|&  &电梯静止\\
   floor-nowfloor&  &电梯上行且nowfloor<floor\\
   OUTOFWAY&  &电梯上行且nowfloor>floor\\
   floor-nowfloor&  &电梯下行且minfloor>floor\\
   -(nowfloor+floor-2minfloor)&  &电梯下行且minfloor<floor
   \end{array}\right.
   $$

   $$
   \begin{array}\
   注：\\
   \;\;\;\;\;floor:按上行键的楼层数\\
   \;\;\;\;\;nowfloor:此电梯目前所在的楼层数\\
   \;\;\;\;\;minfloor:此电梯正向队列的第一个元素，即此电梯下行目标楼层中的最底层\\
   \;\;\;\;\;IFINITE=200\;代表无穷大\\
   \;\;\;\;\;OUTFOWAY=100\;代表不顺路
   \end{array}
   $$

3. 从五部电梯中选出“最佳可调度性”`bestScore`(可调度性的绝对值最小)，以及对应的电梯。

   - 如果`bestScore == INFINITE`，说明电梯全部损坏，直接返回。
   - 如果`bestScore == OUTOFWAY`，说明电梯全部上行且不顺路，将此楼层加入公共等待队列`waitQueue`。
   - 如果`bestScore < 0`，则说明最佳电梯需要逆行，将此楼层加入最佳电梯的反向队列`msgQueueReverse`。
   - 如果`bestScore > 0`，则直接将此楼层加入最佳电梯的正向队列`msgQueue`。

```c++
void ElevatorControl::upDispatch(int floor)
{
    if(floor == FLOOR_MAX)return;//如果在最高层，直接返回
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
                    ele[i]->openDoor(1);
                    return;
                }
                else if(score <= 0)//如果电梯在这一层之上，则不顺路
                    score = OUTOFWAY;
            }break;

            case STANDSTILL://第i个电梯静止
            {
                if(score == 0)//电梯就在这一层原地不动
                {
                    ele[i]->openDoor(1);
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
```

### 电梯运行（内部控制逻辑，线程函数）

1. 如果电梯损坏，直接返回，**退出线程**（电梯损坏状态相当于**线程打断**标志位）。
2. 如果电梯门打开，则跳过这次循环，直到门关闭。
3. 如果电梯**正向队列非空**，
   1. 如果电梯到达正向队列中的目标楼层，则将此楼层从队列中删除。
   2. 如果**楼层删除后**，
      - **正向队列为空**，说明**电梯上/下运行一趟结束**，将电梯运行状态置为停止，
        - 如果**反向消息队列非空**，**公共等待队列非空**，则将反向队列加入正向队列，等待队列加入反向队列，等待队列置空。
        - 如果**反向消息队列非空**，**公共等待队列为空**，则将反向队列加入正向队列，反向队列置空。
        - 如果**反向消息队列为空**，**公共等待队列非空**，则将等待队列加入正向队列，等待队列置空。
        - 如果**反向消息队列为空**，**公共等待队列为空**，则说明电梯已没有目标楼层，直接返回，**退出线程**。
      - **正向队列非空**，说明**电梯上/下运行一趟未结束**或需要**从停止状态开始运行**，
        1. 如果**电梯停止**，根据正向队列中的目标楼层设置电梯运行状态。
        2. 更新电梯状态图标，并禁用开门键。
        3. 改变电梯当前楼层，使线程等待2秒，最后改变LCD楼层显示。
4. 循环，返回第1步。

```c++
void Elevator::run(std::vector<int> * waitQueue)
{
    while(1)
    {
        if(runStatus == BROKENDOWN)
            return;
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
```

### 多线程实现

在主线程中，每0.1秒调用一次刷新函数`updateEle()`，在刷新函数中遍历5部电梯，找出不在开门或损坏或运行状态，并且需要启动（正向队列中有目标楼层）的电梯。分别启动这些电梯的线程`eleThread`，并在线程上运行函数`run()`，从而实现多线程。

```c++
void ElevatorControl::updateEle()
{
    for(int i = 0; i < ELEVATOR_NUM; i++)
    {
        //电梯正在运行或开门或已损坏，则直接跳过
        if(ele[i]->eleThread.isRunning() || ele[i]->doorStatus == DOOR_OPEN || ele[i]->runStatus == BROKENDOWN)
            continue;
        if(!ele[i]->msgQueue->empty())
            ele[i]->eleThread = QtConcurrent::run(ele[i], &Elevator::run, waitQueue);//多线程实现
        else
            ele[i]->eleStatus->setStyleSheet("QLabel{image: url(:/pic/stop.png);}");
    }
}
```

