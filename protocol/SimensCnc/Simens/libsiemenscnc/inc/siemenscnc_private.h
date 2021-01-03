#ifndef SIMENSCNC_PRIVATE_H
#define SIMENSCNC_PRIVATE_H
#include "siemenscnc.h"
//# include <ws2tcpip.h>
SIEMENSCNC_BEGIN_DECLS
#define _SIEMENSCNC_MIN_REQ_LENGTH 1024
/* Timeouts in microsecond (0.5 s) */
#define SIEMENSCNC_RESPONSE_TIMEOUT    500000
#define SIEMENSCNC_BYTE_TIMEOUT        500000

#define SIEMENSCNC_RES_OK       0//成功
#define SIEMENSCNC_RES_TIMEOUT      -1//超时
#define SIEMENSCNC_RES_REV_TOO_LONG -2//接收到的数据太长
#define SIEMENSCNC_RES_DECODE_ERR   -10//数据接收错误
#define SIEMENSCNC_RES_PARAM_ERR    -11//参数错误
#define SIEMENSCNC_RES_COMMAN_ERR   -12//未生成发送的指令
#define SIEMENSCNC_RES_DECODE_ERR_DATATYPE -13 //解析到错误的数据类型
#define SIEMENSCNC_RES_SYS_NOT_CARRY    -14//系统不支持的指令
#define SIEMENSCNC_RES_SYS_ERR_CODE     -15//系统返回错误码

/* 3步来解析数据 */
typedef enum {
    _SIEMENSCNC_STEP_HEADER,
    _SIEMENSCNC_STEP_META,
    _SIEMENSCNC_STEP_DATA
} _siemenscnc_step_t;







enum{
    /* 系统相关 */
    _SIEMENSCNC_SERIAL_NUMBER=0,//硬件序列号
    _SIEMENSCNC_CNC_TYPE,//机床类型
    _SIEMENSCNC_VERSION,//版本信息
    _SIEMENSCNC_MANUFACTURE_DATE,//出厂日期
    _SIEMENSCNC_PROCESS_NUMBER,//加工数量
    _SIEMENSCNC_PROCESS_SET_NUMBER,//设定的加工数量
    _SIEMENSCNC_RUN_TIME,//循环时间,一次自动运行起动时间的累计值
    _SIEMENSCNC_REMAIN_TIME,//剩余时间
    _SIEMENSCNC_PROGRAM_NAME,//加工程序名
    /* 运行状态 */
    _SIEMENSCNC_OPERATE_MODE=10,//操作模式
    _SIEMENSCNC_STATUS,//运行状态
    /* 轴相关 */
    _SIEMENSCNC_AXIS_NAME=20,//轴名称
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
    _SIEMENSCNC_TOOL_CUR_NAME=40,//当前刀具名字
    _SIEMENSCNC_TOOL_CUR_T_NO,//当前刀具号
    _SIEMENSCNC_TOOL_CUR_D_NO,//当前刀沿号
    _SIEMENSCNC_TOOL_CUR_H_NO,//刀具当前H号
    _SIEMENSCNC_TOOL_LIST,//刀具列表
    _SIEMENSCNC_TOOL_X_OFFSET,//长度补偿X
    _SIEMENSCNC_TOOL_Z_OFFSET,//长度补偿Z
    /* 报警相关 */
    _SIEMENSCNC_CNC_NC_ALARM_NO=60,//NC报警数量
    _SIEMENSCNC_CNC_NC_ALARM,//NC报警
    /* 参数相关 */
    _SIEMENSCNC_R_VAR=65,//R变量
    _SIEMENSCNC_S_R_P_PARAM,//驱动器R P参数
    _SIEMENSCNC_GENERAL_MACHINE_PARAM,//通用机床数据
    _SIEMENSCNC_CHANNEL_MACHINE_PARAM,//通道机床数据
    _SIEMENSCNC_AXIS_MACHINE_PARAM,//轴机床数据
    _SIEMENSCNC_CONTROL_UNIT_PARAM,//控制单元参数
    _SIEMENSCNC_POWER_UNIT_PARAM//电源模块参数
};

typedef struct _siemenscnc_backend {
    unsigned int header_length;//帧头长度
    unsigned int max_adu_length;//数据区最大长度
    int (*build_request_basis) (siemenscnc_t *ctx,
                                int function,
                                unsigned short axis,
                                unsigned char axis_number,
                                int addr,
                                int sub_addr,
                                int src_nb,
                                uint8_t *req,
                                const uint8_t *src);//生成请求指令
    int (*receive_msg)  (siemenscnc_t *ctx, uint8_t *msg, uint16_t *length);//接受数据
    int (*decode_msg)(siemenscnc_t *ctx,uint8_t function,const uint8_t *msg,uint8_t *dest,uint16_t *length);//解析数据
    int (*send) (siemenscnc_t *ctx, const uint8_t *req, int req_length);//发送数据
    int (*recv) (siemenscnc_t *ctx, uint8_t *rsp, int rsp_length);//接受数据
    int (*connect) (siemenscnc_t *ctx);//连接
    void (*close) (siemenscnc_t *ctx);//关闭
    int (*flush) (siemenscnc_t *ctx);//清空缓存
    int (*select) (siemenscnc_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);
} siemenscnc_backend_t;

struct _siemenscnc {
    int s;/* Socket 描述符 */
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const siemenscnc_backend_t *backend;
    void *backend_data;
};

void _siemenscnc_init_common(siemenscnc_t *ctx);
int _siemenscnc_strlcpy(char *dest, const char *src, int dest_size);
void _siemenscnc_swap_data(uint8_t *src, int number);//交换顺序
int _siemenscnc_sleep_and_flush(siemenscnc_t *ctx);
SIEMENSCNC_END_DECLS
#endif // SIMENSCNC_PRIVATE_H
