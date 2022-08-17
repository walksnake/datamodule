#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
/* Win32 with MinGW, supplement to <errno.h> */
#include <winsock2.h>
#define ECONNRESET   WSAECONNRESET
#define ECONNREFUSED WSAECONNREFUSED
#define ETIMEDOUT    WSAETIMEDOUT
#define ENOPROTOOPT  WSAENOPROTOOPT
#endif

#include "ezsocket.h"
#include "ezsocket_private.h"



/* Sends a request/response */
static int send_msg(ezsocket_t *ctx, uint8_t *msg, int msg_length)
{
    int rc;
    int i;

    if (ctx->debug) {
        printf("send:");
        for (i = 0; i < msg_length; i++)
            printf("[%.2X]", msg[i]);
        printf("\n");
    }

    /* In recovery mode, the write command will be issued until to be
       successful! Disabled by default. */
    do {
        rc = ctx->backend->send(ctx, msg, msg_length);
        if (rc == -1) {
            //_error_print(ctx, NULL);
            if (ctx->error_recovery & EZSOCKET_ERROR_RECOVERY_PROTOCOL) {
                int saved_errno = errno;

                if ((errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
                    ezsocket_close(ctx);
                    ezsocket_connect(ctx);
                } else {
                    _ezsocket_sleep_and_flush(ctx);
                }
                errno = saved_errno;
            }
        }
    } while ((ctx->error_recovery & EZSOCKET_ERROR_RECOVERY_PROTOCOL) &&
             rc == -1);

    if (rc > 0 && rc != msg_length) {
        //errno = EMBBADDATA;
        return -1;
    }

    return rc;
}

/* 设置模式 */
int ezsocket_set_error_recovery(ezsocket_t *ctx, ezsocket_error_recovery_mode error_recovery)
{
    if(ctx == NULL)
        return -1;
    ctx->error_recovery = error_recovery;
}
/* 设置socket嵌套字 */
void ezsocket_set_socket(ezsocket_t *ctx, int socket)
{
    if(ctx == NULL)
        return;
    ctx->s = socket;
}

/* 获取嵌套字 */
int ezsocket_get_socket(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->s;
}

/* 获取响应超时时间 */
void ezsocket_get_response_timeout(ezsocket_t *ctx, struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    *timeout = ctx->response_timeout;
}

/* 设置超时时间 */
void ezsocket_set_response_timeout(ezsocket_t *ctx, const struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    ctx->response_timeout = *timeout;
}

/* 获取字节间接受超时时间 */
void ezsocket_get_byte_timeout(ezsocket_t *ctx, struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    *timeout = ctx->byte_timeout;
}

/* 设置字节间接受超时时间 */
void ezsocket_set_byte_timeout(ezsocket_t *ctx, const struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    ctx->byte_timeout = *timeout;
}

/* 获取帧头长度 */
int ezsocket_get_header_length(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->header_length;
}

/* 建立连接 */
int ezsocket_connect(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->connect(ctx);
}

/* 断开连接 */
void ezsocket_close(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return;
    return ctx->backend->close(ctx);
}

/* 释放 */
void ezsocket_free(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return;
    free(ctx->backend_data);
    free(ctx);
}

/* 清空缓存流 */
int ezsocket_flush(ezsocket_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->flush(ctx);
}

/* 设置调试模式 */
void ezsocket_set_debug(ezsocket_t *ctx, int boolean)
{
    if(ctx == NULL)
        return;
    ctx->debug = boolean;
}

/* 协议交互 */
/* 系统相关 */
int ezsocket_read_version(ezsocket_t *ctx, uint8_t *dst, uint16_t *length)//版本信息
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_VERSION,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_VERSION,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

/*int ezsocket_read_system_date(ezsocket_t *ctx, uint8_t *dst)//系统日期
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,_EZSOCKET_SYSTEM_DATE,
                                                   0,
                                                   NULL, 0,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc == -1)
            return -1;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SYSTEM_DATE,rsp,dst,(uint16_t*)&rsp_length);
    }

    return rc;
}*/

/*int ezsocket_read_system_time(ezsocket_t *ctx, uint8_t *dst)//系统时间
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,_EZSOCKET_SYSTEM_TIME,
                                                   0,
                                                   NULL, 0,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc == -1)
            return -1;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SYSTEM_TIME,rsp,dst,(uint16_t*)&rsp_length);
    }

    return rc;
}*/


int ezsocket_read_run_time(ezsocket_t *ctx, int *time)//运行时间 格式举例：返回值位13914052 则为1391：40：52
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8] = {0};
    int temp_time;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_RUN_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_RUN_TIME,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
        {
            temp_time = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *time = temp_time%60;
            temp_time -= temp_time%60;
            *time += (temp_time%6000)/100*60;
            temp_time -= temp_time%6000;
            temp_time /= 10000;
            *time +=  temp_time*3600;
        }
    }

    return rc;
}

