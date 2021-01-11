#ifndef EZSOCKET_PRIVATE_H
#define EZSOCKET_PRIVATE_H
#include "ezsocket.h"
//# include <ws2tcpip.h>



EZSOCKET_BEGIN_DECLS
#define _EZSOCKET_MIN_REQ_LENGTH 1024
/* Timeouts in microsecond (0.5 s) */
#define EZSOCKET_RESPONSE_TIMEOUT    500000
#define EZSOCKET_BYTE_TIMEOUT        500000



#define EZSOCKET_RES_OK               0//成功
#define EZSOCKET_RES_TIMEOUT          -1//超时
#define EZSOCKET_RES_REV_TOO_LONG     -2//接收到的数据太长
#define EZSOCKET_RES_DECODE_ERR       -3//数据接收错误
#define EZSOCKET_RES_PARAM_ERR        -4//参数错误
#define EZSOCKET_RES_COMMAN_ERR       -5//未生成发送的指令
#define EZSOCKET_RES_AXIS_RANGE       -6//超过轴的范围
#define EZSOCKET_RES_SYSTEM_ERRCODE   -7//系统错误码


enum{
    /* 系统相关 */
    _EZSOCKET_VERSION=0,//版本信息
    _EZSOCKET_RUN_TIME=1,//运行时间
    _EZSOCKET_CYCLE_TIME=2,//循环时间
    _EZSOCKET_START_TIME=3,//开始时间
    _EZSOCKET_ESTIMATE_TIME=4,//预计的时间
    _EZSOCKET_KEEP_ALIVE_TIME=5,//上电时间
    /* 状态相关 */
    _EZSOCKET_CUTTING_MODEL=10,//切削模式
    _EZSOCKET_RUN_STATUS=11,//运行状态
    /* 加工相关 */
    _EZSOCKET_PROCESS_NUMBER=20,//加工数量
    _EZSOCKET_PROGRAM_NO=21,//程序号
    _EZSOCKET_SEQUENCE_NUM=22,//执行的语句
    _EZSOCKET_PROGRAM_BLOCK=23,//当前执行的内容
    /* 轴相关 */
    _EZSOCKET_AXIX_NUMBER=30,//轴数量
    _EZSOCKET_CURRENT_POSITION=31,//当前坐标
    _EZSOCKET_WORK_POSITION=32,//工作坐标
    _EZSOCKET_DISTANCE_POSITION=33,//剩余坐标
    _EZSOCKET_MACHINE_POSTION=34,//机械坐标
    _EZSOCKET_SPINDLE_SET_SPEED=35,//主轴设定速度
    _EZSOCKET_SPINDLE_ACTUAL_SPEED=36,//主轴实际速度
    _EZSOCKET_FEED_SET_SPEED=37,//进给设定速度
    _EZSOCKET_FEED_ACTUAL_SPEED=38,//进给实际速度
    _EZSOCKET_SPINDLE_LOAD=39,//主轴负载
    _EZSOCKET_SERVO_LOAD=40,//伺服轴负载
    _EZSOCKET_SERVO_SPEED=41,//伺服轴转速
    /* 刀具相关 */
    _EZSOCKET_MAG_TOOL_NO=50,//刀库刀号
    _EZSOCKET_SPD_TOOL_NO=51,//主轴刀具号
    _EZSOCKET_TOOLOFFSET_D_SHAPE=52,//刀具补偿D SHAPE
    _EZSOCKET_TOOLOFFSET_D_WEAR=53,//刀具补偿D WEAR
    _EZSOCKET_TOOLOFFSET_H=54,//刀具补偿H
    _EZSOCKET_TOOL_SET_SIZE=55,//刀具数量
    _EZSOCKET_TOOL_OFFSET_TYPE=56,//刀具补偿得类型
    _EZSOCKET_TOOL_OFFSET_VALUE=57,//刀具补偿值

    /* 模态相关 */
    _EZSOCKET_M_COMMAND=60,//M COMMAND
    _EZSOCKET_S_COMMAND=61,//S COMMAND
    _EZSOCKET_T_COMMAND=62,//T COMMAND
    _EZSOCKET_B_COMMAND=63,//B COMMAND
    /* 报警相关 */
    _EZSOCKET_ALARM_MSG=70,//报警信息
    /* PLC 相关 */
    _EZSOCKET_PLC_X=80,//PLC X PLC的输入信号
    _EZSOCKET_PLC_Y=81,//PLC Y PLC的输出信号
    _EZSOCKET_PLC_R=82,//PLC R PLC的文件寄存器
    _EZSOCKET_PLC_F=83,//PLC F PLC的临时存储区 存储报警信息 暂不确定 面板上的值为0
    _EZSOCKET_PLC_D=84,//PLC D PLC的数据库存储区
    _EZSOCKET_G_CODE=85,//G代码
    /* 参数 */
    _EZSOCKET_PARAM_READ=90,//参数读取
    /* 宏变量 */
    _EZSOCKET_READ_MACRO_VAR=95,//读取宏变量
    _EZSOCKET_WRITE_MACRO_VAR=97//写宏变量

};


typedef struct _ezsocket_backend {
    unsigned int header_length;//帧头长度
    unsigned int max_adu_length;//数据区最大长度
    int (*build_request_basis) (ezsocket_t *ctx,
                                int function,
                                int address,
                                int nb,
                                int src_nb,
                                const uint8_t *src,
                                uint8_t *req);//生成请求指令
    int (*receive_msg)  (ezsocket_t *ctx, uint8_t *msg, uint16_t *length);//接受数据
    int (*decode_msg)(ezsocket_t *ctx,uint8_t function,const uint8_t *msg,uint8_t *dest,uint16_t *length);//解析数据
    int (*send) (ezsocket_t *ctx, const uint8_t *req, int req_length);//发送数据
    int (*recv) (ezsocket_t *ctx, uint8_t *rsp, int rsp_length);//接受数据
    int (*connect) (ezsocket_t *ctx);//连接
    void (*close) (ezsocket_t *ctx);//关闭
    int (*flush) (ezsocket_t *ctx);//清空缓存
    int (*select) (ezsocket_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);//阻塞直到有数据
} ezsocket_backend_t;

struct _ezsocket {
    int s;/* Socket 描述符 */
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const ezsocket_backend_t *backend;
    void *backend_data;
};

void _ezsocket_init_common(ezsocket_t *ctx);
int _ezsocket_strlcpy(char *dest, const char *src, int dest_size);
void _ezsocket_swap_data(uint8_t *src, int number);//交换顺序
int _ezsocket_sleep_and_flush(ezsocket_t *ctx);
EZSOCKET_END_DECLS
#endif // SIMENSCNC_PRIVATE_H
