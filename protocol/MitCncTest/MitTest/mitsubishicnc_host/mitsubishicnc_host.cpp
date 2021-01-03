#include "mitsubishicnc_host.h"

mitsubishicnc_host::mitsubishicnc_host(QObject *parent) :
    QThread(parent)
{

}

mitsubishicnc_host::~mitsubishicnc_host()
{

}

int mitsubishicnc_host::read_param(const char *hardware,
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
                                   uint16_t *dest_length)
{
    int rc=0;
    int axis = 0;
    int axis_number =0;
    int address = 0;
    if(!map.contains(index))
    {
        mitsubishicnc_info_c *ctx;
        ctx = _mitsubishicnc_init(hardware,port,timeout);
        if(ctx == NULL)
        {
            return -1;
        }
        map[index] = ctx;
    }
    if(!map[index]->is_init)
    {
        //重新连接
        if(ezsocket_connect(map[index]->cnc) < 0)
        {
            return -1;
        }
        map[index]->is_init = true;
    }


    /* 解析地址 */
    _mitsubishicnc_decode_address(regname,axis,axis_number,address);
    qDebug()<<"das"<<map[index]->is_init<<axis<<axis_number;
    switch(function)
    {
    /* 系统相关 */
    case _EZSOCKET_VERSION://版本信息
        rc = ezsocket_read_version(map[index]->cnc,dest,dest_length);
        break;
    case _EZSOCKET_SYSTEM_DATE://系统日期
        rc = ezsocket_read_system_date(map[index]->cnc,dest);
        break;
    case _EZSOCKET_SYSTEM_TIME://系统时间
        rc = ezsocket_read_system_time(map[index]->cnc,dest);
        break;
        /* 状态相关 */
    case _EZSOCKET_CUTTING_MODEL://切削模式
        rc = ezsocket_read_cutting_model(map[index]->cnc,dest);
        break;
    case _EZSOCKET_RUN_STATUS://运行状态
        rc = ezsocket_read_run_status(map[index]->cnc,dest);
        break;
        /* 加工相关 */
    case _EZSOCKET_PROCESS_NUMBER://加工数量
    {
        int count;
        rc = ezsocket_read_process_number(map[index]->cnc, &count);
        if(rc >= 0)
        {
            dest[0] = count >> 24;
            dest[1] = count >> 16;
            dest[2] = count >> 8;
            dest[3] = count & 0xFF;
        }
    }
        break;
    case _EZSOCKET_PROGRAM_NO://程序号
        rc = ezsocket_read_program_no(map[index]->cnc,dest,dest_length);
        break;
    case _EZSOCKET_SEQUENCE_NUM://执行的语句
        //rc = ezsocket_read_sequence_num(map[index]->cnc,&value);
        break;
    case _EZSOCKET_PROGRAM_BLOCK://当前执行的内容
        rc = ezsocket_read_program_block(map[index]->cnc,dest,dest_length);
        break;
    case _EZSOCKET_RUN_TIME://运行时间
    {
        int time;
        rc = ezsocket_read_run_time(map[index]->cnc, &time);
        if(rc >= 0)
        {
            dest[0] = time >> 24;
            dest[1] = time >> 16;
            dest[2] = time >> 8;
            dest[3] = time & 0xFF;
        }
    }
        break;
    case _EZSOCKET_CYCLE_TIME://循环时间
    {
        int time;
        rc = ezsocket_read_cycle_time(map[index]->cnc, &time);
        if(rc >= 0)
        {
            dest[0] = time >> 24;
            dest[1] = time >> 16;
            dest[2] = time >> 8;
            dest[3] = time & 0xFF;
        }
    }
        break;
    case _EZSOCKET_START_TIME://开始时间
    {
        int time;
        rc = ezsocket_read_start_time(map[index]->cnc, &time);
        if(rc >= 0)
        {
            dest[0] = time >> 24;
            dest[1] = time >> 16;
            dest[2] = time >> 8;
            dest[3] = time & 0xFF;
        }
    }
        break;
    case _EZSOCKET_ESTIMATE_TIME://经过的时间
    {
        int time;
        rc = ezsocket_read_estimate_time(map[index]->cnc, &time);
        if(rc >= 0)
        {
            dest[0] = time >> 24;
            dest[1] = time >> 16;
            dest[2] = time >> 8;
            dest[3] = time & 0xFF;
        }
    }
        break;
    case _EZSOCKET_KEEP_ALIVE_TIME://上电时间
    {
        int time;
        rc = ezsocket_read_keep_alive_time(map[index]->cnc, &time);
        if(rc >= 0)
        {
            dest[0] = time >> 24;
            dest[1] = time >> 16;
            dest[2] = time >> 8;
            dest[3] = time & 0xFF;
        }
    }
        break;
        /* 轴相关 */
    case _EZSOCKET_AXIX_NUMBER://轴数量
        rc = ezsocket_read_axis_number(map[index]->cnc,dest);
        break;
    case _EZSOCKET_CURRENT_POSITION://当前坐标
        rc = ezsocket_read_current_position(map[index]->cnc,axis,axis_number,(double*)dest);
        if(rc >= 0)
        {
            *dest_length = 8*axis_number;
        }
        break;
    case _EZSOCKET_WORK_POSITION://工作坐标
        rc = ezsocket_read_work_position(map[index]->cnc,axis,axis_number,(double*)dest);
        if(rc >= 0)
        {
            *dest_length = 8*axis_number;
        }
        break;
    case _EZSOCKET_DISTANCE_POSITION://剩余坐标
        rc = ezsocket_read_distance_position(map[index]->cnc,axis,axis_number,(double*)dest);
        if(rc >= 0)
        {
            *dest_length = 8*axis_number;
        }
        break;
    case _EZSOCKET_MACHINE_POSTION://机械坐标
        rc = ezsocket_read_machine_position(map[index]->cnc,axis,axis_number,(double*)dest);
        if(rc >= 0)
        {
            *dest_length = 8*axis_number;
        }
        break;
    case _EZSOCKET_SPINDLE_SET_SPEED://主轴设定速度
    {
        int speed;
        rc = ezsocket_read_spindle_set_speed(map[index]->cnc, &speed);
        if(rc >= 0)
        {
            dest[0] = speed >> 24;
            dest[1] = speed >> 16;
            dest[2] = speed >> 8;
            dest[3] = speed & 0xFF;
        }
    }
        break;
    case _EZSOCKET_SPINDLE_ACTUAL_SPEED://主轴实际速度
    {
        int speed;
        rc = ezsocket_read_spindle_act_speed(map[index]->cnc, &speed);
        if(rc >= 0)
        {
            dest[0] = speed >> 24;
            dest[1] = speed >> 16;
            dest[2] = speed >> 8;
            dest[3] = speed & 0xFF;
        }
    }
        break;
    case _EZSOCKET_FEED_SET_SPEED://进给设定速度
        rc = ezsocket_read_feed_set_speed(map[index]->cnc, (double*)dest);
        break;
    case _EZSOCKET_FEED_ACTUAL_SPEED://进给实际速度
        rc = ezsocket_read_feed_act_speed(map[index]->cnc, (double*)dest);
        break;
    case _EZSOCKET_SPINDLE_LOAD://主轴负载
    {
        int load;
        rc = ezsocket_read_spindle_load(map[index]->cnc, &load);
        if(rc >= 0)
        {
            dest[0] = load >> 24;
            dest[1] = load >> 16;
            dest[2] = load >> 8;
            dest[3] = load & 0xFF;
        }
    }
        break;
    case _EZSOCKET_SERVO_LOAD://伺服轴负载
    {
        uint16_t load[axis_number];
        rc = ezsocket_read_servo_load(map[index]->cnc,axis,axis_number, load);
        if(rc >= 0)
        {
            for(int i=0;i<axis_number;i++)
            {
                dest[2*i] = load[i] >> 8;
                dest[2*i+1] = load[i] & 0xFF;
            }
            *dest_length = 2*axis_number;
        }
    }
        break;
        /* 刀具相关 */
    case _EZSOCKET_MAG_TOOL_NO://刀库刀具号
    {
        uint16_t number;
        rc = ezsocket_read_mag_tool_no(map[index]->cnc,&number);
        if(rc >= 0)
        {
            dest[0] = number >> 8;
            dest[1] = number & 0xFF;
        }
    }
    case _EZSOCKET_SPD_TOOL_NO://主轴刀具号
    {
        uint16_t number;
        rc = ezsocket_read_spd_tool_no(map[index]->cnc,&number);
        if(rc >= 0)
        {
            dest[0] = number >> 8;
            dest[1] = number & 0xFF;
        }
    }
        break;
    case _EZSOCKET_TOOLOFFSET_D_SHAPE://刀偏移D SHAPE
        rc = ezsocket_read_tooloffset_d_shape(map[index]->cnc,(double*)dest);
        break;
    case _EZSOCKET_TOOLOFFSET_D_WEAR://刀偏移D WEAR
        rc = ezsocket_read_tooloffset_d_wear(map[index]->cnc,(double*)dest);
        break;
    case _EZSOCKET_TOOLOFFSET_H://刀偏H
        rc = ezsocket_read_tooloffset_h(map[index]->cnc,axis,(double*)dest);
        break;
        /* 报警相关 */
    case _EZSOCKET_ALARM_MSG://报警信息
        rc = ezsocket_read_alarm_msg(map[index]->cnc,dest,dest_length);
        break;
        /* PLC 相关 */
    case _EZSOCKET_PLC_X://PLC X
        rc = ezsocket_read_plc_x(map[index]->cnc,address,dest);
        break;
    case _EZSOCKET_PLC_Y://PLC Y
        rc = ezsocket_read_plc_y(map[index]->cnc,address,dest);
        break;
    case _EZSOCKET_PLC_R://PLC R
        rc = ezsocket_read_plc_r(map[index]->cnc,address,dest);
        break;
    case _EZSOCKET_PLC_F://PLC F
        rc = ezsocket_read_plc_f(map[index]->cnc,address,dest);
        break;
    case _EZSOCKET_G_CODE://G代码
        rc = -1;
        break;
        /* 参数 */
    case _EZSOCKET_PARAM_READ://参数读取
        rc = ezsocket_read_param(map[index]->cnc,address,(double*)dest);
        break;
        /* 宏变量 */
    case _EZSOCKET_READ_MACRO_VAR://读取宏变量
        rc = ezsocket_read_macro(map[index]->cnc, address, (double*)dest);
        break;
    }
    if(rc < 0)
    {
        map[index]->is_init = false;
        //断开连接
        ezsocket_close(map[index]->cnc);
        return -1;
    }
    return 1;
}

