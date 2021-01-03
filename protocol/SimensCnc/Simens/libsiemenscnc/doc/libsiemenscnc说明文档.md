# libsiemenscnc 库介绍
libsiemenscnc用户跟西门子CNC包括828d/808d/840d/840dsl带x130版本进行通信，该报文是基于购买的协议报文以及自己测试增加接口而来，整个是基于S7协议。购买方提供的调试软件为WindowsFormsApp1，可基于这个进行抓包。

## 支持功能

libsiemenscnc支持TCP硬件所用协议为西门子S7协议，支持读写cnc内部数据包括状态/操作模式/nc报警/轴坐标等

## 安装库



## 库的用法

测试历程中包含库的基本用法，更多的使用方式可见API接口

## 跨平台支持

libsiemenscnc支持多平台，包括windows，linuxs，openBSD等平台，且32位和64位系统均支持。

## 报文格式

```
/* 报文格式说明 以读取通用机床参数为例 完整报文如下
 * 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x6e, 0x00, 0x01, 0x1a, 0x01
 * 0x03 0x00 固定协议报头
 * 0x00 0x1d 报文长度(包含协议报头)
 * 0x02 0xf0 0x80
 * 0x32  协议ID，通常为0x32
 * 0x01  即PDU的类型，一般有以下值
                    0x01：JOB
                    即作业请求，如，读/写存储器，读/写块，启动/停止设备，设置通信
                    0x02：ACK
                    即确认相应，这是一个没有数据的简单确认
                    0x03：ACK_DATA
                    即确认数据相应，一般是响应JOB的请求
                    0x07：USERDATA
                    即扩展协议，其参数字段包含请求/响应ID，一般用于编程/调试、读取SZL等
 * 0x00 0x00 冗余数据，通常为0x0000
 * 0x00 0x14 协议数据单元参考，它通过请求事件增加(由主站生成，每次新传输递增，用于链接对其请求的响应) Little-Endian
 * 0x00 0x0c 参数的总长度 Big-Endian
 * 0x00 0x00 数据长度，如果读取PLC/CNC内部数据，此处为0x0000；而对于其他功能，则为Data部分的数据长度；Big-Endian
 * 0x04 操作的功能，含义如下：0x04	Read Var	读取值
                           0x05	Write Var	写入值
 * 0x01 要操作的不同块的数量 默认1
 * 0x12 结构标识，通常为 0x12 ，代表变量规范
 * 0x08 地址规范长度，主要是以此往后的地址长度
 * 0x82 IDS 的地址规范的格式类型，用于确定寻址模式和其余项目结构的格式。常见的变量的结构标识如下表所示:
                                0x10	S7ANY	Address data S7-Any pointer-like DB1.DBX10.2
                                0x13	PBC-R_ID	R_ID for PBC
                                0x15	ALARM_LOCKFREE	Alarm lock/free dataset
                                0x16	ALARM_IND	Alarm indication dataset
                                0x19	ALARM_ACK	Alarm acknowledge message dataset
                                0x1a	ALARM_QUERYREQ	Alarm query request dataset
                                0x1c	NOTIFY_IND	Notify indication dataset
                                0xa2	DRIVEESANY	seen on Drive ES Starter with routing over S7
                                0xb2	1200SYM	Symbolic address mode of S7-1200
                                0xb0	DBREAD	Kind of DB block read, seen only at an S7-400
                                0x82	NCK	Sinumerik NCK HMI access
 * 0x01 对于CNC 可以为轴号，对于PLC 数据传输大小，常见值如下表:
                                Hex|值|描述
                                | :----- | :----- | :----- |
                                | 0| NULL |
                                | 3 | BIT | bit access, len is in bits
                                | 4 | BYTE/WORD/DWORD | byte/word/dword access, len is in bits
                                | 5 | INTEGER | integer access, len is in bits
                                | 6 | DINTEGER | integer access, len is in bytes
                                | 7 | REAL | real access, len is in bytes
                                | 9 | OCTET STRING | octet string, len is in bytes
 * 0x46 0x6e 可以为数据长度也可以为要操作的地址Big-Endian
 * 0x00 0x01 可以为地址的副地址，或者为DB编号
 * 0x1a 表示功能码
 * 0x01 表示要读取的数量或者其他
 */


/* 返回值解析
 * 第17 18字节为系统错误码 含义如下：
错误码	含义
0x0000	没有错误
0x0110	块号无效
0x0111	请求长度无效
0x0112	参数无效
0x0113	块类型无效
0x0114	找不到块
0x0115	块已存在
0x0116	块被写保护
0x0117	块/操作系统更新太大
0x0118	块号无效
0x0119	输入的密码不正确
0x011A	PG资源错误
0x011B	PLC资源错误
0x011C	协议错误
0x011D	块太多（与模块相关的限制）
0x011E	不再与数据库建立连接，或者S7DOS句柄无效
0x011F	结果缓冲区太小
0x0120	块结束列表
0x0140	可用内存不足
0x0141	由于缺少资源，无法处理作业
0x8001	当块处于当前状态时，无法执行请求的服务
0x8003	S7协议错误：传输块时发生错误
0x8100	应用程序，一般错误：远程模块未知的服务
0x8104	未在模块上实现此服务或报告了帧错误
0x8204	对象的类型规范不一致
0x8205	复制的块已存在且未链接
0x8301	模块上的内存空间或工作内存不足，或者指定的存储介质不可访问
0x8302	可用资源太少或处理器资源不可用
0x8304	无法进一步并行上传。存在资源瓶颈
0x8305	功能不可用
0x8306	工作内存不足（用于复制，链接，加载AWP）
0x8307	保持性工作记忆不够（用于复制，链接，加载AWP）
0x8401	S7协议错误：无效的服务序列（例如，加载或上载块）
0x8402	由于寻址对象的状态，服务无法执行
0x8404	S7协议：无法执行该功能
0x8405	远程块处于DISABLE状态（CFB）。该功能无法执行
0x8500	S7协议错误：帧错误
0x8503	来自模块的警报：服务过早取消
0x8701	寻址通信伙伴上的对象时出错（例如，区域长度错误）
0x8702	模块不支持所请求的服务
0x8703	拒绝访问对象
0x8704	访问错误：对象已损坏
0xD001	协议错误：非法的作业号
0xD002	参数错误：非法的作业变体
0xD003	参数错误：模块不支持调试功能
0xD004	参数错误：作业状态非法
0xD005	参数错误：作业终止非法
0xD006	参数错误：非法链路断开ID
0xD007	参数错误：缓冲区元素数量非法
0xD008	参数错误：扫描速率非法
0xD009	参数错误：执行次数非法
0xD00A	参数错误：非法触发事件
0xD00B	参数错误：非法触发条件
0xD011	调用环境路径中的参数错误：块不存在
0xD012	参数错误：块中的地址错误
0xD014	参数错误：正在删除/覆盖块
0xD015	参数错误：标签地址非法
0xD016	参数错误：由于用户程序错误，无法测试作业
0xD017	参数错误：非法触发号
0xD025	参数错误：路径无效
0xD026	参数错误：非法访问类型
0xD027	参数错误：不允许此数据块数
0xD031	内部协议错误
0xD032	参数错误：结果缓冲区长度错误
0xD033	协议错误：作业长度错误
0xD03F	编码错误：参数部分出错（例如，保留字节不等于0）
0xD041	数据错误：非法状态列表ID
0xD042	数据错误：标签地址非法
0xD043	数据错误：找不到引用的作业，检查作业数据
0xD044	数据错误：标签值非法，检查作业数据
0xD045	数据错误：HOLD中不允许退出ODIS控制
0xD046	数据错误：运行时测量期间非法测量阶段
0xD047	数据错误：“读取作业列表”中的非法层次结构
0xD048	数据错误：“删除作业”中的非法删除ID
0xD049	“替换作业”中的替换ID无效
0xD04A	执行'程序状态'时出错
0xD05F	编码错误：数据部分出错（例如，保留字节不等于0，...）
0xD061	资源错误：没有作业的内存空间
0xD062	资源错误：作业列表已满
0xD063	资源错误：触发事件占用
0xD064	资源错误：没有足够的内存空间用于一个结果缓冲区元素
0xD065	资源错误：没有足够的内存空间用于多个结果缓冲区元素
0xD066	资源错误：可用于运行时测量的计时器被另一个作业占用
0xD067	资源错误：“修改标记”作业过多（特别是多处理器操作）
0xD081	当前模式下不允许使用的功能
0xD082	模式错误：无法退出HOLD模式
0xD0A1	当前保护级别不允许使用的功能
0xD0A2	目前无法运行，因为正在运行的函数会修改内存
0xD0A3	I / O上活动的“修改标记”作业太多（特别是多处理器操作）
0xD0A4	'强制'已经建立
0xD0A5	找不到引用的作业
0xD0A6	无法禁用/启用作业
0xD0A7	无法删除作业，例如因为当前正在读取作业
0xD0A8	无法替换作业，例如因为当前正在读取或删除作业
0xD0A9	无法读取作业，例如因为当前正在删除作业
0xD0AA	处理操作超出时间限制
0xD0AB	进程操作中的作业参数无效
0xD0AC	进程操作中的作业数据无效
0xD0AD	已设置操作模式
0xD0AE	作业是通过不同的连接设置的，只能通过此连接进行处理
0xD0C1	访问标签时至少检测到一个错误
0xD0C2	切换到STOP / HOLD模式
0xD0C3	访问标记时至少检测到一个错误。模式更改为STOP / HOLD
0xD0C4	运行时测量期间超时
0xD0C5	块堆栈的显示不一致，因为块被删除/重新加载
0xD0C6	作业已被删除，因为它所引用的作业已被删除
0xD0C7	由于退出了STOP模式，因此作业被自动删除
0xD0C8	由于测试作业和正在运行的程序之间不一致，“块状态”中止
0xD0C9	通过复位OB90退出状态区域
0xD0CA	通过在退出前重置OB90并访问错误读取标签退出状态范围
0xD0CB	外设输出的输出禁用再次激活
0xD0CC	调试功能的数据量受时间限制
0xD201	块名称中的语法错误
0xD202	函数参数中的语法错误
0xD205	RAM中已存在链接块：无法进行条件复制
0xD206	EPROM中已存在链接块：无法进行条件复制
0xD208	超出模块的最大复制（未链接）块数
0xD209	（至少）模块上找不到给定块之一
0xD20A	超出了可以与一个作业链接的最大块数
0xD20B	超出了一个作业可以删除的最大块数
0xD20C	OB无法复制，因为关联的优先级不存在
0xD20D	SDB无法解释（例如，未知数）
0xD20E	没有（进一步）阻止可用
0xD20F	超出模块特定的最大块大小
0xD210	块号无效
0xD212	标头属性不正确（与运行时相关）
0xD213	SDB太多。请注意对正在使用的模块的限制
0xD216	无效的用户程序 - 重置模块
0xD217	不允许在模块属性中指定的保护级别
0xD218	属性不正确（主动/被动）
0xD219	块长度不正确（例如，第一部分或整个块的长度不正确）
0xD21A	本地数据长度不正确或写保护错误
0xD21B	模块无法压缩或压缩早期中断
0xD21D	传输的动态项目数据量是非法的
0xD21E	无法为模块（例如FM，CP）分配参数。系统数据无法链接
0xD220	编程语言无效。请注意对正在使用的模块的限制
0xD221	连接或路由的系统数据无效
0xD222	全局数据定义的系统数据包含无效参数
0xD223	通信功能块的实例数据块错误或超出最大背景数据块数
0xD224	SCAN系统数据块包含无效参数
0xD225	DP系统数据块包含无效参数
0xD226	块中发生结构错误
0xD230	块中发生结构错误
0xD231	至少有一个已加载的OB无法复制，因为关联的优先级不存在
0xD232	加载块的至少一个块编号是非法的
0xD234	块在指定的内存介质或作业中存在两次
0xD235	该块包含不正确的校验和
0xD236	该块不包含校验和
0xD237	您将要加载块两次，即CPU上已存在具有相同时间戳的块
0xD238	指定的块中至少有一个不是DB
0xD239	至少有一个指定的DB在装载存储器中不可用作链接变量
0xD23A	至少有一个指定的DB与复制和链接的变体有很大不同
0xD240	违反了协调规则
0xD241	当前保护级别不允许该功能
0xD242	处理F块时的保护冲突
0xD250	更新和模块ID或版本不匹配
0xD251	操作系统组件序列不正确
0xD252	校验和错误
0xD253	没有可用的可执行加载程序; 只能使用存储卡进行更新
0xD254	操作系统中的存储错误
0xD280	在S7-300 CPU中编译块时出错
0xD2A1	块上的另一个块功能或触发器处于活动状态
0xD2A2	块上的触发器处于活动状态。首先完成调试功能
0xD2A3	块未激活（链接），块被占用或块当前被标记为删除
0xD2A4	该块已被另一个块函数处理
0xD2A6	无法同时保存和更改用户程序
0xD2A7	块具有“未链接”属性或未处理
0xD2A8	激活的调试功能阻止将参数分配给CPU
0xD2A9	正在为CPU分配新参数
0xD2AA	当前正在为模块分配新参数
0xD2AB	当前正在更改动态配置限制
0xD2AC	正在运行的激活或取消激活分配（SFC 12）暂时阻止R-KiR过程
0xD2B0	在RUN（CiR）中配置时发生错误
0xD2C0	已超出最大工艺对象数
0xD2C1	模块上已存在相同的技术数据块
0xD2C2	无法下载用户程序或下载硬件配置
0xD401	信息功能不可用
0xD402	信息功能不可用
0xD403	服务已登录/注销（诊断/ PMC）
0xD404	达到的最大节点数。不再需要登录诊断/ PMC
0xD405	不支持服务或函数参数中的语法错误
0xD406	当前不可用的必需信息
0xD407	发生诊断错误
0xD408	更新已中止
0xD409	DP总线错误
0xD601	函数参数中的语法错误
0xD602	输入的密码不正确
0xD603	连接已合法化
0xD604	已启用连接
0xD605	由于密码不存在，因此无法进行合法化
0xD801	至少有一个标记地址无效
0xD802	指定的作业不存在
0xD803	非法的工作状态
0xD804	非法循环时间（非法时基或多个）
0xD805	不能再设置循环读取作业
0xD806	引用的作业处于无法执行请求的功能的状态
0xD807	功能因过载而中止，这意味着执行读取周期所需的时间比设置的扫描周期时间长
0xDC01	日期和/或时间无效
0xE201	CPU已经是主设备
0xE202	由于闪存模块中的用户程序不同，无法进行连接和更新
0xE203	由于固件不同，无法连接和更新
0xE204	由于内存配置不同，无法连接和更新
0xE205	由于同步错误导致连接/更新中止
0xE206	由于协调违规而拒绝连接/更新
0xEF01	S7协议错误：ID2错误; 工作中只允许00H
0xEF02	S7协议错误：ID2错误; 资源集不存在
 *
 *  第21字节返回码，响应报文中Data部分的常见返回码如下表:
                 Hex	值	        描述
                0x00	Reserved	未定义，预留
                0x01	Hardware error	硬件错误
                0x03	Accessing the object not allowed	对象不允许访问
                0x05	Invalid address	无效地址，所需的地址超出此PLC的极限
                0x06	Data type not supported	数据类型不支持
                0x07	Data type inconsistent	日期类型不一致
                0x0a	Object does not exist	对象不存在
                0xff	Success	成功
 *  第22字节 数据传输大小，常见的 data 中数据传输大小的值如下表:
                                Hex	值	描述
                                0	NULL
                                3	BIT	bit access, len is in bits
                                4	BYTE/WORD/DWORD	byte/word/dword access, len is in bits
                                5	INTEGER	integer access, len is in bits
                                6	DINTEGER	integer access, len is in bytes
                                7	REAL	real access, len is in bytes
                                9	OCTET STRING	octet string, len is in bytes
 *  第23 24字节表示数据的长度
 *   25-- 数据内容
 *
 *
 */
```

