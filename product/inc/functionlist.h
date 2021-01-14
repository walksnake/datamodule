/**
 * @file functionlist.h
 * @brief  define function list
 * @author Fan Chuanlin, fanchuanlin@aliyun.com
 * @version 1.0
 * @date 2021-01-09
 * @attention
 * @brief History:
 * <table>
 * <tr><th> Date <th> Version <th> Author <th> Description
 * <tr><td> xxx-xx-xx <td> 1.0 <td> Fan Chuanlin <td> Create
 * </table>
 *
 */

#ifndef _FUNCTION_LIST_H_
#define _FUNCTION_LIST_H_
#include "datacollecter.h"

/// 西门子CNC test
#include "../../protocol/SimensCnc/Simens/libsiemenscnc/inc/siemenscnc.h"
#include "../../protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.h"

/// 三菱CNC
#include "../../protocol/MitCncTest/MitTest/libezsocket/ezsocket.h"
#include "../../protocol/MitCncTest/MitTest/libezsocket/ezsocket_new.h"
#include "../../protocol/MitCncTest/MitTest/libezsocket/ezsocket_private.h"


typedef enum DATATYPE
{
    DATATYPE_BOOL = 0,
    DATATYPE_BYTE = 1,
    DATATYPE_WORD = 2,
    DATATYPE_DWORD = 3,
    DATATYPE_FLOAT = 4,
    DATATYPE_DOUBLE = 5,
    DATATYPE_STRING = 6,
    DATATYPE_LONG = 7,
    DATATYPE_BCD16 = 8,
    DATATYPE_BCD32 = 9
} DATATYPE_E;

#if 0
enum
{
    /* 系统相关 */
    _SIEMENSCNC_SERIAL_NUMBER = 0, //硬件序列号
    _SIEMENSCNC_CNC_TYPE,//机床类型
    _SIEMENSCNC_VERSION,//版本信息
    _SIEMENSCNC_MANUFACTURE_DATE,//出厂日期
    _SIEMENSCNC_PROCESS_NUMBER,//加工数量
    _SIEMENSCNC_PROCESS_SET_NUMBER,//设定的加工数量
    _SIEMENSCNC_RUN_TIME,//循环时间,一次自动运行起动时间的累计值
    _SIEMENSCNC_REMAIN_TIME,//剩余时间
    _SIEMENSCNC_PROGRAM_NAME,//加工程序名
    /* 运行状态 */
    _SIEMENSCNC_OPERATE_MODE = 10, //操作模式
    _SIEMENSCNC_STATUS,//运行状态
    /* 轴相关 */
    _SIEMENSCNC_AXIS_NAME = 20, //轴名称
    _SIEMENSCNC_MACHINE_POS,//机械坐标
    _SIEMENSCNC_RELATIVELY_POS,//工件坐标 相对坐标
    _SIEMENSCNC_REMAIN_POS,//剩余坐标
    _SIEMENSCNC_FEED_SET_SPEED,//设定进给速度
    _SIEMENSCNC_FEED_ACT_SPEED,//实际进给速度
    _SIEMENSCNC_FEED_RATE,//进给倍率
    _SIEMENSCNC_SPINDLE_SET_SPEED,//主轴设定速度
    _SIEMENSCNC_SPINDLE_ACT_SPEED,//主轴实际速度
    _SIEMENSCNC_SPINDLE_RATE,//主轴倍率
    _SIEMENSCNC_G_COORDINATE_T,//G工件坐标系 T
    _SIEMENSCNC_G_COORDINATE_M,//G工件坐标系 M
    /* 刀具相关 */
    _SIEMENSCNC_TOOL_CUR_NAME = 40, //当前刀具名字
    _SIEMENSCNC_TOOL_CUR_T_NO,//当前刀具号
    _SIEMENSCNC_TOOL_CUR_D_NO,//当前刀沿号
    _SIEMENSCNC_TOOL_CUR_H_NO,//刀具当前H号
    _SIEMENSCNC_TOOL_LIST,//刀具列表
    _SIEMENSCNC_TOOL_X_OFFSET,//长度补偿X
    _SIEMENSCNC_TOOL_Z_OFFSET,//长度补偿Z
    /* 报警相关 */
    _SIEMENSCNC_CNC_NC_ALARM_NO = 60, //NC报警数量
    _SIEMENSCNC_CNC_NC_ALARM,//NC报警
    /* 参数相关 */
    _SIEMENSCNC_R_VAR = 65, //R变量
    _SIEMENSCNC_S_R_P_PARAM,//驱动器R P参数
    _SIEMENSCNC_GENERAL_MACHINE_PARAM,//通用机床数据
    _SIEMENSCNC_CHANNEL_MACHINE_PARAM,//通道机床数据
    _SIEMENSCNC_AXIS_MACHINE_PARAM,//轴机床数据
    _SIEMENSCNC_CONTROL_UNIT_PARAM,//控制单元参数
    _SIEMENSCNC_POWER_UNIT_PARAM//电源模块参数
};


#endif
extern "C"
{
    IOFunctionList functionlist[] =
    {
        {
            0,
            _SIEMENSCNC_SERIAL_NUMBER,
            3,
            2,
            (TYPE_BASE_USHORT<<8)|(TYPE_BASE_BUFF_PTR<<4)|TYPE_BASE_VOID ,
						"",
            NULL,
            ( pJobFunc )siemenscnc_read_serial_number,
        },
        {
            1,
            _SIEMENSCNC_CNC_TYPE,
            3,
            2,
            (TYPE_BASE_USHORT<<8)|(TYPE_BASE_BUFF_PTR<<4)|TYPE_BASE_VOID ,
						"",
            NULL,
            ( pJobFunc )siemenscnc_read_cnc_type,
        }
    };
}

#endif

