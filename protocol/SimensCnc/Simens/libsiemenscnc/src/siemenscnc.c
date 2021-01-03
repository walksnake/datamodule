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

#include "../inc/siemenscnc.h"
#include "../inc/siemenscnc_private.h"

/* Sends a request/response */
static int send_msg(siemenscnc_t *ctx, uint8_t *msg, int msg_length)
{
    int rc;
    int i;

    if (ctx->debug) {
        printf("send length %d:",msg_length);
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
            if (ctx->error_recovery & SIEMENSCNC_ERROR_RECOVERY_PROTOCOL) {
                int saved_errno = errno;

                if ((errno == EBADF || errno == ECONNRESET || errno == EPIPE)) {
                    siemenscnc_close(ctx);
                    siemenscnc_connect(ctx);
                } else {
                    _siemenscnc_sleep_and_flush(ctx);
                }
                errno = saved_errno;
            }
        }
    } while ((ctx->error_recovery & SIEMENSCNC_ERROR_RECOVERY_PROTOCOL) &&
             rc == -1);

    if (rc > 0 && rc != msg_length) {
        //errno = EMBBADDATA;
        return -1;
    }

    return rc;
}

/* 设置模式 */
int siemenscnc_set_error_recovery(siemenscnc_t *ctx, siemenscnc_error_recovery_mode error_recovery)
{
    if(ctx == NULL)
        return -1;
    ctx->error_recovery = error_recovery;
}
/* 设置socket嵌套字 */
void siemenscnc_set_socket(siemenscnc_t *ctx, int socket)
{
    if(ctx == NULL)
        return;
    ctx->s = socket;
}

/* 获取嵌套字 */
int siemenscnc_get_socket(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->s;
}

/* 获取响应超时时间 */
void siemenscnc_get_response_timeout(siemenscnc_t *ctx, struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    *timeout = ctx->response_timeout;
}

/* 设置超时时间 */
void siemenscnc_set_response_timeout(siemenscnc_t *ctx, const struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    ctx->response_timeout = *timeout;
}

/* 获取字节间接受超时时间 */
void siemenscnc_get_byte_timeout(siemenscnc_t *ctx, struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    *timeout = ctx->byte_timeout;
}

/* 设置字节间接受超时时间 */
void siemenscnc_set_byte_timeout(siemenscnc_t *ctx, const struct timeval *timeout)
{
    if(ctx == NULL)
        return;
    ctx->byte_timeout = *timeout;
}

/* 获取帧头长度 */
int siemenscnc_get_header_length(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->header_length;
}

/* 建立连接 */
int siemenscnc_connect(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->connect(ctx);
}

/* 断开连接 */
void siemenscnc_close(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return;
    return ctx->backend->close(ctx);
}

/* 释放 */
void siemenscnc_free(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return;
    free(ctx->backend_data);
    free(ctx);
}

/* 清空缓存流 */
int siemenscnc_flush(siemenscnc_t *ctx)
{
    if(ctx == NULL)
        return -1;
    return ctx->backend->flush(ctx);
}

/* 设置调试模式 */
void siemenscnc_set_debug(siemenscnc_t *ctx, int boolean)
{
    if(ctx == NULL)
        return;
    ctx->debug = boolean;
}

/* 协议交互 */
/* 系统相关 */
int siemenscnc_read_serial_number(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length)//硬件序列号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_SERIAL_NUMBER,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc == -1)
            return -1;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_SERIAL_NUMBER,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_cnc_type(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length)//设备类型
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_CNC_TYPE,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_CNC_TYPE,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_verison(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length)//版本信息
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_VERSION,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_VERSION,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_manufacture_data(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length)//出厂日期
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_MANUFACTURE_DATE,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_MANUFACTURE_DATE,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_process_number(siemenscnc_t *ctx, double *dst)//加工数量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_PROCESS_NUMBER,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_PROCESS_NUMBER,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_process_set_number(siemenscnc_t *ctx, double *dst)//设定的加工数量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_PROCESS_SET_NUMBER,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_PROCESS_SET_NUMBER,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_run_time(siemenscnc_t *ctx, double *dst)//循环时间,一次自动运行起动时间的累计值
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;


    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_RUN_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;
        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_RUN_TIME,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_remain_time(siemenscnc_t *ctx, double *dst)//剩余时间
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_REMAIN_TIME,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_REMAIN_TIME,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            // *dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_program_name(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length)//加工程序名
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_PROGRAM_NAME,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_PROGRAM_NAME,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