int ezsocket_read_cycle_time(ezsocket_t *ctx, int *time)//循环时间
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_CYCLE_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_CYCLE_TIME,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *time = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_start_time(ezsocket_t *ctx, int *time)//开始时间 格式举例：返回值位13914052 则为1391：40：52
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    int temp_time;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_START_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_START_TIME,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
        {
            temp_time = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *time = temp_time%60;
            temp_time -= temp_time%60;
            *time += (temp_time%6000)/100*60;
            temp_time -= temp_time%6000;
            temp_time /= 10000;
            *time +=  temp_time*3600;
        }
    }

    return rc;
}

/*
 * 外部经过的时间
 * external_runout_type 含义
 * 0 Counted when PLC device Y234 is ON (Y344 for M6x5M; Y704 for CNC700)
 * 1 Counted when PLC device Y235 is ON (Y345 for M6x5M; Y705 for CNC700)
 * Example: 9999 : 59 : 59 = 99995959
 */
int ezsocket_read_estimate_time(ezsocket_t *ctx,  int external_runout_type,  int *time)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    int command;
    int temp_time;
    if(external_runout_type < 0 || external_runout_type > 1 )
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    command = 0x04+external_runout_type;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_ESTIMATE_TIME,
                                                   command,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_ESTIMATE_TIME,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
        {
            temp_time = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *time = temp_time%60;
            temp_time -= temp_time%60;
            *time += (temp_time%6000)/100*60;
            temp_time -= temp_time%6000;
            temp_time /= 10000;
            *time +=  temp_time*3600;
        }
    }

    return rc;
}

int ezsocket_read_keep_alive_time(ezsocket_t *ctx, int *time)//上电时间 格式举例：返回值位13914052 则为1391：40：52
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    int temp_time;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_KEEP_ALIVE_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_KEEP_ALIVE_TIME,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
        {
            temp_time = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *time = temp_time%60;
            temp_time -= temp_time%60;
            *time += (temp_time%6000)/100*60;
            temp_time -= temp_time%6000;
            temp_time /= 10000;
            *time +=  temp_time*3600;
        }
    }

    return rc;
}

/* 状态相关 */
int ezsocket_read_cutting_model(ezsocket_t *ctx, uint8_t *dst)//切削模式
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_CUTTING_MODEL,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_CUTTING_MODEL,rsp,dst,(uint16_t*)&rsp_length);
    }

    return rc;
}

/* 运行状态 */
static int _ezsocket_read_run_status(ezsocket_t *ctx, uint8_t index, uint8_t *status)//单个运行状态
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_RUN_STATUS,
                                                   index,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_RUN_STATUS,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *status = dst[0];
    }

    return rc;
}
int ezsocket_read_run_status(ezsocket_t *ctx, uint8_t *dst)//运行状态
{
    int i;
    int rc;
    uint8_t status;

    /* status[0] == 0 未执行刀具长度测量 1 执行刀具长度测量
     * status[1] == 0 不处于自动操作 1 处于自动操作
     * status[2] == 0 未执行自动操作启动 1 执行自动操作启动
     */
    *dst = 0;
    for( i=0;i<3;i++)
    {
        rc = _ezsocket_read_run_status(ctx,i,&status);
        *dst |= (status << i);
        if(rc < 0)
        {
            return rc;
        }
    }
    /* *dst == 2/3 暂停
     * *dst == 6/7 运行
     */
    /*if(status[0] == 0x00 && status[1] == 0x00 && status[2] == 0x00)
    {
        dst[0] = EZSOCKET_STATUS_RESET;//复位
    }
    else if(status[0] == 0x01 && status[1] == 0x00 && status[2] == 0x00)
    {
        dst[0] = EZSOCKET_STATUS_STOP;//停止
    }
    else if(status[0] == 0x01 && status[1] == 0x00 && status[2] == 0x01)
    {
        dst[0] = EZSOCKET_STATUS_HOLDING;
    }
    else if(status[0] == 0x01 && status[1] == 0x01 && status[2] == 0x00)
    {
        dst[0] = EZSOCKET_STATUS_START;//启动
    }
    else
    {
        dst[0] = EZSOCKET_STATUS_OTHER;//空闲
    }*/
    return rc;
}

