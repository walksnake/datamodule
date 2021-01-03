#ifndef EZSOCKET_TCP_PRIVATE_H
#define EZSOCKET_TCP_PRIVATE_H

#include "libezsocket/ezsocket.h"
//# include <ws2tcpip.h>
EZSOCKET_BEGIN_DECLS
typedef struct _ezsocket_tcp {
    /* TCP port */
    int port;
    /* IP address */
    char ip[16];
} ezsocket_tcp_t;


int _ezsocket_tcp_send(ezsocket_t *ctx, const uint8_t *req, int req_length);
int _ezsocket_tcp_recv(ezsocket_t *ctx, uint8_t *rsp, int rsp_length);
int _ezsocket_tcp_connect(ezsocket_t *ctx);
void _ezsocket_tcp_close(ezsocket_t *ctx);
int _ezsocket_tcp_flush(ezsocket_t *ctx);
int _ezsocket_tcp_select(ezsocket_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read);
EZSOCKET_END_DECLS

#endif // EZSOCKET_TCP_PRIVATE_H