/* 状态相关 */
int siemenscnc_read_operate_mode(siemenscnc_t *ctx, uint8_t *dst)//操作模式
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_OPERATE_MODE,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_OPERATE_MODE,rsp,dst,(uint16_t*)&rsp_length);
    }

    return rc;
}

int siemenscnc_read_status(siemenscnc_t *ctx, uint8_t *dst)//运行状态
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_STATUS,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_STATUS,rsp,dst,(uint16_t*)&rsp_length);
    }

    return rc;
}

/* 轴相关 */
int siemenscnc_read_axis_name(siemenscnc_t *ctx, uint8_t axis, uint8_t *dst, uint16_t *length)//轴名称
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_AXIS_NAME,
                                                   axis+1,
                                                   1,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_AXIS_NAME,rsp,dst,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_machine_pos(siemenscnc_t *ctx, uint8_t axis, double *dst)//机械坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint8_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_MACHINE_POS,
                                                   axis+1,
                                                   1,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_MACHINE_POS,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }

    }

    return rc;
}

int siemenscnc_read_relatively_pos(siemenscnc_t *ctx, uint8_t axis, double *dst)//工件坐标 相对坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;


    req_length = ctx->backend->build_request_basis(ctx,_SIEMENSCNC_RELATIVELY_POS,
                                                   axis+1,
                                                   1,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_RELATIVELY_POS,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_remain_pos(siemenscnc_t *ctx, uint8_t axis, double *dst)//剩余坐标
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_REMAIN_POS,
                                                   axis+1,
                                                   1,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_REMAIN_POS,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_feed_set_speed(siemenscnc_t *ctx, double *dst)//设定进给速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,_SIEMENSCNC_FEED_SET_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_FEED_SET_SPEED,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_feed_act_speed(siemenscnc_t *ctx, double *dst)//实际进给速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_FEED_ACT_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }
    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_FEED_ACT_SPEED,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}
int siemenscnc_read_feed_rate(siemenscnc_t *ctx, double *dst)//进给倍率
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_FEED_RATE,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_FEED_RATE,rsp,data,(uint16_t*)&length);
        if(length > 8)
        {
            return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
        }
        if( SIEMENSCNC_RES_OK == rc)
        {
            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}
int siemenscnc_read_spindle_set_speed(siemenscnc_t *ctx, double *dst)//主轴设定速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_SPINDLE_SET_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_SPINDLE_SET_SPEED,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}
int siemenscnc_read_spindle_act_speed(siemenscnc_t *ctx, double *dst)//主轴实际速度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_SPINDLE_ACT_SPEED,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_SPINDLE_ACT_SPEED,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
            //*dst = siemenscnc_swaped_double(*dst);
        }
    }

    return rc;
}

int siemenscnc_read_spindle_rate(siemenscnc_t *ctx, double *dst)//主轴倍率
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_SPINDLE_RATE,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_SPINDLE_RATE,rsp,data,(uint16_t*)&length);
        if( SIEMENSCNC_RES_OK == rc)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_driver_voltage(siemenscnc_t *ctx, uint8_t driver, float *dst)//母线电压
{
    //驱动器参数r26
    int rc;
    int data[8] = {0};
    uint16_t length;
    rc = siemenscnc_read_s_r_p_param(ctx,driver,26,0,data,&length);
    if( SIEMENSCNC_RES_OK == rc)
    {
        *dst = siemenscnc_get_float(data);
    }
    return rc;
}

int siemenscnc_read_driver_current(siemenscnc_t *ctx, uint8_t driver, float *dst)//实际电流
{
    //驱动器参数r30
    int rc;
    int data[8] = {0};
    uint16_t length;
    rc = siemenscnc_read_s_r_p_param(ctx,driver,30,0,data,&length);
    if( SIEMENSCNC_RES_OK == rc)
    {
        *dst = siemenscnc_get_float(data);
    }
    return rc;
}

