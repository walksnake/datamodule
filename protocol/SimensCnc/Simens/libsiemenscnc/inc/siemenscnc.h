#ifndef SIEMENSCNC_H
#define SIEMENSCNC_H

#ifndef _MSC_VER
#include <stdint.h>
#include <sys/time.h>
#include <stdio.h>
#else
#include "stdint.h"
#include <time.h>
#endif

#if defined(_WIN32)
#include <ws2tcpip.h>
#endif


#ifdef  __cplusplus
# define SIEMENSCNC_BEGIN_DECLS  extern "C" {
# define SIEMENSCNC_END_DECLS    }
#else
# define SIEMENSCNC_BEGIN_DECLS
# define SIEMENSCNC_END_DECLS
#endif
SIEMENSCNC_BEGIN_DECLS

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SIEMENSCNC_LITTLE_ENDIAN //默认将数据转为小端

//西门子CNC为小端模式 float为大端

/* Max between RTU and TCP max adu length (so TCP) */
#define SIEMENSCNC_MAX_MESSAGE_LENGTH          512
#define SIEMENSCNC_MAX_READ_REGISTERS          460


typedef struct _siemenscnc siemenscnc_t;
typedef enum
{
    SIEMENSCNC_ERROR_RECOVERY_NONE          = 0,
    SIEMENSCNC_ERROR_RECOVERY_LINK          = (1<<1),//自动重连
    SIEMENSCNC_ERROR_RECOVERY_PROTOCOL      = (1<<2),//延时并冲洗已经有的数据
} siemenscnc_error_recovery_mode;

enum
{
    SIEMENSCNC_MODE_OTHER=0,
    SIEMENSCNC_MODE_JOG,//点动 手动
    SIEMENSCNC_MODE_MDA,//半自动
    SIEMENSCNC_MODE_AUTO,//自动
    SIEMENSCNC_MODE_REFPOS,//参考点
    SIEMENSCNC_MODE_REF_POINT//零点
};

enum
{
    _SIEMENSCNC_STATUS_OTHER=0,//空闲
    _SIEMENSCNC_STATUS_RESET,//复位 SPINDLE CW STOP
    _SIEMENSCNC_STATUS_STOP,//停止 (SPINDLE CW 主轴正反转)
    _SIEMENSCNC_STATUS_START,//启动 SPINDLE CW
    _SIEMENSCNC_STATUS_SPENDLE_CW_CCW// SPINDLE CCW(主轴反转)
};


int siemenscnc_connect(siemenscnc_t *ctx);//建立连接
void siemenscnc_close(siemenscnc_t *ctx);//关闭连接
void siemenscnc_free(siemenscnc_t *ctx);//释放句柄
int siemenscnc_flush(siemenscnc_t *ctx);//清空数据流
void siemenscnc_set_socket(siemenscnc_t *ctx, int socket);//设置socket
int siemenscnc_get_socket(siemenscnc_t *ctx);//获取socket
int siemenscnc_set_error_recovery(siemenscnc_t *ctx, siemenscnc_error_recovery_mode error_recovery);//错误处理
void siemenscnc_get_response_timeout(siemenscnc_t *ctx, struct timeval *timeout);//获取超时时间
void siemenscnc_set_response_timeout(siemenscnc_t *ctx, const struct timeval *timeout);//设置超时时间
void siemenscnc_get_byte_timeout(siemenscnc_t *ctx, struct timeval *timeout);//获取去字节间超时
void siemenscnc_set_byte_timeout(siemenscnc_t *ctx, const struct timeval *timeout);//设置字节间超时
int siemenscnc_get_header_length(siemenscnc_t *ctx);//获取协议头长度
void siemenscnc_set_debug(siemenscnc_t *ctx, int boolean);//打印调试信息

