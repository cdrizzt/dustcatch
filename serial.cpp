#include "serial.h"
#include <QDebug>

const QByteArray MonthesString[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const QByteArray MonthesInteger[13] = {"00","01","02","03","04","05","06","07","08","09","10","11","12"};

serial::serial()
{
    data_structre = {0};
    step = 0;
}
//解码函数
bool serial::data_analysis(QByteArray buf)
{
    for(int i=0;i<buf.size();i++)
    {
        data_structre.buff[step] = buf.at(i);
        switch (step)
        {
            case 0:         //获取头
            {
               if(uint8_t(buf.at(i))==HEAD_1){

                   step++;
               }else{
                   step=0;
               }
            }break;

            case 1:         //获取头
            {
               if(buf.at(i)==HEAD_2){
                   step++;
               }else{
                   step=0;
               }
            }break;

            case BUFF_LEN:        //接校验和
            {
                step = 0;
            }break;

            default:
            {
                step++;
                if(step>=BUFF_LEN-DATA_LEN)
                {
                    if(step==data_structre.len)
                    {
                        step=0;
                        uint8_t sum = 0;
                        for(int i=0;i<data_structre.len;i++){
                            if(i==2){continue;}
                            sum += data_structre.buff[i];
                        }

                        if(sum == data_structre.sum){       //解码成功
                            return true;
                        }
                    }
                }
            }break;
        }
    }
    return false;
}


QString serial::versiongst(uint8_t *buff)
{
    QString version;
    QByteArray str_buf((const char *)buff);

    QByteArray month;
    QByteArray day;
    QByteArray year;

    int mode = 0;
    for(int i=5;i<25;i++)   //提出年月日
    {
        switch (mode) {
        case 0:
        {
            month.append(str_buf[i]);
            if(str_buf[i+1] == ' '){
                mode++;
                i++;
            }
        }
        break;
        case 1:
        {
            day.append(str_buf[i]);
            if(str_buf[i+1] == ' '){
                mode++;
                i++;
            }
        }
        break;
        case 2:
        {
            year.append(str_buf[i]);
            if(str_buf[i+1] == ' '){
                mode++;
            }
        }
        break;
        default:mode=3;break;
        }
        if(mode==3){break;}
    }


    QByteArray month_num=MonthesInteger[0];
    for(int i=0;i<12;i++){
        if(month==MonthesString[i]){
            month_num = MonthesInteger[i+1];
        }
    }
    if(atoi(day)<10)
    {
        QByteArray cache = day;
        day[0]='0';
        day.append(cache);
    }

    version.append('_');
    version.append(year);
    version.append(month_num);
    version.append(day);
    version.append('_');

    int year_int = atoi(year);
    year_int -= 2016;

    version.append(QString::number(year_int,10));
    version.append(month_num);
    version.append(day);

    QByteArray str_buf_time((const char *)&buff[17]);
    for(int i=0;i<str_buf_time.size();i++){
        if(str_buf_time[i]==':'){
            str_buf_time[i]='\0';
            break;
        }
    }
    version.append(str_buf_time);
    version.append("(320AL)");

    return version;

}
