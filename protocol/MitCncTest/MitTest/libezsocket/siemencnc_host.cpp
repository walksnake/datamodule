#include "siemencnc_host.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define msleep(x) usleep(x*1000)
#define SIEMENSCNC_NUM  32
#define SIEMENSCNC_RETRY_TIMES      3

typedef struct
{
    siemenscnc_t *ctx[SIEMENSCNC_NUM];
    char isInit[SIEMENSCNC_NUM];
}siemenscnc_handle_t;

siemenscnc_handle_t siemenscnc_handle = {NULL, NULL};


/* 从要操作的地址中解析实际参数 */
static int _siemenscnc_decode_axis_address(const char *name,uint8_t length,uint8_t *axis,uint8_t *axis_number,uint16_t *address,uint8_t *sub_address)
{
    int ptr1 = 0;
    int ptr2 = 0;
    char buf[8]={0};
    int rc = 0;
    uint8_t axis_flag = 1;
    if(strstr(name,":")!=NULL)//表示既有轴号又有地址
    {
        while(ptr1 < length)
        {
            if(name[ptr1] >= '0' && name[ptr1] <= '9')
            {
                buf[ptr2++] = name[ptr1];
            }
            else if(name[ptr1] == ':')
            {
                *axis_number = atoi(buf);
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
                axis_flag = 0;
                rc |= (1<<1);
            }
            else if(name[ptr1] == '.')
            {
                if(axis_flag)
                {
                    *axis = atoi(buf);
                    rc |= (1<<0);
                }
                else
                {
                    *address = atoi(buf);
                    rc |= (1<<2);
                }
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
            }
            if(ptr2 >= 8)
            {
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
                break;
            }
            ptr1++;
            if(ptr1 >= length)
            {
                /* 副地址 */
                *sub_address = atoi(buf);
                rc |= (1<<2);
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
            }
        }
    }
    else//只有轴号 或者 地址
    {
        while(ptr1 < length)
        {
            if(name[ptr1] >= '0' && name[ptr1] <= '9')
            {
                buf[ptr2++] = name[ptr1];
            }
            else if(name[ptr1] == '.')
            {
                /* 地址或者轴编号 */
                *address = atoi(buf);
                *axis = *address;
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
                rc |= (1<<0);
                rc |= (1<<2);
            }
            if(ptr2 >= 8)
            {
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
                break;
            }
            ptr1++;
            if(ptr1 >= length)
            {
                /* 副地址或者轴数量 */
                *sub_address = atoi(buf);
                *axis_number = *sub_address;
                rc |= (1<<0);
                rc |= (1<<2);
                memset(buf,0,sizeof(buf));
                ptr2 = 0;
            }
        }
    }
    return rc;
}


/* 整型转IP */
static void _siemenscnc_host_int_2_ip(const char *hardware,char *ip,uint16_t *port)
{
    *port=((uint8_t)hardware[4]<<8)+(uint8_t)hardware[5];
    sprintf(ip,"%d.%d.%d.%d",(uint8_t)hardware[0],(uint8_t)hardware[1],(uint8_t)hardware[2],(uint8_t)hardware[3]);
}


/* 参数初始化 */
int _siemenscnc_host_init(const char *hardware, uint8_t index, uint16_t timeout)
{
    char ip[24]={0};
    uint16_t port;
    int rc;
    _siemenscnc_host_int_2_ip(hardware,ip,&port);
    siemenscnc_handle.isInit[index] = 0;
    siemenscnc_handle.ctx[index] = siemenscnc_828d_new(ip,port);
    if(siemenscnc_handle.ctx[index] == NULL)
    {
        return -1;
    }
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout*1000;   //设置超时时间
    siemenscnc_set_response_timeout(siemenscnc_handle.ctx[index],&t);
    siemenscnc_set_debug(siemenscnc_handle.ctx[index],1);
    rc = siemenscnc_connect(siemenscnc_handle.ctx[index]);
    if(rc < 0)
    {
        return -1;
    }
    return 1;

}