## 结构体说明

```
typedef struct _siemenscnc_backend {
    unsigned int header_length;//帧头长度
    unsigned int max_adu_length;//数据区最大长度
    int (*build_request_basis) (siemenscnc_t *ctx, int function,
                                unsigned short axis, unsigned char axis_number,
                                int addr,int sub_addr,
                                int dnb, uint8_t *req, const uint8_t *data);//生成请求指令
    int (*receive_msg)  (siemenscnc_t *ctx, uint8_t *msg, uint16_t *length);//接受数据
    int (*decode_msg)(siemenscnc_t *ctx,uint8_t function,const uint8_t *msg,uint8_t *dest,uint16_t *length);//解析数据
    int (*send) (siemenscnc_t *ctx, const uint8_t *req, int req_length);//发送数据
    int (*recv) (siemenscnc_t *ctx, uint8_t *rsp, int rsp_length);//接受数据
    int (*connect) (siemenscnc_t *ctx);//连接
    void (*close) (siemenscnc_t *ctx);//关闭
    int (*flush) (siemenscnc_t *ctx);//清空缓存
    int (*select) (siemenscnc_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);
} siemenscnc_backend_t;
siemenscnc_backend_t为协议通用模块，具体解释如下:
1、unsigned int header_length;//帧头长度
2、unsigned int max_adu_length;//数据区最大长度
3、 int (*build_request_basis) (siemenscnc_t *ctx, int function,
                                unsigned short axis, unsigned char axis_number,
                                int addr,int sub_addr,
                                int dnb, uint8_t *req, uint8_t *data);//生成请求指令
  siemenscnc_t *ctx, int function,协议句柄unsigned short axis,轴编号 unsigned char axis_number,轴数量int addr,要操作的地址int sub_addr,要操作的副地址int dnb,要写入的数据长度 uint8_t *req,存放生成的指令 const uint8_t *data要写入的数据                              
4、int (*receive_msg)  (mc_t *ctx, uint8_t *msg, uint16_t *length);//从CNC端接受数据会调用recv
5、int (*send) (siemenscnc_t *ctx, const uint8_t *req, int req_length);//发送数据
6、int (*recv) (siemenscnc_t *ctx, uint8_t *rsp, int rsp_length);//接受数据
7、 int (*connect) (siemenscnc_t *ctx);//建立连接
8、void (*close) (siemenscnc_t *ctx);//关闭
9、int (*flush) (siemenscnc_t *ctx);//清空数据流
10、int (*select) (siemenscnc_t *ctx, fd_set *rfds, struct timeval *tv, int msg_length);//阻塞直到有数据返回

struct _siemenscnc {
    int s;/* Socket 描述符 */
    int debug;
    int error_recovery;
    struct timeval response_timeout;
    struct timeval byte_timeout;
    const siemenscnc_backend_t *backend;
    void *backend_data;
};
struct _siemenscnc即siemenscnc_t,是libsiemenscnc中的操作的句柄。
void *backend_data;是硬件参数，本地为网络参数ip和端口号
const siemenscnc_backend_t *backend; 为828d自己的数据处理接口，包括生成报文，解析报文等
typedef enum
{
    SIEMENSCNC_ERROR_RECOVERY_NONE          = 0,
    SIEMENSCNC_ERROR_RECOVERY_LINK          = (1<<1),//自动重连
    SIEMENSCNC_ERROR_RECOVERY_PROTOCOL      = (1<<2),//延时并冲洗已经有的数据
} siemenscnc_error_recovery_mode;
siemenscnc_error_recovery_mode为遇到错误时候的处理。

```

