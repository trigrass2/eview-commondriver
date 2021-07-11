
#ifndef _ABCIP_DRIVER_H_
#define _ABCIP_DRIVER_H_

#include "pkdriver/pkdrvcmn.h"
#include <vector>
#include <string.h>
#include "ace/Basic_Types.h"
#include "AutoGroup_BlkDev.h"
using namespace std;

#define COMMAND_REGISTERSESSION		0x0065
#define COMMAND_UNREGISTERSESSION	0x0066
#define COMMAND_SENDRRDATA			0x006F
#define COMMAND_SENDUNITDATA		0x0070

#define SERVICE_FWDOPEN				0x54
#define SERVICE_FWDOPENREPLY		0xD4
#define SERVICE_READFRAGDATA		0x52	// Logix5000用该服务将多个读命令组织到一个MultiRequest（0A）中
#define SERVICE_WRITEFRAGDATA		0x53
#define SERVICE_READMODIFYWRITE		0x4E
#define SERVICE_READDATA			0x4C
#define SERVICE_WRITEDATA			0x4D	// single write
#define SERVICE_MULTIREQUEST		0x0A
#define SERVICE_MULTIREQUESTREPLY	0x8A

#define SERVICE_UNCONNECTED_SEND	0x52	// CIP Connection Manager Service = Unconnected Send SLC500用该服务包装4C（ReadData）


#define ITEMNO_ADDRESS_NULL						0x0		// 项ID, 项类型:地址, 无地址,长度:0，不用
#define ITEMNO_ADDRESS_SEQUENCE					0x8002	// 项ID, 项类型:地址, 序列地址？ 不用
#define ITEMNO_ADDRESS_CONNECTION_BASED			0xA1	// 项ID, 项类型:地址, 面向连接,Connected address item,长度:4字节
#define ITEMNO_DATA_CONNECTED_TRANSPORT_PACKET	0xB1	// 项ID, 项类型:数据, 面向连接的传输包
#define ITEMNO_DATA_UNCONNECTED_MESSAGE			0xB2	// 项ID, 项类型:数据, 无连接消息

#define SENDCONTEXT_BYTE1_READ		0x01
#define SENDCONTEXT_BYTE1_WRITE		0x02

#define CIP_VENDOR_ID_ROCKWELL		0x574B	// CIP厂家代码，罗克韦尔，推测
#define CIP_TRANSPORTCLASS_TRAIGGER_APPLICATION	0xA3	// 0xA3, Server transport, class 3, application trigger, 应用程序调用请求数据

#define PACKET_HEADER_LEN			24	// sizeof(CIPHEADER)

#define ABPLC_MODEL_LOGIX5000		5000		// LOGIX5000
#define ABPLC_MODEL_SLC500			500			// SLC500
#define ABPLC_MODEL_UNKNOWN			0			// 未知

bool is_little_endian();
void to_normal(void* src, size_t size);
void to_little_endian(void* src, size_t size);

#pragma pack(1)  // “对齐系数”改为1字节对其
#define PACKED
//#ifdef _WIN32
//#pragma pack(push, 1) // “对齐系数”改为1字节对其
//#define PACKED 
//#else
//#define PACKED __attribute__((__packed__))
//#endif
// 所有CIP请求和应答数据包的头部，不包含命令相关数据
// 长度：24个字节
typedef struct _CIPHEADER
{
	ACE_UINT16 PACKED 	uCommand;			// 命令类型，注册应为：0x0065
	ACE_UINT16 PACKED	uCommandDataLen;		// 长度，数据去长度为4个字节。
	ACE_UINT32 PACKED	ulSessionHandle;	// 由目标产生返回给源端
	ACE_UINT32 PACKED	ulStatus;			// 状态，指明接收者是否能执行请求的封装命令。0表示成功
	ACE_UINT8  PACKED	bySendContext[8];	// 任何发送者内容，会在应答时原样返回，可以用作事务号等
	ACE_UINT32 PACKED	ulOptions;			// 0
	_CIPHEADER()
	{
		memset(this, 0, sizeof(_CIPHEADER));
	}

	void to_normal()
	{
		::to_normal(&uCommand, sizeof(ACE_UINT16));
		::to_normal(&uCommandDataLen, sizeof(ACE_UINT16));
		::to_normal(&ulSessionHandle, sizeof(ACE_UINT32));
		::to_normal(&ulStatus, sizeof(ACE_UINT32));
		::to_normal(&ulOptions, sizeof(ACE_UINT32));
	}

	void to_little_endian()
	{
		::to_little_endian(&uCommand, sizeof(ACE_UINT16));
		::to_little_endian(&uCommandDataLen, sizeof(ACE_UINT16));
		::to_little_endian(&ulSessionHandle, sizeof(ACE_UINT32));
		::to_little_endian(&ulStatus, sizeof(ACE_UINT32));
		::to_little_endian(&ulOptions, sizeof(ACE_UINT32));
	}
}CIPHEADER;

