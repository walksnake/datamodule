# libezsocket 库介绍
libezsocket库用于跟三菱得CNC进行通信，目前支持三菱M70,E70,M700等带网口的CNC，端口号为683和64758，根据实际使用的时候来确定。64758 is set for C64 and 683 is set for CNC700,

该库是基于官方库抓包抓出来的报文进行的解析。抓包工具见C:\Program Files (x86)\EZSocket\EZSocketNc\tools中的EZNcTest.exe这个工具，需要安装，安装包会单独提供，配合ezsocket官方协议接口参考"ezsocket协议"

## 支持功能

libezsocket支持与CNC的网口进行通信，能够读取到CNC的大部分数据。

## 安装库



## 库的用法

测试例程中包含库的基本用法，更多的使用方式可见API接口

## 跨平台支持

libezsocket支持多平台，包括windows，linuxs，openBSD等平台，且32位和64位系统均支持。

## 协议报文说明

具体的协议报文参考代码中ezsocket_new.c中的command，

```
以读取PLC的X输入点报文举例
0x47, 0x49, 0x4f, 报头
0x50, 报文长度
0x01, 0x00, 0x01, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0x21, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x47, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0xdc, 0x88, 0x03, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 
中间报文
0xFF, 0xFF, 0xFF, 0xFF, 要读取的地址的地址
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
#Xc18,c18转10进制为3096,3096+1=3097,小端序转4byte为190c0000 输入点X 从第60个字节开始连续4个字节  小端序
```

## 结构体说明

```
typedef struct _ezsocket_backend {
    unsigned int header_length;//帧头长度
    unsigned int max_adu_length;//数据区最大长度
    int (*build_request_basis) (ezsocket_t *ctx,
    							int function,
                                int address,
                                int nb,
                                int src_nb,
                                uint8_t *req, 
                                const uint8_t *src);//生成请求指令
    int (*receive_msg)  (ezsocket_t *ctx, uint8_t *msg, uint16_t *length);//接受数据
    int (*decode_msg)(ezsocket_t *ctx,uint8_t function,const uint8_t *msg,uint8_t *dest,uint16_t *length);//解析数据
    int (*send) (ezsocket_t *ctx, const uint8_t *req, int req_length);//发送数据
    int (*recv) (ezsocket_t *ctx, uint8_t *rsp, int rsp_length);//接受数据
    int (*connect) (ezsocket_t *ctx);//连接
    void (*close) (ezsocket_t *ctx);//关闭
    int (*flush) (ezsocket_t *ctx);//清空缓存
    int (*select) (ezsocket_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);//阻塞直到有数据
} ezsocket_backend_t;
ezsocket_backend_t为协议通用模块，具体解释如下:
1、unsigned int header_length;//帧头长度
2、unsigned int max_adu_length;//数据区最大长度
3、 int (*build_request_basis) (ezsocket_t *ctx, int function,unsigned short area, int addr,
                                int nb, int src_nb, uint8_t *req, const uint8_t *src);生成请求指令 其中function为要执行的功能，address为要操作的地址，nb为地址的数量，src_nb为最后src的数据长度，req为生成的指令存放的缓存，src为要写入的数据
4、int (*receive_msg)  (ezsocket_t *ctx, uint8_t *msg, uint16_t *length);//从PLC端接受数据会调用recv
5、int (*decode_msg)   (ezsocket_t *ctx, uint8_t *src, uint8_t *dest, uint16_t length);//解析从PLC端接收的数据
6、int (*send) (ezsocket_t *ctx, const uint8_t *req, int req_length);//发送数据到PLC
7、int (*recv) (ezsocket_t *ctx, uint8_t *rsp, int rsp_length);//从PLC读数据
8、int (*connect) (ezsocket_t *ctx);//连接PLC
9、void (*close) (ezsocket_t *ctx);//关闭链接
10、int (*flush) (ezsocket_t *ctx);//清空数据流
11、int (*select) (ezsocket_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);//阻塞直到有数据返回

struct _ezsocket {
    int s;/* Socket 描述符 */
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const ezsocket_backend_t *backend;
    void *backend_data;
};
struct _ezsocket即为ezsocket_t,是libezsocket中的操作的句柄。
void *backend_data;保存的是硬件参数，此处为网口参数，包括ip和端口号
const ezsocket_backend_t *backend;为ezsocket协议处理接口，包括生成报文。解析报文等。
typedef enum
{
    EZSOCKET_ERROR_RECOVERY_NONE          = 0,
    EZSOCKET_ERROR_RECOVERY_LINK          = (1<<1),//自动重连
    EZSOCKET_ERROR_RECOVERY_PROTOCOL      = (1<<2),//延时并冲洗已经有的数据
} ezsocket_error_recovery_mode;
ezsocket_error_recovery_mode为遇到错误时候的处理。

```