## API接口说明

```
1、siemenscnc_t *siemenscnc_828d_new(const char *ip_address, int port);

初始化一个828d连接的句柄 const char *ip_address要连接的设备的IP const int port端口号
2、int siemenscnc_connect(siemenscnc_t *ctx);//建立连接
3、void siemenscnc_close(siemenscnc_t *ctx);//关闭连接
4、void siemenscnc_free(siemenscnc_t *ctx);//释放句柄
5、int siemenscnc_flush(siemenscnc_t *ctx);//清空数据流
6、void siemenscnc_set_socket(siemenscnc_t *ctx, int socket);//设置socket
7、int siemenscnc_get_socket(siemenscnc_t *ctx);//获取socket
8、int siemenscnc_set_error_recovery(siemenscnc_t *ctx, siemenscnc_error_recovery_mode error_recovery);//错误处理
9、void siemenscnc_get_response_timeout(siemenscnc_t *ctx, struct timeval *timeout);//获取超时时间
10、void siemenscnc_set_response_timeout(siemenscnc_t *ctx, const struct timeval *timeout);//设置超时时间
11、void siemenscnc_get_byte_timeout(siemenscnc_t *ctx, struct timeval *timeout);//获取去字节间超时
12、void siemenscnc_set_byte_timeout(siemenscnc_t *ctx, const struct timeval *timeout);//设置字节间超时
13、int siemenscnc_get_header_length(siemenscnc_t *ctx);//获取协议头长度
void siemenscnc_set_debug(siemenscnc_t *ctx, int boolean);//打印调试信息
14、uint16_t siemenscnc_reverse_bytes_uint16(uint16_t value);//word大小端转换
15、uint32_t siemenscnc_reverse_bytes_uint32(uint32_t value);//dword大小端转换
16、float siemenscnc_reverse_bytes_float(float value);//float大小端转换
17、double siemenscnc_reverse_bytes_double(double value);//double大小端转换
18、uint16_t siemenscnc_get_16(const uint8_t *src);//获取16位整型
19、uint32_t siemenscnc_get_32(const uint8_t *src);//获取32位整型
20、float siemenscnc_get_float(const uint8_t *src);//获取float
21、double siemenscnc_get_double(const uint8_t *src);//获取double
```

