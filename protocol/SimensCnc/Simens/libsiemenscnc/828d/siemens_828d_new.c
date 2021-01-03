#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <signal.h>
#include <sys/types.h>

#if defined(_WIN32)
# define OS_WIN32
/* ws2_32.dll has getaddrinfo and freeaddrinfo on Windows XP and later.
 * minwg32 headers check WINVER before allowing the use of these */
# ifndef WINVER
# define WINVER 0x0501
# endif
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <sys/ioctl.h>

#if defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD__ < 5)
# define OS_BSD
# include <netinet/in_systm.h>
#endif

# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <poll.h>
# include <netdb.h>
#endif

#include "siemens_828d_new.h"


#define SIEMENSCNC_HEADER_LENGTH      4
#define SIEMENSCNC_MAX_ADU_LENGTH     512


typedef struct
{
    uint8_t command[128];//命令
    uint16_t command_length;//命令的长度
}_siemenscnc_command_t;

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



static const uint8_t _siemenscnc_sync[] = {0x03, 0x00, 0x00, 0x07, 0x02, 0xf0, 0x00};//加上这个数据表示同步


static const uint8_t _siemenscnc_axis[] = {0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8};//0xa1表示主轴编号 0xa2表示x 0xa3表示y 一次类推


static const uint8_t _siemenscnc_hand_shark1[] = { 0x03, 0x00, 0x00, 0x16, 0x11, 0xe0, 0x00, 0x00, 0x00, 0x48, 0x00, 0xc1, 0x02, 0x04, 0x00, 0xc2, 0x02, 0x0d, 0x04, 0xc0, 0x01, 0x0a};//第一次握手
static const uint8_t _siemenscnc_hand_shark2[] = { 0x03, 0x00, 0x00, 0x19, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x64, 0x00, 0x64, 0x03, 0xc0 };//第二次握手
static const uint8_t _siemenscnc_hand_shark3[] = { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x00, 0x14, 0x00, 0x01, 0x3b, 0x01, 0x03, 0x00, 0x00, 0x07, 0x02, 0xf0, 0x00 };//第三次握手

