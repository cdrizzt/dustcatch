#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort>
#include <QFile>
#include <QFileDialog>

#include "serial.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

 void Timer_Init(QTimer *timer,void (Widget::*task)(void),uint16_t time);       //定时器初始化


 private slots:              //槽
     void time0_task(void);                 //定时器回调
     void time1_task(void);                 //定时器回调
     void Serial_Open(void);                //串口打开或关闭
     void data_analysis(void);
     void on_cave_box_clicked(bool checked);
     void version_button_clicked(bool checked);

private:
    Ui::Widget *ui;

       bool save_flag;
       bool begin;
       uint32_t time_cnt;
       uint32_t package_cnt;

       QTimer   *myTime_0;
       QTimer   *myTime_1;

       QStringList serialport_list;          //串口端口号
       QSerialPort *SerialPort;              //串口

       QFile    *save_file;                  //文件
       serial   station;                      //解码
};

#endif // WIDGET_H