## API接口之读写接口

```
/* 系统相关 */
1、int siemenscnc_read_serial_number(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//硬件序列号
2、int siemenscnc_read_cnc_type(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//设备类型
3、int siemenscnc_read_verison(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//版本信息
4、int siemenscnc_read_manufacture_data(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//出厂日期
5、int siemenscnc_read_process_number(siemenscnc_t *ctx, double *dst);//加工数量
6、int siemenscnc_read_process_set_number(siemenscnc_t *ctx, double *dst);//设定的加工数量
7、int siemenscnc_read_run_time(siemenscnc_t *ctx, double *dst);//循环时间,一次自动运行起动时间的累计值
8、int siemenscnc_read_remain_time(siemenscnc_t *ctx, double *dst);//剩余时间
9、int siemenscnc_read_program_name(siemenscnc_t *ctx, uint8_t *dst, uint16_t *length);//加工程序名
/* 状态相关 */
10、int siemenscnc_read_operate_mode(siemenscnc_t *ctx, uint8_t *dst);//操作模式
11、int siemenscnc_read_status(siemenscnc_t *ctx, uint8_t *dst);//运行状态
/* 轴相关 */
12、int siemenscnc_read_axis_name(siemenscnc_t *ctx, uint8_t axis, uint8_t *dst, uint16_t *length);//轴名称 同上
13、int siemenscnc_read_machine_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//机械坐标 根据主界面（加工界面/坐标界面）坐标显示的轴顺序
14、int siemenscnc_read_relatively_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//工件坐标 相对坐标 同上
15、int siemenscnc_read_remain_pos(siemenscnc_t *ctx, uint8_t axis, double *dst);//剩余坐标 同上
16、int siemenscnc_read_feed_set_speed(siemenscnc_t *ctx, double *dst);//设定进给速度
17、int siemenscnc_read_feed_act_speed(siemenscnc_t *ctx, double *dst);//实际进给速度
18、int siemenscnc_read_feed_rate(siemenscnc_t *ctx, double *dst);//进给倍率
19、int siemenscnc_read_spindle_set_speed(siemenscnc_t *ctx, double *dst);//主轴设定速度
20、int siemenscnc_read_spindle_act_speed(siemenscnc_t *ctx, double *dst);//主轴实际速度
21、int siemenscnc_read_spindle_rate(siemenscnc_t *ctx, double *dst);//主轴倍率
22、int siemenscnc_read_driver_voltage(siemenscnc_t *ctx, uint8_t driver, float *dst);//母线电压 调试->驱动参数->面板右侧选择驱动，第一个即为第一个驱动器，依次类推
23、int siemenscnc_read_driver_current(siemenscnc_t *ctx, uint8_t driver, float *dst);//实际电流 同上
24、int siemenscnc_read_driver_power(siemenscnc_t *ctx, uint8_t driver, float *dst);//电机功率 同上
25、int siemenscnc_read_driver_temper(siemenscnc_t *ctx, uint8_t driver, float *dst);//电机温度 同上
26、int siemenscnc_read_g_coordinate_t(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst);//G T坐标系
27、int siemenscnc_read_g_coordinate_m(siemenscnc_t *ctx, int g_coordinate, uint8_t axis, double *dst);//G T坐标系
/* 刀具相关 */
28、int siemenscnc_read_tool_cur_name(siemenscnc_t *ctx, uint8_t *tool_no, uint16_t *length);//刀具号
29、int siemenscnc_read_tool_cur_t_no(siemenscnc_t *ctx, double *dst);//当前刀具号
30、int siemenscnc_read_tool_cur_d_no(siemenscnc_t *ctx, double *dst);//当前刀具刀沿号
31、int siemenscnc_read_tool_cur_h_no(siemenscnc_t *ctx, double *dst);//当前刀具H号
32、int siemenscnc_read_tool_type(siemenscnc_t *ctx, int tool_edge_no,  int tool_row, double *dst);//刀具类型
33、int siemenscnc_read_tool_length(siemenscnc_t *ctx, int tool_edge_no,  int tool_row, double *dst);//刀具长度
34、int siemenscnc_read_tool_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具半径
35、int siemenscnc_read_tool_edge(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀沿位置
36、int siemenscnc_read_tool_tip(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具齿数
37、int siemenscnc_read_tool_h_no(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀沿H号
38、int siemenscnc_read_tool_wear_length(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具长度磨损
39、int siemenscnc_read_tool_wear_radius(siemenscnc_t *ctx, int tool_edge_no, int tool_row, double *dst);//刀具半径磨损
40、int siemenscnc_read_tool_x_offset(siemenscnc_t *ctx, double *dst);//X补偿
41、int siemenscnc_read_tool_z_offset(siemenscnc_t *ctx, double *dst);//Z补偿

/* 报警相关 */
42、int siemenscnc_read_cnc_nc_alarm_no(siemenscnc_t *ctx, uint16_t *dst);//NC报警数量
43、int siemenscnc_read_cnc_nc_alarm(siemenscnc_t *ctx, uint16_t alarm_no, uint32_t *dst);//NC报警
/* 参数相关 */
44、int siemenscnc_read_r_var(siemenscnc_t *ctx, uint16_t address, double *dst);//R变量 查看方法 参数->R用户变量->选择要读取的参数
45、int siemenscnc_read_s_r_p_param(siemenscnc_t *ctx, uint8_t driver, uint16_t address, uint8_t sub_address, uint8_t *dest, uint16_t *length);//驱动器R参数  查看方法：调试->机床数据->驱动参数->面板右侧选择驱动，第一个即为第一个驱动器，依次类推->选择要读写的地址，若地址无[]这个，则sub_address为0
46、int siemenscnc_read_general_machine_param(siemenscnc_t *ctx, uint16_t address, uint8_t sub_address, uint8_t *dest, uint16_t *length);//通用机床数据 查看方法：调试->机床数据->机床数据->通用机床数据
47、int siemenscnc_read_channel_machine_param(siemenscnc_t *ctx, uint16_t address, uint8_t sub_address, uint8_t *dest, uint16_t *length);//通道机床数据 查看方法：调试->机床数据->通道机床数据
48、int siemenscnc_read_axis_machine_param(siemenscnc_t *ctx, uint16_t address, uint8_t sub_address, uint8_t *dest, uint16_t *length);//轴机床数据 查看方法：调试->机床数据->轴机床数据
49、int siemenscnc_read_control_unit_param(siemenscnc_t *ctx, uint16_t address, uint8_t sub_address, uint8_t *dest, uint16_t *length);//控制单元参数 查看方法：调试->机床数据->控制单元参数



```