/*
 * 初始化
 */
mitsubishicnc_info_c* mitsubishicnc_host::_mitsubishicnc_init(const char *hardware,int port,uint16_t timeout)
{
    mitsubishicnc_info_c* ctx = new mitsubishicnc_info_c;

    int rc;
    ctx->cnc = ezsocket_new(hardware,port);
    if(ctx->cnc == NULL)
    {
        delete ctx;
        return NULL;
    }
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout*1000;   //设置超时时间
    ezsocket_set_response_timeout(ctx->cnc,&t);
    //mitsubishicnc_set_debug(ctx->cnc,TRUE);
    rc = ezsocket_connect(ctx->cnc);
    if(rc < 0)
    {
        delete ctx;
        return NULL;
    }
    ctx->is_init = true;
    return ctx;
}

/*
 * 释放
 */
void mitsubishicnc_host::_mitsubishicnc_free(void)
{

}

/*
 * 解析地址
 */
void mitsubishicnc_host::_mitsubishicnc_decode_address(QString str, int &axis,int &axis_number,int &address)
{
    if(str.contains(":"))//表示为轴和轴数量
    {
        QStringList list = str.split(":");
        axis = list.at(0).toInt();
        axis_number = list.at(1).toInt();
    }
    else
    {
        axis = str.toInt();
        address = axis;
    }
}