/* 急停状态  读PLC参数R69 */
int ezsocket_read_emg_status(ezsocket_t *ctx, uint8_t *dst)
{
    uint16_t data;
    int rc;
    rc = ezsocket_read_plc_r(ctx,69,&data);
    if(EZSOCKET_RES_OK == rc)
    {
        if(0xFFFF == data)
        {
            *dst = 0;
        }
        else
        {
            *dst = 1;
        }
    }
    return rc;
}

/* 加工相关 */
int ezsocket_read_process_number(ezsocket_t *ctx, int *count)//加工数量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8]={0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PROCESS_NUMBER,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PROCESS_NUMBER,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *count = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }
    return rc;
}

int ezsocket_read_program_no(ezsocket_t *ctx, uint8_t *dst, uint16_t *length)//程序号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PROGRAM_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PROGRAM_NO,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int ezsocket_read_sequence_num(ezsocket_t *ctx, int *seq)//执行的语句编号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8]={0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SEQUENCE_NUM,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SEQUENCE_NUM,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *seq = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}


int ezsocket_read_program_block(ezsocket_t *ctx, uint8_t *dst, uint16_t *length)//程序块内容
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PROGRAM_BLOCK,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PROGRAM_BLOCK,rsp,dst,(uint16_t*)length);
    }

    return rc;
}


/* 轴相关 */
int ezsocket_read_axis_number(ezsocket_t *ctx, uint8_t *axis_number)//轴数量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_AXIX_NUMBER,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_AXIX_NUMBER,rsp,dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *axis_number = dest[0];
    }

    return rc;
}

int ezsocket_read_feed_rate(ezsocket_t *ctx, uint8_t *dst)//进给倍率
{
    int value;
    int i;
    int rc = ezsocket_read_plc_y(ctx,0xC60,0xFF,(char*)&value);
    if(rc == EZSOCKET_RES_OK )
    {
        if((value & 0x80) == 0x80 )
        {
            uint16_t w_value;
            rc = ezsocket_read_plc_r(ctx,2500,(uint16_t*)&w_value);
            *dst = w_value;
            return rc;
        }
        else
        {
            char rates[32] = { 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A ,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01};
            for(i = 0;i< 31;i++)
            {
                if((value & 0x1F) == rates[i])
                {
                    *dst = (31-rates[i])*10;
                    break;
                }
            }
        }
    }
    return rc;
}

int ezsocket_read_spindle_rate(ezsocket_t *ctx, uint8_t *dst)//主轴倍率
{
    int value;
    int rc = ezsocket_read_plc_y(ctx,0x1888,0xFF,(char*)&value);
    if(rc == EZSOCKET_RES_OK )
    {
        if((value & 0x80) == 0x80 )
        {
            uint16_t w_value;
            rc = ezsocket_read_plc_r(ctx,7008,(uint16_t*)&w_value);
            *dst = w_value;
            return rc;
        }
        else
        {
            value &= 0x07;
            switch(value)
            {
            case 0x07:
                *dst = 50;
                break;
            case 0x06:
                *dst = 80;
                break;
            case 0x05:
                *dst = 120;
                break;
            case 0x04:
                *dst = 90;
                break;
            case 0x03:
                *dst = 60;
                break;
            case 0x02:
                *dst = 70;
                break;
            case 0x01:
                *dst = 110;
                break;
            case 0x00:
                *dst = 100;
                break;
            }
        }
    }
    return rc;
}

int ezsocket_read_fast_mov_rate(ezsocket_t *ctx, uint8_t *dst)//快速移动倍率
{
    int value;
    int rc = ezsocket_read_plc_y(ctx,0xC68,0xFF,(char*)&value);
    if(rc == EZSOCKET_RES_OK )
    {
        if((value & 0x80) == 0x80 )
        {
            uint16_t w_value;
            rc = ezsocket_read_plc_r(ctx,2502,(uint16_t*)&w_value);
            *dst = w_value;
            return rc;
        }
        else
        {
            value &= 0x03;
            switch(value)
            {
            case 0:
                *dst = 100;
                break;
            case 1:
                *dst = 50;
                break;
            case 2:
                *dst = 25;
                break;
            case 3:
                *dst = 1;
                break;
            }
        }
    }
    return rc;
}


