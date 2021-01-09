#include "include/normal_typedef.h"
#include "feature/log/inc/easylogging.h"
#include "thirdparty/restclient/include/restclient-cpp/restclient.h"
#include "common/typeany/inc/typeany.h"

#include "product/inc/datacollecter.h"
/// 西门子CNC test
//
#include "protocol/SimensCnc/Simens/libsiemenscnc/inc/siemenscnc.h"
#include "protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.h"

/// JSON
#include "include/json.hpp"
#include <fstream>

using namespace std;
using json = nlohmann::json;

INITIALIZE_EASYLOGGINGPP

void easylogginginit()
{
    // 加载配置文件，构造一个配置器对象
    el::Configurations conf( "/home/fchuanlin/datamodule/doc/log.conf" );
    // 重新配置一个单一日志记录器
    el::Loggers::reconfigureLogger( "default", conf );
    // 用配置文件配置所有的日志记录器
    el::Loggers::reconfigureAllLoggers( conf );
}

int TestJson()
{
    auto text = R"(
    {
        "Image": {
            "Width":  800,
            "Height": 600,
            "Title":  "View from 15th Floor",
            "Thumbnail": {
                "Url":    "http://www.example.com/image/481989943",
                "Height": 125,
                "Width":  100
            },
            "Animated" : false,
            "IDs": [116, 943, 234, 38793]
        }
    }
    )";

    // fill a stream with JSON text
    std::stringstream ss;
    ss << text;

    // parse and serialize JSON
    json j_complete = json::parse( ss );
    std::cout << std::setw( 4 ) << j_complete << "\n\n";


    // define parser callback
    json::parser_callback_t cb = []( int depth, json::parse_event_t event, json & parsed )
    {
        // skip object elements with key "Thumbnail"
        if ( event == json::parse_event_t::key and parsed == json( "Thumbnail" ) )
        {
            return false;
        }
        else
        {
            return true;
        }
    };

    // fill a stream with JSON text
    ss.clear();
    ss << text;

    // parse (with callback) and serialize JSON
    json j_filtered = json::parse( ss, cb );
    std::cout << std::setw( 4 ) << j_filtered << '\n';
    return 0;
}

int main( int argc, char * argv[] )
{
    easylogginginit();

    LOG_IF( ( ( 1 % 2 ) == 0 ), INFO ) << "Logged if condition is true";
    LOG( DEBUG ) << "My first info log using default logger! ";

    //// 西门子采集协议测试程序
#if 1
    siemenscnc_t *ctx;
    ctx = siemenscnc_828d_new( "192.168.0.55", 102 );
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

    /// 开启采集线程
    DataCollecter *pDC = new DataCollecter();
    pDC->Init();
    if( argc > 1 )
    {
#ifdef PRINT_DEBUG_INFO
        pDC->InitIOListByJson( ( const CHAR * )"xDC_Get.json" );
#else
        /// 读取配置文件
        pDC->InitIOListByJson( ( const CHAR * )argv[1] );
#endif
    }
    else
    {
        pDC->InitIOListByJson( ( const CHAR * )"xDC_Get.json" );
    }

    pDC->Start();

    //// HTTP Post
    RestClient::Response resp = RestClient::post( "http://192.168.0.23:9080/ping", "application/x-www-form-urlencoded", "{\"foo\":\"test\"}" );

    cout << resp.body;

    LOG( ERROR ) << "Error log, need more info! ";

    std::cout << "Hello X-DataCollecter!\n" << std::endl;

    return 0;
}





