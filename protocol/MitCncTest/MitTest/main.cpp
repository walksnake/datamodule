#include <QCoreApplication>
#include <QDebug>
#include "libezsocket/ezsocket.h"
//#include "mitsubishicnc_host/mitsubishicnc_host.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    char test[8];
    test[0] = 0x7F;
    test[1] = 0xFF;
    test[2] = 0xFF;
    test[3] = 0xFF;
    test[4] = 0xFF;
    test[5] = 0xFF;
    test[6] = 0xFF;
    test[7] = 0xFF;
    double d_test;
    memcpy(&d_test,test,8);
    qDebug()<<d_test<<ezsocket_swaped_double(d_test);
    /*uint8_t dest[128] = {0};
    double d_value[12] = {0};
    uint16_t length;
    int rc = -1;
    mitsubishicnc_host *host = new mitsubishicnc_host;
    while(1)
    {
        memset((char*)d_value,0,sizeof(d_value));
        rc = host->read_param("192.168.0.2",
                              683,
                              1,
                              _EZSOCKET_CURRENT_POSITION,
                              5,
                              "0:3",
                              1,
                              1000,
                              3,
                              10,
                              1,
                              (uint8_t*)d_value,
                              &length);
        qDebug()<<rc<<d_value[0]<<d_value[1]<<d_value[2];
    }*/
#if 1
    ezsocket_t *ctx;
    int rc;
    uint8_t dest[256];
    uint16_t length;
    uint16_t s_val;
    int value;
    uint16_t w_value[5];
    int address;
    ctx = ezsocket_new("192.168.0.2",683);
    double d_val[5];

    if(ctx == NULL)
    {
        qDebug()<<"int err";
    }
    ezsocket_set_debug(ctx, TRUE);
    ezsocket_set_error_recovery(ctx,EZSOCKET_ERROR_RECOVERY_LINK);
    rc = ezsocket_connect(ctx);
    if(rc < 0)
    {
        qDebug()<<"connect err";
    }
    else
    {
        qDebug()<<"connect ok";
    }
    struct timeval t;
    t.tv_sec=0;
    t.tv_usec=2000000;   //设置超时时间为100毫秒
    ezsocket_set_response_timeout(ctx,&t);
#if 0
    /* 系统相关 */
    memset(dest,0,sizeof(dest));
    length = 0;
    rc = ezsocket_read_version(ctx,dest,&length);//版本
    qDebug()<<"版本信息"<<QByteArray((char*)dest,length)<<rc;
    memset(dest,0,sizeof(dest));
    rc = ezsocket_read_run_time(ctx,&value);//运行时间
    qDebug()<<"运行时间"<<value<<rc;
    rc = ezsocket_read_cycle_time(ctx,&value);//循环时间
    qDebug()<<"循环时间"<<value<<rc;
    rc = ezsocket_read_start_time(ctx,&value);//开始时间
    qDebug()<<"开始时间"<<value<<rc;
    rc = ezsocket_read_estimate_time(ctx,1,&value);//经过的时间
    qDebug()<<"经过的时间"<<value<<rc;
    rc = ezsocket_read_keep_alive_time(ctx,&value);//上电时间
    qDebug()<<"上电时间"<<value<<rc;
#endif
#if 0
    /* 状态相关 */
    memset(dest,0,sizeof(dest));
    rc = ezsocket_read_cutting_model(ctx,dest);//是否处在切削模式
    qDebug()<<"切削模式与否"<<dest[0]<<rc;
    rc = ezsocket_read_run_status(ctx,dest);//运行状态
    qDebug()<<"运行状态"<<dest[0]<<rc;
    Sleep(100);
    rc = ezsocket_read_plc_r(ctx,69,w_value);
    qDebug()<<"急停状态"<<w_value[0]<<rc;
#endif
    rc = ezsocket_read_process_number(ctx, &value);//加工数量
    qDebug()<<"加工数量"<<value<<rc;
    // rc = ezsocket_read_program_block(ctx,dest,&length);//程序块内容
    //qDebug()<<"程序内容"<<(char*)dest<<rc;
#if 0
    /* 程序相关 */
    memset(dest,0,sizeof(dest));
    rc = ezsocket_read_program_no(ctx,dest,&length);//程序号
    qDebug()<<"程序号"<<(char*)dest<<rc<<length;
    rc = ezsocket_read_sequence_num(ctx,&value);//执行的语句
    qDebug()<<"执行的序号"<<value<<rc;
    memset(dest,0,sizeof(dest));
    rc = ezsocket_read_program_block(ctx,dest,&length);//程序块内容
    qDebug()<<"程序内容"<<(char*)dest<<rc;

