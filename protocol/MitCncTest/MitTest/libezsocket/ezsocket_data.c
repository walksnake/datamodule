#include <stdlib.h>
#ifndef _MSC_VER
#include <stdint.h>
#else
#include "stdint.h"
#endif
#include <string.h>
#include <assert.h>
#include "ezsocket_private.h"
/* word大小端转换 */
uint16_t ezsocket_swaped_uint16(uint16_t value)
{
    return ((uint16_t)((value&0x00FFU)<<8) | ((value&0xFF00U)>>8));
}

/* dword大小端转换 */
uint32_t ezsocket_swaped_uint32(uint32_t value)
{
    return ((uint32_t)((value & 0x000000FFU) << 24) | ((value & 0x0000FF00U) << 8) |
            ((value & 0x00FF0000U) >> 8) | ((value & 0xFF000000U) >> 24));
}

/* uint64_t 大小端转换 */
uint64_t ezsocket_swaped_uint64(uint64_t value)
{
    union {
        uint64_t a;
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

/* float大小端转换 */
float ezsocket_swaped_float(float value)
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

/* double 大小端转换 */
double ezsocket_swaped_double(double value)
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
