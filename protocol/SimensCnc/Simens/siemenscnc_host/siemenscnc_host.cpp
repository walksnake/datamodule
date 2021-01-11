#include "siemenscnc_host.h"

siemenscnc_host::siemenscnc_host( QObject *parent ) :
    QThread( parent )
{
    map.clear();
}

siemenscnc_host::~siemenscnc_host()
{

}

/*
 * 读取数据
 */
int siemenscnc_host::read_param( int protocol, //协议名
                                 const void *hardware_param,//IP/串口参数
                                 const void *param,//协议参数
                                 QString ioname,//ioname
                                 int function,//功能码
                                 int bits,//数据类型
                                 QByteArray regname,//要操作的内容
                                 int regnum,//要操作的内容的数量
                                 const void *attributes,//属性
                                 int dataorder,//数据排序
                                 uint8_t *dest,//返回的数据
                                 int &dest_length ) //返回的数据长度
{
    int rc = 0;
    int axis = 0;
    int address = 0;
    int sub_address = 0;
    float f_value;
    double d_value;
    uint16_t length;
    siemenscnc_attributes_t *siemenscnc_attributes = ( siemenscnc_attributes_t* )attributes;
    if( !map.contains( ioname ) )
    {
        siemenscnc_info_c *ctx;
        ctx = _siemenscnc_init( hardware_param, siemenscnc_attributes->timeout );
        if( ctx == NULL )
        {
            return -1;
        }
        if( map[ioname]->is_init )
        {
            map[ioname] = ctx;
            siemenscnc_flush( map[ioname]->cnc );
        }
    }
    if( !map[ioname]->is_init )
    {
        //重新连接
        if( siemenscnc_connect( map[ioname]->cnc ) < 0 )
        {

            return -1;
        }
        map[ioname]->is_init = true;
        siemenscnc_flush( map[ioname]->cnc );
    }

    /* 解析地址 */
    _siemenscnc_decode_address( QString( regname ), axis, address, sub_address );
    msleep( siemenscnc_attributes->comm_delay );
    switch( function )
    {
        case SIEMENSCNC_SERIAL_NUMBER://硬件序列号
            rc = siemenscnc_read_serial_number( map[ioname]->cnc, dest + 2, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_CNC_TYPE://机床类型
            rc = siemenscnc_read_cnc_type( map[ioname]->cnc, dest + 2, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_VERSION://版本信息
            rc = siemenscnc_read_verison( map[ioname]->cnc, dest + 2, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_MANUFACTURE_DATE://出厂日期
            rc = siemenscnc_read_manufacture_data( map[ioname]->cnc, dest + 2, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_OPERATE_MODE://操作模式
            rc = siemenscnc_read_operate_mode( map[ioname]->cnc, dest );
            break;
        case SIEMENSCNC_STATUS://运行状态
            rc = siemenscnc_read_status( map[ioname]->cnc, dest );
            break;
        case SIEMENSCNC_PROCESS_NUMBER://加工数量
        {
            rc = siemenscnc_read_process_number( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_PROCESS_SET_NUMBER://设定的加工数量
        {
            rc = siemenscnc_read_process_set_number( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_FEED_SET_SPEED://设定进给速度
        {
            rc = siemenscnc_read_feed_set_speed( map[ioname]->cnc,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_FEED_ACT_SPEED://实际进给速度
        {
            rc = siemenscnc_read_feed_act_speed( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_FEED_RATE://进给倍率
        {
            rc = siemenscnc_read_feed_rate( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_SPINDLE_SET_SPEED://主轴设定速度
        {
            rc = siemenscnc_read_spindle_set_speed( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_SPINDLE_ACT_SPEED://主轴实际速度
        {
            rc = siemenscnc_read_spindle_act_speed( map[ioname]->cnc,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_SPINDLE_RATE://主轴倍率
        {
            rc = siemenscnc_read_spindle_rate( map[ioname]->cnc,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_RUN_TIME://循环时间,一次自动运行起动时间的累计值
        {
            rc = siemenscnc_read_run_time( map[ioname]->cnc,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_REMAIN_TIME://剩余时间
        {
            rc = siemenscnc_read_remain_time( map[ioname]->cnc,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_PROGRAM_NAME://加工程序名
            rc = siemenscnc_read_program_name( map[ioname]->cnc, dest + 2, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_TOOL_NUMBER://刀具号
        {
            uint32_t i_value;
            rc = siemenscnc_read_tool_number( map[ioname]->cnc, &i_value );
            dest[0] = i_value >> 24;
            dest[1] = i_value >> 16;
            dest[2] = i_value >> 8;
            dest[3] = i_value & 0xFF;
        }
        break;
        case SIEMENSCNC_TOOL_D_RADIUS://刀具半径D
        {
            rc = siemenscnc_read_tool_d_radius( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_TOOL_H_LENGTH://刀具长度
        {
            rc = siemenscnc_read_tool_h_length( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_TOOL_X_LENGTH://长度补偿X
        {
            rc = siemenscnc_read_tool_x_length( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_TOOL_Z_LENGTH://长度补偿Z
        {
            rc = siemenscnc_read_tool_z_length( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_TOOL_RADIU://磨损半径
        {
            // rc = siemenscnc_read_tool_wear_radius(map[ioname]->cnc, (double*)&d_value);

        }
        break;
        case SIEMENSCNC_TOOL_EDG://刀沿位置
        {
            rc = siemenscnc_read_tool_edge( map[ioname]->cnc, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_MACHINE_POS://机械坐标
        {
            rc = siemenscnc_read_machine_pos( map[ioname]->cnc, axis,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_RELATIVELY_POS://工件坐标 相对坐标
        {
            rc = siemenscnc_read_relatively_pos( map[ioname]->cnc, axis,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_REMAIN_POS://剩余坐标
        {
            rc = siemenscnc_read_remain_pos( map[ioname]->cnc, axis,  ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_AXIS_NAME://轴名称
            rc = siemenscnc_read_axis_name( map[ioname]->cnc, axis, dest, &length );
            dest[0] = length >> 8;
            dest[1] = length & 0xFF;
            dest_length = length;
            break;
        case SIEMENSCNC_DRIVE_VOLTAGE://母线电压
        {
            rc = siemenscnc_read_driver_voltage( map[ioname]->cnc, axis, ( float* )&f_value );
        }
        break;
        case SIEMENSCNC_DRIVER_CURRENT://实际电流
        {
            rc = siemenscnc_read_driver_current( map[ioname]->cnc, axis, ( float* )&f_value );
        }
        break;
        case SIEMENSCNC_DRIVER_POWER://电机功率
        {
            rc = siemenscnc_read_driver_power( map[ioname]->cnc, axis, ( float* )&f_value );
        }
        break;
        case SIEMENSCNC_DRIVER_TEMPER://电机温度
        {
            rc = siemenscnc_read_driver_temper( map[ioname]->cnc, axis, ( float* )&f_value );
        }
        break;
        case SIEMENSCNC_CNC_NC_ALARM://NC报警
        {
            uint16_t s_value;
            uint32_t i_value;
            uint32_t temp = 0xFFFFFFFF;
            char data[128];
            int ptr = 0;
            int pos = 1;
            rc = siemenscnc_read_cnc_nc_alarm_no( ctx, &s_value ); //NC报警
            if( rc == SIEMENSCNC_RES_OK )
            {
                if( s_value > 0 )
                {
                    for( int i = 0; i < s_value; i++ )
                    {
                        rc = siemenscnc_read_cnc_nc_alarm( ctx, i, &i_value );
                        if( rc == SIEMENSCNC_RES_OK )
                        {
                            if( i_value != temp )
                            {
                                ptr++;
                                memset( data, 0, sizeof( data ) );
                                sprintf( data, "%d", i_value );
                                dest[pos++] = strlen( data ); //报警编号长度
                                memcpy( dest + pos, data, strlen( data ) ); //报警编号
                                pos += strlen( data );
                                dest[pos++] = 0; //内容长度置为0
                                dest[pos++] = 0; //内容长度置为0
                                temp = i_value;
                            }
                        }
                    }
                    if( pos > 1 )
                    {
                        rc = SIEMENSCNC_RES_OK;
                        dest[0] = ptr;//总的报警数量
                    }
                    else
                    {
                        dest[0] = 0;
                    }
                }
                else
                {
                    dest[0] = 0;
                }
            }
        }
        break;
        case SIEMENSCNC_R_VAR://R变量
        {
            rc = siemenscnc_read_r_var( map[ioname]->cnc, axis, ( double* )&d_value );
        }
        break;
        case SIEMENSCNC_S_R_PARAM://驱动器R参数
            rc = siemenscnc_read_s_r_param( map[ioname]->cnc, axis, address, sub_address, dest, &length );
            switch( bits )
            {
                case SIEMENSCNC_WORD_TYPE://字
                {
                    uint16_t s_value = siemenscnc_reverse_bytes_uint16( dest );
                    dest[0] = s_value >> 8;
                    dest[1] = s_value & 0xFF;
                }
                break;
                case SIEMENSCNC_DWORD_TYPE://双字
                {
                    uint16_t s_value = siemenscnc_reverse_bytes_uint32( dest );
                    dest[0] = s_value >> 24;
                    dest[1] = s_value >> 16;
                    dest[2] = s_value >> 8;
                    dest[3] = s_value & 0xFF;
                }
                break;
                case SIEMENSCNC_FLOAT_TYPE://浮点
                    f_value = siemenscnc_reverse_bytes_float( dest );
                    break;
                case SIEMENSCNC_DOUBLE_TYPE://双精度
                    d_value = siemenscnc_reverse_bytes_double( dest );
                    break;
                case SIEMENSCNC_STRING_TYPE://字符串
                    dest_length = length;
                    break;
            }

            break;
        case SIEMENSCNC_S_P_PARAM://驱动器P参数
            rc = siemenscnc_read_s_r_param( map[ioname]->cnc, axis, address, sub_address, dest, &length );
            switch( bits )
            {
                case SIEMENSCNC_WORD_TYPE://字
                {
                    uint16_t s_value = siemenscnc_reverse_bytes_uint16( dest );
                    dest[0] = s_value >> 8;
                    dest[1] = s_value & 0xFF;
                }
                break;
                case SIEMENSCNC_DWORD_TYPE://双字
                {
                    uint16_t s_value = siemenscnc_reverse_bytes_uint32( dest );
                    dest[0] = s_value >> 24;
                    dest[1] = s_value >> 16;
                    dest[2] = s_value >> 8;
                    dest[3] = s_value & 0xFF;
                }
                break;
                case SIEMENSCNC_FLOAT_TYPE://浮点
                    f_value = siemenscnc_reverse_bytes_float( dest );
                    break;
                case SIEMENSCNC_DOUBLE_TYPE://双精度
                    d_value = siemenscnc_reverse_bytes_double( dest );
                    break;
                case SIEMENSCNC_STRING_TYPE://字符串
                    dest_length = length;
                    break;
            }
            break;
    }
    if( bits == SIEMENSCNC_FLOAT_TYPE ) //浮点
    {
        f_value = siemenscnc_reverse_bytes_float( f_value );
        memcpy( dest, ( char* )&d_value, 4 );
    }
    else if( SIEMENSCNC_DOUBLE_TYPE ) //双精度
    {
        d_value = siemenscnc_reverse_bytes_double( d_value );
        memcpy( dest, ( char* )&d_value, 8 );
    }
    if( rc == SIEMENSCNC_RES_OK )
    {
        return 1;
    }
    else
    {
        map[ioname]->is_init = false;
        //断开连接
        siemenscnc_close( map[ioname]->cnc );
        return -1;
    }

}

/* 关闭连接 */
void siemenscnc_host::close_connect( QString ioname )
{
    if( map.contains( ioname ) )
    {
        if( map[ioname]->is_init == true )
        {
            siemenscnc_close( map[ioname]->cnc );
            map[ioname]->is_init = false;
        }
    }
}

void siemenscnc_host::_siemenscnc_swap_data( uint8_t *src, int number ) //交换顺序
{
    int i;
    uint8_t temp;
    for( i = 0; i < number / 2; i++ )
    {
        temp = src[number - 1 - i];
        src[number - 1 - i] = src[i];
        src[i] = temp;
    }
}

/*
 * 初始化
 */
siemenscnc_info_c* siemenscnc_host::_siemenscnc_init( const void *hardware_param, int timeout )
{
    siemenscnc_info_c* ctx = new siemenscnc_info_c;
    siemenscnc_net_para_t *net_param = ( siemenscnc_net_para_t* )hardware_param;
    int rc;
    ctx->cnc = siemenscnc_828d_new( net_param->ip, net_param->port );
    if( ctx->cnc == NULL )
    {
        delete ctx;
        return NULL;
    }
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout * 1000; //设置超时时间
    siemenscnc_set_response_timeout( ctx->cnc, &t );
    //siemenscnc_set_debug(ctx->cnc,1);
    rc = siemenscnc_connect( ctx->cnc );
    if( rc < 0 )
    {
        return ctx;
    }
    ctx->is_init = true;
    return ctx;
}

/*
 * 释放
 */
void siemenscnc_host::_siemenscnc_free( void )
{

}

/* 解析地址 */
void siemenscnc_host::_siemenscnc_decode_address( QString str, int &axis, int &address, int &sub_address )
{
    QStringList list;
    if( str.contains( ":" ) )
    {
        QString temp;
        list = str.split( ":" );
        //先解析:前的
        temp = list.at( 0 );
        axis = temp.toInt();
        //解析:后的
        temp = list.at( 1 );
        if( temp.contains( ( "." ) ) )
        {
            list.clear();
            list = temp.split( "." );
            address = list.at( 0 ).toInt();
            sub_address = list.at( 1 ).toInt();
        }
        else
        {
            address = temp.toInt();
        }
    }
    else if( str.contains( ( "." ) ) )
    {
        list = str.split( "." );
        address = list.at( 0 ).toInt();
        sub_address = list.at( 1 ).toInt();
        axis = address;
    }
    else
    {
        address = str.toInt();
        axis = address;
    }
}