## API接口说明

```
1、int ezsocket_connect(ezsocket_t *ctx);//建立连接
2、void ezsocket_close(ezsocket_t *ctx);//关闭连接
3、void ezsocket_set_socket(ezsocket_t *ctx, int socket);//设置socket
4、int ezsocket_get_socket(ezsocket_t *ctx);//获取socket
5、int ezsocket_set_error_recovery(ezsocket_t *ctx, ezsocket_error_recovery_mode error_recovery);//设置错误处理
6、void ezsocket_get_response_timeout(ezsocket_t *ctx, struct timeval *timeout);//获取响应超时时间
7、void ezsocket_set_response_timeout(ezsocket_t *ctx, const struct timeval *timeout);//设置响应超时时间
8、void ezsocket_get_byte_timeout(ezsocket_t *ctx, struct timeval *timeout);//获取字节超时时间
9、void ezsocket_set_byte_timeout(ezsocket_t *ctx, const struct timeval *timeout);//设置字节超时时间
10、int ezsocket_get_header_length(ezsocket_t *ctx);//获取帧头长度
11、void ezsocket_set_debug(ezsocket_t *ctx, int boolean);//设置调试开关
12、void ezsocket_free(ezsocket_t *ctx);//释放句柄
13、int ezsocket_flush(ezsocket_t *ctx);//清空数据流
14、uint16_t ezsocket_swaped_uint16(uint16_t value);//word大小端转换
15、uint32_t ezsocket_swaped_uint32(uint32_t value);//dword大小端转换
16、uint64_t ezsocket_swaped_uint64(uint64_t value);//uint64_t 大小端转换
17、float ezsocket_swaped_float(float value);//float大小端转换
18、double ezsocket_swaped_double(double value);//double 大小端转换
```

## API接口之读写接口

