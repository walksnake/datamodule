#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <signal.h>
#include <sys/types.h>

#if defined(_WIN32)
# define OS_WIN32
/* ws2_32.dll has getaddrinfo and freeaddrinfo on Windows XP and later.
 * minwg32 headers check WINVER before allowing the use of these */
# ifndef WINVER
# define WINVER 0x0501
# endif
# include <ws2tcpip.h>
# define SHUT_RDWR 2
# define close closesocket
#else
# include <sys/socket.h>
# include <sys/ioctl.h>

#if defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD__ < 5)
# define OS_BSD
# include <netinet/in_systm.h>
#endif

# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <poll.h>
# include <netdb.h>
#endif

#if !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif


#include "libezsocket/ezsocket_tcp_private.h"
#include "libezsocket/ezsocket_private.h"

#ifdef OS_WIN32
static int _ezsocket_tcp_init_win32(void)
{
    /* Initialise Windows Socket API */
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup() returned error code %d\n",
                (unsigned int)GetLastError());
        errno = EIO;
        return -1;
    }
    return 0;
}
#endif

int _ezsocket_tcp_send(ezsocket_t *ctx, const uint8_t *req, int req_length)
{
    /* MSG_NOSIGNAL
       Requests not to send SIGPIPE on errors on stream oriented
       sockets when the other end breaks the connection.  The EPIPE
       error is still returned. */
    return send(ctx->s, (const char*)req, req_length, MSG_NOSIGNAL);
}

int _ezsocket_tcp_recv(ezsocket_t *ctx, uint8_t *rsp, int rsp_length) {
    return recv(ctx->s, (char *)rsp, rsp_length, 0);
}



static int _ezsocket_tcp_set_ipv4_options(int s)
{
    int rc;
    int option;

    /* Set the TCP no delay flag */
    /* SOL_TCP = IPPROTO_TCP */
    option = 1;
    rc = setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        return -1;
    }

#ifndef OS_WIN32
    /**
     * Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's
     * necessary to workaround that problem.
     **/
    /* Set the IP low delay option */
    option = IPTOS_LOWDELAY;
    rc = setsockopt(s, IPPROTO_IP, IP_TOS,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        return -1;
    }
#endif

    return 0;
}

/* Establishes a ezsocket TCP connection with a Modbus server. */
int _ezsocket_tcp_connect(ezsocket_t *ctx)
{
    int rc;
    struct sockaddr_in addr;
    ezsocket_tcp_t *ctx_tcp = ctx->backend_data;

#ifdef OS_WIN32
    if (_ezsocket_tcp_init_win32() == -1) {
        return -1;
    }
#endif

#if defined(_WIN32)
    ctx->s = socket(PF_INET, SOCK_STREAM, 0);
#else
    ctx->s = socket(PF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
#endif
    if (ctx->s == -1) {
        return -1;
    }

    rc = _ezsocket_tcp_set_ipv4_options(ctx->s);
    if (rc == -1) {
        close(ctx->s);
        ctx->s = -1;
        return -1;
    }

    if (ctx->debug) {
        printf("Connecting to %s port %d\n", ctx_tcp->ip, ctx_tcp->port);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ctx_tcp->port);
    addr.sin_addr.s_addr = inet_addr(ctx_tcp->ip);
    rc = connect(ctx->s, (struct sockaddr *)&addr,
                 sizeof(struct sockaddr_in));
    if (rc == -1) {
        close(ctx->s);
        ctx->s = -1;
        return -1;
    }

    return 0;
}

/* Closes the network connection and socket in TCP mode */
void _ezsocket_tcp_close(ezsocket_t *ctx)
{
    shutdown(ctx->s, SHUT_RDWR);
    close(ctx->s);
    ctx->s = -1;
}

int _ezsocket_tcp_flush(ezsocket_t *ctx)
{
    int rc;
    int rc_sum = 0;

    do {
        /* Extract the garbage from the socket */
        char devnull[EZSOCKET_MAX_MESSAGE_LENGTH];
#ifndef OS_WIN32
        rc = recv(ctx->s, devnull, EZSOCKET_MAX_MESSAGE_LENGTH, MSG_DONTWAIT);
#else
        /* On Win32, it's a bit more complicated to not wait */
        fd_set rfds;
        struct timeval tv;

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(ctx->s, &rfds);
        rc = select(ctx->s+1, &rfds, NULL, NULL, &tv);
        if (rc == -1) {
            return -1;
        }

        if (rc == 1) {
            /* There is data to flush */
            rc = recv(ctx->s, devnull, EZSOCKET_MAX_MESSAGE_LENGTH, 0);
        }
#endif
        if (rc > 0) {
            rc_sum += rc;
        }
    } while (rc == EZSOCKET_MAX_MESSAGE_LENGTH);

    return rc_sum;
}

int _ezsocket_tcp_select(ezsocket_t *ctx, fd_set *rfds, struct timeval *tv, int length_to_read)
{
    int s_rc;
    while ((s_rc = select(ctx->s+1, rfds, NULL, NULL, tv)) == -1) {
        if (errno == EINTR) {
            if (ctx->debug) {
                fprintf(stderr, "A non blocked signal was caught\n");
            }
            /* Necessary after an error */
            FD_ZERO(rfds);
            FD_SET(ctx->s, rfds);
        } else {
            return -1;
        }
    }

    if (s_rc == 0) {
        errno = ETIMEDOUT;
        return -1;
    }

    return s_rc;
}

