#include <stdlib.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif
#include <string.h>
#include <assert.h>
#include "../inc/siemenscnc_private.h"

/* word大小端转换 */
uint16_t siemenscnc_swaped_uint16(uint16_t value)
{
    return (((value&0x00FFU)<<8) | ((value&0xFF00U)>>8));
}

/* dword大小端转换 */
uint32_t siemenscnc_swaped_uint32(uint32_t value)
{
    return (((value & 0x000000FFU) << 24) | ((value & 0x0000FF00U) << 8) |
            ((value & 0x00FF0000U) >> 8) | ((value & 0xFF000000U) >> 24));
}

/* float大小端转换 */
float siemenscnc_swaped_float(float value)
{
    union {
        float a;
        uint8_t b[4];
    } f;
    uint8_t c;

    f.a=value;
    c=f.b[0];
    f.b[0]=f.b[3];
    f.b[3]=c;
    c=f.b[1];
    f.b[1]=f.b[2];
    f.b[2]=c;
    return f.a;
}

/* double大小端转换 */
double siemenscnc_swaped_double(double value)
{
    union {
        double a;
        uint8_t b[8];
    } f;
    uint8_t c;

    f.a=value;
    c=f.b[0];
    f.b[0]=f.b[7];
    f.b[7]=c;
    c=f.b[1];
    f.b[1]=f.b[6];
    f.b[6]=c;
    c=f.b[2];
    f.b[2]=f.b[5];
    f.b[5]=c;
    c=f.b[3];
    f.b[3]=f.b[4];
    f.b[4]=c;
    return f.a;
}


/* 获取16位整型 */
uint16_t siemenscnc_get_16(const uint8_t *src)
{
    return ((src[1]<<8) | src[0]);
}

/* 获取32位整型 */
uint32_t siemenscnc_get_32(const uint8_t *src)
{
    return ((src[3]<<24) |(src[2]<<16) | (src[1]<<8) | src[0]);
}

/* 获取float */
float siemenscnc_get_float(const uint8_t *src)
{
    //转为小端显示
    union {
        float a;
        uint8_t b[4];
    } u;

    u.b[3]=*src;
    src++;
    u.b[2]=*src;
    src++;
    u.b[1]=*src;
    src++;
    u.b[0]=*src;

    return u.a;
}

/*  获取double */
double siemenscnc_get_double(const uint8_t *src)
{
    //转为小端模式显示
    union {
        double a;
        uint8_t b[8];
    } u;

    u.b[0]=*src;
    src++;
    u.b[1]=*src;
    src++;
    u.b[2]=*src;
    src++;
    u.b[3]=*src;
    src++;
    u.b[4]=*src;
    src++;
    u.b[5]=*src;
    src++;
    u.b[6]=*src;
    src++;
    u.b[7]=*src;

    return u.a;
}

