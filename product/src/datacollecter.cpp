/**
 * @file datacollecter.cpp
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

#include "../inc/datacollecter.h"
#include "../inc/functionlist.h"
#include <vector>


DataCollecter* DataCollecter::m_pInstance_s = NULL;


/**
 * @brief  construct
 */
DataCollecter::DataCollecter( )
{
    //SEM
    sem_init( &m_sem_post, 0, 0 );
    sem_init( &m_start_timer_Fast, 0, 0 );
    sem_init( &m_start_timer_Slow, 0, 0 );
    sem_init( &m_start_post, 0, 0 );

    ///pthread mutex
    pthread_mutex_init( &m_mutex_post, NULL );

    ///create thread
    pthread_create( &m_thread_timer_Fast, NULL, TimerProcessFast, ( void * )this );
    pthread_create( &m_thread_timer_Slow, NULL, TimerProcessSlow, ( void * )this );
    pthread_create( &m_thread_post, NULL, PostHandler, ( void * )this );
}

BOOLEAN DataCollecter::Init()
{
    /// default value = false
    m_thread_fast_stop = FALSE;
    m_thread_slow_stop = FALSE;
    m_thread_post_stop = FALSE;

    m_timeout_fast = new TimeOutCondition();
    m_timeout_slow = new TimeOutCondition();

    return TRUE;
}

INT8 DataCollecter::Start()
{
    UINT32 ret = Lret_success;

    sem_post( &m_start_timer_Fast );
    sem_post( &m_start_timer_Slow );
    sem_post( &m_start_post );

    pthread_join( m_thread_timer_Fast, NULL );
    pthread_join( m_thread_timer_Slow, NULL );
    pthread_join( m_thread_post, NULL );

    return ret;
}

INT8 DataCollecter::Stop()
{
    //// stop thread
    m_thread_fast_stop = TRUE;
    m_thread_slow_stop = TRUE;
    m_thread_post_stop = TRUE;

    /// need sem
    sem_post( &m_sem_post );
    return Lret_success;
}


BOOLEAN DataCollecter::InitIOListByJson( const CHAR *fileName )
{
    ifstream fjson( ( const char * )fileName );
    json jsoncfg;
    fjson >> jsoncfg;
    if( ( string )jsoncfg["command"] == "get" )
    {
        LOG( INFO ) << "get" << endl;
        LOG( INFO ) << jsoncfg["device"] << endl;
        LOG( INFO ) << jsoncfg["listGet"] << endl;
				
        for( UINT32 len = 0; len < jsoncfg["listGet"].size(); len++ )
        {
            json tmpjson = jsoncfg["listGet"][len];
						cout << tmpjson;
						cout << tmpjson["frequency"]; 
						string freq = tmpjson["frequency"]; 

            if( freq == "0.1" )
            {
                InitFuncList( m_iolist_100ms, tmpjson );
            }
            else if( freq == "1" )
            {
                InitFuncList( m_iolist_1s, tmpjson );
            }
            else if( freq == "5" )
            {
                InitFuncList( m_iolist_5s, tmpjson );
            }
            else if( freq == "10" )
            {
                InitFuncList( m_iolist_10s, tmpjson );
            }
            else if( freq == "30" )
            {
                InitFuncList( m_iolist_30s, tmpjson );
            }
            else if( freq == "60" )
            {
                InitFuncList( m_iolist_60s, tmpjson );
            }
        }
    }
    else
    {
        LOG( INFO ) << jsoncfg["device"] << endl;
        LOG( INFO ) << jsoncfg["listSet"].count( jsoncfg["listSet"] ) << endl;
        LOG( INFO ) << jsoncfg["listSet"] << endl;
    }
    fjson.close();

    return TRUE;
}

void DataCollecter::InitFuncList( vector<IOFunctionList> funclist, json tmpjson )
{
    for( UINT32 len = 0; len < tmpjson["list"].size() ; len++ )
    {
        INT32 funcIndex = GetIOFunctionFromType( UINT32( tmpjson["list"][len]["type"] ) ) ;
        if( funcIndex < 0 )
        {
            LOG( ERROR ) << "frequency = " << tmpjson["frequency"] << "s: " <<  tmpjson["list"][len] << "not found!";
        }
        else
        {
            funclist.push_back( functionlist[funcIndex] );
            LOG( INFO ) << "frequency = " << tmpjson["frequency"] << "s: " <<  tmpjson["list"][len];
        }
    }
}

/**
 * @brief  this thread is for fast data collecttng
 *
 * @param[in]  pThis
 *
 * @returns
 */
