/*
simens PLC S7 驱动。
在初始化中后来解析出来的:
物理块：DBn、I、Q等


地址表示中，西门子无法表示小数，小数（32位，REAL）都是用DBD表示的。
西门子地址：DB1.DBD 5，DB1.DBW 0,其中DB1为块名，DBD为数据类型（DWORD） 5为块内起始地址（字节），从0开始
允许的地址：
数据类型：
实例(英语助记符)：

输入 | 输出 | 位存储器
BOOL
I 1.0 | Q 1.7 | M 10.1

输入 | 输出 | 位存储器
BYTE
IB 1 | QB 10 | MB 100

输入 | 输出 | 位存储器
WORD
IW 1 | QW 10 | MW 100

输入 | 输出 | 位存储器
DWORD
ID 1 | QD 10 | MD 100

I/O (输入 | 输出)
BYTE
PIB 0 | PQB 1

I/O (输入 | 输出)
WORD
PIW 0 | PQW 1

I/O (输入 | 输出)
DWORD
PID 0 | PQD 1

定时器,32位
TIMER
T 1

计数器，32位
COUNTER
C1

数据块
BOOL
DB1.DBX 1.0

数据块
BYTE
DB1.DBB 1

数据块
WORD
DB1.DBW 1

数据块
DWORD
DB1.DBD 1

bool 布尔量 就是开关量 只有 0，1
byte 字节
word 字 16位整数
dword 双字 32位整数
int 带符号16位整数
dint 带符号32位整数
real 浮点数 实数 32位
字符串（STRING）是由最多254个字符组成的一维数组。
4、日期和时间（DATE-AND-TIME）  用于存储年、月、日、时、分、秒、毫秒和星期的数据。占用8个字节，BCD编码。星期天代码为1，星期一～星期六代码分别是2～7。
*/

#pragma once
#include "pkdriver/pkdrvcmn.h"
#include "nodave.h"
#include "AutoGroup_BlkDev.h"

#define SIMENS_PLC_TYPE_200			1	
#define SIMENS_PLC_TYPE_300			2	
#define SIMENS_PLC_TYPE_400			3	
#define SIMENS_PLC_TYPE_1200		4
#define SIMENS_PLC_TYPE_UNKNOWN		0

// 数据块的类型。和各个数据块的开始数字相同
#define  BLOCK_TYPE_UNDEFINED			"undef"		// 未定义
#define  BLOCK_TYPE_DB					"DB"		// 数据块
#define  BLOCK_TYPE_I					"I"			// 输入
#define  BLOCK_TYPE_Q					"Q"			// 输出
#define  BLOCK_TYPE_P					"P"			//??
#define  BLOCK_TYPE_PI					"PI"		// IO输入
#define  BLOCK_TYPE_PO					"PO"		// IO输出
#define  BLOCK_TYPE_MEMORY				"M"			// 位寄存器
#define  BLOCK_TYPE_COUNTER				"C"			// 计数器
#define  BLOCK_TYPE_TIMER				"T"			// 定时器
#define  BLOCK_TYPE_V					"V"			// 定时器
#define  BLOCK_TYPE_FLAGS				"F"			// Flags
#define  BLOCK_TYPE_DI					"DI"		// instance data blocks 

#define  BLOCK_TYPE_LOCAL				"LC"			// S5 Local
#define  BLOCK_TYPE_S7_200_AI			"AI"			// analog inputs of 200 family
#define  BLOCK_TYPE_S7_200_AO			"AO"			// analog outputs of 200 family 
#define  BLOCK_TYPE_S7_200_SYSINFO		"SI"			// System info of 200 family
#define  BLOCK_TYPE_S5_SYSDATA			"SD"			// BLOCK_TYPE_SYSDATAS5
#define  BLOCK_TYPE_S5_RAWMEMORY		"RM"			// S5 Raw memory

#define	DEFAULT_REQUEST_MAXLEN			1024
#define	DEFAULT_RESPONSE_MAXLEN			1024
#define	BITS_PER_BYTE					8
#define	PK_DATABLOCKTYPE_MAXLEN			16

#define SIMENS_S7_PROTOCOL_TCP			daveProtoISOTCP		// 通过网口通讯
#define SIMENS_S7_PROTOCOL_MPI			daveProtoMPI		// 通过DB9针RS485通讯的MPI
#define SIMENS_S7_PROTOCOL_PPI			daveProtoPPI		// 通过DB9针RS485通讯的PPI协议

#define NUSERDATA_INDEX_S7DEVICE			0	// 参数0表示对应的设备指针
#define PUSERDATA_INDEX_TAGGROUP			0	// 变量组
/*
驱动根据地址的连续情况自动计算合适的数据块大小。数据块的长度需与PLC程序中实际开辟的数据块长度匹配，不然数据读取失败。
例如对于寄存器是DB类型的来说，s7-400长度限制416, s7-300长度限制188。
对于其他寄存器类型的数据块长度一般会小于DB类型，具体长度需要查阅西门子相关手册。
*/
#define MAX_BLOCK_BYTE_NUM			188	// 经实证，西门子每次可以最多读取238个

class CS7Device
{
public:
	PKDEVICE *	m_pDevice;
	string		m_strPLCType;
	int			m_nPLCType;
	int			m_nProtocolNo;	// 协议，目前用的都是
	int			m_nMPIAddress; // MPI地址？
	int			m_nRackNo;		// 机架号，缺省为0，从0开始
	int			m_nSlotNo;		// 槽号，缺省为1，从1开始？S7-200 Smart型号的槽号为1
	unsigned short m_uTransID;
	bool		m_bNeedClearRecvBuffer; // 是否需要清空接收缓冲区
public:
	_daveOSserialType m_nfd;
	daveInterface	* m_pDaveInterface;
	daveConnection *  m_pDaveConnection;
	bool		m_bBigEndian;
public:
	CS7Device(PKDEVICE *pDevice);
	virtual ~CS7Device();

public:
	int		OnInitDevice(PKDEVICE *pDevice);
	int		OnTimer_ReadData(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
	int		OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId);
	int		UpdateTagDataWithByteOrder(PKDEVICE *pDevice, PKTAG *pTag, char *szDataBuff, int nDataBuffLen);
	bool	isBigEndian();
public:
	void CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess);
	int	GetDaveBlockInfo(int nPLCType, char *szBlockName, int *pnDaveAreaType, int *pnIndex);
	long UpdateGroupData_Simens(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *szBuffer, long lBufLen, short nStatus);
	int CheckRequestPDU(); // 检查设备请求数据前是否已经取到PDU大小了
};