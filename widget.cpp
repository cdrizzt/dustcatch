#include "widget.h"
#include "ui_widget.h"
#include <QDebug>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    SerialPort = NULL;
    save_file = NULL;
    save_flag = false;
    time_cnt = 0;
    begin = false;
    Timer_Init(myTime_0,time0_task,1000);
    Timer_Init(myTime_1,time1_task,10);

    connect(ui->serial_button,&QPushButton::clicked,this,&Widget::Serial_Open);
    connect(ui->save_button,&QPushButton::clicked,this,&Widget::on_cave_box_clicked);
    connect(ui->version_button,&QPushButton::clicked,this,&Widget::version_button_clicked);
    connect(ui->fanopen_button,&QPushButton::clicked,this,&Widget::fanopen_button_clicked);
    connect(ui->fanclose_button,&QPushButton::clicked,this,&Widget::fanclose_button_clicked);

    connect(ui->debug_button,&QPushButton::clicked,this,&Widget::debug_button_clicked);

}

Widget::~Widget()
{
    delete ui;
}

//定时器初始化
void Widget::Timer_Init(QTimer *timer,void (Widget::*task)(void),uint16_t time)
{
    timer = new QTimer();
    timer->stop();
    timer->setInterval(time);
    connect(timer,&QTimer::timeout,this,task);
    timer->start();
}

//定时器任务
//端口检测
void Widget::time0_task(void)
{
    QStringList port_num;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        port_num += info.portName()+"  "+info.description();
    }
    if(port_num.size()!=serialport_list.size())
    {
        QStringList same;
        QStringList diff_now,diff_last;
        //比较出相同项及增加项
        for(uint8_t i=0;i<port_num.size();i++)
        {
            bool flag=false;
            for(uint8_t j=0;j<serialport_list.size();j++)
            {
                if(port_num.value(i)==serialport_list.value(j))
                {
                    same<<port_num.value(i);
                    flag=true;
                    break;
                }
            }
            if(flag==false)
            {
                diff_now<<port_num.value(i);
            }
        }
        //比较出减少项并删除
        for(uint8_t i=0;i<serialport_list.size();i++)
        {
            bool flag=false;
            for(uint8_t j=0;j<same.size();j++)
            {
                if(serialport_list.value(i)==same.value(j))
                {
                   flag=true;
                   break;
                }
            }
            if(flag==false)
                ui->com_box->removeItem(ui->com_box->findText(serialport_list.value(i)));
        }
        ui->com_box->addItems(diff_now);
        serialport_list=port_num;
        qDebug()<<port_num;
    }
}
//串口中断检测
void Widget::time1_task(void)
{
    static uint32_t cnt = 0;
    static uint8_t delay = 0;

    if(begin==true){
        if(++cnt>100){      //数据包计时
            cnt = 0;
            time_cnt++;
            QString str = QString::number(time_cnt);
            ui->label_7->setText(str);
        }
        if(++delay>10){     //数据读取超时

        }
    }
}
//打开/关闭串口
void Widget::Serial_Open(void)
{
    static bool flag = true;
    if(flag)
    {

        if(serialport_list.isEmpty()){
            qDebug()<<"error";
            QMessageBox::warning(this,"ERROR","No serial port connection detected",QMessageBox::Cancel|QMessageBox::Escape|QMessageBox::Default,0);
            return;
        }
        qDebug()<<"open";

        SerialPort = new QSerialPort;
        QString port_set;
        port_set = ui->com_box->currentText();
        port_set = port_set.left(5);
        qDebug()<<port_set;

        SerialPort->setPortName(port_set);
        if(SerialPort->open(QIODevice::ReadWrite))
        {
            SerialPort->setBaudRate(QSerialPort::Baud9600);
            SerialPort->setDataBits(QSerialPort::Data8);
            SerialPort->setStopBits(QSerialPort::OneStop);
            SerialPort->setParity(QSerialPort::NoParity);
            SerialPort->setFlowControl(QSerialPort::NoFlowControl);
        }


        time_cnt = 0;                                           //计数清0
        package_cnt = 0;

        begin = true;
        ui->com_box->setEnabled(false);

        ui->serial_button->setText("close");
        connect(SerialPort,&QSerialPort::readyRead,this,&Widget::data_analysis);

    }
    else
    {
        qDebug()<<"close";
        SerialPort->close();
        begin = false;
        ui->serial_button->setText("open");
        delete SerialPort;  SerialPort = NULL;
        ui->com_box->setEnabled(true);
        disconnect(SerialPort,&QSerialPort::readyRead,this,&Widget::data_analysis);
    }
    flag = !flag;
}

