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
#include <cstdio>
#include <functional>
#include <string>
#include <vector>


DataCollecter* DataCollecter::m_pInstance_s = NULL;

#define TYPE_BASE_MASK  0xf

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

    m_protocol_cxt = NULL;
    m_connect = FALSE;
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


        /// Init connect
        if( jsoncfg["device"]["interface"] == 2 )
        {
            LOG( INFO ) << jsoncfg["device"]["ip"] << "-" << jsoncfg["device"]["port"];

            /// get string
            string ip = jsoncfg["device"]["ip"];
            m_ipaddr = ip;
            UINT16 port = ( UINT16 )jsoncfg["device"]["port"];
            m_port = port;
            UINT32 protocol_id = ( UINT16 )jsoncfg["device"]["protocol"];
            m_protocol_id = protocol_id;

            if( m_protocol_id == SIEMENSCNC )
            {
#if 0
                m_protocol_cxt = ( VOID * )siemenscnc_828d_new( ( const char * ) m_ipaddr.data(), m_port );
                ///connect
                if( siemenscnc_connect( ( siemenscnc_t * )m_protocol_cxt ) > 0 )
                {
                    m_connect = TRUE;
                    LOG( INFO ) << "siemenscnc connect success!";
                }
                else
                {
                    m_connect = FALSE;
                    LOG( ERROR ) << "siemenscnc connect fail!";
                }
#endif
                m_connect = TRUE;
            }
            else
            {
                LOG( ERROR ) << "protocol not support !";
            }
        }

        for( UINT32 len = 0; len < jsoncfg["listGet"].size(); len++ )
        {
            float freq = ( float )( jsoncfg["listGet"][len]["frequency"] );
            LOG( INFO ) << "Parse frequency ->"  << freq ;

            if( freq == 0.1 )
            {
                InitFuncList( &m_iolist_100ms, jsoncfg["listGet"][len] );
            }
            else if( freq == 1 )
            {
                InitFuncList( &m_iolist_1s, jsoncfg["listGet"][len] );
            }
            else if( freq == 5 )
            {
                InitFuncList( &m_iolist_5s, jsoncfg["listGet"][len] );
            }
            else if( freq == 10 )
            {
                InitFuncList( &m_iolist_10s, jsoncfg["listGet"][len] );
            }
            else if( freq == 30 )
            {
                InitFuncList( &m_iolist_30s, jsoncfg["listGet"][len] );
            }
            else if( freq == 60 )
            {
                InitFuncList( &m_iolist_60s, jsoncfg["listGet"][len] );
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

void DataCollecter::InitFuncList( vector<IOFunctionList> *funclist, json tmpjson )
{
    for( UINT32 len = 0; len < tmpjson["list"].size() ; len++ )
    {
        UINT32 functionNum = tmpjson["list"][len]["type"];
        FLOAT freq = tmpjson["frequency"] ;
        string regName = to_string( tmpjson["list"][len]["name"] ) ;
        INT32 funcIndex = GetIOFunctionFromType( functionNum ) ;
        if( funcIndex < 0 )
        {
            LOG( ERROR ) << "frequency = " << freq << "s: regName = " << regName << " functionNum = " << functionNum << " not found!";
        }
        else
        {
            FormatParamList( funclist, funcIndex, regName );
            LOG( INFO ) << "frequency = " << freq << "s:  regName = " << regName << " functionNum = " << functionNum << " ok!";
        }
    }
}


VOID DataCollecter::FormatParamList( vector<IOFunctionList> *funclist, INT32 funcIndex, string regName )
{
    IOFunctionList *tempFunc = new IOFunctionList;
    tempFunc = ( IOFunctionList * )&functionlist_siemens[funcIndex];

    /// this is common
    /// Max paramlist size - 8
    VOID **paramList = new VOID *[8];
    tempFunc->paramList = paramList;
    for( UINT32 i = 0; i < tempFunc->paramNum; i++ )
    {
        ///得到每个参数的类型
        OCTET datatye = ( tempFunc->datatype & ( ( TYPE_BASE_MASK ) << ( i * 4 ) ) ) >> ( i * 4 );
        if( 0 == i )
        {
            /// The first param is protocol cxt
            TypeAny * param1 =  new TypeAny( m_protocol_cxt, datatye );
            tempFunc->paramList[i] = ( void * )param1;
        }
        else
        {
            /// all param is pointer, the maxsize is 8 bytes
            OCTET * ptr = new OCTET[8];
            TypeAny * parami =  new TypeAny( ptr, datatye );
            tempFunc->paramList[i] = ( void * )parami;
        }
        LOG( INFO ) << "Type of param [" << i << "] is " << type_base_str[datatye];
    }
    funclist->push_back( *tempFunc );
}

INT32 DataCollecter::JobFunctionCall( pJobFunc JobFunc, UINT32 paramNum, void **paramList )
{
    INT32 *ret = NULL;

    switch ( paramNum )
    {
        case 2:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1] ) ) );
            break;
        case 3:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2] ) ) );
            break;
        case 4:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2], paramList[3] ) ) );
            break;
        case 5:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2], paramList[3], paramList[4] ) ) );
            break;
        case 6:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5] ) ) );
            break;
        case 7:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2], paramList[3], paramList[4], paramList[5], paramList[6] ) ) );
            break;
        default:
            ret = 0;
            break;
    }

    if( ret == NULL )
    {
        return 0;
    }
    else
    {
        return *ret;
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

        if( !pObj->m_connect )
        {
            continue;
        }
        ///Timeout process
        pObj->m_timeout_fast->UpdateTimeout();

        if( TRUE == pObj->m_timeout_fast->CheckTimeout( TIMEOUT_TYPE_Fast ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_100ms.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_100ms[i].func, pObj->m_iolist_100ms[i].paramNum, pObj->m_iolist_100ms[i].paramList );
                if( pObj->m_iolist_100ms[i].paramNum > pObj->m_iolist_100ms[i].returnpos )
                {
                    cout << pObj->m_iolist_100ms[i].paramList[pObj->m_iolist_100ms[i].paramNum - 2];
                    cout << pObj->m_iolist_100ms[i].paramList[pObj->m_iolist_100ms[i].paramNum - 1];
                }
                else
                {
                    cout << pObj->m_iolist_100ms[i].paramList[pObj->m_iolist_100ms[i].paramNum - 1];
                }

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
        if( !pObj->m_connect )
        {
            continue;
        }

        ///Timeout process
        pObj->m_timeout_slow->UpdateTimeout();

        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_1S ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_1s.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_1s[i].func, pObj->m_iolist_1s[i].paramNum, pObj->m_iolist_1s[i].paramList );
                if( pObj->m_iolist_1s[i].paramNum > pObj->m_iolist_1s[i].returnpos )
                {
                    cout <<  pObj->m_iolist_1s[i].paramList[pObj->m_iolist_1s[i].paramNum - 2];
                    cout <<  pObj->m_iolist_1s[i].paramList[pObj->m_iolist_1s[i].paramNum - 1];
                }
                else
                {
                    cout <<  pObj->m_iolist_1s[i].paramList[pObj->m_iolist_1s[i].paramNum - 1];
                }
            }

            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_1S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_1S, 100 );
            LOG( INFO ) << "reset 1s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_5S ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_5s.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_5s[i].func, pObj->m_iolist_5s[i].paramNum, pObj->m_iolist_5s[i].paramList );
                if( pObj->m_iolist_5s[i].paramNum > pObj->m_iolist_5s[i].returnpos )
                {
                    LOG( INFO ) << "value = 0x" << ( CHAR * )( pObj->m_iolist_5s[i].paramList[pObj->m_iolist_5s[i].paramNum - 2] ) ;
                    LOG( INFO ) << "len = "  << *( ( int * )pObj->m_iolist_5s[i].paramList[pObj->m_iolist_5s[i].paramNum - 1] );
                }
                else
                {
                    LOG( INFO ) << "value = " <<  *( ( int * )pObj->m_iolist_5s[i].paramList[pObj->m_iolist_5s[i].paramNum - 1] );
                }