```
/* 系统相关 */
1、int ezsocket_read_version(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//版本信息 对应官方协议栈getversion
//int ezsocket_read_system_date(ezsocket_t *ctx, uint8_t *dst);//系统日期
//int ezsocket_read_system_time(ezsocket_t *ctx, uint8_t *dst);//系统时间
2、int ezsocket_read_run_time(ezsocket_t *ctx, int *time);//运行时间 对应官方协议栈getruntime
3、int ezsocket_read_cycle_time(ezsocket_t *ctx, int *time);//循环时间
4、int ezsocket_read_start_time(ezsocket_t *ctx, int *time);//开始时间 对应官方协议栈getstarttime
5、int ezsocket_read_estimate_time(ezsocket_t *ctx, int external_runout_type, int *time);//外部经过的时间 对应官方协议栈getestimatetime
6、int ezsocket_read_keep_alive_time(ezsocket_t *ctx, int *time);//上电时间 getalivetime

/* 状态相关 */
7、int ezsocket_read_cutting_model(ezsocket_t *ctx, uint8_t *dst);//是否处在切削模式  对应官方协议栈getcuttingmode
8、int ezsocket_read_run_status(ezsocket_t *ctx, uint8_t *dst);//运行状态 对应官方协议栈getrunstatus
9、int ezsocket_read_emg_status(ezsocket_t *ctx, uint8_t *dst);//急停状态 读PLC参数R69
/* 加工相关 */
10、int ezsocket_read_process_number(ezsocket_t *ctx, int *count);//加工数量 读参数8002
11、int ezsocket_read_program_no(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//程序号 对应官方协议栈getprogramnumber2
12、int ezsocket_read_sequence_num(ezsocket_t *ctx, int *seq);//执行的序号/行数 对应官方协议栈getsequencenumber
13、int ezsocket_read_program_block(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//程序块内容 对应官方协议栈currentblockread
/* 轴相关 */
14、int ezsocket_read_axis_number(ezsocket_t *ctx, uint8_t *dst);//轴数量 参数1002
15、int ezsocket_read_feed_rate(ezsocket_t *ctx, uint8_t *dst);//进给倍率 读PLC参数
16、int ezsocket_read_spindle_rate(ezsocket_t *ctx, uint8_t *dst);//主轴倍率 读PLC参数
17、int ezsocket_read_fast_mov_rate(ezsocket_t *ctx, uint8_t *dst);//快速移动倍率 读PLC参数
18、int ezsocket_read_current_position(ezsocket_t *ctx, int axis, double *pos);//当前坐标 getcurrentpositon
19、int ezsocket_read_work_position(ezsocket_t *ctx, int axis, double *pos);//工作坐标 get workpostion
20、int ezsocket_read_distance_position(ezsocket_t *ctx, int axis, double *pos);//剩余坐标 getdistance
21、int ezsocket_read_machine_position(ezsocket_t *ctx, int axis, double *pos);//机械坐标 getmachinepositon
22、int ezsocket_read_spindle_set_speed(ezsocket_t *ctx, int *speed);//主轴设定速度 待确认
23、int ezsocket_read_spindle_act_speed(ezsocket_t *ctx, int *speed);//主轴实际速度 待确认GetSpindleMonitor
24、int ezsocket_read_feed_set_speed(ezsocket_t *ctx, double *speed);//进给设定速度 对应官方协议栈getfeedcommand
25、int ezsocket_read_feed_act_speed(ezsocket_t *ctx, double *speed);//进给实际速度 对应官方协议栈getfeedcommand
26、int ezsocket_read_spindle_load(ezsocket_t *ctx, int *load);//主轴负载 待测 GetSpindleMonitor
27、int ezsocket_read_servo_load(ezsocket_t *ctx, int axis, int *load);//伺服轴负载 GetServoMinitor
28、int ezsocket_read_servo_act_speed(ezsocket_t *ctx, int axis, int *speed);//伺服轴转速 GetServoMinitor
/* 刀具相关 */
29、int ezsocket_read_mag_tool_no(ezsocket_t *ctx, int *tool_no);//刀库刀具号
30、int ezsocket_read_spd_tool_no(ezsocket_t *ctx, int *tool_no);//主轴刀具号 GETMGNREADY
31、int ezsocket_read_tooloffset_d_shape(ezsocket_t *ctx, int *shape);//刀具补偿D shape 待确认  对应官方协议栈gettoolcommand
32、int ezsocket_read_tooloffset_d_wear(ezsocket_t *ctx,  int *wear);//刀具补偿D wear  待确认    对应官方协议栈gettoolcommand
33、int ezsocket_read_tooloffset_h(ezsocket_t *ctx, int axis, int *offset_h);//刀具补偿 H 待确认 可指定轴 对应官方协议栈gettoolcommand
34、int ezsocket_read_tool_set_size(ezsocket_t *ctx, int *tool_set_size);//刀具数量 对应官方协议栈gettoolsetsize
35、int ezsocket_read_tool_offset_type(ezsocket_t *ctx, int *type);//刀具补偿类型 对应官方协议栈gettype
36、int ezsocket_read_tool_offset_value(ezsocket_t *ctx, int type, int kind, int tool_set_no,double *offset);//获取刀具补偿值 对应官方协议栈getoffset
/* 模态相关 */
37、int ezsocket_read_m_command_value(ezsocket_t *ctx, int command_index, int *m_value);//读取M模态值，指定模态编号 getcommand2
38、int ezsocket_read_s_command_value(ezsocket_t *ctx, int command_index, int *s_value);//读取S模态值，指定模态编号 getcommand2
39、int ezsocket_read_t_command_value(ezsocket_t *ctx, int command_index, int *t_value);//读取T模态值，指定模态编号 getcommand2
40、int ezsocket_read_b_command_value(ezsocket_t *ctx, int command_index, int *b_value);//读取B模态值，指定模态编号 getcommand2
/* 报警相关 */
41、int ezsocket_read_alarm_msg(ezsocket_t *ctx, uint8_t *dst, uint16_t *length);//报警信息 getalarm 最多10条
/* PLC 相关 */
42、int ezsocket_read_plc_x(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC X readdevice
43、int ezsocket_read_plc_y(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC Y readdevice
44、int ezsocket_read_plc_r(ezsocket_t *ctx, int address, uint16_t *dst);//PLC R 字 readdevice
45、int ezsocket_read_plc_f(ezsocket_t *ctx, int address, int sub_address, uint8_t *dst);//PLC F 位信号 地址 待确认 readdevice
46、int ezsocket_read_plc_d(ezsocket_t *ctx, int address, uint16_t *dst);//PLC D 字 readdevice
47、int ezsocket_read_g_code(ezsocket_t *ctx, int address, double *dst);//G代码 readdevice
/* 参数相关 */
48、int ezsocket_read_param(ezsocket_t *ctx, int address, double *value);//参数读取 getparameterdata2
/* 宏变量 */
49、int ezsocket_read_macro(ezsocket_t *ctx, int address, double *value);//读取宏变量 commonVread 100-199 599-999
50、int ezsocket_write_marco_var(ezsocket_t *ctx, int address, uint8_t *dst);//写宏变量

```

