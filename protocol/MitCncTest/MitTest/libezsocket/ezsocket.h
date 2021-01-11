#ifndef EZSOCKET_H
#define EZSOCKET_H

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
# define EZSOCKET_BEGIN_DECLS  extern "C" {
# define EZSOCKET_END_DECLS    }
#else
# define EZSOCKET_BEGIN_DECLS
# define EZSOCKET_END_DECLS
#endif
EZSOCKET_BEGIN_DECLS

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/* Max between RTU and TCP max adu length (so TCP) */
#define EZSOCKET_MAX_MESSAGE_LENGTH          1280


typedef struct _ezsocket ezsocket_t;
typedef enum
{
    EZSOCKET_ERROR_RECOVERY_NONE          = 0,
    EZSOCKET_ERROR_RECOVERY_LINK          = (1<<1),//自动重连
    EZSOCKET_ERROR_RECOVERY_PROTOCOL      = (1<<2),//延时并冲洗已经有的数据
} ezsocket_error_recovery_mode;

enum
{
    EZSOCKET_MODE_OTHER=0,
    EZSOCKET_MODE_JOG,//点动
    EZSOCKET_MODE_MDA,//半自动
    EZSOCKET_MODE_AUTO,//自动
    EZSOCKET_MODE_REF//参考点
};

enum
{
    EZSOCKET_STATUS_OTHER=0,//空闲
    EZSOCKET_STATUS_RESET,//复位
    EZSOCKET_STATUS_STOP,//停止
    EZSOCKET_STATUS_START,//启动
    EZSOCKET_STATUS_HOLDING//空闲

};
enum
{
    EZSOCKET_AXIS_SPINDLE=0,//主轴
    EZSOCKET_AXIS_X,//X轴
    EZSOCKET_AXIS_Y,//Y轴
    EZSOCKET_AXIS_Z,//Z轴

};


/* 3步来解析数据 */
typedef enum {
    _EZSOCKET_STEP_HEADER,
    _EZSOCKET_STEP_META,
    _EZSOCKET_STEP_DATA
} _ezsocket_step_t;


enum
{
    EZSOCKET_TOOL_TYPE_M_TYPE_I=0,//M system type-I : 1-axis compensation amount
    EZSOCKET_TOOL_TYPE_M_TYPE_II,//M system type-II : 1-axis compensation amount with wear compensation amount
    EZSOCKET_TOOL_TYPE_L_TYPE//L system type : 2-axis compensation amount


};


int ezsocket_connect(ezsocket_t *ctx);//建立连接
void ezsocket_close(ezsocket_t *ctx);//关闭连接
void ezsocket_set_socket(ezsocket_t *ctx, int socket);//设置socket
int ezsocket_get_socket(ezsocket_t *ctx);//获取socket
int ezsocket_set_error_recovery(ezsocket_t *ctx, ezsocket_error_recovery_mode error_recovery);//设置错误处理
void ezsocket_get_response_timeout(ezsocket_t *ctx, struct timeval *timeout);//获取响应超时时间
void ezsocket_set_response_timeout(ezsocket_t *ctx, const struct timeval *timeout);//设置响应超时时间
void ezsocket_get_byte_timeout(ezsocket_t *ctx, struct timeval *timeout);//获取字节超时时间
void ezsocket_set_byte_timeout(ezsocket_t *ctx, const struct timeval *timeout);//设置字节超时时间
int ezsocket_get_header_length(ezsocket_t *ctx);//获取帧头长度
void ezsocket_set_debug(ezsocket_t *ctx, int boolean);//设置调试开关
void ezsocket_free(ezsocket_t *ctx);//释放句柄
int ezsocket_flush(ezsocket_t *ctx);//清空数据流


uint16_t ezsocket_swaped_uint16(uint16_t value);//word大小端转换
uint32_t ezsocket_swaped_uint32(uint32_t value);//dword大小端转换
uint64_t ezsocket_swaped_uint64(uint64_t value);//uint64_t 大小端转换
float ezsocket_swaped_float(float value);//float大小端转换
double ezsocket_swaped_double(double value);//double 大小端转换


