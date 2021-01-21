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
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <ios>
#include <string>
#include <sys/types.h>
#include <vector>


DataCollecter* DataCollecter::m_pInstance_s = NULL;

#define TYPE_BASE_MASK  0xf
UINT32 G_LOG_ENABLE = 1;
UINT32 G_DEBUG_LOG_ENABLE = 1;

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


/**
 * @brief  init function list by json file
 *
 * @param[in]  fileName
 *
 * @returns
 */
BOOLEAN DataCollecter::InitIOListByJson( const CHAR *fileName )
{
    ifstream fjson( ( const char * )fileName );
    json jsoncfg;
    fjson >> jsoncfg;
    if( ( string )jsoncfg["command"] == "get" )
    {
        LOG_IF( G_LOG_ENABLE, INFO ) << "get" << endl;
        LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["device"] << endl;
        LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["listGet"] << endl;


        /// Init connect
        if( jsoncfg["device"]["interface"] == 2 )
        {
            LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["device"]["ip"] << "-" << jsoncfg["device"]["port"];

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
                if( siemenscnc_connect( ( siemenscnc_t * )m_protocol_cxt ) >= 0 )
                {
                    m_connect = TRUE;
                    LOG_IF( G_LOG_ENABLE, INFO ) << "siemenscnc connect success!";
                }
                else
                {
                    m_connect = FALSE;
                    LOG( ERROR ) << "siemenscnc connect fail!";
                }
#endif
            }
            else
            {
                LOG( ERROR ) << "protocol not support !";
            }
        }

        m_connect = TRUE;

        /// connect failed, return
        if( FALSE == m_connect )
        {
            fjson.close();
            return FALSE;
        }

        for( UINT32 len = 0; len < jsoncfg["listGet"].size(); len++ )
        {
            float freq = ( float )( jsoncfg["listGet"][len]["frequency"] );
            LOG_IF( G_LOG_ENABLE, INFO ) << "Parse frequency ->"  << freq ;

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
        LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["device"] << endl;
        LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["listSet"].count( jsoncfg["listSet"] ) << endl;
        LOG_IF( G_LOG_ENABLE, INFO ) << jsoncfg["listSet"] << endl;
    }
    fjson.close();

    return TRUE;
}



/**
 * @brief  for funciton list
 *
 * @param[in]  funclist
 * @param[in]  tmpjson
 */
void DataCollecter::InitFuncList( vector<IOFunctionList> *funclist, json tmpjson )
{
    for( UINT32 len = 0; len < tmpjson["list"].size() ; len++ )
    {
        UINT32 functionNum = tmpjson["list"][len]["type"];
        UINT32 key = tmpjson["list"][len]["id"];
        FLOAT freq = tmpjson["frequency"] ;
        string regName = to_string( tmpjson["list"][len]["name"] ) ;
        INT32 funcIndex = GetIOFunctionFromType( functionNum ) ;
        if( funcIndex < 0 )
        {
            LOG( ERROR ) << "frequency = " << freq << "s: regName = " << regName << " functionNum = " << functionNum << " not found!";
        }
        else
        {
            FormatParamList( funclist, funcIndex, key, regName );
            LOG_IF( G_LOG_ENABLE, INFO ) << "frequency = " << freq << "s:  regName = " << regName << " functionNum = " << functionNum << " ok!";
        }
    }
}


/**
 * @brief  format the param list
 *
 * @param[in]  funclist
 * @param[in]  funcIndex
 * @param[in]  key
 * @param[in]  regName
 */
VOID DataCollecter::FormatParamList( vector<IOFunctionList> *funclist, INT32 funcIndex, UINT32 key, string regName )
{
    IOFunctionList *tempFunc = new IOFunctionList;
    tempFunc = ( IOFunctionList * )&functionlist_siemens[funcIndex];

    /// return json ID
    tempFunc->Key = key;
    /// this is common
    /// Max paramlist size - 8
    VOID **paramList = new VOID *[8];
    tempFunc->paramList = paramList;
    for( UINT32 i = 0; i < tempFunc->paramNum; i++ )
    {
        ///得到每个参数的类型
        OCTET datatye = ( tempFunc->paramtype & ( ( TYPE_BASE_MASK ) << ( i * 4 ) ) ) >> ( i * 4 );
        if( 0 == i )
        {
            /// The first param is protocol cxt
            TypeAny * param1 =  new TypeAny( m_protocol_cxt, datatye );
            tempFunc->paramList[i] = ( void * )( param1->v.m_void );
        }
        else
        {
            /// all param is pointer, the maxsize is 8 bytes
            OCTET * ptr = new OCTET[8];
            if( TypeAny::IsSimpleDateType( datatye ) )
            {
                TypeAny * parami =  new TypeAny( *ptr, datatye );
                tempFunc->paramList[i] = ( void * )parami;
            }
            else
            {
                TypeAny * parami =  new TypeAny( ptr, datatye );
                tempFunc->paramList[i] = ( void * )parami;
            }
        }
        LOG_IF( G_LOG_ENABLE, INFO ) << "Type of param [" << i << "] is " << type_base_str[datatye];
    }

    /// 根据regname得到输入的参数列表，统一为int类型，int转ushort或者uint8的位宽足够
    tempFunc->regName = regName;
    FillParamInput( tempFunc->returnpos, tempFunc->paramNum, regName, tempFunc->paramList );

    funclist->push_back( *tempFunc );
}



