# Fanuc应用层协议介绍



       enum{
            /* 系统相关 */
            //SIEMENSCNC_FUNC_SERIAL_NUMBER=0,//硬件序列号 string
           // SIEMENSCNC_FUNC_CNC_TYPE=1,//机床类型  string
           // SIEMENSCNC_FUNC_VERSION=2,//版本信息 string
            //SIEMENSCNC_FUNC_MANUFACTURE_DATE=3,//出厂日期 string 
            SIEMENSCNC_FUNC_PROCESS_NUMBER=4,//加工数量 double
            SIEMENSCNC_FUNC_PROCESS_SET_NUMBER=5,//设定的加工数量 double
            SIEMENSCNC_FUNC_RUN_TIME=6,//循环时间,一次自动运行起动时间的累计值 double
            SIEMENSCNC_FUNC_REMAIN_TIME=7,//剩余时间（剩余运行时间） double
            SIEMENSCNC_FUNC_PROGRAM_NAME=8,//加工程序名 string
            /* 运行状态 */ 
            SIEMENSCNC_FUNC_OPERATE_MODE=10,//操作模式 byte 含义见下方
            enum
            {
                SIEMENSCNC_MODE_OTHER=0,
                SIEMENSCNC_MODE_JOG,//点动 手动
                SIEMENSCNC_MODE_MDA,//半自动
                SIEMENSCNC_MODE_AUTO,//自动
                SIEMENSCNC_MODE_REFPOS,//参考点
                SIEMENSCNC_MODE_REF_POINT//零点
            };
            SIEMENSCNC_FUNC_STATUS=11,//运行状态 byte 含义见下方
            enum
            {
                _SIEMENSCNC_STATUS_OTHER=0,//空闲
                _SIEMENSCNC_STATUS_RESET,//复位 SPINDLE CW STOP
                _SIEMENSCNC_STATUS_STOP,//停止 (SPINDLE CW 主轴正反转)
                _SIEMENSCNC_STATUS_START,//启动 SPINDLE CW
                _SIEMENSCNC_STATUS_SPENDLE_CW_CCW// SPINDLE CCW(主轴反转)
            };
            /* 轴相关 */
            SIEMENSCNC_FUNC_AXIS_NAME=20,//轴名称 string
            SIEMENSCNC_FUNC_MACHINE_POS=21,//机械坐标 double 指定轴 从0开始 regname=x
            SIEMENSCNC_FUNC_RELATIVELY_POS=22,//工件坐标 相对坐标 同上
            SIEMENSCNC_FUNC_REMAIN_POS=23,//剩余坐标 同上
            SIEMENSCNC_FUNC_FEED_SET_SPEED=24,//设定进给速度 double
            SIEMENSCNC_FUNC_FEED_ACT_SPEED=25,//实际进给速度 double
            SIEMENSCNC_FUNC_FEED_RATE=26,//进给倍率 double
            SIEMENSCNC_FUNC_SPINDLE_SET_SPEED=27,//主轴设定速度 double
            SIEMENSCNC_FUNC_SPINDLE_ACT_SPEED=28,//主轴实际速度 double
            SIEMENSCNC_FUNC_SPINDLE_RATE=29,//主轴倍率 double
            SIEMENSCNC_FUNC_DRIVE_VOLTAGE=30,//母线电压 float 指定驱动器 从0开始
            SIEMENSCNC_FUNC_DRIVER_CURRENT=31,//实际电流 同上
            SIEMENSCNC_FUNC_DRIVER_POWER=32,//电机功率  同上
            SIEMENSCNC_FUNC_DRIVER_TEMPER=33,//电机温度 同上
           /* 刀具相关 */
            SIEMENSCNC_FUNC_TOOL_CUR_NAME=40,//当前刀具名 string
            SIEMENSCNC_FUNC_TOOL_CUR_T_NO=41,//当前刀具号  double
            SIEMENSCNC_FUNC_TOOL_CUR_D_NO=42,//当前刀具刀沿号 double
            SIEMENSCNC_FUNC_TOOL_CUR_H_NO=43,//当前刀具H号 double
            SIEMENSCNC_FUNC_TOOL_TYPE=44,//刀具类型  double regname格式a.b a表示刀沿号，b表示刀具列表中序号 
            SIEMENSCNC_FUNC_TOOL_LENGTH=45,//刀具长度 同上
            SIEMENSCNC_FUNC_TOOL_RADIUS=46,//刀具半径 同上
            SIEMENSCNC_FUNC_TOOL_EDGE=47,//刀具刀沿位置 同上
            SIEMENSCNC_FUNC_TOOL_TIP=48,//刀具齿数 同上
            SIEMENSCNC_FUNC_TOOL_WEAR_LENGTH=49,//刀具长度磨损 double regname格式a.b a表示刀沿号，b表示刀具磨损列表中序号 
            SIEMENSCNC_FUNC_TOOL_WEAR_RADIUS=50,//刀具半径磨损 同上
            SIEMENSCNC_FUNC_TOOL_X_OFFSET=51,//X补偿 double
            SIEMENSCNC_FUNC_TOOL_Z_OFFSET=52,//Z补偿 double
    
            /* 报警相关 */
            SIEMENSCNC_FUNC_CNC_NC_ALARM=60,//NC报警 string
            /* 参数相关 */
            SIEMENSCNC_FUNC_R_VAR=70,//R变量 double regname a
            SIEMENSCNC_FUNC_S_R_P_PARAM=71,//驱动器R P参数 double 需指定驱动器和地址，格式为a:b.c
            SIEMENSCNC_FUNC_GENERAL_MACHINE_PARAM=72,//通用机床数据 double 需指定地址regname
            a.b
            SIEMENSCNC_FUNC_CHANNEL_MACHINE_PARAM=73,//通道机床数据 同上
            SIEMENSCNC_FUNC_AXIS_MACHINE_PARAM=74,//轴机床数据 同上
            SIEMENSCNC_FUNC_CONTROL_UNIT_PARAM=75//控制单元参数 同上
    };