/* 系统相关 */
int ezsocket_read_version(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//版本信息 对应官方协议栈getversion
//int ezsocket_read_system_date(ezsocket_t *ctx, uint8_t *dst);//系统日期
//int ezsocket_read_system_time(ezsocket_t *ctx, uint8_t *dst);//系统时间
int ezsocket_read_run_time(ezsocket_t *ctx, int *time);//运行时间 对应官方协议栈getruntime
int ezsocket_read_cycle_time(ezsocket_t *ctx, int *time);//循环时间
int ezsocket_read_start_time(ezsocket_t *ctx, int *time);//开始时间 对应官方协议栈getstarttime
int ezsocket_read_estimate_time(ezsocket_t *ctx, int external_runout_type, int *time);//外部经过的时间 对应官方协议栈getestimatetime
int ezsocket_read_keep_alive_time(ezsocket_t *ctx, int *time);//上电时间 getalivetime
/* 状态相关 */
int ezsocket_read_cutting_model(ezsocket_t *ctx, uint8_t *dst);//是否处在切削模式  对应官方协议栈getcuttingmode
int ezsocket_read_run_status(ezsocket_t *ctx, uint8_t *dst);//运行状态 对应官方协议栈getrunstatus
int ezsocket_read_emg_status(ezsocket_t *ctx, uint8_t *dst);//急停状态 读PLC参数R69
/* 加工相关 */
int ezsocket_read_process_number(ezsocket_t *ctx, int *count);//加工数量 读参数8002
int ezsocket_read_program_no(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//程序号 对应官方协议栈getprogramnumber2
int ezsocket_read_sequence_num(ezsocket_t *ctx, int *seq);//执行的序号/行数 对应官方协议栈getsequencenumber
int ezsocket_read_program_block(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//程序块内容 对应官方协议栈currentblockread
/* 轴相关 */
int ezsocket_read_axis_number(ezsocket_t *ctx, uint8_t *dst);//轴数量 参数1002
int ezsocket_read_feed_rate(ezsocket_t *ctx, uint8_t *dst);//进给倍率 读PLC参数
int ezsocket_read_spindle_rate(ezsocket_t *ctx, uint8_t *dst);//主轴倍率 读PLC参数
int ezsocket_read_fast_mov_rate(ezsocket_t *ctx, uint8_t *dst);//快速移动倍率 读PLC参数
int ezsocket_read_current_position(ezsocket_t *ctx, int axis, double *pos);//当前坐标 getcurrentpositon
int ezsocket_read_work_position(ezsocket_t *ctx, int axis, double *pos);//工作坐标 get workpostion
int ezsocket_read_distance_position(ezsocket_t *ctx, int axis, double *pos);//剩余坐标 getdistance
int ezsocket_read_machine_position(ezsocket_t *ctx, int axis, double *pos);//机械坐标 getmachinepositon
int ezsocket_read_spindle_set_speed(ezsocket_t *ctx, int *speed);//主轴设定速度 待确认
int ezsocket_read_spindle_act_speed(ezsocket_t *ctx, int *speed);//主轴实际速度 待确认GetSpindleMonitor
int ezsocket_read_feed_set_speed(ezsocket_t *ctx, double *speed);//进给设定速度 对应官方协议栈getfeedcommand
int ezsocket_read_feed_act_speed(ezsocket_t *ctx, double *speed);//进给实际速度 对应官方协议栈getfeedcommand
int ezsocket_read_spindle_load(ezsocket_t *ctx, int *load);//主轴负载 待测 GetSpindleMonitor
int ezsocket_read_servo_load(ezsocket_t *ctx, int axis, int *load);//伺服轴负载 GetServoMinitor
int ezsocket_read_servo_act_speed(ezsocket_t *ctx, int axis, int *speed);//伺服轴转速 GetServoMinitor

/* 刀具相关 */
int ezsocket_read_mag_tool_no(ezsocket_t *ctx, int *tool_no);//刀库刀具号
int ezsocket_read_spd_tool_no(ezsocket_t *ctx, int *tool_no);//主轴刀具号 GETMGNREADY
int ezsocket_read_tooloffset_d_shape(ezsocket_t *ctx, int *shape);//刀具补偿D shape 待确认  对应官方协议栈gettoolcommand
int ezsocket_read_tooloffset_d_wear(ezsocket_t *ctx,  int *wear);//刀具补偿D wear  待确认    对应官方协议栈gettoolcommand
int ezsocket_read_tooloffset_h(ezsocket_t *ctx, int axis, int *offset_h);//刀具补偿 H 待确认 可指定轴 对应官方协议栈gettoolcommand
int ezsocket_read_tool_set_size(ezsocket_t *ctx, int *tool_set_size);//刀具数量 对应官方协议栈gettoolsetsize
int ezsocket_read_tool_offset_type(ezsocket_t *ctx, int *type);//刀具补偿类型 对应官方协议栈gettype
int ezsocket_read_tool_offset_value(ezsocket_t *ctx, int type, int kind, int tool_set_no,double *offset);//获取刀具补偿值 对应官方协议栈getoffset
/* 模态相关 */
int ezsocket_read_m_command_value(ezsocket_t *ctx, int command_index, int *m_value);//读取M模态值，指定模态编号 getcommand2
int ezsocket_read_s_command_value(ezsocket_t *ctx, int command_index, int *s_value);//读取S模态值，指定模态编号 getcommand2
int ezsocket_read_t_command_value(ezsocket_t *ctx, int command_index, int *t_value);//读取T模态值，指定模态编号 getcommand2
int ezsocket_read_b_command_value(ezsocket_t *ctx, int command_index, int *b_value);//读取B模态值，指定模态编号 getcommand2
/* 报警相关 */
int ezsocket_read_alarm_msg(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//报警信息 getalarm 最多10条
/* PLC 相关 */
int ezsocket_read_plc_x(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC X readdevice
int ezsocket_read_plc_y(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC Y readdevice
int ezsocket_read_plc_r(ezsocket_t *ctx, int address, uint16_t *dst);//PLC R 字 readdevice
int ezsocket_read_plc_f(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC F 位信号 地址 待确认 readdevice
int ezsocket_read_plc_d(ezsocket_t *ctx, int address, uint16_t *dst);//PLC D 字 readdevice
int ezsocket_read_g_code(ezsocket_t *ctx, int address, double *dst);//G代码 readdevice
/* 参数相关 */
int ezsocket_read_param(ezsocket_t *ctx, int address, double *value);//参数读取 getparameterdata2
/* 宏变量 */
int ezsocket_read_macro(ezsocket_t *ctx, int address, double *value);//读取宏变量 commonVread 100-199 599-999
int ezsocket_write_marco_var(ezsocket_t *ctx, int address, uint8_t *dst);//写宏变量

/* MSTB功能 见三菱ezsocket协议中page60
 * GetCommand2
 */
/* 刀具补偿类型
 * 1 M system type-I : 1-axis compensation amount
 * 4 M system type-II : 1-axis compensation amount with wear compensation amount
 * 6 L system type : 2-axis compensation amount
*/

/* G CODE 共21组 1-21
 * 1 Group 1 (Interpolation mode). G00, G01 G02, G03, and G33 command modal.
 * 2 Group 2 (Plane selection). G17, G18, and G19 command modal.
 * 3 Group3 (Absolute) G90 and (Incremental) G91 command modal.
 * 4 Group 4 (Chuck barrier) G22 and G23 command modal.
 * 5 Group 5 (Feed mode) G94 and G95 command modal.
 * 6 Group 6 (Inch) G20 and (mm) G21 command modal.
 * 7 Group 7 (Tool nose R compensation mode) G40, G41, and G42 command modal.
 * 8 Group 8 (Tool length compensation mode) G43, G44 and G49 command modal.
 * 9 Group 9 (Fixed cycle mode) G70, G71, G72, G73, G74, G75, G76, G77, G78, G79, G80,
  G81, G82, G83, G84, G85, G86, G87, G88, and G89 command modal.
 * 10 Group 10 (Initial return) G98, (Reference point return) G99 command modal.
 * 11 Group 11 G50.2 and G51.2 command modal. (Invalid with M6x5L and C64)
 * 12 Group 12 (Workpiece coordinate system modal) G54, G55, G56, G57, G58, and G59
command modal.
 * 13 Group 13 (Cutting mode) G61, G62, G63, and G64 command modal.
 * 14 Group 14 (Modal call) G66, G66.1, and G67 command modal.
 * 15 Group 15 (Normal line control) G40.1, G41.1 and G42.1 command modal. (Valid only with
    M6x5M or CNC700M)(Mirror image for facing tool posts) G68 and G69 modal (For C64L)
 * 16 Group 16 (Coordinate system rotation) G68 and G69 (Valid only with CNC700M)
 * 17 Group 17 (Constant peripheral speed control) G96 and G97 command modal.
 * 18 Group 18 (Balance cut/polar coordinate command) G14, G15 and G16 command modal.
 * 19 Group 19 (G command mirror image) G50.1 and G51.1 command modal.
 * 20 Group 20 (Spindle selection) G43.1, G44.1 and G47.1 command modal. (Invalid with
   M6x5M/L)
 * 21 Group 21(Cylindrical interpolation/polar coordinate interpolation) G07.1, G107, G12.1,
   G112, G13.1, G113 (Valid only with CNC700M)
 */

/* 急停
 * R69 0有效
 * R69.0 内置PLC停止
 * 69.1 外部PLC FROM TO 命令未执行
 * 69.2 外部PLC准备未完成
 * 69.3 外部PLC通信异常
 * 69.4 控制装置EMG连接器紧急停止状态
 * 69.5 电源供给外部紧急停止状态
 * 69.6 内置PLC S/W紧急停止输出YC2C未1
 * 69.7 NULL
 * 69.8 接触器阻断测试
 * 69.9 LINE
 * 69.A null
 * 69.B 用户PLC有错误代码
 * 69.C plc高速处理异常
 * 69.D 门互锁 dog/OT任意布局 装置错误
 * 69.E 主轴驱动单元 紧急停止输出
 * 69.F 伺服驱动单元 紧急停止输出
*/

/* 电机转速 见M70PLC接口手册 PAGE 77-78.
 * 系统1    系统2   系统3   系统4   信号名称
R4820     R4836   R4852   R4868
R4821     R4837   R4853   R4869  电机转速 第1轴
* 依次类推 共8轴
*/

/* 电机负载电流 见M70PLC接口手册 PAGE 78
 *系统1   系统2    系统3    系统4   信号名称
R4884    R4900   R4916    R4932
R4885    R4901   R4917    R4933 电机负载电流 第1轴
* 依次类推 共8轴
*/

/* 主轴倍率
 * Y1888(hex)
 *
*/
/* 进给倍率
 * YC60(hex)
*/
/* 快速移动倍率
 * YC68
*/
/* 主轴转速调整率D5
 */

/**
 * UTILS FUNCTIONS
 **/

#define EZSOCKET_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define EZSOCKET_GET_LOW_BYTE(data) ((data) & 0xFF)
#define EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(tab_int8, index) ((tab_int8[(index) + 3] << 24) + (tab_int8[(index) + 2] << 16) + (tab_int8[(index) + 1] << 8) + tab_int8[(index)])
#define EZSOCKET_GET_INT16_FROM_INT8_LITTLE_ENDAIN(tab_int8, index) ((tab_int8[(index) + 1] << 8) + tab_int8[(index)])
#define EZSOCKET_SET_INT16_TO_INT8_LITTLE_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index) + 1] = (value) >> 8;  \
    tab_int8[(index)    ] = (value) & 0xFF; \
    } while (0)
#define EZSOCKET_SET_INT32_TO_INT8_LITTLE_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index) + 3] = (value) >> 24; \
    tab_int8[(index) + 2] = (value) >> 16; \
    tab_int8[(index) + 1] = (value) >> 8; \
    tab_int8[(index)    ] = (value); \
    } while (0)



#define EZSOCKET_GET_INT32_FROM_INT8_BIG_ENDAIN(tab_int8, index) ((tab_int8[(index)] << 24) + (tab_int8[(index) + 1] << 16) + (tab_int8[(index) + 2] << 8) + tab_int8[(index) + 3])
#define EZSOCKET_GET_INT16_FROM_INT8_BIG_ENDAIN(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define EZSOCKET_SET_INT16_TO_INT8_BIG_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index)] = (value) >> 8;  \
    tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define EZSOCKET_SET_INT32_TO_INT8_BIG_ENDAIN(tab_int8, index, value) \
    do { \
    tab_int8[(index)    ] = (value) >> 24; \
    tab_int8[(index) + 1] = (value) >> 16; \
    tab_int8[(index) + 2] = (value) >> 8; \
    tab_int8[(index) + 3] = (value); \
    } while (0)
#include "ezsocket_private.h"
#include "ezsocket_new.h"
EZSOCKET_END_DECLS
#endif // SIMENSCNC_H