/* 当前坐标 */
//static int _ezsocket_read_current_position(ezsocket_t *ctx, int index, uint8_t *dst)//当前坐标
int ezsocket_read_current_position(ezsocket_t *ctx, int axis,  double *pos)//当前坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_CURRENT_POSITION,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_CURRENT_POSITION,rsp,dst,(uint16_t*)&rsp_length);
        if( EZSOCKET_RES_OK == rc)
            memcpy((char*)pos,dst,8);
    }

    return rc;
}

/*int ezsocket_read_current_position(ezsocket_t *ctx, int start,int number,  double *pos)//当前坐标
{
    int i;
    int rc;
    int ptr = 0;
    uint8_t dst[12] = {0};
    if(start < 0 || number >5)//最多不超过5个轴 并且起始轴编号药大于等于0
    {
        return EZSOCKET_RES_PARAM_ERR;
    }
    for(i=start;i<number;i++)
    {
        rc = _ezsocket_read_current_position(ctx,i,dst);
        if(rc < 0)
        {
            return rc;
        }
        memcpy((char*)&pos[ptr++],dst,8);
    }
    return rc;
}*/

/* 工作坐标 */
//static int _ezsocket_read_work_position(ezsocket_t *ctx, int index, uint8_t *dst)//工作坐标
int ezsocket_read_work_position(ezsocket_t *ctx, int axis,  double *pos)//工作坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_WORK_POSITION,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_WORK_POSITION,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)pos,dst,8);
    }

    return rc;
}

/*int ezsocket_read_work_position(ezsocket_t *ctx, int start,int number,  double *pos)//工作坐标
{
    int i;
    int rc;
    volatile int ptr = 0;
    uint8_t dst[12];
    if(start < 0 || number >5)//最多不超过5个轴 并且起始轴编号药大于等于0
    {
        return EZSOCKET_RES_PARAM_ERR;
    }
    for(i=start;i<number;i++)
    {
        rc = _ezsocket_read_work_position(ctx,i,dst);
        if(rc < 0)
        {
            break;
        }
        memcpy((char*)&pos[ptr++],dst,8);
    }
    return rc;
}*/


/* 剩余坐标 */
//static int _ezsocket_read_distance_position(ezsocket_t *ctx, int index, uint8_t *dst)//剩余坐标
int ezsocket_read_distance_position(ezsocket_t *ctx, int axis, double *pos)//剩余坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_DISTANCE_POSITION,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_DISTANCE_POSITION,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)pos,dst,8);
    }

    return rc;
}

/*int ezsocket_read_distance_position(ezsocket_t *ctx, int start,int number,  double *pos)//剩余坐标
{
    int i;
    int rc;
    int ptr = 0;
    uint8_t dst[12];
    if(start < 0 || number >5)//最多不超过5个轴 并且起始轴编号药大于等于0
    {
        return EZSOCKET_RES_PARAM_ERR;
    }

    for(i=start;i<number;i++)
    {
        rc = _ezsocket_read_distance_position(ctx,i,dst);
        if(rc < 0)
        {
            return rc;
        }
        memcpy((char*)&pos[ptr++],dst,8);
    }
    return rc;
}*/

/* 机械坐标 */
//static int _ezsocket_read_machine_position(ezsocket_t *ctx, int index, uint8_t *dst)//机械坐标
int ezsocket_read_machine_position(ezsocket_t *ctx, int axis, double *pos)//机械坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_MACHINE_POSTION,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_MACHINE_POSTION,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)pos,dst,8);
    }

    return rc;
}

/*int ezsocket_read_machine_position(ezsocket_t *ctx, int start,int number,  double *pos)//机械坐标
{
    int i;
    int rc;
    int ptr = 0;
    uint8_t dst[12];
    if(start < 0 || number >5)//最多不超过5个轴 并且起始轴编号药大于等于0
    {
        return EZSOCKET_RES_PARAM_ERR;
    }

    for(i=start;i<number;i++)
    {
        rc = _ezsocket_read_machine_position(ctx,i,dst);
        if(rc < 0)
        {
            return rc;
        }
        memcpy((char*)&pos[ptr++],dst,8);
    }
    return rc;
}*/



