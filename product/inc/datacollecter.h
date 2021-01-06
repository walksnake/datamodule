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
/// JSON
#include "../../include/json.hpp"
#include <fstream>
using namespace std;
using json = nlohmann::json;


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


typedef struct IOFunctionList
{
    /// function list

} IOFunctionList_s;


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

class DataCollecter
{
public:
    DataCollecter();
    ~DataCollecter();

    BOOLEAN Init();
    INT8 Start();
    INT8 Stop();

    BOOLEAN InitIOListByJson( CHAR *fileName );

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

    /// collection list inti by json
    std::vector<IOFunctionList> m_iolist_100ms;
    std::vector<IOFunctionList> m_iolist_1s;
    std::vector<IOFunctionList> m_iolist_5s;
    std::vector<IOFunctionList> m_iolist_10s;
    std::vector<IOFunctionList> m_iolist_30s;
    std::vector<IOFunctionList> m_iolist_60s;

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