#if 0
                //// HTTP Post
                RestClient::Response resp = RestClient::post( "http://192.168.0.23:9080/ping", "application/x-www-form-urlencoded", "{\"foo\":\"test\"}" );

                cout << resp.body;
#endif
            }
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_5S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_5S, 510 );
            LOG( INFO ) << "reset 5s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_10S ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_10s.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_10s[i].func, pObj->m_iolist_10s[i].paramNum, pObj->m_iolist_10s[i].paramList );
            }
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_10S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_10S, 1020 );
            LOG( INFO ) << "reset 10s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_30S ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_30s.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_30s[i].func, pObj->m_iolist_30s[i].paramNum, pObj->m_iolist_30s[i].paramList );
            }
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_30S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_30S, 3030 );
            LOG( INFO ) << "reset 30s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_60S ) )
        {
            for( UINT32 i = 0; i < pObj->m_iolist_60s.size(); i++ )
            {
                pObj->JobFunctionCall( pObj->m_iolist_60s[i].func, pObj->m_iolist_60s[i].paramNum, pObj->m_iolist_60s[i].paramList );
            }
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

#if 0
        //// HTTP Post
        RestClient::Response resp = RestClient::post( "http://192.168.0.23:9080/ping", "application/x-www-form-urlencoded", "{\"foo\":\"test\"}" );

        cout << resp.body;
#endif

        if( pObj->m_thread_post_stop )
        {
            break;
        }

    }

    return NULL;
}

INT32 DataCollecter::GetIOFunctionFromType( UINT32 type )
{
    IOFunctionList *pFuncList = NULL;
    UINT32 funclist_len = 0;

    /// maybe many protocol
    if( m_protocol_id == SIEMENSCNC )
    {
        pFuncList = functionlist_siemens;
        funclist_len = sizeof( functionlist_siemens ) / sizeof( IOFunctionList );
    }
    else
    {
        LOG( ERROR ) << "Protocol not support!";
    }

    if( pFuncList != NULL )
    {
        for( UINT32 i = 0; i < funclist_len; i++ )
        {
            if( type == pFuncList[i].functionNum )
            {
                LOG( INFO ) << "function is " << pFuncList[i].funcName;
                return i;
            }
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


