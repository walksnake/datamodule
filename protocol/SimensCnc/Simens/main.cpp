#include "iostream"
#include "./libsiemenscnc/inc/siemenscnc.h"
#include "./libsiemenscnc/828d/siemens_828d_new.h"

using namespace std;

int main(int argc, char *argv[])
{
#if 0
    uint8_t test[8];
    test[0] = 0x3F;
    test[1] = 0x7F;
    test[2] = 0xFF;
    test[3] = 0xFD;
    test[4] = 0x0;
    test[5] = 0x0;
    test[6] = 0x0;
    test[7] = 0x80;
    float ff = 0.9997;
    memcpy(test,(char*)&ff,4);
    cout<<ff;
    for(int i=0;i<4;i++)
    {
        qDebug("%X\r\n",test[i]);
    }
#endif
#if 0
    char hardware[6];
    int rc;
    float value;
    uint8_t dest[256]={0};
    int length;
    siemenscnc_host *host = new siemenscnc_host;
    {
        memset(dest,0,256);
        rc = host->read_param("192.168.0.199",
                              102,
                              "2-0",
                              _SIEMENSCNC_FEED_RATE,
                              SIEMENSCNC_DOUBLE_TYPE,
                              "1:37.2",
                              1,
                              500,
                              3,
                              5,
                              0,
                              dest,
                              length);


        for(int i = 0;i<8;i++)
        {
            qDebug("%X\r\n",dest[i]);
        }
        cout<<"返回值"<<rc;
    }
#endif

#if 1

    siemenscnc_t *ctx;
    ctx = siemenscnc_828d_new("192.168.2.99",102);
    //
    siemenscnc_set_debug(ctx,1);
    struct timeval t;
    int rc;
    t.tv_sec=0;
    t.tv_usec=1000000;   //设置超时时间为1000毫秒
    siemenscnc_set_error_recovery(ctx,SIEMENSCNC_ERROR_RECOVERY_LINK);
    siemenscnc_set_response_timeout(ctx,&t);
    rc = siemenscnc_connect(ctx);
    cout<<"connnect"<<rc;
    if(rc >= 0)
    {
        uint8_t dest[256] = {0};
        uint16_t length;
        uint16_t s_value[128]={0};
        uint32_t i_value;
        double d_value[10]={0.0};
        float f_value;
              // rc = siemenscnc_read_feed_rate(ctx, d_value);//进给倍率
              // cout<<"进给倍率"<<d_value[0]<<"返回值"<<rc;
              // rc = siemenscnc_read_spindle_rate(ctx, d_value);//主轴倍率
              //  cout<<"主轴倍率"<<d_value[0]<<"返回值"<<rc;
        // rc = siemenscnc_read_tool_h_length(ctx,dest);
        // cout<<dest[0]<<rc;
        // rc = siemenscnc_read_s_r_param(ctx, 1,46, 0, &f_value);//驱动器R参数
        //cout<<f_value<<"返回值"<<rc;
        //int time1= GetTickCount();
        /* while(1)
       {
           Sleep(1000);
       rc = siemenscnc_read_cnc_nc_alarm(ctx,dest); //报警 int16
        cout<<dest[0];
       }*/

       // rc = siemenscnc_read_g_coordinate_m(ctx,0,0,d_value);
       // cout<<"返回值"<<rc<<d_value[0];
        /*rc = siemenscnc_read_g_coordinate_t(ctx,0x08,0,d_value);
        cout<<"返回值"<<rc<<d_value[0];*/
#if 1
        // while(1)
        {
            //Sleep(100);
            //rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_Y, 37, 2, &f_value);//驱动器R参数
            // cout<<"功率3"<<f_value<<"返回值"<<rc;
            //rc = siemenscnc_read_driver_power(ctx,SIEMENSCNC_AXIS_Y,&f_value);//电机功率
            //cout<<"电机功率"<<f_value<<"返回值"<<rc;

            // siemenscnc_flush(ctx);
            // memset(dest,0,sizeof(dest));
            // rc = siemenscnc_read_serial_number(ctx, dest, &length);//硬件序列号
            //cout<<"硬件序列号"<<(char*)dest<<"返回值"<<rc;

            // siemenscnc_flush(ctx);
            // memset(dest,0,sizeof(dest));
            // rc = siemenscnc_read_cnc_type(ctx, dest, &length);//设备类型
            // cout<<"设备类型"<<(char*)dest<<"返回值"<<rc;

            //siemenscnc_flush(ctx);
            //memset(dest,0,sizeof(dest));
            //rc = siemenscnc_read_verison(ctx, dest, &length);//版本信息
            //cout<<"版本信息"<<(char*)dest<<"返回值"<<rc;
            // siemenscnc_flush(ctx);
            //memset(dest,0,sizeof(dest));
            //rc = siemenscnc_read_manufacture_data(ctx, dest, &length);//出厂日期
            //cout<<"出厂日期"<<(char*)dest<<"返回值"<<rc;
            //siemenscnc_flush(ctx);
            //memset(dest,0,sizeof(dest));

            // rc = siemenscnc_read_operate_mode(ctx, dest);//操作模式
            // cout<<"操作模式"<<dest[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);
            // rc = siemenscnc_read_status(ctx, dest);//运行状态
            //cout<<"运行状态"<<dest[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);

             //rc = siemenscnc_read_process_number(ctx, d_value);//加工数量
             //cout<<"加工数量"<<d_value[0]<<"返回值"<<rc;
            // siemenscnc_flush(ctx);
           // rc = siemenscnc_read_process_set_number(ctx, d_value);//设定的加工数量
           // cout<<"加工设定数量"<<d_value[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);

            // rc = siemenscnc_read_feed_set_speed(ctx, d_value);//设定进给速度
            // cout<<"设定进给速度"<<d_value[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);
            // rc = siemenscnc_read_feed_act_speed(ctx, d_value);//实际进给速度
            //cout<<"实际进给速度"<<d_value[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);

            //rc = siemenscnc_read_feed_rate(ctx, d_value);//进给倍率
            // cout<<"进给倍率"<<d_value[0]<<"返回值"<<rc;
            //siemenscnc_flush(ctx);

            /*rc = siemenscnc_read_spindle_set_speed(ctx, d_value);//主轴设定速度
            cout<<"主轴设定速度"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_spindle_act_speed(ctx, d_value);//主轴实际速度
            cout<<"主轴实际速度"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);*/

            // rc = siemenscnc_read_spindle_rate(ctx, d_value);//主轴倍率
            // cout<<"主轴倍率"<<d_value[0]<<"返回值"<<rc;
            // siemenscnc_flush(ctx);

            // rc = siemenscnc_read_run_time(ctx, d_value);//循环时间,一次自动运行起动时间的累计值
           // cout<<"运行时间"<<d_value[0]<<"返回值"<<rc;
           // siemenscnc_flush(ctx);
           // rc = siemenscnc_read_remain_time(ctx, d_value);//剩余时间
           // cout<<"剩余时间"<<d_value[0]<<"返回值"<<rc;
           // siemenscnc_flush(ctx);
           // memset(dest,0,sizeof(dest));
           // rc = siemenscnc_read_program_name(ctx, dest, &length);//加工程序名
           // cout<<"加工程序名"<<(char*)dest<<"返回值"<<rc;

            // siemenscnc_flush(ctx);
            // memset(dest,0,sizeof(dest));
            /*rc = siemenscnc_read_tool_cur_name(ctx,  dest, &length);//刀具号
             if(rc == 0)
             cout<<length<<"当前刀具名"<<QByteArray((char*)dest,length)<<"\r\n";
             rc = siemenscnc_read_tool_cur_t_no(ctx, d_value);//刀具刀沿号
             cout<<"当前刀具号"<<d_value[0]<<"返回值"<<rc;
             rc = siemenscnc_read_tool_cur_d_no(ctx, d_value);//刀具刀沿号
             cout<<"当前刀具刀沿号"<<d_value[0]<<"返回值"<<rc;
             //siemenscnc_flush(ctx);
             rc = siemenscnc_read_tool_cur_h_no(ctx, d_value);//刀具H号
             cout<<"当前刀具H号"<<d_value[0]<<"返回值"<<rc;


            //rc = siemenscnc_read_tool_type(ctx, 1, 5, d_value);//刀具类型
           // cout<<"刀具类型"<< d_value[0];
            rc = siemenscnc_read_tool_length(ctx,1, 1, d_value);//刀具长度
            cout<<"返回值"<<rc<<"刀具长度"<< d_value[0];
            rc = siemenscnc_read_tool_radius(ctx,1, 1, d_value);//刀具半径
            cout<<"返回值"<<rc<<"刀具半径"<< d_value[0];
            //rc = siemenscnc_read_tool_edge_no(ctx, 1, d_value);//刀沿号
            rc = siemenscnc_read_tool_edge(ctx,1, 1, d_value);//刀沿位置
            cout<<"返回值"<<rc<<"刀具位置"<< d_value[0];
            //siemenscnc_flush(ctx);
            rc = siemenscnc_read_tool_tip(ctx,1, 1, d_value);//刀具齿数
            cout<<"返回值"<<rc<<"刀具齿数"<< d_value[0];
            rc = siemenscnc_read_tool_h_no(ctx,1, 1, d_value);
            cout<<"返回值"<<rc<<"刀具H号"<< d_value[0];
*/

           /* while(1)
            {
                Sleep(100);
                rc = siemenscnc_read_tool_type(ctx, 1, d_value);//刀具类型
                cout<<"返回值"<<rc<<"刀具类型"<< d_value[0];
            }*/

           /* rc = siemenscnc_read_tool_wear_length(ctx,1,1,d_value);
            if(rc == 0)
            {
                cout<<"长度磨损"<<d_value[0];
            }
            rc = siemenscnc_read_tool_wear_radius(ctx,1,1,d_value);
            if(rc == 0)
            {
                cout<<"半径磨损"<<d_value[0];
            }*/
            // siemenscnc_flush(ctx);
            /*rc = siemenscnc_read_tool_x_offset(ctx, d_value);//长度补偿X
            cout<<"长度补偿X"<<d_value[0]<<d_value[1]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
             rc = siemenscnc_read_tool_z_offset(ctx, d_value);//长度补偿Z
             cout<<"长度补偿Z"<<d_value[0]<<"返回值"<<rc;
             siemenscnc_flush(ctx);*/

            /* siemenscnc_flush(ctx);
            rc = siemenscnc_read_machine_pos(ctx,0, d_value);//机械坐标
            cout<<"机械坐标1"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_relatively_pos(ctx,0, d_value);//工件坐标 相对坐标
            cout<<"相对坐标1"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_remain_pos(ctx,0, d_value);//剩余坐标
            cout<<"剩余坐标1"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);

            rc = siemenscnc_read_axis_name(ctx,0,dest, &length);//轴名称
            cout<<"轴名称"<<(char*)dest<<"返回值"<<rc;
            siemenscnc_flush(ctx);*/
#if 0
            while(1)
            {
                Sleep(1000);
            rc = siemenscnc_read_driver_voltage(ctx,1,&f_value);//母线电压
            //f_value = siemenscnc_swaped_float(f_value);
            cout<<"母线电压"<<f_value<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_driver_current(ctx,1,&f_value);//实际电流
            cout<<"实际电流"<<f_value<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_driver_power(ctx,1,&f_value);//电机功率
            cout<<"电机功率"<<f_value<<"返回值"<<rc;
            rc = siemenscnc_read_driver_temper(ctx,1,&f_value);//电机温度
            cout<<"电机温度"<<f_value<<"返回值"<<rc;
            }
#endif
#if 0
            rc = siemenscnc_read_cnc_nc_alarm_no(ctx, s_value);//NC报警
            if(rc == SIEMENSCNC_RES_OK)
            {
                cout<<s_value[0];
                for(int i=0;i<s_value[0];i++)
                {
                    rc = siemenscnc_read_cnc_nc_alarm(ctx,i,&i_value);
                    if(rc == SIEMENSCNC_RES_OK)
                    {
                        cout<<i_value;
                    }
                }
            }
#endif
            /*for(int i = 0;i<128;i++)
            {
                cout<<"NC报警"<<s_value[i]<<"返回值"<<rc;
                if(s_value[i] == 0)
                {
                    break;
                }
            }*/
           /* uint16_t alarm_no;
            rc = siemenscnc_read_cnc_nc_alarm_no(ctx, &alarm_no);//NC报警
            cout<<"NC报警"<<alarm_no<<"返回值"<<rc;
            for(int i = 0; i < alarm_no;i++)
            {
                rc = siemenscnc_read_cnc_nc_alarm(ctx,i, &i_value);//NC报警
                cout<<"NC报警"<<i_value<<"返回值"<<rc;
            }*/

#if 1
            //siemenscnc_flush(ctx);
            //rc = siemenscnc_read_r_var(ctx, 0, d_value);//R变量
            //cout<<"R1"<<d_value[0]<<"返回值"<<rc;
           // rc = siemenscnc_read_r_var(ctx, 1, d_value);//R变量
           // cout<<"R1"<<d_value[0]<<"返回值"<<rc;
            //rc = siemenscnc_read_r_var(ctx, 1, d_value);//R变量
           // cout<<"R1"<<d_value[0]<<"返回值"<<rc;
           // rc = siemenscnc_read_r_var(ctx, 1, d_value);//R变量
           // cout<<"R1"<<d_value[0]<<"返回值"<<rc;
           rc = siemenscnc_read_s_r_p_param(ctx, 0, 37, 2, dest,&length);//驱动器R参数
           cout<<"驱动器r37[2]"<<siemenscnc_get_float(dest)<<"返回值"<<rc;
            //            rc = siemenscnc_read_s_p_param(ctx, SIEMENSCNC_AXIS_SPINDLE, 115, 0, &f_value);//驱动器P参数
            //            cout<<"驱动器p115[0]"<<f_value<<"返回值"<<rc;
#endif
           /* rc = siemenscnc_read_axis_machine_param(ctx,2,33120,0,dest,&length);
            if(rc == 0)
            {
                cout<<"rc"<<rc<<siemenscnc_get_double(dest);
            }*/

           /* rc = siemenscnc_read_general_machine_param(ctx,13123232322,0,dest,&length);
            if(rc == 0)
            {
                cout<<"rc"<<rc<<222<<siemenscnc_get_double(dest);
            }
             rc = siemenscnc_read_channel_machine_param(ctx,20212,0,dest,&length);
            if(rc == 0)
            {
                cout<<"rc"<<rc<<siemenscnc_get_double(dest);
            }
            rc = siemenscnc_read_axis_machine_param(ctx,30330,0,dest,&length);
            if(rc == 0)
            {
                cout<<"rc"<<rc<<siemenscnc_get_double(dest);
            }*/
            rc = siemenscnc_read_control_unit_param(ctx,115,0,dest,&length);
            cout<<"115"<<"rc"<<rc<<siemenscnc_get_float(dest)<<siemenscnc_get_32(dest);
            rc = siemenscnc_read_power_unit_param(ctx,32,0,dest,&length);
            cout<<"32"<<"rc"<<rc<<siemenscnc_get_float(dest)<<siemenscnc_get_32(dest);
            /*int a1 = 0x01;
            bool ok = false;
            while(!ok)
            {
                Sleep(1);
                for(int i = 0;i<255;i++)
                {
                    rc = siemenscnc_read_control_unit_param(ctx,a1,i,799,0,dest,&length);
                    if(rc == 0)
                    {
                        if(siemenscnc_get_double(dest) == 4000 || siemenscnc_get_float(dest) == 4000){
                            ok = true;
                            cout<<"rc"<<rc<<siemenscnc_get_double(dest);
                            break;
                        }
                    }
                }
                if(a1 == 0xF1)
                {
                    break;
                }
                a1 += 0x10;

            }*/
            /*rc = siemenscnc_read_tool_number(ctx,  &i_value);//刀具号
            cout<<"刀具号"<<i_value<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_tool_d_radius(ctx, d_value);//刀具半径D
            cout<<"刀具半径D"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_tool_h_length(ctx, d_value);//刀具长度
            cout<<"刀具长度"<<d_value[0]<<"返回值"<<rc;
            siemenscnc_flush(ctx);
            rc = siemenscnc_read_tool_x_length(ctx, d_value);//长度补偿X
            cout<<"长度补偿X"<<d_value[0]<<d_value[1]<<"返回值"<<rc;*/
            //siemenscnc_flush(ctx);
            // rc = siemenscnc_read_tool_z_length(ctx, d_value);//长度补偿Z
            // cout<<"长度补偿Z"<<d_value[0]<<"返回值"<<rc;
            // siemenscnc_flush(ctx);
            //rc = siemenscnc_read_tool_wear_radius(ctx, d_value);//磨损半径
            // cout<<"磨损半径"<<d_value[0]<<"返回值"<<rc;
            // siemenscnc_flush(ctx);
            // rc = siemenscnc_read_tool_edge(ctx, d_value);//刀沿位置
            // cout<<"刀沿位置"<<d_value[0]<<"返回值"<<rc;

            /* rc = siemenscnc_read_s_r_param(ctx, 1, 462, 0, dest,&length);//驱动器R参数
             cout<<rc;
             rc = siemenscnc_read_s_r_param(ctx, 1, 462, 1, dest,&length);//驱动器R参数
             cout<<rc;*/
            //rc = siemenscnc_read_axis_name(ctx,SIEMENSCNC_AXIS_Z,1,dest, &length);//轴名称
            //cout<<"轴名称"<<(char*)dest<<"返回值"<<rc;
            //rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_SPINDLE, 30, 0, &f_value);//驱动器R参数
            //cout<<"电流"<<f_value<<"返回值"<<rc;

            /*rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_SPINDLE, 32, 0, &f_value);//驱动器R参数
            cout<<"功率1"<<f_value<<"返回值"<<rc;
            rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_X, 32, 0, &f_value);//驱动器R参数
            cout<<"功率2"<<f_value<<"返回值"<<rc;
            rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_Y, 32, 0, &f_value);//驱动器R参数
            cout<<"功率3"<<f_value<<"返回值"<<rc;
            rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_Z, 32, 0, &f_value);//驱动器R参数
            cout<<"功率4"<<f_value<<"返回值"<<rc;
            rc = siemenscnc_read_s_r_param(ctx, SIEMENSCNC_AXIS_A, 32, 0, &f_value);//驱动器R参数
            cout<<"功率5  "<<f_value<<"返回值"<<rc;
            cout<<"\r\n";*/

        }
        //int time2 = GetTickCount();
        //cout<<"经过的时间"<<time2-time1<<time1<<time2;
#endif
    }
#endif

}