/**
 * @brief  get input params
 *
 * @param[in]  regName
 * @param[in]  axis
 * @param[in]  address
 * @param[in]  subaddress
 */
UINT8 DataCollecter::ParseParamsFromRegName( string regName, INT32 *param1, INT32 *param2, INT32 *param3 )
{
    UINT8 paramNum = 0;

    /// remove "
    if( regName.npos != regName.find( "\"" ) )
    {
        regName = regName.substr( 1, regName.length() - 2 );
        LOG_IF( G_LOG_ENABLE, INFO ) << "regName = " << regName;
    }

    /// find :
    if( regName.npos != regName.find( ":" ) )
    {
        string left_str = regName.substr( 0, regName.find( ":" ) );
        string right_str = regName.substr( 1 + regName.find( ":" ) );

        /// first get axis
        *param1 = atoi( left_str.data() );
        LOG_IF( G_LOG_ENABLE, INFO ) << "param1 = " << left_str;
        paramNum++;
        if( right_str.npos != right_str.find( "." ) )
        {
            string address_left_str = right_str.substr( 0, right_str.find( "." ) );
            string address_right_str = right_str.substr( 1 + right_str.find( "." ) );
            *param2 = atoi( address_left_str.data() );
            paramNum++;
            *param3 = atoi( address_right_str.data() );
            paramNum++;
            LOG_IF( G_LOG_ENABLE, INFO ) << "param2 = " << *param2;
            LOG_IF( G_LOG_ENABLE, INFO ) << "param3 = " << *param3;
        }
        else
        {
            *param2 = atoi( right_str.data() );
        }
    }
    else
    {
        if( regName.npos != regName.find( "." ) )
        {
            string address_left_str = regName.substr( 0, regName.find( "." ) );
            string address_right_str = regName.substr( 1 + regName.find( "." ) );
            *param1 = atoi( address_left_str.data() );
            paramNum++;
            *param2 = atoi( address_right_str.data() );
            paramNum++;
            LOG_IF( G_LOG_ENABLE, INFO ) << "param1 = " << *param1;
            LOG_IF( G_LOG_ENABLE, INFO ) << "param2 = " << *param2;
        }
        else
        {
            *param1 = atoi( regName.data() );
            LOG_IF( G_LOG_ENABLE, INFO ) << "param1 = " << *param1 ;
            paramNum++;
        }
    }

    return paramNum;
}
/**
 * @brief  Fill the input param, param is prase from regName
 *
 * @param[in]  retpos
 * @param[in]  paramNum
 * @param[in]  regName
 * @param[in]  paramList
 *
 * @returns
 */
INT32 DataCollecter::FillParamInput( UINT32 retpos, UINT32 paramNum, string regName, void **paramList )
{
    /// regName, test value
    INT32 *param1 = new INT32( 0 );
    INT32 *param2 = new INT32( 0 );
    INT32 *param3 = new INT32( 0 );
    INT32 *param4 = new INT32( 0 );

    if( !regName.empty() )
    {
        UINT8 jsonParmNum = ParseParamsFromRegName( regName, param1, param2, param3 );
        LOG_IF( G_LOG_ENABLE, INFO ) << "regName = " << regName << "-->jsonParamNum = " << ( INT32 )jsonParmNum;
    }

    switch ( paramNum )
    {
        case 2:
            break;
        case 3:
            if( retpos == paramNum )
            {
                paramList[1] = param1;
            }
            break;
        case 4:
            if( retpos == paramNum )
            {
                paramList[1] = param1;
                paramList[2] = param2;
            }
            else
            {
                paramList[1] = param1;
            }
            break;
        case 5:
            if( retpos == paramNum )
            {
                paramList[1] = param1;
                paramList[2] = param2;
                paramList[3] = param3;
            }
            else
            {
                paramList[1] = param1;
                paramList[2] = param2;
            }
            break;
        case 6:
            if( retpos == paramNum )
            {
                paramList[1] = param1;
                paramList[2] = param2;
                paramList[3] = param3;
                paramList[4] = param4;
            }
            else
            {
                paramList[1] = param1;
                paramList[2] = param2;
                paramList[3] = param3;
            }

            break;
        case 7:
            break;
        default:
            break;
    }

    return Lret_success;
}


