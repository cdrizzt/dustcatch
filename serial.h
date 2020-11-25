#ifndef SERIAL_H
#define SERIAL_H

#include <QWidget>

#define HEAD_1      0xA5
#define HEAD_2      0x5A
#define HEAD_LEN    (2)
#define TYPE_LEN    (1)
#define SUM_LEN     (1)
#define LEN_LEN     (1)
#define DATA_LEN    (25)

#define BUFF_LEN (HEAD_LEN+SUM_LEN+TYPE_LEN+LEN_LEN+DATA_LEN)  //头 + 8

typedef union{
    uint8_t buff[BUFF_LEN];
    struct __attribute__((packed)){
        uint8_t head_1;             //头1
        uint8_t head_2;             //头2
        uint8_t sum;                //校验和高
        uint8_t type;
        uint8_t len;
        uint8_t data[DATA_LEN];
    };
}DATA_STRUCTURE;

class serial
{
public:
    serial();
    int step;
    bool data_analysis(QByteArray buf);
    QString versiongst(uint8_t *buff);
    DATA_STRUCTURE data_structre;   //数据解码


};

#endif // SERIAL_H