## 系统返回值说明

```
#define EZSOCKET_RES_OK               0//成功
#define EZSOCKET_RES_TIMEOUT          -1//超时
#define EZSOCKET_RES_REV_TOO_LONG     -2//接收到的数据太长
#define EZSOCKET_RES_DECODE_ERR       -3//数据接收错误
#define EZSOCKET_RES_PARAM_ERR        -4//参数错误
#define EZSOCKET_RES_COMMAN_ERR       -5//未生成发送的指令
#define EZSOCKET_RES_AXIS_RANGE       -6//超过轴的范围
#define EZSOCKET_RES_SYSTEM_ERRCODE   -7//系统错误码


```

## 部分参数解释

```
/* MSTB功能 见三菱ezsocket协议中page60
 * GetCommand2
 */
/* 刀具补偿类型
 * 1 M system type-I : 1-axis compensation amount
 * 4 M system type-II : 1-axis compensation amount with wear compensation amount
 * 6 L system type : 2-axis compensation amount
*/

/* G CODE 共21组 1-21
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
 */

/* 急停
 * R69 0有效
 * R69.0 内置PLC停止
 * 69.1 外部PLC FROM TO 命令未执行
 * 69.2 外部PLC准备未完成
 * 69.3 外部PLC通信异常
 * 69.4 控制装置EMG连接器紧急停止状态
 * 69.5 电源供给外部紧急停止状态
 * 69.6 内置PLC S/W紧急停止输出YC2C未1
 * 69.7 NULL
 * 69.8 接触器阻断测试
 * 69.9 LINE
 * 69.A null
 * 69.B 用户PLC有错误代码
 * 69.C plc高速处理异常
 * 69.D 门互锁 dog/OT任意布局 装置错误
 * 69.E 主轴驱动单元 紧急停止输出
 * 69.F 伺服驱动单元 紧急停止输出
*/

/* 电机转速 见M70PLC接口手册 PAGE 77-78.
 * 系统1    系统2   系统3   系统4   信号名称
R4820     R4836   R4852   R4868
R4821     R4837   R4853   R4869  电机转速 第1轴
* 依次类推 共8轴
*/

/* 电机负载电流 见M70PLC接口手册 PAGE 78
 *系统1   系统2    系统3    系统4   信号名称
R4884    R4900   R4916    R4932
R4885    R4901   R4917    R4933 电机负载电流 第1轴
* 依次类推 共8轴
*/

/* 主轴倍率
 * Y1888(hex)
 *
*/
/* 进给倍率
 * YC60(hex)
*/
/* 快速移动倍率
 * YC68
*/
/* 主轴转速调整率D5
 */
```