#endif
#if 0
    // while(1)
    {
        //Sleep(100);
        memset(dest,0,sizeof(dest));
        Sleep(10);
        rc = ezsocket_read_axis_number(ctx,dest);//轴数量 byte
        qDebug()<<"轴数量"<<dest[0]<<rc;
        Sleep(10);
        rc = ezsocket_read_current_position(ctx,0,d_val);//当前坐标 double
        qDebug()<<"当前坐标"<<d_val[0]<<d_val[1]<<d_val[2]<<rc;
        Sleep(10);
        rc = ezsocket_read_work_position(ctx,0,d_val);//工作坐标 double
        qDebug()<<"工作坐标"<<d_val[0]<<d_val[1]<<d_val[2]<<rc;
        Sleep(10);
        rc = ezsocket_read_distance_position(ctx,0,d_val);//剩余坐标 dpouble
        qDebug()<<"剩余坐标"<<d_val[0]<<d_val[1]<<d_val[2]<<rc;
        Sleep(10);
        rc = ezsocket_read_machine_position(ctx,0,d_val);//机械坐标 double
        qDebug()<<"机械坐标"<<d_val[0]<<d_val[1]<<d_val[2]<<rc;
        Sleep(10);
        rc = ezsocket_read_spindle_set_speed(ctx,&value);//主轴设定速度 int
        qDebug()<<"主轴设定速度"<<value<<rc;
        Sleep(10);
        rc = ezsocket_read_spindle_act_speed(ctx,&value);//主轴实际速度 int
        qDebug()<<"主轴实际速度"<<value<<rc;
        Sleep(10);
        rc = ezsocket_read_feed_set_speed(ctx,d_val);//进给设定速度 dpuble
        qDebug()<<"进给设定速度"<<d_val[0]<<rc;
        Sleep(10);
        rc = ezsocket_read_feed_act_speed(ctx,d_val);//进给实际速度 DOUBLE
        qDebug()<<"进给实际速度"<<d_val[0]<<rc;
        Sleep(10);
        rc = ezsocket_read_spindle_load(ctx,&value);//主轴负载 int
        qDebug()<<"主轴负载"<<value<<rc;
        rc = ezsocket_read_servo_load(ctx,0,&value);//伺服轴负载 UINT16
        qDebug()<<"伺服负载"<<value<<rc;
        rc = ezsocket_read_servo_act_speed(ctx,1,&value);//伺服轴负载 UINT16
        qDebug()<<"伺服负载"<<value<<rc;
        rc = ezsocket_read_feed_rate(ctx, dest);//进给倍率
        qDebug()<<"进给倍率"<<dest[0]<<rc;
        rc = ezsocket_read_spindle_rate(ctx, dest);//主轴倍率
        qDebug()<<"主轴倍率"<<dest[0]<<rc;
        rc = ezsocket_read_fast_mov_rate(ctx, dest);//快速移动倍率
        qDebug()<<"快速移动倍率"<<dest[0]<<rc;

    }
#endif
#if 0
    //while(1)
    {
        Sleep(10);
        /* 刀具相关 */
        s_val = 0;
        rc = ezsocket_read_mag_tool_no(ctx, &value);//刀具号
        qDebug()<<"刀具号"<<value<<rc;
        Sleep(10);
        s_val = 0;
        rc = ezsocket_read_spd_tool_no(ctx, &value);//主刀具号
        qDebug()<<"主刀具号"<<value<<rc;
        Sleep(10);
        rc = ezsocket_read_tooloffset_d_shape(ctx, d_val);//刀偏移D shape
        qDebug()<<"shape"<<d_val[0]<<rc;
        Sleep(10);
        rc = ezsocket_read_tooloffset_d_wear(ctx, d_val);//刀偏移D wear
        qDebug()<<"shape"<<d_val[0]<<rc;
        Sleep(10);
        rc = ezsocket_read_tooloffset_h(ctx, 2, d_val);//刀偏H
        qDebug()<<"h"<<d_val[0]<<rc;
        rc = ezsocket_read_tool_set_size(ctx, &value);//刀具数量
        qDebug()<<"刀具数量"<<value;
        rc = ezsocket_read_tool_offset_type(ctx, &value);//刀具补偿类型
        qDebug()<<"刀具补偿类型"<<value;
        rc = ezsocket_read_tool_offset_value(ctx, 1, 0 , 1,d_val);//获取刀具偏移值 对应官方协议栈getoffset
        qDebug()<<"刀具偏移"<<d_val[0]<<rc;
    }
#endif
#if 0
    rc = ezsocket_read_m_command_value(ctx,1,&value);
    qDebug()<<"m command"<<value<<rc;
    rc = ezsocket_read_s_command_value(ctx,1,&value);
    qDebug()<<"s command"<<value<<rc;
    rc = ezsocket_read_t_command_value(ctx,1,&value);
    qDebug()<<"t command"<<value<<rc;
    rc = ezsocket_read_b_command_value(ctx,1,&value);
    qDebug()<<"m command"<<value<<rc;
#endif
    /* 报警相关 */
#if 0
    rc = ezsocket_read_alarm_msg(ctx,dest,&length);//报警信息
    if(rc == 0)
        qDebug()<<"报警信息"<<QByteArray((char*)dest,length).toHex()<<rc;
#endif
    /* PLC相关 */
#if 1
    rc = ezsocket_read_plc_f(ctx,0,1,dest);
    qDebug()<<"X"<<rc<<dest[0];
    rc = ezsocket_read_plc_d(ctx,5,w_value);
    qDebug()<<"d"<<w_value[0]<<rc;
    rc = ezsocket_read_g_code(ctx,1,d_val);
    qDebug()<<"G0"<<d_val[0]<<rc;
#endif
#if 0
    /* 参数读取 */
    address = 8007;
    rc = ezsocket_read_param(ctx,address,d_val);
    qDebug()<<"参数"<<address<<"的值："<<d_val[0];
#endif
    /* 宏变量 */
#if 0
    address = 100;
    rc = ezsocket_read_macro(ctx, address, d_val);//读取宏变量 double
    qDebug()<<"宏变量"<<address<<"的值："<<d_val[0];
    rc = ezsocket_read_macro(ctx, 510, d_val);//读取宏变量 double
    qDebug()<<"宏变量"<<address<<"的值："<<d_val[0];
#endif

#endif
    return a.exec();
}
