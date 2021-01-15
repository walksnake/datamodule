/**
 * @file datacollecter.h
 * @brief  data collecter
 * @author Fan Chuanlin, fanchuanlin@aliyun.com
 * @version 1.0
 * @date 2021-01-06
 * @attention
 * @brief History:
 * <table>
 * <tr><th> Date <th> Version <th> Author <th> Description
 * <tr><td> xxx-xx-xx <td> 1.0 <td> Fan Chuanlin <td> Create
 * </table>
 *
 */

#ifndef _DATA_COLLECTER_H_
#define _DATA_COLLECTER_H_

#include "../../include/normal_typedef.h"
#include "../../feature/log/inc/easylogging.h"
#include "../../common/typeany/inc/typeany.h"
/// JSON
#include "../../include/json.hpp"
#include <cstdint>
#include <fstream>
#include <vector>
using namespace std;
using json = nlohmann::json;

/// function param list is unknow
typedef void * ( *pJobFunc )( ... );

typedef struct IOFunctionList
{
    /// function list
    UINT32 index;                               ///< 索引
    UINT32 functionNum;                             ///< 功能码，即是json中的type
    UINT32 paramNum;                            ///< 参数个数
    UINT32 returnpos;                           ///< 返回值的位置
    UINT32 datatype;                            ///< 数据类型按照最多8个参数排列8 x 4bits = 32bits
    ///< [31:28][27:24][23:20][19:16][15:12][11:8][7:4][3:0]
    string regName;                                                         ///< 采集函数的输入参数, 按照目前的规律，
    ///< 第一个参数是cxt，
    ///< 最后一个参数如果是返回值,则类型为普通指针
    ///< 倒数第二个参数是返回值，则最后一个参数是长度信息
    VOID **paramList;                               ///< 参数列表
    pJobFunc func;                              ///< 对应函数
    string funcName;                                                        ///< function name
} IOFunctionList_s;


/// timeout define
typedef enum
{
    TIMEOUT_TYPE_CONNECT = 0,
    TIMEOUT_TYPE_POST = 1,
    TIMEOUT_TYPE_Fast = 2,
    TIMEOUT_TYPE_1S = 3,
    TIMEOUT_TYPE_5S = 4,
    TIMEOUT_TYPE_10S = 5,
    TIMEOUT_TYPE_30S = 6,
    TIMEOUT_TYPE_60S = 7,
    TIMEOUT_TYPE_MAX
} TIMEOUT_TYPE_E;


/// return key-value, this is "ID" : Object
typedef struct KeyValue
{
    string key;
    TypeAny value;
} KeyValue_s;



/**
 * @brief  this class is only for timeout period
 */
class TimeOutCondition
{
public:
    /// timeout
    BOOLEAN m_timeout_enable[TIMEOUT_TYPE_MAX];
    UINT32 m_timeout_count[TIMEOUT_TYPE_MAX];
    UINT32 m_timeout_threshold[TIMEOUT_TYPE_MAX];
public:
    void StartTimeout( UINT8 type, UINT32 threshold );
    void StopTimeout( UINT8 type );
    void UpdateTimeout();
    BOOLEAN CheckTimeout( UINT8 type );
};



/**
 * @brief  this class is for data collecter, open three threads, such as fast-slow-post
 * Init IOList from JSON file, post the collectted information by restclient.
 */
class DataCollecter
{
public:
    DataCollecter();
    ~DataCollecter();

    BOOLEAN Init();
    INT8 Start();
    INT8 Stop();

    BOOLEAN InitIOListByJson( const CHAR *fileName );
    void InitFuncList( vector<IOFunctionList> *funclist, json tmpjson );
    VOID FormatParamList( vector<IOFunctionList> *funclist, INT32 funcIndex, string regName );
    INT32 GetIOFunctionFromType( UINT32 type );
    INT32 JobFunctionCall( pJobFunc JobFunc, UINT32 paramNum, VOID **paramList );
public:
    static void * TimerProcessFast( void *pThis );
    static void * TimerProcessSlow( void *pThis );
    static void * PostHandler( void *pThis );

    static DataCollecter * getInstance()
    {
        return m_pInstance_s;
    };

    /// singleton
    static DataCollecter *m_pInstance_s;

    /// timeout
    TimeOutCondition *m_timeout_fast;
    TimeOutCondition *m_timeout_slow;
public:

    /// collection list init by json
    std::vector<IOFunctionList> m_iolist_100ms;
    std::vector<IOFunctionList> m_iolist_1s;
    std::vector<IOFunctionList> m_iolist_5s;
    std::vector<IOFunctionList> m_iolist_10s;
    std::vector<IOFunctionList> m_iolist_30s;
    std::vector<IOFunctionList> m_iolist_60s;

    /// report data
    vector<KeyValue> m_cur_data;
    vector<KeyValue> m_his_data;

    UINT32 m_protocol_id;
    VOID *m_protocol_cxt;
    string m_ipaddr;
    UINT16 m_port;
    BOOLEAN m_connect;
private:
    /// thread
    pthread_t m_thread_timer_Fast;
    pthread_t m_thread_timer_Slow;
    pthread_t m_thread_post;

    /// mutex
    pthread_mutex_t m_mutex_post;

    /// sem
    sem_t m_sem_post;

    /// start thread
    sem_t m_start_timer_Fast;
    sem_t m_start_timer_Slow;
    sem_t m_start_post;

    BOOLEAN m_thread_fast_stop;
    BOOLEAN m_thread_slow_stop;
    BOOLEAN m_thread_post_stop;
};

#endif
