#ifndef SIEMENSCNC_TCP_PRIVATE_H
#define SIEMENSCNC_TCP_PRIVATE_H

#include "siemenscnc.h"
//# include <ws2tcpip.h>
SIEMENSCNC_BEGIN_DECLS
typedef struct _mc_tcp {
    /* TCP port */
    int port;
    /* IP address */
    char ip[16];
} siemenscnc_tcp_t;


int _siemenscnc_tcp_send(siemenscnc_t *ctx, const uint8_t *req, int req_length);
int _siemenscnc_tcp_recv(siemenscnc_t *ctx, uint8_t *rsp, int rsp_length);
int _siemenscnc_tcp_connect(siemenscnc_t *ctx);
void _siemenscnc_tcp_close(siemenscnc_t *ctx);
int _siemenscnc_tcp_flush(siemenscnc_t *ctx);
int _siemenscnc_tcp_select(siemenscnc_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read);
SIEMENSCNC_END_DECLS

#endif // MC_TCP_PRIVATE_H