int ezsocket_read_spindle_set_speed(ezsocket_t *ctx, int *speed)//主轴设定速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8];
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SPINDLE_SET_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SPINDLE_SET_SPEED,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *speed = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_spindle_act_speed(ezsocket_t *ctx, int *speed)//主轴实际速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8];
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SPINDLE_ACTUAL_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SPINDLE_ACTUAL_SPEED,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *speed = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_feed_set_speed(ezsocket_t *ctx, double *speed)//进给设定速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12];
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_FEED_SET_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_FEED_SET_SPEED,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)speed,dst,8);
    }


    return rc;
}

int ezsocket_read_feed_act_speed(ezsocket_t *ctx, double *speed)//进给实际速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_FEED_ACTUAL_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_FEED_ACTUAL_SPEED,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)speed,dst,8);
    }

    return rc;
}

int ezsocket_read_spindle_load(ezsocket_t *ctx, int *load)//主轴负载
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SPINDLE_LOAD,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SPINDLE_LOAD,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *load = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

/* 伺服轴负载 */
int ezsocket_read_servo_load(ezsocket_t *ctx, int axis,  int *load)//伺服轴负载
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SERVO_LOAD,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SERVO_LOAD,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *load = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

/* 伺服轴转速 */
int ezsocket_read_servo_act_speed(ezsocket_t *ctx, int axis, int *speed)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SERVO_SPEED,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SERVO_SPEED,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *speed = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}



/* 刀具相关 */

int ezsocket_read_mag_tool_no(ezsocket_t *ctx, int *tool_no)//主刀具号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_MAG_TOOL_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_MAG_TOOL_NO,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *tool_no = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_spd_tool_no(ezsocket_t *ctx, int *tool_no)//主刀具号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[8] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_SPD_TOOL_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_SPD_TOOL_NO,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *tool_no = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_tooloffset_d_shape(ezsocket_t *ctx, int *shape)//刀具补偿D shape
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOLOFFSET_D_SHAPE,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOLOFFSET_D_SHAPE,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *shape = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_tooloffset_d_wear(ezsocket_t *ctx, int *wear)//刀具补偿D wear
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOLOFFSET_D_WEAR,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOLOFFSET_D_WEAR,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *wear = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_tooloffset_h(ezsocket_t *ctx, int axis, int *offset_h)//刀具补偿H
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(axis < 0 || axis > 15 )//超过范围
    {
        return EZSOCKET_RES_AXIS_RANGE;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOLOFFSET_H,
                                                   axis,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOLOFFSET_H,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *offset_h = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

/*  刀具数量 */
int ezsocket_read_tool_set_size(ezsocket_t *ctx, int *tool_set_size)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOL_SET_SIZE,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOL_SET_SIZE,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *tool_set_size = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }


    return rc;
}

/* 刀具补偿类型
 * 1 M system type-I : 1-axis compensation amount
 * 4 M system type-II : 1-axis compensation amount with wear compensation amount
 * 6 L system type : 2-axis compensation amount
*/
int ezsocket_read_tool_offset_type(ezsocket_t *ctx, int *type)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOL_OFFSET_TYPE,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOL_OFFSET_TYPE,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *type = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }


    return rc;
}

/* 获取刀具偏移值
 * int type 刀具补偿的类型
 * 0 M SYTEM TYPE-I
   1 M SYSTEM TYPE-II
   2 L SYSTEM TYPE
 * int kind 刀具补偿值得种类
 * M system type-I   0: Tool offset value
 * M system type-II  0: Tool length offset value(Dimension)
                     1: Ditto (Wear offset value)
                     2: Tool diameter offset value(Dimension)
                     3: Ditto (Wear offset value)
 * L system type     0: Tooltip wear amount X
                     1: Ditto Z
                     2: Ditto C (Y*)
                     3: Tool length X
                     4: Ditto Z
                     5: Ditto C (Y*)
                     6: Tooltip radius R
                     7: Tooltip radius wear amount r
                     8: Hypothetical tooltip # P
 * int tool_set_no 刀具得编号
 */
