#ifndef SIEMENCNC_HOST_H
#define SIEMENCNC_HOST_H

#include "libsimenscnc/simenscnc.h"
#include <stdint.h>


enum
{
    SIEMENSCNC_BIT_TYPE=0,//位
    SIEMENSCNC_BYTE_TYPE,//字节
    SIEMENSCNC_WORD_TYPE,//字
    SIEMENSCNC_DWORD_TYPE,//双字
    SIEMENSCNC_FLOAT_TYPE,//浮点
    SIEMENSCNC_STRING_TYPE//字符串

};


void siemenscnc_free(void);
int siemenscnc_host_rw_param(const char *hardware,
                             uint8_t index,
                             uint8_t function,
                             uint8_t bits,
                             const char *regname,
                             uint8_t reglength,
                             uint16_t regnum,
                             uint16_t timeout,
                             uint8_t retry,
                             uint16_t commdelay,
                             uint8_t dataorder,
                             uint8_t *dest,
                             uint16_t *dest_length);

/* 备注 */
/* 对于regname,如仅需要设定轴或者地址的格式位 X.XX,第一个X表示轴编号或者地址，第二个XX表示轴数量或者副地址(下角标)

    X.XX:X.XX冒号前的表示轴编号和轴数量，冒号后的表示地址和副地址.根据实际功能，若不填该值位0
*/

#endif // SIEMENCNC_HOST_H