/* 系统相关 */
static const _siemenscnc_command_t _siemenscnc_system_command[] = {
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x6e, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//CNC_ID 硬件序列号 18030[0]
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x78, 0x00, 0x04, 0x1a, 0x01}, 0x001d},//机床类型
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x78, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//设备版本信息
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x78, 0x00, 0x03, 0x1a, 0x01}, 0x001d},//出厂时间
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x79, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//实际工件数 加工数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x77, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//预设得加工数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x01, 0x29, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//循环时间,一次自动运行起动时间的累计值
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x01, 0x2a, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//剩余时间
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x0c, 0x00, 0x01, 0x7a, 0x01}, 0x001d}//当前运行程序名
};
/* 运行状态 */
static const _siemenscnc_command_t __SIEMENSCNC_STATUS_command[] = {
    { { 0x03, 0x00, 0x00, 0x27, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x16, 0x00, 0x00, 0x04, 0x02, 0x12, 0x08, 0x82, 0x21, 0x00, 0x03, 0x00, 0x01, 0x7f, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x0c, 0x00, 0x01, 0x7f, 0x01}, 0x0027},//操作模式
    { { 0x03, 0x00, 0x00, 0x27, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x16, 0x00, 0x00, 0x04, 0x02, 0x12, 0x08, 0x82, 0x41, 0x00, 0x0b, 0x00, 0x01, 0x7f, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x0d, 0x00, 0x01, 0x7f, 0x01}, 0x0027},//运行状态
};
/* 轴相关 */
static const _siemenscnc_command_t _siemenscnc_axis_command[] = {
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x4e, 0x70, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//轴名称 最后一位表示读取的轴的数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x01, 0x74, 0x01}, 0x001d},//机械坐标 倒数第三位0x01表示轴 XYZ等 最后一个表示读取的轴的数量  //0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x02, 0x74, 0x01, //0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x03, 0x74, 0x01
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x19, 0x00, 0x01, 0x70, 0x01}, 0x001d},//相对坐标 工件坐标 含义同上
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x03, 0x00, 0x01, 0x74, 0x01}, 0x001d},//剩余坐标 含义同上
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x12, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//进给设定速度
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x12, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x01, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//进给实际速度
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x13, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x03, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//进给倍率
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x00, 0x03, 0x00, 0x04, 0x72, 0x01}, 0x001d},//主轴设定速度
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x01, 0x72, 0x01}, 0x001d},//主轴实际速度
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x04, 0x00, 0x01, 0x72, 0x01}, 0x001d},//主轴倍率
    //{ { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa1, 0x00, 0x1a, 0x00, 0x01, 0x82, 0x01}, 0x001d},//母线电压 倒数第7位 0xa1 表示主轴 倒数第5位和第六位 001a表示参数编号
    //{ { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa1, 0x00, 0x1e, 0x00, 0x01, 0x82, 0x01}, 0x001d},//实际电流 倒数第7位 0xa1 表示主轴 倒数第5位和第六位 001e表示参数编号
    //{ { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa3, 0x00, 0x20, 0x00, 0x01, 0x82, 0x01}, 0x001d},//电机功率 倒数第7位 0xa2为X轴  倒数第5位和第六位 0020表示参数编号
    //{ { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa3, 0x00, 0x25, 0x00, 0x01, 0x82, 0x01}, 0x001d}//电机温度 倒数第7位 0xa1 表示主轴   倒数第5位和第六位 0025表示参数编号
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x01, 0x12, 0x01}, 0x001d},//G工件坐标系 T 倒数第三位0x01表示轴 XYZ等 最后一个表示读取的轴的数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x01, 0x12, 0x01}, 0x001d},//G工件坐标系 M 倒数第三位0x01表示轴 XYZ等 最后一个表示读取的轴的数量
};
/* 刀具相关 */
static const _siemenscnc_command_t _siemenscnc_tool_command[] = {
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x21, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//828d刀具名  //0x12, 0x08, 0x82, 0x41, 0x00, 0x21, 0x00, 0x01, 0x7f, 0x01,}//840dsl自定义刀具号
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x17, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//T刀具号
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x18, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//D刀沿号    840DSL 0x12, 0x08, 0x82, 0x41, 0x00, 0x23, 0x00, 0x01, 0x7f, 0x01,
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x01, 0xB6, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//刀具 H号
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x81, 0x00, 0x01, 0x00, 0x01, 0x14, 0x01}, 0x001d},//828d刀具列表 0x12, 0x08, 0x82, 0x81, 0x00, 0x01(行), 0x00, 0x01(列), 0x14, 0x07
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x21, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x71, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//刀具补偿X
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x73, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//刀具补偿Z  840DSL 0x12, 0x08, 0x82, 0x81, 0x00, 0x11, 0x00, 0x01, 0x14, 0x23
    // { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x1B, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//刀具磨损半径   840DSL 0x12, 0x08, 0x82, 0x41, 0x00, 0x17, 0x00, 0x01, 0x7f, 0x01
    //  { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x02, 0x00, 0x02, 0x14, 0x01}, 0x001d}//刀沿位置
};
/* 报警相关 */
static const _siemenscnc_command_t _siemenscnc_alarm_command[] = {

    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x00, 0x07, 0x00, 0x01, 0x7f, 0x01}, 0x001d},//报警数量
    // { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x01, 0x54, 0x01}, 0x001d},
    //{ { 0x03, 0x00, 0x00, 0x2b, 0x02, 0xf0, 0x80, 0x32, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x00, 0x12, 0x00, 0x01, 0x12, 0x04, 0x11, 0x02, 0x01, 0x00, 0xff, 0x09, 0x00, 0x0e, 0x00, 0x01, 0x0d, 0x11, 0x12, 0x08, 0x82, 0x01, 0x00, 0x00, 0x00, 0x01, 0x54, 0x01}, 0x002b},//NC报警信息
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x16, 0x00, 0x00, 0x04, 0x02, 0x12, 0x08, 0x82, 0x01, 0x00, 0x01, 0x00, 0x01, 0x77, 0x01}, 0x001d}//NC报警信息//0x12, 0x08, 0x82, 0x01, 0x00, 0x04, 0x00, 0x01, 0x77, 0x01 报警时间
};
/* 参数相关 */
static const _siemenscnc_command_t _siemenscnc_param_command[] = {
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x00, 0x01, 0x01, 0x2c, 0x15, 0x01}, 0x001d},//读R变量 25，26个字节表示r参数号 最后一位01 表示读取的数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa1, 0x00, 0x25, 0x00, 0x11, 0x82, 0x01}, 0x001d},//读驱动器r p参数 倒数第三位R驱动器下标，如R37[1],0X03=R37[2];倒数第五位和第六位为地址，如0025为十进制37，第七位为轴，依次递增切换轴 最后一位01 表示读取的数量
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x01, 0x46, 0x6e, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//读通用机床参数 倒数第三位参数下标，如18030[0],0X01=18030[0]括号的0;倒数第五位和第六位为地址，如466e为十进制18030，
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x41, 0x4e, 0x70, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//读通道机床参数 倒数第三位参数下标，如20080[0],0X01=20080[0]括号的0;倒数第五位和第六位为地址，如4e80为十进制20080，
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0x61, 0x4e, 0x70, 0x00, 0x01, 0x1a, 0x01}, 0x001d},//读轴机床参数 倒数第三位参数下标，如20080[0],0X01=20080[0]括号的0;倒数第五位和第六位为地址，如4e80为十进制20080，
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa1, 0x4e, 0x70, 0x00, 0x01, 0x80, 0x01}, 0x001d},//读控制模块参数 倒数第三位参数下标，如20080[0],0X01=20080[0]括号的0;倒数第五位和第六位为地址，如4e80为十进制20080，
    { { 0x03, 0x00, 0x00, 0x1d, 0x02, 0xf0, 0x80, 0x32, 0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00, 0x04, 0x01, 0x12, 0x08, 0x82, 0xa1, 0x4e, 0x70, 0x00, 0x01, 0x81, 0x01}, 0x001d},//读电源模块参数 倒数第三位参数下标，如20080[0],0X01=20080[0]括号的0;倒数第五位和第六位为地址，如4e80为十进制20080，
};