int ezsocket_read_tool_offset_value(ezsocket_t *ctx, int type, int kind, int tool_set_no,double *offset)
{
    if(type > EZSOCKET_TOOL_TYPE_L_TYPE )
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[12] = {0};
    int command;

    switch(type)
    {
    case EZSOCKET_TOOL_TYPE_M_TYPE_I://M system type-I : 1-axis compensation amount
        if(kind > 0)
        {
            return EZSOCKET_RES_PARAM_ERR;//参数错误
        }
        command = tool_set_no;
        type = 0x0C;
        break;
    case EZSOCKET_TOOL_TYPE_M_TYPE_II://M system type-II : 1-axis compensation amount with wear compensation amount
        if(kind > 3)
        {
            return EZSOCKET_RES_PARAM_ERR;//参数错误
        }
        switch(kind)
        {
        case 0:
            command = tool_set_no;
            break;
        case 1:
            command = tool_set_no+2000;
            break;
        case 2:
            command = tool_set_no+1000;
            break;
        case 3:
            command = tool_set_no+3000;
            break;
        }
        type = 0x0D;
        break;
    case EZSOCKET_TOOL_TYPE_L_TYPE://L system type : 2-axis compensation amount
        if(kind > 8)
        {
            return EZSOCKET_RES_PARAM_ERR;//参数错误
        }
        switch(kind)
        {
        case 0:
            command = tool_set_no+5000;
            break;
        case 1:
            command = tool_set_no+7000;
            break;
        case 2:
            command = tool_set_no+6000;
            break;
        case 3:
            command = tool_set_no+1000;
            break;
        case 4:
            command = tool_set_no+3000;
            break;
        case 5:
            command = tool_set_no+2000;
            break;
        case 6:
            command = tool_set_no+4000;
            break;
        case 7:
            command = tool_set_no+8000;
            break;
        case 8:
            command = tool_set_no;
            break;
        }
        type = 0x0E;
        break;
    }

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_TOOL_OFFSET_VALUE,
                                                   command,
                                                   type,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_TOOL_OFFSET_VALUE,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy(offset,dest,8);
    }


    return rc;
}

/* 模态相关 */
/* 读取M模态值，指定模态编号 */
int ezsocket_read_m_command_value(ezsocket_t *ctx, int command_index, int *m_value)
{
    if(command_index  < 1 || command_index  > 4)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};
    command_index += 0xC8;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_M_COMMAND,
                                                   command_index,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_M_COMMAND,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *m_value = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }


    return rc;
}

/* 读取S模态值，指定模态编号 */
int ezsocket_read_s_command_value(ezsocket_t *ctx, int command_index, int *s_value)
{
    if(command_index  < 1 || command_index  > 4)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_S_COMMAND,
                                                   command_index,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_S_COMMAND,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *s_value = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }

    return rc;
}

/* 读取T模态值，指定模态编号 */
int ezsocket_read_t_command_value(ezsocket_t *ctx, int command_index, int *t_value)
{
    if(command_index  != 1)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    command_index += 0x64;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_T_COMMAND,
                                                   command_index,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_T_COMMAND,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *t_value = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }

    return rc;
}

/* 读取B模态值，指定模态编号 */
int ezsocket_read_b_command_value(ezsocket_t *ctx, int command_index, int *b_value)
{
    if(command_index  < 1 || command_index  > 4)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dest[8] = {0};

    command_index += 0x012C;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_B_COMMAND,
                                                   command_index,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_B_COMMAND,rsp,(uint8_t*)dest,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *b_value = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dest,0);
    }

    return rc;
}


/* 报警相关 */
int ezsocket_read_alarm_msg(ezsocket_t *ctx, uint8_t *dst, uint16_t *length)//报警信息
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_ALARM_MSG,
                                                   0,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        *length = rsp_length;
        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_ALARM_MSG,rsp,dst,(uint16_t*)length);
    }

    return rc;
}


/* PLC 相关 */
int ezsocket_read_plc_x(ezsocket_t *ctx, int address, int sub_address, uint8_t *value)//PLC X
{
    /*if( sub_address > 7 )//位最高位bit7
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }*/
    if(address%8)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    address += 1;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PLC_X,
                                                   address,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PLC_X,rsp,dst,(uint16_t*)&rsp_length);
    }
    if( EZSOCKET_RES_OK == rc )
    {
        if(sub_address <= 7)
        {
            *value = (dst[0] >> sub_address) & 0x01;
        }
        else
        {
            *value = dst[0];
        }
    }

    return rc;
}