int siemenscnc_read_driver_power(siemenscnc_t *ctx, uint8_t driver, float *dst)//电机功率
{
    //驱动器参数r32
    int rc;
    int data[8] = {0};
    uint16_t length;
    rc = siemenscnc_read_s_r_p_param(ctx,driver,32,0,data,&length);
    if( SIEMENSCNC_RES_OK == rc)
    {
        *dst = siemenscnc_get_float(data);
    }
    return rc;
}

int siemenscnc_read_driver_temper(siemenscnc_t *ctx, uint8_t driver, float *dst)//电机温度
{
    //驱动器参数r37[0]
    int rc;
    int data[8] = {0};
    uint16_t length;
    rc = siemenscnc_read_s_r_p_param(ctx,driver,35,0,data,&length);
    if( SIEMENSCNC_RES_OK == rc)
    {
        *dst = siemenscnc_get_float(data);
    }
    return rc;
}


int siemenscnc_read_g_coordinate_t(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst)//G T坐标系
{
    //G54-X = 0x05
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_G_COORDINATE_T,
                                                   1,
                                                   1,
                                                   g_coordinate,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_G_COORDINATE_T,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_g_coordinate_m(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst)//G T坐标系
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_G_COORDINATE_M,
                                                   axis+1,
                                                   1,
                                                   g_coordinate,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc == -1)
            return -1;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_G_COORDINATE_M,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

/* 刀具相关 */
int siemenscnc_read_tool_cur_name(siemenscnc_t *ctx,  uint8_t *tool_name, uint16_t *length)//当前刀具名
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    //uint8_t data[128];

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_CUR_NAME,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_CUR_NAME,rsp,tool_name,(uint16_t*)length);
    }

    return rc;
}

int siemenscnc_read_tool_cur_t_no(siemenscnc_t *ctx, double *dst)//刀具号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_CUR_T_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;


        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_CUR_T_NO,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }

    }

    return rc;
}

int siemenscnc_read_tool_cur_d_no(siemenscnc_t *ctx, double *dst)//刀具刀沿号
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_CUR_D_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;


        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_CUR_D_NO,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }

    }

    return rc;
}

int siemenscnc_read_tool_cur_h_no(siemenscnc_t *ctx, double *dst)//刀具长度
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_CUR_H_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_CUR_H_NO,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}



int siemenscnc_read_tool_type(siemenscnc_t *ctx,  int tool_edge_no, int tool_row, double *dst)//刀具类型
{
    //刀具列表中一把刀占用35列,一组里面含多个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12] = {0};
    uint16_t length;




    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x01+35*tool_edge_no,//刀具类型0x01
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}


int siemenscnc_read_tool_length(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀具长度
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12] = {0};
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x03+35*tool_edge_no,//刀具长度X
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀具半径
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12] = {0};
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x06+35*tool_edge_no,//刀具半径
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_edge(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀沿位置
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x02+35*tool_edge_no,//刀沿位置
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
            {
                *dst = siemenscnc_get_double(data);
            }
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_tip(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀具齿数
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x22+35*tool_edge_no,//刀具齿数
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_h_no(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀沿H号
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x1A+35*tool_edge_no,//刀具H号
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_wear_length(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀具长度磨损
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12] = {0};
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x0C+35*tool_edge_no,//表示长度磨损X
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_wear_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst)//刀具半径磨损
{
    //刀具列表中一把刀占用35列,一组里面含两个刀沿
    tool_edge_no--;
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[12] = {0};
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_LIST,
                                                   0,
                                                   0,
                                                   tool_row,
                                                   0x0F+35*tool_edge_no,//表示刀具半径磨损
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_LIST,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_x_offset(siemenscnc_t *ctx, double *dst)//X补偿
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_X_OFFSET,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_X_OFFSET,rsp,data,(uint16_t*)&length);

        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://word
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_tool_z_offset(siemenscnc_t *ctx,  double *dst)//长度补偿Z
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_TOOL_Z_OFFSET,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_TOOL_Z_OFFSET,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = (double)siemenscnc_get_32(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

/* 报警相关 */
int siemenscnc_read_cnc_nc_alarm_no(siemenscnc_t *ctx, uint16_t *dst)//NC报警数量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_CNC_NC_ALARM_NO,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   0 ,
                                                   req,
                                                   NULL);

    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);

        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_CNC_NC_ALARM_NO,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 4)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = data[0];
                break;
            case 2://word
                *dst = siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = siemenscnc_get_32(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

int siemenscnc_read_cnc_nc_alarm(siemenscnc_t *ctx, uint16_t alarm_no, uint32_t *dst)//NC报警
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;

    alarm_no += 1;
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_CNC_NC_ALARM,
                                                   alarm_no ,
                                                   0,
                                                   0 ,
                                                   0,
                                                   0 ,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {
        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_CNC_NC_ALARM,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 4)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = data[0];
                break;
            case 2://word
                *dst = siemenscnc_get_16(data);
                break;
            case 4://dword
                *dst = siemenscnc_get_32(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

/* 参数相关 */
int siemenscnc_read_r_var(siemenscnc_t *ctx, uint16_t address, double *dst)//R变量
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    uint8_t data[128];
    uint16_t length;


    address += 1;//R参数比实际要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_R_VAR,
                                                   0 ,
                                                   0,
                                                   address ,
                                                   0,
                                                   0 ,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_R_VAR,rsp,data,(uint16_t*)&length);
        if(rc == SIEMENSCNC_RES_OK)
        {
            if(length > 8)
            {
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
            }

            switch(length)
            {
            case 1://byte
                *dst = (double)data[0];
                break;
            case 2://word
                *dst = (double)siemenscnc_get_16(data);
                break;
            case 4://float
                *dst = (double)siemenscnc_get_float(data);
                break;
            case 8://double
                *dst = siemenscnc_get_double(data);
                break;
            default:
                return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
                break;
            }
        }
    }

    return rc;
}