/**
 * @brief  common call function
 *
 * @param[in]  JobFunc
 * @param[in]  retpos
 * @param[in]  paramNum
 * @param[in]  paramList
 *
 * @returns
 */
INT32 DataCollecter::JobFunctionCall( pJobFunc JobFunc, UINT32 retpos, UINT32 paramNum, void **paramList )
{
    INT32 *ret = NULL;

    switch ( paramNum )
    {
        case 2:
            ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1] ) ) );
            break;
        case 3:
            if( retpos == paramNum )/// one input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], paramList[2] ) ) );
            }
            else/// no input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], paramList[1], paramList[2] ) ) );
            }
            break;
        case 4:
            if( retpos == paramNum )/// two input param
            {

                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], *( int * )paramList[2], paramList[3] ) ) );
            }
            else/// one input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], paramList[2], paramList[3] ) ) );
            }
            break;
        case 5:
            if( retpos == paramNum )/// three input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], *( int * )paramList[2], *( int * )paramList[3], paramList[4] ) ) );
            }
            else/// two input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], *( int * )paramList[2], paramList[3], paramList[4] ) ) );
            }
            break;
        case 6:
            if( retpos == paramNum )/// four input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], *( int * )paramList[2], *( int * )paramList[3], *( int * )paramList[4], paramList[5] ) ) );
            }
            else/// three input param
            {
                ret = ( ( INT32 * )( JobFunc( paramList[0], *( int * )paramList[1], *( int * )paramList[2], *( int * )paramList[3], paramList[4], paramList[5] ) ) );
            }
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
    LOG_IF( G_LOG_ENABLE, INFO ) << "TimerProcess create!";
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
            pObj->LoopFuncList( pObj->m_iolist_100ms );
            /// reset timeout
            pObj->m_timeout_fast->StopTimeout( TIMEOUT_TYPE_Fast );
            pObj->m_timeout_fast->StartTimeout( TIMEOUT_TYPE_Fast, 10 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset Fast counter";
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
    LOG_IF( G_LOG_ENABLE, INFO ) << "TimerProcessSlow create!";
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
            pObj->LoopFuncList( pObj->m_iolist_1s );
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_1S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_1S, 100 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset 1s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_5S ) )
        {

            pObj->LoopFuncList( pObj->m_iolist_5s );
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_5S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_5S, 510 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset 5s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_10S ) )
        {
            pObj->LoopFuncList( pObj->m_iolist_10s );
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_10S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_10S, 1020 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset 10s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_30S ) )
        {
            pObj->LoopFuncList( pObj->m_iolist_30s );
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_30S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_30S, 3030 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset 30s counter";
        }
        if( TRUE == pObj->m_timeout_slow->CheckTimeout( TIMEOUT_TYPE_60S ) )
        {
            pObj->PrintAllList();
            pObj->LoopFuncList( pObj->m_iolist_60s );
            /// reset timeout
            pObj->m_timeout_slow->StopTimeout( TIMEOUT_TYPE_60S );
            pObj->m_timeout_slow->StartTimeout( TIMEOUT_TYPE_60S, 6040 );
            LOG_IF( G_LOG_ENABLE, INFO ) << "reset 60s counter";
        }

        if( pObj-> m_thread_slow_stop )
        {
            break;
        }

    }

    return NULL;
}


/**
 * @brief  for post json string
 *
 * @param[in]  pThis
 *
 * @returns
 */
