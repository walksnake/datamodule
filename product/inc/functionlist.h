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

typedef enum PROTOCOL
{
    SIEMENSCNC  = 54,
} PROTOCOL_E;

extern "C"
{
    IOFunctionList functionlist_siemens[] =
    {
        {
            0,
            _SIEMENSCNC_SERIAL_NUMBER,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_serial_number,
            "siemenscnc_read_serial_number"
        },
        {
            1,
            _SIEMENSCNC_CNC_TYPE,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_cnc_type,
            "siemenscnc_read_cnc_type"
        },
        {
            2,
            _SIEMENSCNC_VERSION,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_verison,
            "siemenscnc_read_verison"
        },
        {
            3,
            _SIEMENSCNC_MANUFACTURE_DATE,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_manufacture_data,
            "siemenscnc_read_manufacture_data"
        },
        {
            4,
            _SIEMENSCNC_PROCESS_NUMBER,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_process_number,
            "siemenscnc_read_process_number"
        },
        {
            5,
            _SIEMENSCNC_PROCESS_SET_NUMBER,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_process_set_number,
            "siemenscnc_read_process_set_number"
        },
        {
            6,
            _SIEMENSCNC_RUN_TIME,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_run_time,
            "siemenscnc_read_run_time"
        },
        {
            7,
            _SIEMENSCNC_REMAIN_TIME,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_remain_time,
            "siemenscnc_read_remain_time"
        },
        {
            8,
            _SIEMENSCNC_PROGRAM_NAME,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_program_name,
            "siemenscnc_read_program_name"
        },
        {
            9,
            _SIEMENSCNC_OPERATE_MODE,
            2,
            2,
            ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_operate_mode,
            "siemenscnc_read_operate_mode"
        },
        {
            10,
            _SIEMENSCNC_STATUS,
            2,
            2,
            ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_status,
            "siemenscnc_read_status"
        },
        {
            11,
            _SIEMENSCNC_AXIS_NAME,
            4,
            3,
            ( TYPE_BASE_USHORT << 12 ) | ( TYPE_BASE_BUFF_PTR << 8 )| ( TYPE_BASE_OCTET << 4 )  | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_axis_name,
            "siemenscnc_read_axis_name"
        },
        {
            12,
            _SIEMENSCNC_MACHINE_POS,
            3,
            3,
            ( TYPE_BASE_DOUBLE << 8 ) | ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_machine_pos,
            "siemenscnc_read_machine_pos"
        },
        {
            13,
            _SIEMENSCNC_RELATIVELY_POS,
            3,
            3,
            ( TYPE_BASE_DOUBLE << 8 ) | ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_relatively_pos,
            "siemenscnc_read_relatively_pos"
        },
        {
            14,
            _SIEMENSCNC_REMAIN_POS,
            3,
            3,
            ( TYPE_BASE_DOUBLE << 8 ) | ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_remain_pos,
            "siemenscnc_read_remain_pos"
        },
        {
            15,
            _SIEMENSCNC_FEED_SET_SPEED,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_feed_set_speed,
            "siemenscnc_read_feed_set_speed"
        },
        {
            16,
            _SIEMENSCNC_FEED_ACT_SPEED,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_feed_act_speed,
            "siemenscnc_read_feed_act_speed"
        },
        {
            17,
            _SIEMENSCNC_FEED_RATE,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_feed_rate,
            "siemenscnc_read_feed_rate"
        },
        {
            18,
            _SIEMENSCNC_SPINDLE_SET_SPEED,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_spindle_set_speed,
            "siemenscnc_read_spindle_set_speed"
        },
        {
            19,
            _SIEMENSCNC_SPINDLE_ACT_SPEED,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_spindle_act_speed,
            "siemenscnc_read_spindle_act_speed"
        },
        {
            20,
            _SIEMENSCNC_SPINDLE_RATE,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_spindle_rate,
            "siemenscnc_read_spindle_rate"
        },
        {
            21,
            _SIEMENSCNC_G_COORDINATE_T,
            4,
            4,
            ( TYPE_BASE_DOUBLE << 12 ) | ( TYPE_BASE_OCTET << 8 ) | ( TYPE_BASE_INT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_g_coordinate_t,
            "siemenscnc_read_g_coordinate_t"
        },
        {
            22,
            _SIEMENSCNC_G_COORDINATE_M,
            4,
            4,
            ( TYPE_BASE_DOUBLE << 12 ) | ( TYPE_BASE_OCTET << 8 ) | ( TYPE_BASE_INT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_g_coordinate_m,
            "siemenscnc_read_g_coordinate_m"
        },
        {
            23,
            _SIEMENSCNC_TOOL_CUR_NAME,
            3,
            2,
            ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_BUFF_PTR << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_cur_name,
            "siemenscnc_read_tool_cur_name"
        },
        {
            24,
            _SIEMENSCNC_TOOL_CUR_T_NO,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_cur_t_no,
            "siemenscnc_read_tool_cur_t_no"
        },
        {
            25,
            _SIEMENSCNC_TOOL_CUR_D_NO,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_cur_d_no,
            "siemenscnc_read_tool_cur_d_no"
        },
        {
            26,
            _SIEMENSCNC_TOOL_CUR_H_NO,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_cur_h_no,
            "siemenscnc_read_tool_cur_h_no"
        },
        {
            27,
            _SIEMENSCNC_TOOL_LIST,
            4,
            4,
            ( TYPE_BASE_DOUBLE << 12 ) | ( TYPE_BASE_INT << 8 ) | ( TYPE_BASE_INT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_length,
            "siemenscnc_read_tool_length"
        },
        {
            28,
            _SIEMENSCNC_TOOL_X_OFFSET,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_x_offset,
            "siemenscnc_read_tool_x_offset"
        },
        {
            29,
            _SIEMENSCNC_TOOL_Z_OFFSET,
            2,
            2,
            ( TYPE_BASE_DOUBLE << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_tool_z_offset,
            "siemenscnc_read_tool_z_offset"
        },
        {
            30,
            _SIEMENSCNC_CNC_NC_ALARM_NO,
            2,
            2,
            ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_cnc_nc_alarm_no,
            "siemenscnc_read_cnc_nc_alarm_no"
        },
        {
            31,
            _SIEMENSCNC_CNC_NC_ALARM,
            3,
            3,
            ( TYPE_BASE_UINT << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_cnc_nc_alarm,
            "siemenscnc_read_cnc_nc_alarm"
        },
        {
            32,
            _SIEMENSCNC_R_VAR,
            3,
            3,
            ( TYPE_BASE_DOUBLE << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_r_var,
            "siemenscnc_read_r_var"
        },
        {
            33,
            _SIEMENSCNC_S_R_P_PARAM,
            6,
            5,
            ( TYPE_BASE_USHORT << 20 ) | ( TYPE_BASE_BUFF_PTR << 16 ) | ( TYPE_BASE_USHORT << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_s_r_p_param,
            "siemenscnc_read_s_r_p_param"
        },
        {
            34,
            _SIEMENSCNC_GENERAL_MACHINE_PARAM,
            5,
            4,
            ( TYPE_BASE_USHORT << 16 ) | ( TYPE_BASE_BUFF_PTR << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_general_machine_param,
            "siemenscnc_read_general_machine_param"
        },
        {
            35,
            _SIEMENSCNC_CHANNEL_MACHINE_PARAM,
            5,
            4,
            ( TYPE_BASE_USHORT << 16 ) | ( TYPE_BASE_BUFF_PTR << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_channel_machine_param,
            "siemenscnc_read_channel_machine_param"
        },
        {
            36,
            _SIEMENSCNC_AXIS_MACHINE_PARAM,
            6,
            5,
            ( TYPE_BASE_USHORT << 20 ) | ( TYPE_BASE_BUFF_PTR << 16 ) | ( TYPE_BASE_USHORT << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_OCTET << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_axis_machine_param,
            "siemenscnc_read_axis_machine_param"
        },
        {
            37,
            _SIEMENSCNC_CONTROL_UNIT_PARAM,
            5,
            4,
            ( TYPE_BASE_USHORT << 16 ) | ( TYPE_BASE_BUFF_PTR << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_control_unit_param,
            "siemenscnc_read_control_unit_param"
        },
        {
            38,
            _SIEMENSCNC_POWER_UNIT_PARAM,
            5,
            4,
            ( TYPE_BASE_USHORT << 16 ) | ( TYPE_BASE_BUFF_PTR << 12 ) | ( TYPE_BASE_USHORT << 8 ) | ( TYPE_BASE_USHORT << 4 ) | TYPE_BASE_VOID,
            "",
            NULL,
            ( pJobFunc )siemenscnc_read_power_unit_param,
            "siemenscnc_read_power_unit_param"
        }
    };
}

#endif

