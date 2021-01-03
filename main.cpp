#include "include/normal_typedef.h"
#include "feature/log/inc/easylogging.h"
#include "thirdparty/restclient/include/restclient-cpp/restclient.h"
#include "common/typeany/inc/typeany.h"

/// 西门子CNC test
//
#include "protocol/SimensCnc/Simens/libsiemenscnc/inc/siemenscnc.h"
#include "protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.h"

INITIALIZE_EASYLOGGINGPP

void easylogginginit()
{
    // 加载配置文件，构造一个配置器对象
    el::Configurations conf( "../../doc/log.conf" );
    // 重新配置一个单一日志记录器
    el::Loggers::reconfigureLogger( "default", conf );
    // 用配置文件配置所有的日志记录器
    el::Loggers::reconfigureAllLoggers( conf );
}

void testtypeany( UINT8 i )
{
    Any testany( i );

    LOG( INFO ) << "test typeany data = " << ( UINT8 )testany.v.m_octet << "TEID = " << testany.m_typeany_id;
    LOG( INFO ) << "test typeany data = " << testany.v.m_int;
}

int main()
{
    easylogginginit();

    for( int i = 1; i < 10; i++ )
    {
        char mystr[10] = {0};
        sprintf( mystr, "mystr%d", i );
        LOG_IF( ( ( i % 2 ) == 0 ), INFO ) << "Logged if condition is true" << i << "i = " << i << "i = " << i;
        LOG( DEBUG ) << "My first info log using default logger! " << mystr;
        testtypeany( i );
    }


    //// 西门子采集协议测试程序
#if 1
    siemenscnc_t *ctx;
    ctx = siemenscnc_828d_new( "192.168.2.99", 102 );
    //
    siemenscnc_set_debug( ctx, 1 );
    struct timeval t;
    int rc;
    t.tv_sec = 0;
    t.tv_usec = 1000000; //设置超时时间为1000毫秒
    siemenscnc_set_error_recovery( ctx, SIEMENSCNC_ERROR_RECOVERY_LINK );
    siemenscnc_set_response_timeout( ctx, &t );
    rc = siemenscnc_connect( ctx );
    cout << "connnect" << rc;
    if( rc >= 0 )
    {
        uint8_t dest[256] = {0};
        uint16_t length;
        rc = siemenscnc_read_s_r_p_param( ctx, 0, 37, 2, dest, &length ); //驱动器R参数
        cout << "驱动器r37[2]" << siemenscnc_get_float( dest ) << "返回值" << rc;
    }
#endif

    //// HTTP Post
    RestClient::Response resp = RestClient::post( "http://192.168.3.23:9080/ping", "application/x-www-form-urlencoded", "{\"foo\":\"test\"}" );

    cout << resp.body;

    LOG( ERROR ) << "Error log, need more info! ";

    std::cout << "Hello X-DataCollecter!\n" << std::endl;

    return 0;
}