## 系统返回值说明

```
#define SIEMENSCNC_RES_OK       0//成功
#define SIEMENSCNC_RES_TIMEOUT      -1//超时
#define SIEMENSCNC_RES_REV_TOO_LONG -2//接收到的数据太长
#define SIEMENSCNC_RES_DECODE_ERR   -10//数据接收错误
#define SIEMENSCNC_RES_PARAM_ERR    -11//参数错误
#define SIEMENSCNC_RES_COMMAN_ERR   -12//未生成发送的指令
#define SIEMENSCNC_RES_DECODE_ERR_DATATYPE -13 //解析到错误的数据类型
#define SIEMENSCNC_RES_SYS_NOT_CARRY    -14//系统不支持的指令
#define SIEMENSCNC_RES_SYS_ERR_CODE     -15//系统返回错误码


```

## 测试用例

初始化流程为:

1、初始化828d句柄

2、建立连接

3、设置参数，超时时间等

4、读取想要读取的信息例如硬件序列号等

5、关闭连接

以828d为例：

```c
#include "libsiemenscnc/siemenscnc.h"
int main(int argc, char *argv[])
{
    siemenscnc_t *ctx;
    ctx = siemenscnc_828d_new("192.168.0.199",102);
    siemenscnc_set_debug(ctx,1);
    struct timeval t;
    int rc;
    t.tv_sec=0;
    t.tv_usec=100000000;   //设置超时时间为1000毫秒
    siemenscnc_set_response_timeout(ctx,&t);
    rc = siemenscnc_connect(ctx);
    if(rc < 0)
    {
        printf("connect err\r\n");
        return;
    }
   //读硬件序列号
    uint8_t dest[128];
    uint16_t length;
    rc = siemenscnc_read_serial_number(ctx,dest,&length);
    if(SIEMENSCNC_RES_OK == rc)
    {
        printf("%s\r\n",(char*)dest);
    }
    //R用户变量
    double d_value;
    rc = siemenscnc_read_r_var(ctx,0,&d_value);
    if(SIEMENSCNC_RES_OK == rc)
    {
        printf("%lf\r\n",d_value);
    }
    //读s驱动器 r参数
    float f_value;
    rc = siemenscnc_read_s_r_p_var(ctx,0,37,0,&f_value);
    if(SIEMENSCNC_RES_OK == rc)
    {
        printf("%f\r\n",f_value);
    }         
 }

```

