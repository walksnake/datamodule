/**
 * @file normal_typedef.h
 * @brief  this is common header file for lora stack
 * @author Fan Chuanlin, fanchuanlin@aliyun.com
 * @version 1.0
 * @date 2020-06-27
 */

#ifndef _NORMAL_TYPEDEF_H_
#define _NORMAL_TYPEDEF_H_

#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
/// SOCKET
#ifdef LORA_SOCKET_SIM
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <pthread.h>
#endif

/// define c++11 std
#if 11
#include <vector>
#include <set>
#include <map>
#include <list>
#include <queue>
#endif
/**
 * @name  define normal type;
 *
**  @} */

typedef bool                  BOOLEAN;
typedef unsigned char         UINT8;
typedef signed char           INT8;
typedef unsigned char         OCTET;
typedef signed char           CHAR;
typedef unsigned short        UINT16;
typedef signed short          INT16;
typedef unsigned int          UINT32;
typedef signed int            INT32;
typedef unsigned long         ULONG;
typedef signed long           LONG;
typedef unsigned long long    UINT64;
typedef long long             INT64;
typedef float                 FLOAT;
typedef double                DOUBLE;
typedef void 									VOID;
typedef int                   SOCKET_FP;

/// for any type define
typedef unsigned long 				TYPEANY_ID;


/// for socket
#define INVALID_SOCKET -1
typedef int af_family;
typedef int in_port;
typedef int SOCKET;

#define BACKLOG_LEN 10
#define SOCKET_SERVER_PORT 6666
#define SOCKET_CLIENT_PORT 9999
#define LOCAL_HOST "127.0.0.1"

typedef enum
{
    SOCKET_ROLE_SERVER = 0,
    SOCKET_ROLE_CLIENT = 1
} SOCKET_ROLE_E;


#define TRUE true
#define FALSE false

typedef enum
{
    Lret_success = 0,
    Lret_fail = 1,
    Lret_timeout = 2,
    Lret_accept_fail = 3,
    Lret_connet_fail = 4,
    Lret_recv_fail = 5,
} Lret;

#endif