void * DataCollecter::PostHandler( void *pThis )
{
    LOG_IF( G_LOG_ENABLE, INFO ) << "PostHandler create!";
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

        LOG_IF( G_LOG_ENABLE, INFO ) << "PostHandler";

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


/**
 * @brief  loop funclist
 *
 * @param[in]  funclist
 */
VOID DataCollecter::LoopFuncList( vector<IOFunctionList> funclist )
{
    for( UINT32 i = 0; i < funclist.size(); i++ )
    {
        KeyValue tempRtnStr;
        tempRtnStr.key = to_string( funclist[i].Key );

        OCTET datatye = ( funclist[i].paramtype & ( ( TYPE_BASE_MASK ) << ( ( funclist[i].paramNum - 1 ) * 4 ) ) ) >> ( ( funclist[i].paramNum - 1 ) * 4 );

        JobFunctionCall( funclist[i].func, funclist[i].returnpos, funclist[i].paramNum, funclist[i].paramList );
        if( funclist[i].paramNum > funclist[i].returnpos )
        {
            LOG_IF( G_LOG_ENABLE, INFO ) <<  ( CHAR * )( funclist[i].paramList[funclist[i].paramNum - 2] );
            LOG_IF( G_LOG_ENABLE, INFO ) <<  *( ( u_int16_t * )( funclist[i].paramList[funclist[i].paramNum - 1] ) );
            tempRtnStr.value = string( ( char * )( funclist[i].paramList[funclist[i].paramNum - 2] ) );
        }
        else
        {
            if( datatye == TYPE_BASE_BUFF_PTR )
            {
                LOG_IF( G_LOG_ENABLE, INFO ) <<  ( CHAR * )( funclist[i].paramList[funclist[i].paramNum - 2] );
                tempRtnStr.value = string( ( char * )( funclist[i].paramList[funclist[i].paramNum - 2] ) );
            }
            else if( datatye == TYPE_BASE_FLOAT )
            {
                LOG_IF( G_LOG_ENABLE, INFO ) <<  "value" << *( FLOAT * )( funclist[i].paramList[funclist[i].paramNum - 1] );
                tempRtnStr.value = to_string( *( FLOAT * )( funclist[i].paramList[funclist[i].paramNum - 1] ) );
            }
            else if( datatye == TYPE_BASE_DOUBLE )
            {
                LOG_IF( G_LOG_ENABLE, INFO ) <<  "value" << *( DOUBLE * )( funclist[i].paramList[funclist[i].paramNum - 1] );
                tempRtnStr.value = to_string( *( DOUBLE * )( funclist[i].paramList[funclist[i].paramNum - 1] ) );
            }
            else
            {
                LOG_IF( G_LOG_ENABLE, INFO ) <<  "value" << *( OCTET * )( funclist[i].paramList[funclist[i].paramNum - 1] );
                tempRtnStr.value = to_string( *( OCTET * )( funclist[i].paramList[funclist[i].paramNum - 1] ) );
            }
        }

        cout << "{\"" + tempRtnStr.key + "\"" + ":" + "\"" + tempRtnStr.value + "\"}," << endl;
    }
}


/**
 * @brief  get index from function num - type
 *
 * @param[in]  type
 *
 * @returns
 */
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
                LOG_IF( G_LOG_ENABLE, INFO ) << "function is " << pFuncList[i].funcName;
                return i;
            }
        }
    }

    /// invalid index
    return -1;
}


//////////////////////////////////////////////////////DEBUG/////////////////////////////////////////////////////////////////
/**
 * @brief  print function list
 *
 * @param[in]  funclist
 */
VOID DataCollecter::PrintFuncList( vector<IOFunctionList> funclist )
{

    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "-----------------------------------------------------------------";
    for( UINT32 i = 0; i < funclist.size(); i++ )
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "----------------------------" << i + 1 << "------------------------";
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "funcName:" << funclist[i].funcName;
        for( UINT32 type_index = 0; type_index < funclist[i].paramNum; type_index++ )
        {
            OCTET datatye = ( funclist[i].paramtype & ( ( TYPE_BASE_MASK ) << ( type_index * 4 ) ) ) >> ( type_index * 4 );

            LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "Type of param [" << type_index << "] is " << type_base_str[datatye];
        }

        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "functionNum:" << funclist[i].functionNum;
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "paramNum:" << funclist[i].paramNum;
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "returnPos:" << funclist[i].returnpos;
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "paramtype:" << hex << funclist[i].paramtype;
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "regName:" << funclist[i].regName;
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) << "Key:" << dec << funclist[i].Key;
    }
    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "-----------------------------------------------------------------";
}

/**
 * @brief  print debug log for iolist
 */
VOID DataCollecter::PrintAllList()
{
    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 100ms function list:";
    if( m_iolist_100ms.size() > 0 )
    {
        PrintFuncList( m_iolist_100ms );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }

    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 1s function list:";
    if( m_iolist_1s.size() > 0 )
    {
        PrintFuncList( m_iolist_1s );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }

    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 5s function list:";
    if( m_iolist_5s.size() > 0 )
    {
        PrintFuncList( m_iolist_5s );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }
    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 10s function list:";
    if( m_iolist_10s.size() > 0 )
    {
        PrintFuncList( m_iolist_10s );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }
    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 30s function list:";
    if( m_iolist_30s.size() > 0 )
    {
        PrintFuncList( m_iolist_30s );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }
    LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "period of 60s function list:";
    if( m_iolist_60s.size() > 0 )
    {
        PrintFuncList( m_iolist_60s );
    }
    else
    {
        LOG_IF( G_DEBUG_LOG_ENABLE, INFO ) <<   "NULL";
    }
}

VOID DataCollecter::SetLogEnable()
{
    G_LOG_ENABLE = 1;
}


VOID DataCollecter::SetLogDisable()
{
    G_LOG_ENABLE = 0;
}

VOID DataCollecter::SetDebugLogEnable()
{
    G_DEBUG_LOG_ENABLE = 1;
}

VOID DataCollecter::SetDebugLogDisable()
{
    G_DEBUG_LOG_ENABLE = 0;
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


