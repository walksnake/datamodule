#include "../inc/siemenscnc_private.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
/*
*********************************************************************************************************
*	函 数 名: _siemenscnc_init_common
*	功能说明: 初始化ctx
*	形    参: siemenscnc_t *ctx
*	返 回 值:
*********************************************************************************************************
*/
void _siemenscnc_init_common(siemenscnc_t *ctx)
{
    ctx->s = -1;
    ctx->debug =FALSE;
    ctx->error_recovery = SIEMENSCNC_ERROR_RECOVERY_NONE;
    ctx->response_timeout.tv_sec = 0;
    ctx->response_timeout.tv_usec = SIEMENSCNC_RESPONSE_TIMEOUT;

    ctx->byte_timeout.tv_sec = 0;
    ctx->byte_timeout.tv_usec = SIEMENSCNC_BYTE_TIMEOUT;
}

/*
*********************************************************************************************************
*	函 数 名: _siemenscnc_strlcpy
*	功能说明: 拷贝
*	形    参: char *dest, const char *src, size_t dest_size
*	返 回 值:
*********************************************************************************************************
*/
int _siemenscnc_strlcpy(char *dest, const char *src, int dest_size)
{
    register char *d = dest;
    register const char *s = src;
    register int n = dest_size;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dest, add NUL and traverse rest of src */
    if (n == 0) {
        if (dest_size != 0)
            *d = '\0'; /* NUL-terminate dest */
        while (*s++)
            ;
    }

    return (s - src - 1); /* count does not include NUL */
}

void _siemenscnc_swap_data(uint8_t *src, int number)//交换顺序
{
    int i;
    uint8_t temp;
    for(i=0;i<number/2;i++)
    {
        temp = src[number-1-i];
        src[number-1-i] = src[i];
        src[i] = temp;
    }
}


int _siemenscnc_sleep_and_flush(siemenscnc_t *ctx)//休眠并清空数据
{
#ifdef _WIN32
    /* usleep doesn't exist on Windows */
    Sleep((ctx->response_timeout.tv_sec * 1000) +
          (ctx->response_timeout.tv_usec / 1000));
#else
    /* usleep source code */
    struct timespec request, remaining;
    request.tv_sec = ctx->response_timeout.tv_sec;
    request.tv_nsec = ((long int)ctx->response_timeout.tv_usec % 1000000)
        * 1000;
    while (nanosleep(&request, &remaining) == -1 && errno == EINTR)
        request = remaining;
#endif
    return siemenscnc_flush(ctx);
}