/* 数据解析 */
static int _siemenscnc_828d_decode_to_array(const uint8_t *src,uint8_t *dst, uint16_t *length)//将数据解析成array
{

    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度

    if(src_length >= 23)
    {
        if( src[21] != 0xFF )
        {
            return SIEMENSCNC_RES_DECODE_ERR;
        }
    }
    else
    {

        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    memcpy(dst,src+25,valid_length);
    *length = valid_length;

    return SIEMENSCNC_RES_OK;
}



static int _siemenscnc_828d_decode_to_double(const uint8_t *src,uint8_t *dst)//将数据解析成double
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if( src[21] != 0xFF )
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    if(valid_length > 8)
    {
        return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
    }
    memcpy(dst,src+25,valid_length);

    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_828d_decode_to_float(const uint8_t *src,uint8_t *dst)//将数据解析成float
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if( src[21] != 0xFF)
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    if(valid_length > 4)
    {
        return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
    }
    memcpy(dst,src+25,valid_length);
    _siemenscnc_swap_data(dst,valid_length);

    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_828d_decode_to_int(const uint8_t *src,uint8_t *dst)//将数据解析成int 16
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if( src[21] != 0xFF )
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];

    if(valid_length > 4)
    {
        return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
    }

    memcpy(dst,src+25,valid_length);

    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_828d_decode_to_string(const uint8_t *src, uint8_t *dst, uint16_t *length)//将数据解析成字符串
{
    int i;
    int ptr = 0;
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if( src[21] != 0xFF )
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    if(valid_length > _SIEMENSCNC_MIN_REQ_LENGTH)
    {
        return SIEMENSCNC_RES_REV_TOO_LONG;
    }

    /*for(i = 0;i<valid_length;i++)
    {
        if(src[25+i] != 0x00)
        {
            dst[ptr++] = src[25+i];
        }
    }*/
    memcpy(dst,src+25,valid_length);
    *length = valid_length;//strlen(dst);
    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_decode_serial_number(const uint8_t *src, uint8_t *dst, uint16_t *length)//硬件序列号
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_cnc_type(uint8_t *src, uint8_t *dst, uint16_t *length)//设备类型
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_verison(uint8_t *src, uint8_t *dst, uint16_t *length)//版本信息
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_manufacture_data(uint8_t *src, uint8_t *dst, uint16_t *length)//出厂日期
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_operate_mode(uint8_t *src,uint8_t *dst)//操作模式
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    if(src_length >= 28)
    {
        if(src[21] != 0xFF  || src[27] != 0xFF )
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    if (src[24] == 0x02)
    {
        if (src[31] == 0x00)
        {
            if (src[25] == 0x00)//手动
            {
                dst[0] = SIEMENSCNC_MODE_JOG;
            }
            else if (src[25] == 0x01)//MDA
            {
                dst[0] = SIEMENSCNC_MODE_MDA;
            }
            else if (src[25] == 0x02)//自动
            {
                dst[0] = SIEMENSCNC_MODE_AUTO;
            }
            else
            {
                dst[0] = SIEMENSCNC_MODE_OTHER;
            }
        }
        else if (src[31] == 0x03)//REF
        {
            dst[0] = SIEMENSCNC_MODE_REF_POINT;
        }
        else if(src[31] == 0x01 )
        {
            dst[0] = SIEMENSCNC_MODE_REFPOS;
        }
        else
        {
            dst[0] = SIEMENSCNC_MODE_OTHER;
        }
    }
    else
    {
        dst[0] = SIEMENSCNC_MODE_OTHER;
    }

    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_decode_status(uint8_t *src,uint8_t *dst)//运行状态
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    if(src_length >= 28)
    {
        if(src[21] != 0xFF || src[27] != 0xFF )
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    if (src[24] == 0x02)
    {
        if ((src[25] == 0x00) && (src[31] == 0x05))//RESET
        {
            dst[0] = _SIEMENSCNC_STATUS_RESET;
        }
        else if ((src[25] == 0x02) && (src[31] == 0x02))//循环结束
        {
            dst[0] = _SIEMENSCNC_STATUS_STOP;
        }
        else if ((src[25] == 0x01) && (src[31] == 0x03))//循环开始
        {
            dst[0] = _SIEMENSCNC_STATUS_START;
        }
        else if ((src[25] == 0x01) && (src[31] == 0x05))
        {
            dst[0] = _SIEMENSCNC_STATUS_SPENDLE_CW_CCW;
        }
        else
        {
            dst[0] = _SIEMENSCNC_STATUS_OTHER;
        }
    }
    else
    {
        dst[0] = _SIEMENSCNC_STATUS_OTHER;
    }
    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_decode_process_number(uint8_t *src,uint8_t *dst, uint16_t *length)//加工数量
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_process_set_number(uint8_t *src,uint8_t *dst,uint16_t *length)//设定的加工数量
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_feed_set_speed(uint8_t *src,uint8_t *dst,uint16_t *length)//设定进给速度
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_feed_act_speed(uint8_t *src,uint8_t *dst,uint16_t *length)//实际进给速度
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_feed_rate(uint8_t *src,uint8_t *dst, uint16_t *length)//进给倍率
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_spindle_set_speed(uint8_t *src,uint8_t *dst, uint16_t *length)//主轴设定速度
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_spindle_act_speed(uint8_t *src,uint8_t *dst,uint16_t *length)//主轴实际速度
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_spindle_rate(uint8_t *src,uint8_t *dst,uint16_t *length)//主轴倍率
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_run_time(uint8_t *src,uint8_t *dst,uint16_t *length)//循环时间,一次自动运行起动时间的累计值
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_remain_time(uint8_t *src ,uint8_t *dst,uint16_t *length)//剩余时间
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_program_name(uint8_t *src, uint8_t *dst, uint16_t *length)//加工程序名
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_tool_name(uint8_t *src,uint8_t *dst, uint16_t *length)//刀具名
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_tool_list(uint8_t *src,uint8_t *dst, uint16_t *length)//刀具列表
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_tool_d_radius(uint8_t *src,uint8_t *dst)//刀具半径D
{
    return _siemenscnc_828d_decode_to_int(src,dst);
}

static int _siemenscnc_decode_tool_h_length(uint8_t *src,uint8_t *dst)//刀具长度补偿编号
{
    return _siemenscnc_828d_decode_to_int(src,dst);
}

static int _siemenscnc_decode_tool_x_length(uint8_t *src,uint8_t *dst)//长度补偿X
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_tool_z_length(uint8_t *src,uint8_t *dst)//长度补偿Z
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_tool_wear_radius(uint8_t *src,uint8_t *dst)//磨损半径
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_tool_edg(uint8_t *src,uint8_t *dst)//刀沿位置
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_machine_pos(uint8_t *src,uint8_t *dst)//机械坐标
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_relatively_pos(uint8_t *src,uint8_t *dst)//工件坐标 相对坐标
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_remain_pos(uint8_t *src,uint8_t *dst)//剩余坐标
{
    return _siemenscnc_828d_decode_to_double(src,dst);
}

static int _siemenscnc_decode_axis_name(uint8_t *src,uint8_t *dst,uint16_t *length)//轴名称
{
    return _siemenscnc_828d_decode_to_string(src,dst,length);
}

static int _siemenscnc_decode_driver_voltage(uint8_t *src,uint8_t *dst)//母线电压
{
    return _siemenscnc_828d_decode_to_float(src,dst);
}

static int _siemenscnc_decode_driver_current(uint8_t *src,uint8_t *dst)//实际电流
{
    return _siemenscnc_828d_decode_to_float(src,dst);
}

static int _siemenscnc_decode_driver_power(uint8_t *src,uint8_t *dst)//电机功率
{
    return _siemenscnc_828d_decode_to_float(src,dst);
}

static int _siemenscnc_decode_driver_temper(uint8_t *src,uint8_t *dst)//电机温度
{
    return _siemenscnc_828d_decode_to_float(src,dst);
}

static int _siemenscnc_decode_cnc_alarm_no(uint8_t *src,uint8_t *dst)//报警数量
{
    return _siemenscnc_828d_decode_to_int(src,dst);
}

static int _siemenscnc_decode_cnc_alarm(uint8_t *src,uint8_t *dst,uint16_t *length)//NC报警
{
    uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if( src[21] != 0xFF)
        {
            return SIEMENSCNC_RES_SYS_NOT_CARRY;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];

    if(valid_length > 4)
    {
        return SIEMENSCNC_RES_DECODE_ERR_DATATYPE;
    }

    memcpy(dst,src+25,valid_length);
    *length = valid_length;

    return SIEMENSCNC_RES_OK;
}

static int _siemenscnc_decode_r_var(uint8_t *src,uint8_t *dst,uint16_t *length)//R变量
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
}

static int _siemenscnc_decode_s_r_param(uint8_t *src,uint8_t *dst,uint16_t *length)//驱动器R参数
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
    /*uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if(src[21] != 0xFF)
        {
            return SIEMENSCNC_RES_DECODE_ERR;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    if(src[22] == 0x07)//浮点
    {
        memcpy(dst,src+25,valid_length);
        _siemenscnc_swap_data(dst,valid_length);
    }
    else
    {

        uint8_t data[128];
        float f_value;
        memcpy(data,src+25,valid_length);
        switch(valid_length)
        {
        case 1:
            f_value = (float)data[0];
            break;
        case 2:
            f_value = (float)((data[0]<<8)+data[1]);
            break;
        case 4:
            f_value = (float)((data[0]<<24)+(data[1]<<16)+(data[2]<<8)+data[3]);
            printf("hello world--%d\r\n",valid_length);
            break;
        default:
            return SIEMENSCNC_RES_DECODE_ERR;
            break;

        }
        memcpy(dst,(char*)&f_value,sizeof(float));
        //_siemenscnc_swap_data(dst,4);
    }
    return SIEMENSCNC_RES_OK;
    // return _siemenscnc_828d_decode_to_float(src,dst);*/
}

static int _siemenscnc_decode_s_p_param(uint8_t *src,uint8_t *dst, uint16_t *length)//驱动器P参数
{
    return _siemenscnc_828d_decode_to_array(src,dst,length);
    /* uint16_t src_length = (src[2]<<8)+src[3];//总长度
    uint8_t valid_length;//有效数据长度
    if(src_length >= 23)
    {
        if(src[21] != 0xFF)
        {
            return SIEMENSCNC_RES_DECODE_ERR;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    valid_length = (src[23]<<8)+src[24];
    if(src[22] == 0x07)//浮点
    {
        memcpy(dst,src+25,valid_length);
        _siemenscnc_swap_data(dst,valid_length);
    }
    else
    {
        uint8_t data[128];
        float f_value;
        memcpy(data,src+25,valid_length);
        switch(valid_length)
        {
        case 1:
            f_value = (float)data[0];
            break;
        case 2:
            f_value = (float)((data[0]<<8)+data[1]);
            break;
        case 4:
            f_value = (float)((data[0]<<24)+(data[1]<<16)+(data[2]<<8)+data[3]);
            break;
        default:
            return SIEMENSCNC_RES_DECODE_ERR;
            break;

        }
        memcpy(dst,(char*)&f_value,sizeof(float));
    }
    return SIEMENSCNC_RES_OK;
    // return _siemenscnc_828d_decode_to_float(src,dst);*/
}
//检查数据是否读写成功
static int _siemenscnc_828d_check_integrity(const siemenscnc_t *ctx, const uint8_t *msg)
{
    uint16_t msg_length = (msg[2]<<8)+msg[3];//总长度
    if(msg_length >= 19)
    {
        if(msg[17] != 0x00 && msg[18] != 0x00 )
        {
            if (ctx->debug)
            {
                printf("err_code = 0x%02X%02X\r\n",msg[17],msg[18]);
            }

            return SIEMENSCNC_RES_SYS_ERR_CODE;
        }
    }
    else
    {
        return SIEMENSCNC_RES_DECODE_ERR;
    }
    return SIEMENSCNC_RES_OK;
}

/* 计算帧头以后有用数据的长度 */
static uint16_t _siemenscnc_828d_compute_meta_length_after_header(const uint8_t *data)
{
    int length;
    length=(data[2]<<8)+data[3];
    /* 数据长度包含了最前的数据头，因此要减掉 */
    length -= 4;
    return length;
}

/* Builds a TCP request header
    funccode 要执行得功能 axis 轴编号 ， addr 要操作得地址 src_nb 后面data得长度 req 生成得指令 src 要写入得数据
*/
int _siemenscnc_828d_build_request_basis(const siemenscnc_t *ctx,
                                         int function,
                                         unsigned short axis,
                                         unsigned char axis_number,
                                         int addr,
                                         int sub_addr,
                                         int src_nb,
                                         uint8_t *req,
                                         const uint8_t *src)
{
    int ptr = 0;
    switch(function)
    {
    /* 系统相关 */
    case _SIEMENSCNC_SERIAL_NUMBER://硬件序列号
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_SERIAL_NUMBER].command,_siemenscnc_system_command[_SIEMENSCNC_SERIAL_NUMBER].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_SERIAL_NUMBER].command_length;
        break;
    case _SIEMENSCNC_CNC_TYPE://机床类型
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_CNC_TYPE].command,_siemenscnc_system_command[_SIEMENSCNC_CNC_TYPE].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_CNC_TYPE].command_length;
        break;
    case _SIEMENSCNC_VERSION://版本信息
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_VERSION].command,_siemenscnc_system_command[_SIEMENSCNC_VERSION].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_VERSION].command_length;
        break;
    case _SIEMENSCNC_MANUFACTURE_DATE://出厂日期
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_MANUFACTURE_DATE].command,_siemenscnc_system_command[_SIEMENSCNC_MANUFACTURE_DATE].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_MANUFACTURE_DATE].command_length;
        break;
    case _SIEMENSCNC_PROCESS_NUMBER://加工数量
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_PROCESS_NUMBER].command,_siemenscnc_system_command[_SIEMENSCNC_PROCESS_NUMBER].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_PROCESS_NUMBER].command_length;
        break;
    case _SIEMENSCNC_PROCESS_SET_NUMBER://设定的加工数量
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_PROCESS_SET_NUMBER].command,_siemenscnc_system_command[_SIEMENSCNC_PROCESS_SET_NUMBER].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_PROCESS_SET_NUMBER].command_length;
        break;
    case _SIEMENSCNC_RUN_TIME://循环时间,一次自动运行起动时间的累计值
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_RUN_TIME].command,_siemenscnc_system_command[_SIEMENSCNC_RUN_TIME].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_RUN_TIME].command_length;
        break;
    case _SIEMENSCNC_REMAIN_TIME://剩余时间
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_REMAIN_TIME].command,_siemenscnc_system_command[_SIEMENSCNC_REMAIN_TIME].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_REMAIN_TIME].command_length;
        break;
    case _SIEMENSCNC_PROGRAM_NAME://加工程序名
        memcpy(req+ptr,_siemenscnc_system_command[_SIEMENSCNC_PROGRAM_NAME].command,_siemenscnc_system_command[_SIEMENSCNC_PROGRAM_NAME].command_length);
        ptr += _siemenscnc_system_command[_SIEMENSCNC_PROGRAM_NAME].command_length;
        break;
        /* 运行状态相关 */
    case _SIEMENSCNC_OPERATE_MODE://操作模式
        memcpy(req+ptr,__SIEMENSCNC_STATUS_command[_SIEMENSCNC_OPERATE_MODE-_SIEMENSCNC_OPERATE_MODE].command,__SIEMENSCNC_STATUS_command[_SIEMENSCNC_OPERATE_MODE-_SIEMENSCNC_OPERATE_MODE].command_length);
        ptr += __SIEMENSCNC_STATUS_command[_SIEMENSCNC_OPERATE_MODE-_SIEMENSCNC_OPERATE_MODE].command_length;
        break;
    case _SIEMENSCNC_STATUS://运行状态
        memcpy(req+ptr,__SIEMENSCNC_STATUS_command[_SIEMENSCNC_STATUS-_SIEMENSCNC_OPERATE_MODE].command,__SIEMENSCNC_STATUS_command[_SIEMENSCNC_STATUS-_SIEMENSCNC_OPERATE_MODE].command_length);
        ptr += __SIEMENSCNC_STATUS_command[_SIEMENSCNC_STATUS-_SIEMENSCNC_OPERATE_MODE].command_length;
        break;
        /* 轴相关 */
    case _SIEMENSCNC_AXIS_NAME://轴名称
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_AXIS_NAME-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_AXIS_NAME-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = axis >> 8;//设置轴编号 0x01开始顺延
        req[26] = axis & 0xFF;//设置轴编号 0x01开始顺延
        //req[28] = axis_number;//设置读取的轴的数量
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_AXIS_NAME-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_MACHINE_POS://机械坐标
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_MACHINE_POS-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_MACHINE_POS-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = axis >> 8;//设置轴编号 0x01开始顺延
        req[26] = axis & 0xFF;//设置轴编号 0x01开始顺延
        //req[28] = axis_number;//设置读取的轴的数量
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_MACHINE_POS-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_RELATIVELY_POS://工件坐标 相对坐标
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_RELATIVELY_POS-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_RELATIVELY_POS-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = axis >> 8;//设置轴编号 0x01开始顺延
        req[26] = axis & 0xFF;//设置轴编号 0x01开始顺延
        //req[28] = axis_number;//设置读取的轴的数量
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_RELATIVELY_POS-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_REMAIN_POS://剩余坐标
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_REMAIN_POS-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_REMAIN_POS-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = axis >> 8;//设置轴编号 0x01开始顺延
        req[26] = axis & 0xFF;//设置轴编号 0x01开始顺延
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_REMAIN_POS-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_FEED_SET_SPEED://设定进给速度
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_FEED_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_FEED_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_FEED_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_FEED_ACT_SPEED://实际进给速度
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_FEED_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_FEED_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_FEED_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_FEED_RATE://进给倍率
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_FEED_RATE-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_FEED_RATE-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_FEED_RATE-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_SPINDLE_SET_SPEED://主轴设定速度
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_SET_SPEED-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_SPINDLE_ACT_SPEED://主轴实际速度
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_ACT_SPEED-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case _SIEMENSCNC_SPINDLE_RATE://主轴倍率
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_RATE-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_RATE-_SIEMENSCNC_AXIS_NAME].command_length);
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_SPINDLE_RATE-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case  _SIEMENSCNC_G_COORDINATE_T://G工件坐标系 T
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_T-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_T-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = addr >> 8;//Gcode
        req[26] = addr & 0xFF;//Gcode
        req[23] = axis >> 8;//设置轴编号 0x01开始顺延
        req[24] = axis & 0xFF;//设置轴编号 0x01开始顺延
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_T-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
    case  _SIEMENSCNC_G_COORDINATE_M://G工件坐标系 M
        memcpy(req+ptr,_siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_M-_SIEMENSCNC_AXIS_NAME].command,_siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_M-_SIEMENSCNC_AXIS_NAME].command_length);
        req[25] = addr >> 8;//Gcode
        req[26] = addr & 0xFF;//Gcode
        req[23] = axis >> 8;//设置轴编号 0x01开始顺延
        req[24] = axis & 0xFF;//设置轴编号 0x01开始顺延
        //req[28] = axis_number;//设置读取的轴的数量
        ptr += _siemenscnc_axis_command[_SIEMENSCNC_G_COORDINATE_M-_SIEMENSCNC_AXIS_NAME].command_length;
        break;
        /* 刀具相关 */
    case _SIEMENSCNC_TOOL_CUR_NAME://当前刀具名
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_NAME-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_NAME-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_NAME-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
    case _SIEMENSCNC_TOOL_CUR_T_NO://当前刀具号
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_T_NO-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_T_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_T_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
        break;
    case _SIEMENSCNC_TOOL_CUR_D_NO://当前刀沿号
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_D_NO-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_D_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_D_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
    case _SIEMENSCNC_TOOL_CUR_H_NO://当前刀具H号
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_H_NO-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_H_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_CUR_H_NO-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
    case _SIEMENSCNC_TOOL_LIST://刀具列表
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_LIST-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_LIST-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        req[23] = addr >> 8;//刀具列表的行号
        req[24] = addr & 0xFF;
        req[25] = sub_addr >> 8;//刀具列表的列号
        req[26] = sub_addr & 0xFF;
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_LIST-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;

    case _SIEMENSCNC_TOOL_X_OFFSET://X补偿
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_X_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_X_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_X_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
    case _SIEMENSCNC_TOOL_Z_OFFSET://Z补偿
        memcpy(req+ptr,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_Z_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command,_siemenscnc_tool_command[_SIEMENSCNC_TOOL_Z_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command_length);
        ptr += _siemenscnc_tool_command[_SIEMENSCNC_TOOL_Z_OFFSET-_SIEMENSCNC_TOOL_CUR_NAME].command_length;
        break;
        /* 报警相关 */
    case _SIEMENSCNC_CNC_NC_ALARM_NO://NC报警数量
        memcpy(req+ptr,_siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM_NO-_SIEMENSCNC_CNC_NC_ALARM_NO].command,_siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM_NO-_SIEMENSCNC_CNC_NC_ALARM_NO].command_length);
        ptr += _siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM_NO-_SIEMENSCNC_CNC_NC_ALARM_NO].command_length;
        break;
    case _SIEMENSCNC_CNC_NC_ALARM://NC报警
        memcpy(req+ptr,_siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM-_SIEMENSCNC_CNC_NC_ALARM_NO].command,_siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM-_SIEMENSCNC_CNC_NC_ALARM_NO].command_length);
        req[25] = axis>>8;//第几条报警
        req[26] = axis&0xff;
        ptr += _siemenscnc_alarm_command[_SIEMENSCNC_CNC_NC_ALARM-_SIEMENSCNC_CNC_NC_ALARM_NO].command_length;
        break;
        /* 参数相关 */
    case _SIEMENSCNC_R_VAR://R变量
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_R_VAR-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_R_VAR-_SIEMENSCNC_R_VAR].command_length);
        req[25] = addr>>8;
        req[26] = addr&0xff;
        ptr += _siemenscnc_param_command[_SIEMENSCNC_R_VAR-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_S_R_P_PARAM://驱动器R参数
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_S_R_P_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_S_R_P_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[22] = 0xa1+axis;//选择驱动器
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_S_R_P_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_GENERAL_MACHINE_PARAM://通用机床数据
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_GENERAL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_GENERAL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_GENERAL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_CHANNEL_MACHINE_PARAM://通道机床数据
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_CHANNEL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_CHANNEL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_CHANNEL_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_AXIS_MACHINE_PARAM://轴机床数据
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_AXIS_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_AXIS_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[22] = 0x61+axis;//选择轴
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_AXIS_MACHINE_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_CONTROL_UNIT_PARAM://控制单元参数
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_CONTROL_UNIT_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_CONTROL_UNIT_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_CONTROL_UNIT_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    case _SIEMENSCNC_POWER_UNIT_PARAM://电源模块参数
        memcpy(req+ptr,_siemenscnc_param_command[_SIEMENSCNC_POWER_UNIT_PARAM-_SIEMENSCNC_R_VAR].command,_siemenscnc_param_command[_SIEMENSCNC_POWER_UNIT_PARAM-_SIEMENSCNC_R_VAR].command_length);
        req[23] = addr>>8;//参数地址
        req[24] = addr&0xff;//参数地址
        req[25] = sub_addr >> 8;//参数地址下标
        req[26] = sub_addr&0xff;//参数地址下标
        ptr += _siemenscnc_param_command[_SIEMENSCNC_POWER_UNIT_PARAM-_SIEMENSCNC_R_VAR].command_length;
        break;
    default:
        return SIEMENSCNC_RES_COMMAN_ERR;
        break;
    }
    /* 加上这句CNC就不会不断的回应了 同步收发 */
    memcpy(req+ptr,_siemenscnc_sync,7);
    ptr += 7;
    return ptr;
}

