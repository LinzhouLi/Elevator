# ���ݵ���

## ��Ŀ����

ĳ����¥��20�㣬�����岿�����ĵ��ݡ������߳�˼�룬��дһ�����ݵ��ȳ���

## ��������

1. ÿ�����������ñ�Ҫ�Ĺ��ܼ�����������¥��������ż������ż����������ȣ��Լ���ʾ��ǰ���ݵ�¥�����������½�״̬�Լ�������״̬��
2. ÿ��¥ÿ�������ſڣ������к����а�ť��
3. ÿ�������ſڵİ�ť�ǻ����ģ�����һ�����ݵİ�ť����ȥʱ��������ťҲ��ͬʱ��������ʾҲ����ȥ��
4. ���е��ݵĳ�ʼ״̬����һ�㣬ÿ����������������ϲ�����²�û����Ӧ����ʱ����ԭ�ر��ֲ�����

## ��������

- **��������**��Windows10
- **�������**��
  1. Qt  *v5.15.2*
  2. Qt Creator *v4.15.0* (Community)
- **��������**��C++
- **��Ҫ����ģ��**��
  1. Qt�������: QObject, QMainWindow, QCoreApplication
  2. QtWidgets: QMessageBox, QLabel, QComboBox, QLCDNumber, QPushButton
  3. Qt���߳�: QtConcurrent, QFuture, QTimer
  4. STL��׼��: vector

## ��Ŀ�ṹ

```
��  Document.md
��  README.md
��
����Elevator_src
��  ��  elevator.cpp
��  ��  elevator.h
��  ��  Elevator.pro
��  ��  Elevator.pro.user
��  ��  image.qrc
��  ��  logo.ico
��  ��  main.cpp
��  ��  mainwindow.cpp
��  ��  mainwindow.h
��  ��  mainwindow.ui
��  ��
��  ����pic
��          alert.png
��          close.png
��          disabled.png
��          dooropen.png
��          down.png
��          downbtn.png
��          logo.png
��          open.png
��          stop.png
��          up.png
��          upbtn.png
��
����img
        class.svg
        show.png
        test1.png
        test2.png
        test3.png
        test4.png
```

## ����˵��

- ˫������`Elevator.exe`���������ģ��ϵͳ��

<img src="./img\test1.png" width = "600" alt="test1" align=center />

- ���ÿ�����ݵĹ��ܼ�����/���ż���������ť��¥�㰴ť����ģ������ڲ����

<img src="./img\test2.png" width = "600" alt="test2" align=center />

<img src="./img\test3.png" width = "600" alt="test3" align=center />

- ���Ϸ���������ѡ��¥�㣬������Աߵ���/���а�ť��ģ������ⲿ���

<img src="./img\test4.png" width = "600" alt="test4" align=center />

## ϵͳ����

### �������

<img src="./img\show.png" width = "600" alt="show" align=center />

### ״̬���

1. **����״̬��**
   - `#define STANDSTILL 0` //ͣ
   - `#define GOING_UP 1`//����
   - `#define GOING_DOWN -1`//����
   - `#define BROKENDOWN 200`//��
2. **������״̬��**
   - `#define DOOR_OPEN 0`//���ڿ���
   - `#define DOOR_CLOSE 1`//����
3. **������Ϣ��**
   - `#define FLOOR_MAX 20`//¥����
   - `#define ELEVATOR_NUM 5`//������
   - `#define DOOR_OPEN_TIME 3`//����ʱ�䣨�룩
4. **��Ӧ�ⲿ����ʱ�ɵ����ԣ�**
   - `#define INFINITE 200`//�����
   - `#define OUTOFWAY 100`//��˳·

### �����

<img src="./img\class.svg" width = "600" alt="class" align=center />