## 测试用例

初始化流程为:

1、初始化ezsocket句柄

2、建立连接

3、设置参数，超时时间等

4、读取想要读取的信息例如硬件序列号等

5、关闭连接

三菱CNC返回的数据为小端模式，所有处理均已经转为小端模式了。

```c
#include "libmc/mc.h"
int main(int argc, char *argv[])
{
    mc_t *pmc;
    int rc;
    int address;
    int number;
    pmc = mc_new_program("COM4",9600,7,1,'E');
    struct timeval t;
    t.tv_sec=0;
    t.tv_usec=5000000;   //设置超时时间为5000毫秒
    mc_set_response_timeout(pmc,&t);//设置超时时间
    mc_set_debug(pmc,TRUE);//打开调试功能
    //注意 若为计算机连接协议中需要回车换行和和校验则需要
    //mc_set_cr_lf(ctx,TRUE);
    //mc_set_check_sum(ctx,TRUE);
    //若为三菱mc_3e_ascii协议若xy格式为8进制则mc_set_xy_format（ctx,MC_SYS_OCT）；
    //若为16进制则mc_set_xy_format（ctx,MC_SYS_HEX);
    
    rc = mc_connect(pmc);
    if(rc < 0)
    {
        printf("connect err\r\n");
        return;
    }
    address = 1;
    number =8;
     rc = mc_write_bits(pmc,MC_PROGRAM_TYPE_S,address,number,dest);
     if(rc < 0)
     {
         printf("write err\r\n");
         return 0;
     }
     rc = mc_read_bits(pmc,MC_PROGRAM_TYPE_S,address,number,dest);
     if(rc < 0)
     {
         printf("read err\r\n");
         return 0;
     }
     for(int i =0;i<number;i++)
     {
         printf("data[%d] = 0x%X\r\n",address+i,dest[i]);
     }
 }
```

## 测试要点

1、通过GXworks在线查看数据，不带网口的使用GXWORK2(上面的圆口)，带网口的使用GXWORK3

2、测试读写功能的时候注意各个协议区编号代码，不同的协议使用各自的协议区代码，并且协议的地址的数据类型，具体的地址的数据类型查看上面的"各个协议区域代码"

3、测试过程中要保证读写的数据跟预设的数据相同，三菱的写最小单位为位，其次是字，双字，浮点，字符串，字节的话默认也是字，其中读写byte的这个接口不可用。如果某个区的数据例如D区设置的数据类型是浮点，则需读写的时候要连续读两个地址，如果是dword也需要读两个，但是对于数据区域本身就是dword，则直接调用读写dword的那个接口，如果是字符串，该字符串占用几个地址就读写几个。

4、数据若对不上先转换一下字节序，若是使用的mc_read_dword用API接口中的33，若使用mc_read_word，使用32。

## 注意要点

对于三菱计算机连接协议：

计算机链接协议中，对于最小数据类型是字的，例如D区，当将其当双字使用的时候需要将32位整型的高16位放后面，低16位放前面例如12345678写入D12中，则将12345678的低16位的值放在D12中，高16为的值放在D13中，读双字返回值也是一样。