uint16_t siemenscnc_swaped_uint16(uint16_t value);//word大小端转换
uint32_t siemenscnc_swaped_uint32(uint32_t value);//dword大小端转换
float siemenscnc_swaped_float(float value);//float大小端转换
double siemenscnc_swaped_double(double value);//double大小端转换
uint16_t siemenscnc_get_16(const uint8_t *src);//获取16位整型
uint32_t siemenscnc_get_32(const uint8_t *src);//获取32位整型
float siemenscnc_get_float(const uint8_t *src);//获取float
double siemenscnc_get_double(const uint8_t *src);//获取double

/* 协议交互 */
/* 系统相关 */
int siemenscnc_read_serial_number(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//硬件序列号
int siemenscnc_read_cnc_type(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//设备类型
int siemenscnc_read_verison(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//版本信息
int siemenscnc_read_manufacture_data(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//出厂日期
int siemenscnc_read_process_number(siemenscnc_t *ctx, double *dst);//加工数量
int siemenscnc_read_process_set_number(siemenscnc_t *ctx, double *dst);//设定的加工数量
int siemenscnc_read_run_time(siemenscnc_t *ctx, double *dst);//循环时间,一次自动运行起动时间的累计值
int siemenscnc_read_remain_time(siemenscnc_t *ctx, double *dst);//剩余时间
int siemenscnc_read_program_name(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//加工程序名
/* 状态相关 */
int siemenscnc_read_operate_mode(siemenscnc_t *ctx, uint8_t *dst);//操作模式
int siemenscnc_read_status(siemenscnc_t *ctx, uint8_t *dst);//运行状态
/* 轴相关 */
int siemenscnc_read_axis_name(siemenscnc_t *ctx, uint8_t axis, uint8_t *dst, uint16_t *length);//轴名称 同上
int siemenscnc_read_machine_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//机械坐标 根据主界面（加工界面/坐标界面）坐标显示的轴顺序
int siemenscnc_read_relatively_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//工件坐标 相对坐标 同上
int siemenscnc_read_remain_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//剩余坐标 同上
int siemenscnc_read_feed_set_speed(siemenscnc_t *ctx, double *dst);//设定进给速度
int siemenscnc_read_feed_act_speed(siemenscnc_t *ctx, double *dst);//实际进给速度
int siemenscnc_read_feed_rate(siemenscnc_t *ctx, double *dst);//进给倍率
int siemenscnc_read_spindle_set_speed(siemenscnc_t *ctx, double *dst);//主轴设定速度
int siemenscnc_read_spindle_act_speed(siemenscnc_t *ctx, double *dst);//主轴实际速度
int siemenscnc_read_spindle_rate(siemenscnc_t *ctx, double *dst);//主轴倍率
int siemenscnc_read_driver_voltage(siemenscnc_t *ctx, uint8_t driver, float *dst);//母线电压 调试->驱动参数->面板右侧选择驱动，第一个即为第一个驱动器，依次类推
int siemenscnc_read_driver_current(siemenscnc_t *ctx, uint8_t driver, float *dst);//实际电流 同上
int siemenscnc_read_driver_power(siemenscnc_t *ctx, uint8_t driver, float *dst);//电机功率 同上
int siemenscnc_read_driver_temper(siemenscnc_t *ctx, uint8_t driver, float *dst);//电机温度 同上
int siemenscnc_read_g_coordinate_t(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst);//G T坐标系
int siemenscnc_read_g_coordinate_m(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst);//G T坐标系
/* 刀具相关 */
int siemenscnc_read_tool_cur_name(siemenscnc_t *ctx, uint8_t *tool_name, uint16_t *length);//刀具号
int siemenscnc_read_tool_cur_t_no(siemenscnc_t *ctx, double *dst);//当前刀具号
int siemenscnc_read_tool_cur_d_no(siemenscnc_t *ctx, double *dst);//当前刀具刀沿号
int siemenscnc_read_tool_cur_h_no(siemenscnc_t *ctx, double *dst);//当前刀具H号
int siemenscnc_read_tool_type(siemenscnc_t *ctx, int tool_edge_no,  int tool_row, double *dst);//刀具类型
int siemenscnc_read_tool_length(siemenscnc_t *ctx, int tool_edge_no,  int tool_row, double *dst);//刀具长度
int siemenscnc_read_tool_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具半径
int siemenscnc_read_tool_edge(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀沿位置
int siemenscnc_read_tool_tip(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具齿数
int siemenscnc_read_tool_h_no(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀沿H号
int siemenscnc_read_tool_wear_length(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具长度磨损
int siemenscnc_read_tool_wear_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具半径磨损
int siemenscnc_read_tool_x_offset(siemenscnc_t *ctx, double *dst);//X补偿
int siemenscnc_read_tool_z_offset(siemenscnc_t *ctx, double *dst);//Z补偿


/* 报警相关 */
int siemenscnc_read_cnc_nc_alarm_no(siemenscnc_t *ctx, uint16_t *dst);//NC报警数量
int siemenscnc_read_cnc_nc_alarm(siemenscnc_t *ctx, uint16_t alarm_no, uint32_t *dst);//NC报警
/* 参数相关 */
int siemenscnc_read_r_var(siemenscnc_t *ctx, uint16_t address, double *dst);//R变量 查看方法 参数->R用户变量->选择要读取的参数
int siemenscnc_read_s_r_p_param(siemenscnc_t *ctx, uint8_t driver, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//驱动器R参数  查看方法：调试->机床数据->驱动参数->面板右侧选择驱动，第一个即为第一个驱动器，依次类推->选择要读写的地址，若地址无[]这个，则sub_address为0
int siemenscnc_read_general_machine_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//通用机床数据 查看方法：调试->机床数据->通用机床数据
int siemenscnc_read_channel_machine_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//通道机床数据 查看方法：调试->机床数据->通道机床数据
int siemenscnc_read_axis_machine_param(siemenscnc_t *ctx, uint8_t axis, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//轴机床数据 查看方法：调试->机床数据->轴机床数据
int siemenscnc_read_control_unit_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//控制单元参数 查看方法：调试->机床数据->控制单元参数
int siemenscnc_read_power_unit_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length);//电源模块参数 查看方法：调试->机床数据->电源模块参数


/**
 * UTILS FUNCTIONS
 **/

#define SIEMENSCNC_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define SIEMENSCNC_GET_LOW_BYTE(data) ((data) & 0xFF)
#define SIEMENSCNC_GET_INT32_FROM_INT8_LITTLE_ENDAIN(tab_int8, index) ((tab_int8[(index) + 3] << 24) + (tab_int8[(index) + 2] << 16) + (tab_int8[(index) + 1] << 8) + tab_int8[(index)])
#define SIEMENSCNC_GET_INT16_FROM_INT8_LITTLE_ENDAIN(tab_int8, index) ((tab_int8[(index) + 1] << 8) + tab_int8[(index)])
#define SIEMENSCNC_SET_INT16_TO_INT8_LITTLE_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index) + 1] = (value) >> 8;  \
    tab_int8[(index)    ] = (value) & 0xFF; \
    } while (0)
#define SIEMENSCNC_SET_INT32_TO_INT8_LITTLE_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index) + 3] = (value) >> 24; \
    tab_int8[(index) + 2] = (value) >> 16; \
    tab_int8[(index) + 1] = (value) >> 8; \
    tab_int8[(index)    ] = (value); \
    } while (0)



#define SIEMENSCNC_GET_INT32_FROM_INT8_BIG_ENDAIN(tab_int8, index) ((tab_int8[(index)] << 24) + (tab_int8[(index) + 1] << 16) + (tab_int8[(index) + 2] << 8) + tab_int8[(index) + 3])
#define SIEMENSCNC_GET_INT16_FROM_INT8_BIG_ENDAIN(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define SIEMENSCNC_SET_INT16_TO_INT8_BIG_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index)] = (value) >> 8;  \
    tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define SIEMENSCNC_SET_INT32_TO_INT8_BIG_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index)    ] = (value) >> 24; \
    tab_int8[(index) + 1] = (value) >> 16; \
    tab_int8[(index) + 2] = (value) >> 8; \
    tab_int8[(index) + 3] = (value); \
    } while (0)

SIEMENSCNC_END_DECLS
#endif // SIMENSCNC_H
