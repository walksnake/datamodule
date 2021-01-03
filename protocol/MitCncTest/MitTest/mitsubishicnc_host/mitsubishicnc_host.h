#ifndef MITSUBISHICNC_HOST_H
#define MITSUBISHICNC_HOST_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QDebug>
#include <QStringList>
#include <QString>
#include <QByteArray>
#include "libezsocket/ezsocket.h"
#include <stdint.h>


/* regname 格式
若为轴和轴数量 为X:X
若为轴名称则为 X
若为地址则为  X*/

enum
{
    MITSUBISHICNC_BIT_TYPE=0,//位
    MITSUBISHICNC_BYTE_TYPE,//字节
    MITSUBISHICNC_WORD_TYPE,//字
    MITSUBISHICNC_DWORD_TYPE,//双字
    MITSUBISHICNC_FLOAT_TYPE,//浮点
    MITSUBISHICNC_DOUBLE_TYPE,//浮点
    MITSUBISHICNC_STRING_TYPE//字符串

};

class mitsubishicnc_info_c : public QObject
{
    Q_OBJECT
public:
    ezsocket_t *cnc;
    bool is_init;
    mitsubishicnc_info_c()
    {
        cnc = NULL;
        is_init = false;
    }
    ~mitsubishicnc_info_c()
    {
        if(cnc != NULL)
        {
            ezsocket_close(cnc);
            ezsocket_free(cnc);
            cnc = NULL;
        }
    }

    mitsubishicnc_info_c(const mitsubishicnc_info_c& copy)
    {
        this->cnc = copy.cnc;
        this->is_init = copy.is_init;
    }

    mitsubishicnc_info_c& operator= (const mitsubishicnc_info_c& copy)
    {
        this->cnc = copy.cnc;
        this->is_init = copy.is_init;
    }

};


class mitsubishicnc_host : public QThread
{
    Q_OBJECT
public:
    explicit mitsubishicnc_host(QObject *parent = 0);
    ~mitsubishicnc_host();

    int read_param(const char *hardware,
                   int port,
                   uint8_t index,
                   uint8_t function,
                   uint8_t bits,
                   QString regname,
                   uint16_t regnum,
                   uint16_t timeout,
                   uint8_t retry,
                   uint16_t commdelay,
                   uint8_t dataorder,
                   uint8_t *dest,
                   uint16_t *dest_length);
signals:

public slots:
private:
    mitsubishicnc_info_c* _mitsubishicnc_init(const char *hardware,int port,uint16_t timeout);//初始化
    void _mitsubishicnc_free(void);//释放
    void _mitsubishicnc_decode_address(QString str, int &axis,int &axis_number,int &address);
private:
    QMap<int,mitsubishicnc_info_c*> map;

};

#endif // MITSUBISHICNC_HOST_H