// 注册会话(RegisterSession)请求及应答的结构。请求和应答的结构相同
// 总长度应为：24+4=28个字节
typedef struct tagRegisterSessionStruct
{
	CIPHEADER cipHeader;

	// 下面是命令有关的数据
	ACE_UINT16 PACKED  urotocolVersion;		//  请求协议版本应该设置为1
	ACE_UINT16 PACKED	uOptionFlag;		// 会话选项应设置为0，位0-7被保留用于异常（RA），位8-15被保留用于将来扩展

	tagRegisterSessionStruct()
	{
		cipHeader.uCommand = COMMAND_REGISTERSESSION; // 0x65;
		cipHeader.uCommandDataLen = 0x04; // 注册固定为4个字节
		urotocolVersion = 0x01;
		uOptionFlag = 0x00;
	}

	void to_normal()
	{
		cipHeader.to_normal();
		::to_normal(&urotocolVersion, sizeof(ACE_UINT16));
		::to_normal(&uOptionFlag, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		cipHeader.to_normal();
		::to_little_endian(&urotocolVersion, sizeof(ACE_UINT16));
		::to_little_endian(&uOptionFlag, sizeof(ACE_UINT16));
	}
}RegisterSessionStruct;

// 注销(UnRegisterSession)会话请求的结构.该请求不需要应答
// 总长度：24个字节
typedef struct tagUnRegisterSessionStruct
{
	CIPHEADER cipHeader;

	tagUnRegisterSessionStruct()
	{
		cipHeader.uCommand = COMMAND_UNREGISTERSESSION; //; 0x66;
		cipHeader.uCommandDataLen = 0;
	}

	void to_normal()
	{
		cipHeader.to_normal();
	}

	void to_little_endian()
	{
		cipHeader.to_little_endian();
	}
}UnRegisterSessionStruct;


// SENDRRDATA Command common packet format
typedef struct tagSendRRDataStruct
{
	// common，24个字节
	CIPHEADER cipHeader;

	// 命令特定数据
	ACE_UINT32 PACKED	ulInterfaceHandle;	// 0(CIP)，接口句柄
	ACE_UINT16 PACKED	uOpTimeOut;			// 运行超时

	// Item (address item and data item)。下面是封装数据包，参见公共数据包格式规范。8个字节？？
	ACE_UINT16 PACKED	uItemCount;
	ACE_UINT16 PACKED	uAddressType;
	ACE_UINT16 PACKED	uAddressLen;
	ACE_UINT16 PACKED	uDataType;
	ACE_UINT16 PACKED	uDataItemLen;

	tagSendRRDataStruct()
	{
		cipHeader.uCommand = COMMAND_SENDRRDATA;//0x6F;
		cipHeader.uCommandDataLen = 0x10; // 16个字节

		ulInterfaceHandle = 0;
		uOpTimeOut = 0x0A00; // B1:0A ,B2:00

		uItemCount = 0x02;
		uAddressType = 0x00;
		uAddressLen = 0x00;
		uDataType = ITEMNO_DATA_UNCONNECTED_MESSAGE; // 0xB2; // ????
		uDataItemLen = 0x32; // ????
	};

	void to_normal()
	{
		cipHeader.to_normal();

		::to_normal(&ulInterfaceHandle, sizeof(ACE_UINT32));
		::to_normal(&uOpTimeOut, sizeof(ACE_UINT16));
		::to_normal(&uItemCount, sizeof(ACE_UINT16));
		::to_normal(&uAddressType, sizeof(ACE_UINT16));
		::to_normal(&uAddressLen, sizeof(ACE_UINT16));
		::to_normal(&uDataType, sizeof(ACE_UINT16));
		::to_normal(&uDataItemLen, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		cipHeader.to_little_endian();

		::to_little_endian(&ulInterfaceHandle, sizeof(ACE_UINT32));
		::to_little_endian(&uOpTimeOut, sizeof(ACE_UINT16));
		::to_little_endian(&uItemCount, sizeof(ACE_UINT16));
		::to_little_endian(&uAddressType, sizeof(ACE_UINT16));
		::to_little_endian(&uAddressLen, sizeof(ACE_UINT16));
		::to_little_endian(&uDataType, sizeof(ACE_UINT16));
		::to_little_endian(&uDataItemLen, sizeof(ACE_UINT16));
	}
}SendRRDataStruct;

// Service OpenRequest Struct
// 前面还有40个字节
typedef struct tagFwdOpenRequestStruct
{
	SendRRDataStruct	sendRRData;			// 命令头部，40个字节
	// Message Router Header
	ACE_UINT8 PACKED	byServiceCode;		// 服务代码
	ACE_UINT8 PACKED	byRequestPathSize;	// 请求路径尺寸
	ACE_UINT32 PACKED	ulEPath;			// ??

	// Connection Parameters
	ACE_UINT8 PACKED	byTicks;			// ?
	ACE_UINT8 PACKED	byTimeOutTicks;		// 超时
	ACE_UINT32 PACKED	ulO2TConnID;		// 源->目标的连接ID（程序序号）
	ACE_UINT32 PACKED	ulT2OConnID;		// 目标设备返回的到源的连接ID，请求中为0
	ACE_UINT16 PACKED	uT2OConnSN;			// 目标设备返回的连接序列号
	ACE_UINT16 PACKED	uOVendorID;			// 目标设备的厂家ID
	ACE_UINT32 PACKED	ulOSN;				// 序列号
	ACE_UINT8 PACKED	byConnTimeOutMultiplier; // 连接超时倍乘因子
	ACE_UINT8 PACKED	byReserved[3];		// 保留3个字节
	ACE_UINT32 PACKED	ulO2TRPI;			// ？？
	ACE_UINT16 PACKED	uO2TConnParams;		// 源到目标的连接参数
	ACE_UINT32 PACKED	ulT2ORPI;			// 目标到源的？
	ACE_UINT16 PACKED	uT2OConnParams;		// 目标设备返回的连接参数
	ACE_UINT8 PACKED	byTransportClassTrigger;	// 传输触发时机（轮询、定时、主动上报。。）APPLICATION, 0xA3
	ACE_UINT8 PACKED	byConnPathSize;		// 连接路径大小. Number of  words(16 bit) in Connection Path
	ACE_UINT8 PACKED	byPaddedEPath[8];
	tagFwdOpenRequestStruct()
	{
		// memset(this, 0, sizeof(tagFwdOpenRequestStruct)); 会覆盖cipheader内容因此不能调用
		byServiceCode = SERVICE_FWDOPEN;// 0x54;
		byRequestPathSize = 0x02;	// 2个words4个字节
		ulEPath = 0x01240620; // EPATH (or IOI) 20,06 (class, CM object); 24,01(Instance 1)
		byTicks = 0x05; // AS U LIKE, 1-second ticks (priority ignored); e.g. 0x0A
		byTimeOutTicks = 0xF7; // AS U LIKE, e.g. 0x0F Timeout is 245 seconds (ticks*tick time)
		ulO2TConnID = 0x02;// 0x00; // Type 0x0000 in request; Returned by target in reply
		ulT2OConnID = 0x01; // CID chosen by originator;
		uT2OConnSN = 0x02; // Chosen by originator
		uOVendorID = CIP_VENDOR_ID_ROCKWELL; // 0x574B; // From ID object (CIP vendor ID)
		ulOSN = 0x4553; // 0x4553; // From ID object
		byConnTimeOutMultiplier = 0x02; // e.g. 0x07 "512" (mult*RPI) "inactivity" timeout
		memset(byReserved, 0, sizeof(byReserved));
		ulO2TRPI = 0x001E8480; // Requested RPI, originator to target, microseconds
		uO2TConnParams = 0x43F4; // O-T connection parameters
		ulT2ORPI = 0x001E8480; // Requested RPI, target to originator, microseconds
		uT2OConnParams = 0x43F4; // 0x43F4; // T-O connection parameters
		byTransportClassTrigger = CIP_TRANSPORTCLASS_TRAIGGER_APPLICATION; // 0xA3, Server transport, class 3, application trigger
		byConnPathSize = 0x04; /* Number of  words(16 bit) in Connection Path
							   0x03 CLX via backplane
							   0x02 direct network device */
		byPaddedEPath[0] = 0x01; /* (from PLC5E MSG instruction)
								 e.g. 0x010020002401
								 01= Backplane port of 1756-ENET
								 00= Logix5550 in slot 0
								 20 02 = Class segment, 02 is MR
								 24 01 = Instance segment, no. 1 */
		byPaddedEPath[1] = 0x00;/*   CUP模块所在槽号，一般默认为0，
								使用softlogix模拟器依据需要
								修改前台槽号配置 hm 20130904*/
		byPaddedEPath[2] = 0x20;	// Class segment
		byPaddedEPath[3] = 0x02;	// class 02, is Message Router Object (class code = 0x02)
		byPaddedEPath[4] = 0x24;	// Instance Segment
		byPaddedEPath[5] = 0x01;	// Instance Id:01
		byPaddedEPath[6] = 0x2C;	// ??
		byPaddedEPath[7] = 0x01;
	};

	void to_normal()
	{
		sendRRData.to_normal();
		::to_normal(&ulEPath, sizeof(ACE_UINT32));
		::to_normal(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_normal(&ulT2OConnID, sizeof(ACE_UINT32));
		::to_normal(&uT2OConnSN, sizeof(ACE_UINT16));
		::to_normal(&uOVendorID, sizeof(ACE_UINT16));
		::to_normal(&ulOSN, sizeof(ACE_UINT32));
		::to_normal(&ulO2TRPI, sizeof(ACE_UINT32));
		::to_normal(&uO2TConnParams, sizeof(ACE_UINT16));
		::to_normal(&ulT2ORPI, sizeof(ACE_UINT32));
		::to_normal(&uT2OConnParams, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		sendRRData.to_little_endian();
		::to_little_endian(&ulEPath, sizeof(ACE_UINT32));
		::to_little_endian(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_little_endian(&ulT2OConnID, sizeof(ACE_UINT32));
		::to_little_endian(&uT2OConnSN, sizeof(ACE_UINT16));
		::to_little_endian(&uOVendorID, sizeof(ACE_UINT16));
		::to_little_endian(&ulOSN, sizeof(ACE_UINT32));
		::to_little_endian(&ulO2TRPI, sizeof(ACE_UINT32));
		::to_little_endian(&uO2TConnParams, sizeof(ACE_UINT16));
		::to_little_endian(&ulT2ORPI, sizeof(ACE_UINT32));
		::to_little_endian(&uT2OConnParams, sizeof(ACE_UINT16));
	}
}FwdOpenRequestStruct;

// Service OpenReply Struct
typedef struct tagFwdOpenReplyStruct
{
	SendRRDataStruct sendRRData;
	// Message Router Header
	ACE_UINT8 PACKED	byServiceCode;
	ACE_UINT8 PACKED	byReserved;
	ACE_UINT8 PACKED	byStatus;
	ACE_UINT8 PACKED	byAddStatus;
	ACE_UINT32 PACKED	ulO2TConnID;
	ACE_UINT32 PACKED	ulT2OConnID;
	ACE_UINT16 PACKED	uT2OConnSN;
	ACE_UINT16 PACKED	uTVendorID;
	ACE_UINT16 PACKED	uO2TConnSN;
	ACE_UINT32 PACKED	ulO2TAPI;
	ACE_UINT32 PACKED	ulT2OAPI;
	ACE_UINT8 PACKED	byReplySize;
	ACE_UINT8 PACKED	byReserved2;
	ACE_UINT8 PACKED	byReserved3; // 返回70个字节，拼两个
	ACE_UINT8 PACKED	byReserved4; // 返回70个字节，拼两个不用的字符

	tagFwdOpenReplyStruct()
	{
		memset(this, 0, sizeof(tagFwdOpenReplyStruct));
		byServiceCode = SERVICE_FWDOPENREPLY; // 0xD4;
		byStatus = byAddStatus = 0x00;
		ulO2TConnID = 0xFFFF; // EPATH (or IOI) 20,06 (class, CM object); 24,01(Instance 1)
		ulT2OConnID = 0xFFFF; // AS U LIKE, 1-second ticks (priority ignored); e.g. 0x0A
		ulO2TConnID = 0x00; // Type 0x0000 in request; Returned by target in reply
		ulT2OConnID = 0x01; // CID chosen by originator;
		uT2OConnSN = 0xFF; // Chosen by originator
		uTVendorID = 0xFF; // From ID object (CIP vendor ID)
		uO2TConnSN = 0xFF; // From ID object
		ulO2TAPI = 0xFF;
		ulT2OAPI = 0xFF;
		byReplySize = 0x00;
		byReserved2 = byReserved3 = byReserved4 = 0x00;
	};

	void to_normal()
	{
		sendRRData.to_normal();
		::to_normal(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_normal(&ulT2OConnID, sizeof(ACE_UINT32));
		::to_normal(&uT2OConnSN, sizeof(ACE_UINT16));
		::to_normal(&uTVendorID, sizeof(ACE_UINT16));
		::to_normal(&uO2TConnSN, sizeof(ACE_UINT16));
		::to_normal(&ulO2TAPI, sizeof(ACE_UINT32));
		::to_normal(&ulT2OAPI, sizeof(ACE_UINT32));
	}

	void to_little_endian()
	{
		sendRRData.to_little_endian();
		::to_little_endian(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_little_endian(&ulT2OConnID, sizeof(ACE_UINT32));
		::to_little_endian(&uT2OConnSN, sizeof(ACE_UINT16));
		::to_little_endian(&uTVendorID, sizeof(ACE_UINT16));
		::to_little_endian(&uO2TConnSN, sizeof(ACE_UINT16));
		::to_little_endian(&ulO2TAPI, sizeof(ACE_UINT32));
		::to_little_endian(&ulT2OAPI, sizeof(ACE_UINT32));
	}
}FwdOpenReplyStruct;

/////////////下面是读写数据请求，读写数据都封装在服务SendUnitData中//////////////////////////////
// SENDRRDATA Command common packet format
typedef struct tagSendUnitDataStruct
{
	// 通用命令项
	CIPHEADER cipHeader;

	ACE_UINT32 PACKED	ulInterfaceHandle;
	ACE_UINT16 PACKED	uOpTimeOut;

	// 地址和类型项，为了方便我们就放在这里
	ACE_UINT16 PACKED	uItemCount;
	ACE_UINT16 PACKED	uAddressType;
	ACE_UINT16 PACKED	uAddressLen;
	ACE_UINT32 PACKED	ulO2TConnID;
	ACE_UINT16 PACKED	uDataItemType;
	ACE_UINT16 PACKED	uDataItemLen;

	tagSendUnitDataStruct()
	{
		cipHeader.uCommand = COMMAND_SENDUNITDATA; // 0x70;

		ulInterfaceHandle = 0x00;
		uOpTimeOut = 0x00;

		// 地址和类型项，为了方便我们就放在这里
		uItemCount = 0x02;
		uAddressType = ITEMNO_ADDRESS_CONNECTION_BASED;// 0xA1;
		uAddressLen = 0x04; // words==8bytes
		ulO2TConnID = 0xFFFF; // will fill proper O2TConnId
		uDataItemType = ITEMNO_DATA_CONNECTED_TRANSPORT_PACKET; // 0xB1;
		uDataItemLen = 0xFF; // Unknown!will fill with right len
	};

	void to_normal()
	{
		cipHeader.to_normal();

		::to_normal(&ulInterfaceHandle, sizeof(ACE_UINT32));
		::to_normal(&uOpTimeOut, sizeof(ACE_UINT16));
		::to_normal(&uItemCount, sizeof(ACE_UINT16));
		::to_normal(&uAddressType, sizeof(ACE_UINT16));
		::to_normal(&uAddressLen, sizeof(ACE_UINT16));
		::to_normal(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_normal(&uDataItemType, sizeof(ACE_UINT16));
		::to_normal(&uDataItemLen, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		cipHeader.to_little_endian();

		::to_little_endian(&ulInterfaceHandle, sizeof(ACE_UINT32));
		::to_little_endian(&uOpTimeOut, sizeof(ACE_UINT16));
		::to_little_endian(&uItemCount, sizeof(ACE_UINT16));
		::to_little_endian(&uAddressType, sizeof(ACE_UINT16));
		::to_little_endian(&uAddressLen, sizeof(ACE_UINT16));
		::to_little_endian(&ulO2TConnID, sizeof(ACE_UINT32));
		::to_little_endian(&uDataItemType, sizeof(ACE_UINT16));
		::to_little_endian(&uDataItemLen, sizeof(ACE_UINT16));
	}
}SendUnitDataStruct;

// 多个请求服务合并请求中的一次单次读
// SingleRead-MultiRequest Struct
typedef struct tagSingleReadMultiRequestStruct
{
	// 在MultiRequest中插入单个服务
	ACE_UINT16 PACKED	uServiceCount;
	ACE_UINT16 PACKED uServiceOffsetI;
	ACE_UINT8 PACKED byChildSerCode;

	tagSingleReadMultiRequestStruct()
	{
		memset(this, 0, sizeof(tagSingleReadMultiRequestStruct));
		uServiceCount = 1;
		uServiceOffsetI = 4;
		byChildSerCode = 0x4C;
	};

	void to_normal()
	{
		::to_normal(&uServiceCount, sizeof(ACE_UINT16));
		::to_normal(&uServiceOffsetI, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		::to_little_endian(&uServiceCount, sizeof(ACE_UINT16));
		::to_little_endian(&uServiceOffsetI, sizeof(ACE_UINT16));
	}
}SingleReadMultiRequestStruct;

// 多个请求服务合并请求中的一次单次写
// SingleWrite-MultiRequest Struct
typedef struct tagSingleWriteMultiRequestStruct
{
	// 在MultiRequest中插入单个服务
	ACE_UINT16 PACKED	uServiceCount;
	ACE_UINT16 PACKED uServiceOffsetI;
	ACE_UINT8 PACKED byChildSerCode;

	tagSingleWriteMultiRequestStruct()
	{
		memset(this, 0, sizeof(tagSingleWriteMultiRequestStruct));
		uServiceCount = 1;
		uServiceOffsetI = 4;
		byChildSerCode = 0x53;
	};

	void to_normal()
	{
		::to_normal(&uServiceCount, sizeof(ACE_UINT16));
		::to_normal(&uServiceOffsetI, sizeof(ACE_UINT16));
	}

	void to_little_endian()
	{
		::to_little_endian(&uServiceCount, sizeof(ACE_UINT16));
		::to_little_endian(&uServiceOffsetI, sizeof(ACE_UINT16));
	}
}SingleWriteMultiRequestStruct;



// CIP Connection Manager Service = Unconnected Send， Request
typedef struct tagUnconnectedSendServiceRequestStruct
{
	// 在MultiRequest中插入单个服务
	ACE_UINT8 PACKED	byServiceCode;
	ACE_UINT8 PACKED	byWordsOfEPath;	// IOI EPath的长度（words为单位）
	ACE_UINT32 PACKED	ulEPath;
	ACE_UINT8 PACKED	byTricks; // Priority / Ticks multiplier, 0x0A means normal priority, multiple by about 1 second
	ACE_UINT8 PACKED	byTimeAmplifier; // Given above multiplier, this means Time out in 9 seconds
	ACE_UINT16 PACKED	uDataLen; // Length of CIP message we are ‘unconnected sending’ (in BYTES)
	tagUnconnectedSendServiceRequestStruct()
	{
		memset(this, 0, sizeof(tagSingleReadMultiRequestStruct));
		byServiceCode = SERVICE_UNCONNECTED_SEND; // 0x52
		byWordsOfEPath = 0x02; // 2words
		ulEPath = 0x01240620;	// 20 Class Segment, Class=0x06 (Connection Manager); 24 Instant Segement, 01 InstantNo.
		byTricks = 0x0A;
		byTimeAmplifier = 0x09; // means 9 second with tricks==0A
		uDataLen = 0xFF;
	};

	void to_normal()
	{
		::to_normal(&byServiceCode, sizeof(byServiceCode));
		::to_normal(&byWordsOfEPath, sizeof(byWordsOfEPath));
		::to_normal(&ulEPath, sizeof(ulEPath));
		::to_normal(&byTricks, sizeof(byTricks));
		::to_normal(&byTimeAmplifier, sizeof(byTimeAmplifier));
		::to_normal(&uDataLen, sizeof(uDataLen));
	}

	void to_little_endian()
	{
		::to_little_endian(&byServiceCode, sizeof(byServiceCode));
		::to_little_endian(&byWordsOfEPath, sizeof(byWordsOfEPath));
		::to_little_endian(&ulEPath, sizeof(ulEPath));
		::to_little_endian(&byTricks, sizeof(byTricks));
		::to_little_endian(&byTimeAmplifier, sizeof(byTimeAmplifier));
		::to_little_endian(&uDataLen, sizeof(uDataLen));
	}
}UnconnectedSendServiceRequestStruct;


//#ifdef _WIN32
//#pragma pack(pop)	// 恢复默认“对齐系数”
//#endif
#pragma pack()// 恢复默认“对齐系数”
#endif //_ABCIP_DRIVER_H_ 
