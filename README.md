# 电梯调度

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

```
│  Document.md
│  README.md
│
├─Elevator_src
│  │  elevator.cpp
│  │  elevator.h
│  │  Elevator.pro
│  │  Elevator.pro.user
│  │  image.qrc
│  │  logo.ico
│  │  main.cpp
│  │  mainwindow.cpp
│  │  mainwindow.h
│  │  mainwindow.ui
│  │
│  └─pic
│          alert.png
│          close.png
│          disabled.png
│          dooropen.png
│          down.png
│          downbtn.png
│          logo.png
│          open.png
│          stop.png
│          up.png
│          upbtn.png
│
└─img
        class.svg
        show.png
        test1.png
        test2.png
        test3.png
        test4.png
```

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
