#ifndef SIEMENSCNC_HOST_H
#define SIEMENSCNC_HOST_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QDebug>
#include <QStringList>
#include <QString>
#include <QByteArray>
#include "libsiemenscnc/siemenscnc.h"
#include <stdint.h>
/* 备注 */
/* 对于regname,
 * 如仅需要设定轴 X第一个X表示轴编号
 * 如仅需要地址的格式位 X.XX,第一个X地址，第二个XX副地址(下角标)
    X:X.XX冒号前的表示轴编号，冒号后的表示地址和副地址.根据实际功能，若不填该值位0
*/
enum
{
    SIEMENSCNC_BIT_TYPE=0,//位
    SIEMENSCNC_BYTE_TYPE,//字节
    SIEMENSCNC_WORD_TYPE,//字
    SIEMENSCNC_DWORD_TYPE,//双字
    SIEMENSCNC_FLOAT_TYPE,//浮点
    SIEMENSCNC_DOUBLE_TYPE,//双精度
    SIEMENSCNC_STRING_TYPE,//字符串

};

class siemenscnc_info_c : public QObject
{
    Q_OBJECT
public:
    siemenscnc_t *cnc;
    bool is_init;
    siemenscnc_info_c()
    {
        cnc = NULL;
        is_init = false;
    }
    ~siemenscnc_info_c()
    {
        if(cnc != NULL)
        {
            is_init = false;
            siemenscnc_close(cnc);
            siemenscnc_free(cnc);
            cnc = NULL;
        }
    }

    siemenscnc_info_c(const siemenscnc_info_c& copy)
    {
        this->cnc = copy.cnc;
        this->is_init = copy.is_init;
    }

    siemenscnc_info_c& operator= (const siemenscnc_info_c& copy)
    {
        this->cnc = copy.cnc;
        this->is_init = copy.is_init;
    }

};


class siemenscnc_host : public QThread
{
    Q_OBJECT
public:
    explicit siemenscnc_host(QObject *parent = 0);
    ~siemenscnc_host();
    int read_param(int protocol,//协议名
                   const void *hardware_param,//IP/串口参数
                   const void *param,//协议参数
                   QString ioname,//ioname
                   int function,//功能码
                   int bits,//数据类型
                   QByteArray regname,//要操作的内容
                   int regnum,//要操作的内容的数量
                   const void *attributes,//属性
                   int dataorder,//数据排序
                   uint8_t *dest,//返回的数据
                   int &dest_length);//返回的数据长度
    void close_connect(QString ioname);//关闭连接
signals:

public slots:
private:
    siemenscnc_info_c* _siemenscnc_init(const void *hardware_param,int timeout);//初始化
    void _siemenscnc_free(void);//释放
    void _siemenscnc_decode_address(QString str, int &axis,int &address,int &sub_address);
    void _siemenscnc_swap_data(uint8_t *src, int number);//交换顺序
private:
    typedef struct
    {
        char ip[20];//ip
        uint16_t port;//端口号
    }siemenscnc_net_para_t;//需要建立连接的网络参数
    QMap<QString,siemenscnc_info_c*> map;
    typedef struct
    {
        int  timeout;//超时时间 ms
        int  retry;//重试次数
        int  comm_delay;//通信延时 ms
        int  restore_time;//重连时间
    }siemenscnc_attributes_t;//属性信息

};

#endif // SIEMENSCNC_HOST_H