int _siemenscnc_828d_receive_msg(const siemenscnc_t *ctx, uint8_t *msg, uint16_t *length)
{
    int rc;
    fd_set rfds;
    struct timeval tv;
    struct timeval *p_tv;
    int length_to_read;
    int msg_length = 0;
    _siemenscnc_step_t step;


    /* Add a file descriptor to the set */
    FD_ZERO(&rfds);
    FD_SET(ctx->s, &rfds);

    /* We need to analyse the message step by step.  At the first step, we want
     * to reach the function code because all packets contain this
     * information. */
    step = _SIEMENSCNC_STEP_HEADER;
    length_to_read = ctx->backend->header_length;

    tv.tv_sec = ctx->response_timeout.tv_sec;
    tv.tv_usec = ctx->response_timeout.tv_usec;
    p_tv = &tv;


    while (length_to_read != 0) {
        rc = ctx->backend->select(ctx, &rfds, p_tv, length_to_read);
        if (rc == -1) {
            //_error_print(ctx, "select");
            if (ctx->error_recovery & SIEMENSCNC_ERROR_RECOVERY_PROTOCOL) {
                int saved_errno = errno;

                if (errno == ETIMEDOUT) {
                    //_sleep_and_flush(ctx);
                } else if (errno == EBADF) {
                    siemenscnc_close(ctx);
                    siemenscnc_connect(ctx);
                }
                errno = saved_errno;
            }
            return SIEMENSCNC_RES_TIMEOUT;
        }

        rc = ctx->backend->recv(ctx, msg + msg_length, length_to_read);
        if (rc == 0) {
            errno = ECONNRESET;
            rc = SIEMENSCNC_RES_TIMEOUT;
        }

        if (rc == -1) {
            //_error_print(ctx, "read");
            if ((ctx->error_recovery & SIEMENSCNC_ERROR_RECOVERY_PROTOCOL) &&
                    (errno == ECONNRESET || errno == ECONNREFUSED ||
                     errno == EBADF)) {
                int saved_errno = errno;
                siemenscnc_close(ctx);
                siemenscnc_connect(ctx);
                /* Could be removed by previous calls */
                errno = saved_errno;
            }
            return SIEMENSCNC_RES_TIMEOUT;
        }

        /* Display the hex code of each character received */
        if (ctx->debug) {
            int i;
            printf("recive:");
            for (i=0; i < rc; i++)
                printf("<%.2X>", msg[msg_length + i]);
        }

        /* Sums bytes received */
        msg_length += rc;
        /* Computes remaining bytes */
        length_to_read -= rc;

        if (length_to_read == 0) {
            switch (step) {
            case _SIEMENSCNC_STEP_HEADER:
                /* Function code position */
                length_to_read = _siemenscnc_828d_compute_meta_length_after_header(msg);
                if(length_to_read > ctx->backend->max_adu_length)
                {
                    //errno = EMBBADDATA;
                    //_error_print(ctx, "too many data");
                    _siemenscnc_tcp_flush(ctx);
                    return SIEMENSCNC_RES_REV_TOO_LONG;
                }
                if (length_to_read != 0) {
                    step = _SIEMENSCNC_STEP_META;
                    break;
                } /* else switches straight to the next step */
            case _SIEMENSCNC_STEP_META:
                if ((msg_length + length_to_read) > ctx->backend->max_adu_length) {
                    //errno = EMBBADDATA;
                    //_error_print(ctx, "too many data");
                    return SIEMENSCNC_RES_REV_TOO_LONG;
                }
                step = _SIEMENSCNC_STEP_DATA;
                break;
            default:
                break;
            }
        }

        if (length_to_read > 0 && ctx->byte_timeout.tv_sec != -1) {
            /* If there is no character in the buffer, the allowed timeout
               interval between two consecutive bytes is defined by
               byte_timeout */
            tv.tv_sec = ctx->byte_timeout.tv_sec;
            tv.tv_usec = ctx->byte_timeout.tv_usec;
            p_tv = &tv;
        }
    }

    if (ctx->debug)
        printf("\n");

    *length = msg_length;


    return _siemenscnc_828d_check_integrity(ctx,msg);
}