//数据解析
void Widget::data_analysis(void)
{
    QByteArray buf;
    buf = SerialPort->readAll();
    if(!buf.isEmpty())
    {
        if(station.data_analysis(buf))           //解码成功
        {

            if(station.data_structre.type==0x01)
            {

                float temperature = (int16_t)(station.data_structre.data[0]<<8|station.data_structre.data[1]);
                float pressure    = (int16_t)(station.data_structre.data[2]<<8|station.data_structre.data[3]);
                temperature /=100;
                pressure/=100;

                QString str1 = QString("%1").arg(temperature);
                ui->temperature->setText(str1);

                QString str2 = QString("%1").arg(pressure);
                ui->pressure->setText(str2);

                package_cnt++;
            }
            else if(station.data_structre.type==0x02)   //程序版本号
            {
                QString str = station.versiongst(station.data_structre.buff);
                ui->version_edit->setText(str);
            }

            QString str_1;
            str_1 = QString::number(package_cnt);
            ui->label_9->setText(str_1);

            //存储文件
            if(save_flag==true)
            {

            }

        }

    }
}

//请求版本号
void Widget::version_button_clicked(bool checked)
{
    uint8_t buff[6]={HEAD_1,HEAD_2,0x07,0x02,0x06,0x01};
    uint8_t sum = 0;
    for(int i=0;i<6;i++){
        if(i==2){continue;}
        sum += buff[i];
    }

    buff[2]=sum;
    SerialPort->write((char *)buff,6);
}

void Widget::on_cave_box_clicked(bool checked)
{
    save_flag = checked;
    if(save_flag==true)
    {
        QString path = QFileDialog::getExistingDirectory(this,"选择目录","D:\\qtpractice",\
                                                             QFileDialog::ShowDirsOnly);
        qDebug()<<path;
        QDateTime time = QDateTime::currentDateTime();
        int time_int = time.toTime_t();
        QString filename = "/save_"+QString::number(time_int)+".cvs";

        save_file = new QFile;
        save_file->setFileName(path+filename);

        save_file->open(QIODevice::WriteOnly|QIODevice::Append);
    }
    else
    {
        save_file->close();
        delete save_file;
        save_file = NULL;
    }
}

void Widget::fanopen_button_clicked(bool checked)
{
    uint8_t buff[8]={HEAD_1,HEAD_2,0x07,0x03,0x08,0x01,0x01,0x01};


    buff[6] = ui->fanduty_edit->text().toInt();
    buff[7] = ui->fantime_edit->text().toInt();

    uint8_t sum = 0;
    for(int i=0;i<8;i++){
        if(i==2){continue;}
        sum += buff[i];
    }

    buff[2]=sum;
    SerialPort->write((char *)buff,8);
}

void Widget::fanclose_button_clicked(bool checked)
{
    uint8_t buff[8]={HEAD_1,HEAD_2,0x07,0x03,0x08,0x00,0x01,0x01};

    uint8_t sum = 0;
    for(int i=0;i<8;i++){
        if(i==2){continue;}
        sum += buff[i];
    }

    buff[2]=sum;
    SerialPort->write((char *)buff,8);
}

void Widget::debug_button_clicked(bool checked)
{
    static bool flag =true;
    if(flag==false){
        ui->debug_button->setText("open debug");
    }else{
        ui->debug_button->setText("close debug");

    }
    uint8_t buff[6]={HEAD_1,HEAD_2,0x07,0x01,0x06,0x01};

    buff[5]=flag;

    uint8_t sum = 0;
    for(int i=0;i<6;i++){
        if(i==2){continue;}
        sum += buff[i];
    }

    flag=!flag;

    buff[2]=sum;
    SerialPort->write((char *)buff,6);
}