int ezsocket_read_plc_y(ezsocket_t *ctx, int address, int sub_address, uint8_t *value)//PLC Y
{
    /*if( sub_address > 7 )//位最高位bit7
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }*/
    if(address%8)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    address += 10000;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PLC_Y,
                                                   address,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PLC_Y,rsp,dst,(uint16_t*)&rsp_length);
    }
    if(EZSOCKET_RES_OK == rc)
    {
        if(sub_address <= 7)
        {
            *value = (dst[0] >> sub_address) & 0x01;
        }
        else
        {
            *value = dst[0];
        }
    }

    return rc;
}

int ezsocket_read_plc_r(ezsocket_t *ctx, int address,  uint16_t *r_value)//PLC R
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    address += 500000;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PLC_R,
                                                   address,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PLC_R,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *r_value = EZSOCKET_GET_INT16_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_plc_f(ezsocket_t *ctx, int address, int sub_address, uint8_t *value)//PLC F
{
    /*if( sub_address > 7 )//位最高位bit7
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }*/
    if(address%8)
    {
        return EZSOCKET_RES_PARAM_ERR;//参数错误
    }
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    address += 40000;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PLC_F,
                                                   address,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PLC_F,rsp,dst,(uint16_t*)&rsp_length);
    }
    if(EZSOCKET_RES_OK == rc)
    {
        if(sub_address <= 7)
        {
            *value = (dst[0] >> sub_address) & 0x01;
        }
        else
        {
            *value = dst[0];
        }
    }

    return rc;
}

int ezsocket_read_plc_d(ezsocket_t *ctx, int address, uint16_t *d_value)//PLC D
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[8] = {0};
    address += 90000;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_PLC_D,
                                                   address,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PLC_D,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            *d_value = EZSOCKET_GET_INT16_FROM_INT8_LITTLE_ENDAIN(dst,0);
    }

    return rc;
}

int ezsocket_read_g_code(ezsocket_t *ctx, int address, double *g_code)//G代码
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];
    uint8_t dst[12] = {0};
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _EZSOCKET_G_CODE,
                                                   address,
                                                   1,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_G_CODE,rsp,dst,(uint16_t*)&rsp_length);
        if(EZSOCKET_RES_OK == rc)
            memcpy((char*)g_code,dst,8);
    }

    return rc;
}

int ezsocket_read_param(ezsocket_t *ctx, int address, double *value)//参数读取
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t dst[12] = {0};
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    /*if(address >= 8000)
    {

    }
    else if(address <= 8000)
    {
        address -= 1000;
    }*/

    req_length = ctx->backend->build_request_basis(ctx,_EZSOCKET_PARAM_READ,
                                                   address,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_PARAM_READ,rsp,dst,(uint16_t*)&rsp_length);
    }
    if(EZSOCKET_RES_OK == rc)
    {
        switch(rsp_length)
        {
        case 2://uint16
        {
            uint16_t s_value = EZSOCKET_GET_INT16_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *value = (double)s_value;
        }
            break;
        case 4://int
        {
            uint32_t i_value = EZSOCKET_GET_INT32_FROM_INT8_LITTLE_ENDAIN(dst,0);
            *value = (double)i_value;
        }
            break;
        case 8://double
            memcpy((char*)value,dst,8);
            break;
        default:
            *value = dst[0];
            break;
        }
    }

    return rc;
}


/* 宏变量 */
int ezsocket_read_macro(ezsocket_t *ctx, int address, double *value)//读取宏变量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    int function;
    uint8_t dst[12];
    uint8_t req[_EZSOCKET_MIN_REQ_LENGTH];
    uint8_t rsp[EZSOCKET_MAX_MESSAGE_LENGTH];

    if(address >= 500)
    {
        function = _EZSOCKET_READ_MACRO_VAR+1;
        address -= 0x17c;
    }
    else
    {
        function = _EZSOCKET_READ_MACRO_VAR;
        address -= 0x44;
    }
    req_length = ctx->backend->build_request_basis(ctx,function,
                                                   address,
                                                   0,
                                                   0,
                                                   NULL,
                                                   req);
    if(req_length <= 0)//未生成发送的指令
    {
        return EZSOCKET_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_EZSOCKET_READ_MACRO_VAR,rsp,dst,(uint16_t*)&rsp_length);
    }
    if(EZSOCKET_RES_OK == rc)
        memcpy((char*)value,dst,8);
    return rc;
}

int ezsocket_write_marco_var(ezsocket_t *ctx, int address, uint8_t *dst)//写宏变量
{

}
