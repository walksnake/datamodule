# 三菱CNC应用层协议封装

### WEB端配置界面要求

三菱CNC端口号为683和64758，683是对于700系列，64758是对于C64系列，建议web上仅可选择这两个端口号。

功能：

    enum{
            /* 系统相关 */
            EZSOCKET_VERSION=0,//版本信息    字符串
            EZSOCKET_RUN_TIME=1,//运行时间   双字 单位秒
            EZSOCKET_CYCLE_TIME=2,//循环时间 双字 单位秒
            EZSOCKET_START_TIME=3,//开始时间 双字 单位秒
            EZSOCKET_ESTIMATE_TIME=4,//预计的时间 双字 单位秒 reg_name 为0/1，
              0 Counted when PLC device Y234 is ON (Y344 for M6x5M; Y704 for CNC700)
              1 Counted when PLC device Y235 is ON (Y345 for M6x5M; Y705 for CNC700)
            EZSOCKET_KEEP_ALIVE_TIME=5,//上电时间 双字 单位秒
            /* 状态相关 */
            EZSOCKET_CUTTING_MODEL=10,//切削模式 字节 1为切削 0为未切削
            EZSOCKET_RUN_STATUS=11,//运行状态  字节  bit0  0 未执行刀具长度测量 1 执行刀具长度测量
                                                   bit1  0 不处于自动操作 1 处于自动操作
                                                   bit2  0 未执行自动操作启动 1 执行自动操作启动
            EZSOCKET_EMG_STATUS=12,//急停状态 字 0xFFFF为未急停 否则为急停 
           									     * 0有效
     											 * bit0 内置PLC停止
                                                 * bit1 外部PLC FROM TO 命令未执行
                                                 * bit2 外部PLC准备未完成
                                                 * bit3 外部PLC通信异常
                                                 * bit4 控制装置EMG连接器紧急停止状态
                                                 * bit5 电源供给外部紧急停止状态
                                                 * bit6 内置PLC S/W紧急停止输出YC2C未1
                                                 * bit7 NULL
                                                 * bit8 接触器阻断测试
                                                 * bit9 LINE
                                                 * bitA null
                                                 * bitB 用户PLC有错误代码
                                                 * bitC plc高速处理异常
                                                 * bitD 门互锁 dog/OT任意布局 装置错误
                                                 * bitE 主轴驱动单元 紧急停止输出
                                                 * bitF 伺服驱动单元 紧急停止输出
            /* 加工相关 */
            EZSOCKET_PROCESS_NUMBER=20,//加工数量 双字 
            EZSOCKET_PROGRAM_NO=21,//程序号   字符串
            //EZSOCKET_SEQUENCE_NUM=22,//执行的语句序号 双字
            EZSOCKET_PROGRAM_BLOCK=23,//当前执行的内容 字符串
            /* 轴相关 */
            EZSOCKET_AXIX_NUMBER=30,//轴数量 字节
            EZSOCKET_CURRENT_POSITION=31,//当前坐标 double reg_name范围0-15 
            EZSOCKET_WORK_POSITION=32,//工作坐标 double reg_name范围0-15 
            EZSOCKET_DISTANCE_POSITION=33,//剩余坐标 double reg_name范围0-15 
            EZSOCKET_MACHINE_POSTION=34,//机械坐标 vdouble reg_name范围0-15 
            EZSOCKET_SPINDLE_SET_SPEED=35,//主轴设定速度 双字
            EZSOCKET_SPINDLE_ACTUAL_SPEED=36,//主轴实际速度 双字
            EZSOCKET_FEED_SET_SPEED=37,//进给设定速度 double
            EZSOCKET_FEED_ACTUAL_SPEED=38,//进给实际速度 double
            EZSOCKET_SPINDLE_LOAD=39,//主轴负载  双字
            EZSOCKET_SERVO_LOAD=40,//伺服轴负载 双字 reg_name范围0-15
            EZSOCKET_SERVO_SPEED=41,//伺服轴转速  双字  reg_name范围0-15
            EZSOCKET_FEED_RATE=42,//进给倍率    字节
            EZSOCKET_SPINDLE_RATE=43,//主轴倍率 字节
            EZSOCKET_FAST_MOV_RATE=44,//快速移动倍率 字节
            /* 刀具相关 */
            EZSOCKET_MAG_TOOL_NO=50,//刀库刀号 双字
            EZSOCKET_SPD_TOOL_NO=51,//主轴刀具号 双字 
            EZSOCKET_TOOLOFFSET_D_SHAPE=52,//刀偏移D SHAPE double
            EZSOCKET_TOOLOFFSET_D_WEAR=53,//刀偏移D WEAR double
            EZSOCKET_TOOLOFFSET_H=54,//刀偏H   double reg_name范围0-15
            EZSOCKET_TOOL_SET_SIZE=55,//刀具数量 双字
            EZSOCKET_TOOL_OFFSET_TYPE=56,//刀具补偿得类型 双字
            EZSOCKET_TOOL_OFFSET_VALUE=57,//刀具补偿值 double reg_name 格式
                         xxxx:xxx.x 
                         其中xxxx表示 刀具补偿的类型 范围为0-2 
                                                0 M SYTEM TYPE-I
       						                    1 M SYSTEM TYPE-II
      										    2 L SYSTEM TYPE
      	                XXX表示刀具补偿值得类别  范围如下：
      	                 M system type-I   0：Tool offset value
                         M system type-II  0: Tool length offset value(Dimension)
                                           1: Ditto (Wear offset value)
                                           2: Tool diameter offset value(Dimension)
                                           3: Ditto (Wear offset value)
                         L system type     0: Tooltip wear amount X
                                           1: Ditto Z
                                           2: Ditto C (Y*)
                                           3: Tool length X
                                           4: Ditto Z
                                           5: Ditto C (Y*)
                                           6: Tooltip radius R
                                           7: Tooltip radius wear amount r
                                           8: Hypothetical tooltip # P
                        x表示刀具号
                /* 模态相关 */
                EZSOCKET_M_COMMAND=60,//M COMMAND 双字 reg_name范围为1-4 
                EZSOCKET_S_COMMAND=61,//S COMMAND 双字
                EZSOCKET_T_COMMAND=62,//T COMMAND 双字
                EZSOCKET_B_COMMAND=63,//B COMMAND 双字
                /* 报警相关 */
                EZSOCKET_ALARM_MSG=70,//报警信息 字符串
                /* PLC 相关 */
                EZSOCKET_PLC_X=80,//PLC X PLC的输入信号 字节 reg_name为XX.x XX主地址16进制,x表示取其得位
                EZSOCKET_PLC_Y=81,//PLC Y PLC的输出信号 字节 reg_name为XX.x XX主地址16进制,x表示取其得位
                EZSOCKET_PLC_R=82,//PLC R PLC的文件寄存器 字 reg_name为XX 10进制
                EZSOCKET_PLC_F=83,//PLC F PLC的临时存储区 字节 存储报警信息 reg_name为XX.x XX主地址10进制,x表示取其得位
                EZSOCKET_PLC_D=84,//PLC D PLC的数据库存储区 字 reg_name为XX 10进制
                EZSOCKET_G_CODE=85,//G代码 double regname范围为1-21
                G CODE 共21组 1-21
         * 1 Group 1 (Interpolation mode). G00, G01 G02, G03, and G33 command modal.
         * 2 Group 2 (Plane selection). G17, G18, and G19 command modal.
         * 3 Group3 (Absolute) G90 and (Incremental) G91 command modal.
         * 4 Group 4 (Chuck barrier) G22 and G23 command modal.
         * 5 Group 5 (Feed mode) G94 and G95 command modal.
         * 6 Group 6 (Inch) G20 and (mm) G21 command modal.
         * 7 Group 7 (Tool nose R compensation mode) G40, G41, and G42 command modal.
         * 8 Group 8 (Tool length compensation mode) G43, G44 and G49 command modal.
         * 9 Group 9 (Fixed cycle mode) G70, G71, G72, G73, G74, G75, G76, G77, G78, G79, G80,
          G81, G82, G83, G84, G85, G86, G87, G88, and G89 command modal.
         * 10 Group 10 (Initial return) G98, (Reference point return) G99 command modal.
         * 11 Group 11 G50.2 and G51.2 command modal. (Invalid with M6x5L and C64)
         * 12 Group 12 (Workpiece coordinate system modal) G54, G55, G56, G57, G58, and G59
        command modal.
         * 13 Group 13 (Cutting mode) G61, G62, G63, and G64 command modal.
         * 14 Group 14 (Modal call) G66, G66.1, and G67 command modal.
         * 15 Group 15 (Normal line control) G40.1, G41.1 and G42.1 command modal. (Valid only with
            M6x5M or CNC700M)(Mirror image for facing tool posts) G68 and G69 modal (For C64L)
         * 16 Group 16 (Coordinate system rotation) G68 and G69 (Valid only with CNC700M)
         * 17 Group 17 (Constant peripheral speed control) G96 and G97 command modal.
         * 18 Group 18 (Balance cut/polar coordinate command) G14, G15 and G16 command modal.
         * 19 Group 19 (G command mirror image) G50.1 and G51.1 command modal.
         * 20 Group 20 (Spindle selection) G43.1, G44.1 and G47.1 command modal. (Invalid with
           M6x5M/L)
         * 21 Group 21(Cylindrical interpolation/polar coordinate interpolation) G07.1, G107, G12.1,
           G112, G13.1, G113 (Valid only with CNC700M)
                /* 参数 */
                //EZSOCKET_PARAM_READ=90,//参数读取 
                /* 宏变量 */
                EZSOCKET_READ_MACRO_VAR=95,//读取宏变量 double regname 范围为100-199 599-999