/* 读写参数 */
int siemenscnc_host_rw_param(const char *hardware,
                             uint8_t index,
                             uint8_t function,
                             uint8_t bits,
                             const char *regname,
                             uint8_t reglength,
                             uint16_t regnum,
                             uint16_t timeout,
                             uint8_t retry,
                             uint16_t commdelay,
                             uint8_t dataorder,
                             uint8_t *dest,
                             uint16_t *dest_length)
{
    int rc=0;
    uint8_t axis = 0;
    uint8_t axis_number =0;
    uint16_t address = 0;
    uint8_t sub_address = 0;
    if(siemenscnc_handle.isInit[index]<=0)
    {
        rc = _siemenscnc_host_init(hardware,index,timeout);
        if(rc <= 0)
        {
            return 0;
        }
    }
    if(reglength>0)
    {
        if(reglength>12)
        {
            return 0;
        }
        _siemenscnc_decode_axis_address(regname,reglength,&axis,&axis_number,&address,&sub_address);
    }

    printf("axis--%d axis_number--%d address--%d sub_address--%d\r\n",axis,axis_number,address,sub_address);
    if(axis_number == 0)
    {
        axis_number = 1;
    }
    switch(function)
    {
    case SIEMENSCNC_SERIAL_NUMBER://硬件序列号
        rc = siemenscnc_read_serial_number(siemenscnc_handle.ctx[index], dest, dest_length);
        break;
    case SIEMENSCNC_CNC_TYPE://机床类型
        rc = siemenscnc_read_cnc_type(siemenscnc_handle.ctx[index], dest, dest_length);
        break;
    case SIEMENSCNC_VERSION://版本信息
        rc = siemenscnc_read_verison(siemenscnc_handle.ctx[index], dest, dest_length);
        break;
    case SIEMENSCNC_MANUFACTURE_DATE://出厂日期
        rc = siemenscnc_read_manufacture_data(siemenscnc_handle.ctx[index], dest, dest_length);
        break;
    case SIEMENSCNC_OPERATE_MODE://操作模式
        rc = siemenscnc_read_operate_mode(siemenscnc_handle.ctx[index], dest);
        break;
    case SIEMENSCNC_STATUS://运行状态
        rc = siemenscnc_read_status(siemenscnc_handle.ctx[index], dest);
        break;
    case SIEMENSCNC_PROCESS_NUMBER://加工数量
        rc = siemenscnc_read_process_number(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_PROCESS_SET_NUMBER://设定的加工数量
        rc = siemenscnc_read_process_set_number(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_FEED_SET_SPEED://设定进给速度
        rc = siemenscnc_read_feed_set_speed(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_FEED_ACT_SPEED://实际进给速度
        rc = siemenscnc_read_feed_act_speed(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_FEED_RATE://进给倍率
        rc = siemenscnc_read_feed_rate(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_SPINDLE_SET_SPEED://主轴设定速度
        rc = siemenscnc_read_spindle_set_speed(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_SPINDLE_ACT_SPEED://主轴实际速度
        rc = siemenscnc_read_spindle_act_speed(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_SPINDLE_RATE://主轴倍率
        rc = siemenscnc_read_spindle_rate(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_RUN_TIME://循环时间,一次自动运行起动时间的累计值
        rc = siemenscnc_read_run_time(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_REMAIN_TIME://剩余时间
        rc = siemenscnc_read_remain_time(siemenscnc_handle.ctx[index], (double*)dest);
        break;
    case SIEMENSCNC_PROGRAM_NAME://加工程序名
        rc = siemenscnc_read_program_name(siemenscnc_handle.ctx[index], dest, dest_length);
        break;
    case SIEMENSCNC_TOOL_NUMBER://刀具号
        break;
    case SIEMENSCNC_TOOL_D_RADIUS://刀具半径D
        break;
    case SIEMENSCNC_TOOL_H_LENGTH://刀具长度
        break;
    case SIEMENSCNC_TOOL_X_LENGTH://长度补偿X
        break;
    case SIEMENSCNC_TOOL_Z_LENGTH://长度补偿Z
        break;
    case SIEMENSCNC_TOOL_RADIU://磨损半径
        break;
    case SIEMENSCNC_TOOL_EDG://刀沿位置
        break;
    case SIEMENSCNC_MACHINE_POS://机械坐标
        rc = siemenscnc_read_machine_pos(siemenscnc_handle.ctx[index],SIEMENSCNC_AXIS_X,axis_number, (double*)dest);
        break;
    case SIEMENSCNC_RELATIVELY_POS://工件坐标 相对坐标
        rc = siemenscnc_read_relatively_pos(siemenscnc_handle.ctx[index],SIEMENSCNC_AXIS_X,axis_number, (double*)dest);
        break;
    case SIEMENSCNC_REMAIN_POS://剩余坐标
        rc = siemenscnc_read_remain_pos(siemenscnc_handle.ctx[index],SIEMENSCNC_AXIS_X,axis_number, (double*)dest);
        break;
    case SIEMENSCNC_AXIS_NAME://轴名称
        rc = siemenscnc_read_axis_name(siemenscnc_handle.ctx[index],SIEMENSCNC_AXIS_X,axis_number,dest, dest_length);
        break;
    case SIEMENSCNC_DRIVE_VOLTAGE://母线电压
        rc = siemenscnc_read_driver_voltage(siemenscnc_handle.ctx[index],SIEMENSCNC_AXIS_SPINDLE,(float*)dest);
        break;
    case SIEMENSCNC_DRIVER_CURRENT://实际电流
        rc = siemenscnc_read_driver_current(siemenscnc_handle.ctx[index],axis,(float*)dest);
        break;
    case SIEMENSCNC_DRIVER_POWER://电机功率
        rc = siemenscnc_read_driver_power(siemenscnc_handle.ctx[index],axis,(float*)dest);
        break;
    case SIEMENSCNC_DRIVER_TEMPER://电机温度
        rc = siemenscnc_read_driver_temper(siemenscnc_handle.ctx[index],axis,(float*)dest);
        break;
    case SIEMENSCNC_CNC_ALARM://NC报警
        //rc = _siemenscnc_decode_cnc_alarm(msg,dest,length);
        break;
    case SIEMENSCNC_R_VAR://R变量
        rc = siemenscnc_read_r_var(siemenscnc_handle.ctx[index], axis, (double*)dest);
        break;
    case SIEMENSCNC_S_R_PARAM://驱动器R参数
        rc = siemenscnc_read_s_r_param(siemenscnc_handle.ctx[index], axis, address, sub_address, (float*)dest);
        break;
    case SIEMENSCNC_S_P_PARAM://驱动器P参数
        rc = siemenscnc_read_s_p_param(siemenscnc_handle.ctx[index],axis, address, sub_address, (float*)dest);
        break;
    }
    if(rc == SIEMENSCNC_RES_OK)
    {
        return 1;
    }
    else
    {
        if(rc == SIEMENSCNC_RES_TIMEOUT)
        {
            //断开连接
            siemenscnc_close(siemenscnc_handle.ctx[index]);
            siemenscnc_free(siemenscnc_handle.ctx[index]);
        }
        return 0;
    }

    /*switch(bits)
    {
    case SIEMENSCNC_BIT_TYPE://位
        break;
    case SIEMENSCNC_BYTE_TYPE://字节
        break;
    case SIEMENSCNC_WORD_TYPE://字
        break;
    case SIEMENSCNC_DWORD_TYPE://双字
        break;
    case SIEMENSCNC_FLOAT_TYPE://浮点
        break;
    default:
        break;
    }*/
}