//驱动器R参数 查看方法：调试->机床数据->驱动参数
int siemenscnc_read_s_r_p_param(siemenscnc_t *ctx, uint8_t driver, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_S_R_P_PARAM,
                                                   driver,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_S_R_P_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}

//通用机床数据 查看方法：调试->机床数据->通用机床数据
int siemenscnc_read_general_machine_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_GENERAL_MACHINE_PARAM,
                                                   0,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_GENERAL_MACHINE_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}

//通道机床数据 查看方法：调试->机床数据->通道机床数据
int siemenscnc_read_channel_machine_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_CHANNEL_MACHINE_PARAM,
                                                   0,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_CHANNEL_MACHINE_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}

//轴机床数据 查看方法：调试->机床数据->轴机床数据
int siemenscnc_read_axis_machine_param(siemenscnc_t *ctx, uint8_t axis, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_AXIS_MACHINE_PARAM,
                                                   axis,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_AXIS_MACHINE_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}

//控制单元参数 查看方法：调试->机床数据->控制单元参数
int siemenscnc_read_control_unit_param(siemenscnc_t *ctx,uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_CONTROL_UNIT_PARAM,
                                                   0,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_CONTROL_UNIT_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}

//电源模块参数 查看方法：调试->机床数据->电源模块参数
int siemenscnc_read_power_unit_param(siemenscnc_t *ctx, uint16_t address, uint16_t sub_address, uint8_t *dest, uint16_t *length)
{
    int rc;
    int req_length;
    uint16_t rsp_length;
    uint8_t req[_SIEMENSCNC_MIN_REQ_LENGTH];
    uint8_t rsp[SIEMENSCNC_MAX_MESSAGE_LENGTH];
    sub_address += 1;//副标题(下角标)比实际的值要大1
    req_length = ctx->backend->build_request_basis(ctx,
                                                   _SIEMENSCNC_POWER_UNIT_PARAM,
                                                   0,
                                                   0,
                                                   address,
                                                   sub_address,
                                                   0,
                                                   req,
                                                   NULL);
    if(req_length <= 0)//未生成发送的指令
    {
        return SIEMENSCNC_RES_COMMAN_ERR;//未生成发送的指令
    }

    rc = send_msg(ctx, req, req_length);
    if (rc > 0) {

        rc = ctx->backend->receive_msg(ctx, rsp, &rsp_length);
        if (rc < 0)
            return rc;

        rc = ctx->backend->decode_msg(ctx,_SIEMENSCNC_POWER_UNIT_PARAM,rsp,dest,(uint16_t*)length);
    }

    return rc;
}