static int _siemenscnc_828d_decode_msg(const siemenscnc_t *ctx, uint8_t function, const uint8_t *msg, uint8_t *dest, uint16_t *length)
{
    int rc;
    switch(function)
    {
    /* 系统相关 */
    case _SIEMENSCNC_SERIAL_NUMBER://硬件序列号
        rc = _siemenscnc_decode_serial_number(msg,dest,length);
        break;
    case _SIEMENSCNC_CNC_TYPE://机床类型
        rc = _siemenscnc_decode_cnc_type(msg,dest,length);
        break;
    case _SIEMENSCNC_VERSION://版本信息
        rc = _siemenscnc_decode_cnc_type(msg,dest,length);
        break;
    case _SIEMENSCNC_MANUFACTURE_DATE://出厂日期
        rc = _siemenscnc_decode_manufacture_data(msg,dest,length);
        break;
    case _SIEMENSCNC_PROCESS_NUMBER://加工数量
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_PROCESS_SET_NUMBER://设定的加工数量
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_RUN_TIME://循环时间,一次自动运行起动时间的累计值
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_REMAIN_TIME://剩余时间
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_PROGRAM_NAME://加工程序名
        rc = _siemenscnc_decode_program_name(msg,dest,length);
        break;
        /* 状态相关 */
    case _SIEMENSCNC_OPERATE_MODE://操作模式
        rc = _siemenscnc_decode_operate_mode(msg,dest);
        break;
    case _SIEMENSCNC_STATUS://运行状态
        rc = _siemenscnc_decode_status(msg,dest);
        break;
        /* 轴相关 */
    case _SIEMENSCNC_AXIS_NAME://轴名称
        rc = _siemenscnc_decode_axis_name(msg,dest,length);
        break;
    case _SIEMENSCNC_MACHINE_POS://机械坐标
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_RELATIVELY_POS://工件坐标 相对坐标
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_REMAIN_POS://剩余坐标
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_FEED_SET_SPEED://设定进给速度
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_FEED_ACT_SPEED://实际进给速度
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_FEED_RATE://进给倍率
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_SPINDLE_SET_SPEED://主轴设定速度
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_SPINDLE_ACT_SPEED://主轴实际速度
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_SPINDLE_RATE://主轴倍率
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
        /*case _SIEMENSCNC_DRIVE_VOLTAGE://母线电压
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_DRIVER_CURRENT://实际电流
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_DRIVER_POWER://电机功率
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_DRIVER_TEMPER://电机温度
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;*/
    case  _SIEMENSCNC_G_COORDINATE_T://G工件坐标系 T
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case  _SIEMENSCNC_G_COORDINATE_M://G工件坐标系 M
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
        /* 刀具相关 */
    case _SIEMENSCNC_TOOL_CUR_NAME://当前刀具名
        rc = _siemenscnc_decode_tool_name(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_CUR_T_NO://当前刀具T号
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_CUR_D_NO://当前刀具D刀沿号
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_CUR_H_NO://当前刀具H号
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_LIST://刀具列表
        rc = _siemenscnc_decode_tool_list(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_X_OFFSET://X补偿
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_TOOL_Z_OFFSET://Z补偿
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;

        /* 报警相关 */
    case _SIEMENSCNC_CNC_NC_ALARM_NO://NC报警数量
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_CNC_NC_ALARM://NC报警
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
        /* 参数相关 */
    case _SIEMENSCNC_R_VAR://R变量
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_S_R_P_PARAM://驱动器R P参数
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_GENERAL_MACHINE_PARAM://通用机床数据
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_CHANNEL_MACHINE_PARAM://通道机床数据
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_AXIS_MACHINE_PARAM://轴机床数据
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_CONTROL_UNIT_PARAM://控制单元参数
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    case _SIEMENSCNC_POWER_UNIT_PARAM://电源模块参数
        rc = _siemenscnc_828d_decode_to_array(msg,dest,length);
        break;
    default:
        return SIEMENSCNC_RES_COMMAN_ERR;
        break;
    }
    return rc;
}

/* 建立连接 */
int _siemenscnc_828d_connect(siemenscnc_t *ctx)
{
    int rc = 0;
    uint8_t rsq[256];
    int rsp_length =0 ;
    if(_siemenscnc_tcp_connect(ctx) < 0)
    {
        return -1;
    }
    /* 开始第一次握手 */
    rc = _siemenscnc_tcp_send(ctx,_siemenscnc_hand_shark1,0x0016);
    if (ctx->debug) {
        int i;
        printf("send");
        for (i=0; i < 0x0016; i++)
            printf("<%.2X>", _siemenscnc_hand_shark1[i]);
        printf("\n");
    }
    if(rc > 0)
    {
        rc = _siemenscnc_828d_receive_msg(ctx,rsq,&rsp_length);
        if(rc == -1)
        {
            _siemenscnc_tcp_close(ctx);
            return -1;
        }
        else if(rsp_length != 0x0016 )
        {
            _siemenscnc_tcp_close(ctx);

            return -1;
        }
    }
    else
    {
        _siemenscnc_tcp_close(ctx);
        return -1;
    }
    /* 第二次握手 */
    rc = _siemenscnc_tcp_send(ctx,_siemenscnc_hand_shark2,0x0019);
    if (ctx->debug) {
        int i;
        printf("send:");
        for (i=0; i < 0x0019; i++)
            printf("<%.2X>", _siemenscnc_hand_shark2[i]);
        printf("\n");
    }
    if(rc > 0)
    {
        rc = _siemenscnc_828d_receive_msg(ctx,rsq,&rsp_length);
        if(rc == -1)
        {
            _siemenscnc_tcp_close(ctx);
            return -1;
        }
        else if(rsp_length != 0x001b )
        {
            _siemenscnc_tcp_close(ctx);
            return -1;
        }
    }
    else
    {
        _siemenscnc_tcp_close(ctx);
        return -1;
    }
    /* 第三次握手 */
    rc = _siemenscnc_tcp_send(ctx,_siemenscnc_hand_shark3,0x001d);
    if (ctx->debug) {
        int i;
        printf("send");
        for (i=0; i < 0x001b; i++)
            printf("<%.2X>", _siemenscnc_hand_shark3[i]);
        printf("\n");
    }
    if(rc > 0)
    {
        rc = _siemenscnc_828d_receive_msg(ctx,rsq,&rsp_length);
        if(rc == -1)
        {
            _siemenscnc_tcp_close(ctx);
            return -1;
        }
        else if(rsp_length != 0x001b )
        {
            _siemenscnc_tcp_close(ctx);
            return -1;
        }
    }
    else
    {
        _siemenscnc_tcp_close(ctx);
        return -1;
    }
    return 0;
}


const siemenscnc_backend_t _siemenscnc_828d_backend = {
    SIEMENSCNC_HEADER_LENGTH,
    SIEMENSCNC_MAX_ADU_LENGTH,
    _siemenscnc_828d_build_request_basis,
    _siemenscnc_828d_receive_msg,
    _siemenscnc_828d_decode_msg,
    _siemenscnc_tcp_send,
    _siemenscnc_tcp_recv,
    _siemenscnc_828d_connect,
    _siemenscnc_tcp_close,
    _siemenscnc_tcp_flush,
    _siemenscnc_tcp_select
};

/*
*********************************************************************************************************
*	函 数 名: siemenscnc_new_3e_binary
*	功能说明: 初始化3e_binary协议
*	形    参: const char *ip_address, int port
*	返 回 值: siemenscnc_t*
*********************************************************************************************************
*/
siemenscnc_t *siemenscnc_828d_new(const char *ip, int port)
{
    siemenscnc_t *ctx;
    siemenscnc_tcp_t *ctx_tcp;
    int dest_size;
    int ret_size;
    ctx = (siemenscnc_t *) malloc(sizeof(siemenscnc_t));
    if(ctx == NULL)
    {
        return NULL;
    }
    _siemenscnc_init_common(ctx);

    ctx->backend = &(_siemenscnc_828d_backend);
    ctx->backend_data = (siemenscnc_tcp_t *) malloc(sizeof(siemenscnc_tcp_t));
    if(ctx->backend_data == NULL)
    {
        free(ctx);
        return NULL;
    }
    ctx_tcp =  (siemenscnc_tcp_t *)ctx->backend_data;
    dest_size = sizeof(char) * 16;
    ret_size = _siemenscnc_strlcpy(ctx_tcp->ip, ip, dest_size);
    if (ret_size == 0) {
        fprintf(stderr, "The IP string is empty\n");
        siemenscnc_free(ctx);
        errno = EINVAL;
        return NULL;
    }

    if (ret_size >= dest_size) {
        fprintf(stderr, "The IP string has been truncated\n");
        siemenscnc_free(ctx);
        errno = EINVAL;
        return NULL;
    }

    ctx_tcp->port = port;

    return ctx;
}