void * DataCollecter::TimerProcessFast( void *pThis )
{
    LOG( INFO ) << "TimerProcess create!";
    sleep( 1 );
    if( NULL == pThis )
    {
        return NULL;
    }

    DataCollecter *pObj = ( DataCollecter * )pThis;
    prctl( PR_SET_NAME, "TimerProcessFast" );
    sem_wait( &( pObj->m_start_timer_Fast ) );

    /// unit = 10ms, Fast = 10*unit
    pObj->m_timeout_fast->StartTimeout( TIMEOUT_TYPE_Fast, 10 );
    while( TRUE )
    {
        /// 10ms
        usleep( 10000 );

        ///Timeout process
        pObj->m_timeout_fast->UpdateTimeout();

        if( TRUE == pObj->m_timeout_fast->CheckTimeout( TIMEOUT_TYPE_Fast ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_100ms.size(); i++ )
            {
                /// read IO item
            }

            /// reset timeout
            pObj->m_timeout_fast->StopTimeout( TIMEOUT_TYPE_Fast );
            pObj->m_timeout_fast->StartTimeout( TIMEOUT_TYPE_Fast, 10 );
            LOG( INFO ) << "reset Fast counter";
        }

        if( pObj->m_thread_fast_stop )
        {
            break;
        }
    }

    return NULL;
}


/**
 * @brief  this thread is for slow data collectting
 *
 * @param[in]  pThis
 *
 * @returns
 */
void * DataCollecter::TimerProcessSlow( void *pThis )
{
    LOG( INFO ) << "TimerProcessSlow create!";
    sleep( 2 );
    if( NULL == pThis )
    {
        return NULL;
    }

    DataCollecter *pObj = ( DataCollecter * )pThis;
    prctl( PR_SET_NAME, "TimerProcessSlow" );
    sem_wait( &( pObj->m_start_timer_Slow ) );

    /// unit = 10ms, 1s = 100*unit; 5s = 500*unit; 10s = 1000*unit ...
    /// start timeout

    pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_1S, 100 );
    pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_5S, 510 );
    pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_10S, 1020 );
    pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_30S, 3030 );
    pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_60S, 6040 );

    while( TRUE )
    {
        /// 10ms
        usleep( 10000 );

        ///Timeout process
        pObj->m_timeout_slow->UpdateTimeout();

        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_1S ) )
        {
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_1S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_1S, 100 );
            LOG( INFO ) << "reset 1s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_5S ) )
        {
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_5S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_5S, 510 );
            LOG( INFO ) << "reset 5s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_10S ) )
        {
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_10S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_10S, 1020 );
            LOG( INFO ) << "reset 10s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_30S ) )
        {
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_30S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_30S, 3030 );
            LOG( INFO ) << "reset 30s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_60S ) )
        {
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_60S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_60S, 6040 );
            LOG( INFO ) << "reset 60s counter";
        }

        if( pObj-> m_thread_slow_stop )
        {
            break;
        }

    }

    return NULL;
}

void * DataCollecter::PostHandler( void *pThis )
{
    LOG( INFO ) << "PostHandler create!";
    sleep( 3 );
    if( NULL == pThis )
    {
        return NULL;
    }

    DataCollecter *pObj = ( DataCollecter * )pThis;
    prctl( PR_SET_NAME, "PostHandler" );
    sem_wait( &( pObj->m_start_post ) );

    while( TRUE )
    {
        sem_wait( &( pObj->m_sem_post ) );

        LOG( INFO ) << "PostHandler";

        if( pObj->m_thread_post_stop )
        {
            break;
        }

    }

    return NULL;
}

INT32 DataCollecter::GetIOFunctionFromType( UINT32 type )
{
    for( UINT32 i = 0; i < sizeof( functionlist ) / sizeof( IOFunctionList ); i++ )
    {
        if( type == functionlist[i].functionNum )
        {
            return i;
        }
    }

    /// invalid index
    return -1;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief start timeout count
 *
 * @param[in] type state, tx, rx
 * @param[in] threshold --- timeout threshold
 */
/* ----------------------------------------------------------------------------*/
void TimeOutCondition::StartTimeout( UINT8 type, UINT32 threshold )
{
    if( type < TIMEOUT_TYPE_MAX )
    {
        if( TRUE != m_timeout_enable[type] || threshold < m_timeout_count[type] )
        {
            m_timeout_enable[type] = TRUE;
            m_timeout_count[type] = 0;
            m_timeout_threshold[type] = threshold;
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief disable timeout
 *
 * @param[in] type tx, rx, state
 */
/* ----------------------------------------------------------------------------*/
void TimeOutCondition::StopTimeout( UINT8 type )
{
    if( type < TIMEOUT_TYPE_MAX )
    {
        m_timeout_enable[type] = FALSE;
        m_timeout_count[type] = 0;
        m_timeout_threshold[type] = 0xFFFFFFFF;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief update timeout count
 */
/* ----------------------------------------------------------------------------*/
void TimeOutCondition::UpdateTimeout()
{
    for( UINT8 i = TIMEOUT_TYPE_CONNECT; i < TIMEOUT_TYPE_MAX; i++ )
    {
        if( TRUE == m_timeout_enable[i] )
        {
            m_timeout_count[i]++;
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief check the timeout overflow
 *
 * @param[in] type tx, rx, state
 *
 * @returns
 */
/* ----------------------------------------------------------------------------*/
BOOLEAN TimeOutCondition::CheckTimeout( UINT8 type )
{
    if( type < TIMEOUT_TYPE_MAX )
    {
        if( TRUE == m_timeout_enable[type] )
        {
            if( m_timeout_count[type] > m_timeout_threshold[type] )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}


