
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
#define SERVICE_READFRAGDATA		0x52	// Logix5000�ø÷��񽫶����������֯��һ��MultiRequest��0A����
#define SERVICE_WRITEFRAGDATA		0x53
#define SERVICE_READMODIFYWRITE		0x4E
#define SERVICE_READDATA			0x4C
#define SERVICE_WRITEDATA			0x4D	// single write
#define SERVICE_MULTIREQUEST		0x0A
#define SERVICE_MULTIREQUESTREPLY	0x8A

#define SERVICE_UNCONNECTED_SEND	0x52	// CIP Connection Manager Service = Unconnected Send SLC500�ø÷����װ4C��ReadData��


#define ITEMNO_ADDRESS_NULL						0x0		// ��ID, ������:��ַ, �޵�ַ,����:0������
#define ITEMNO_ADDRESS_SEQUENCE					0x8002	// ��ID, ������:��ַ, ���е�ַ�� ����
#define ITEMNO_ADDRESS_CONNECTION_BASED			0xA1	// ��ID, ������:��ַ, ��������,Connected address item,����:4�ֽ�
#define ITEMNO_DATA_CONNECTED_TRANSPORT_PACKET	0xB1	// ��ID, ������:����, �������ӵĴ����
#define ITEMNO_DATA_UNCONNECTED_MESSAGE			0xB2	// ��ID, ������:����, ��������Ϣ

#define SENDCONTEXT_BYTE1_READ		0x01
#define SENDCONTEXT_BYTE1_WRITE		0x02

#define CIP_VENDOR_ID_ROCKWELL		0x574B	// CIP���Ҵ��룬�޿�Τ�����Ʋ�
#define CIP_TRANSPORTCLASS_TRAIGGER_APPLICATION	0xA3	// 0xA3, Server transport, class 3, application trigger, Ӧ�ó��������������

#define PACKET_HEADER_LEN			24	// sizeof(CIPHEADER)

#define ABPLC_MODEL_LOGIX5000		5000		// LOGIX5000
#define ABPLC_MODEL_SLC500			500			// SLC500
#define ABPLC_MODEL_UNKNOWN			0			// δ֪

bool is_little_endian();
void to_normal(void* src, size_t size);
void to_little_endian(void* src, size_t size);

#pragma pack(1)  // ������ϵ������Ϊ1�ֽڶ���
#define PACKED
//#ifdef _WIN32
//#pragma pack(push, 1) // ������ϵ������Ϊ1�ֽڶ���
//#define PACKED 
//#else
//#define PACKED __attribute__((__packed__))
//#endif
// ����CIP�����Ӧ�����ݰ���ͷ���������������������
// ���ȣ�24���ֽ�
typedef struct _CIPHEADER
{
	ACE_UINT16 PACKED 	uCommand;			// �������ͣ�ע��ӦΪ��0x0065
	ACE_UINT16 PACKED	uCommandDataLen;		// ���ȣ�����ȥ����Ϊ4���ֽڡ�
	ACE_UINT32 PACKED	ulSessionHandle;	// ��Ŀ��������ظ�Դ��
	ACE_UINT32 PACKED	ulStatus;			// ״̬��ָ���������Ƿ���ִ������ķ�װ���0��ʾ�ɹ�
	ACE_UINT8  PACKED	bySendContext[8];	// �κη��������ݣ�����Ӧ��ʱԭ�����أ�������������ŵ�
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

// ע��Ự(RegisterSession)����Ӧ��Ľṹ�������Ӧ��Ľṹ��ͬ
// �ܳ���ӦΪ��24+4=28���ֽ�
typedef struct tagRegisterSessionStruct
{
	CIPHEADER cipHeader;

	// �����������йص�����
	ACE_UINT16 PACKED  urotocolVersion;		//  ����Э��汾Ӧ������Ϊ1
	ACE_UINT16 PACKED	uOptionFlag;		// �Ựѡ��Ӧ����Ϊ0��λ0-7�����������쳣��RA����λ8-15���������ڽ�����չ

	tagRegisterSessionStruct()
	{
		cipHeader.uCommand = COMMAND_REGISTERSESSION; // 0x65;
		cipHeader.uCommandDataLen = 0x04; // ע��̶�Ϊ4���ֽ�
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

// ע��(UnRegisterSession)�Ự����Ľṹ.��������ҪӦ��
// �ܳ��ȣ�24���ֽ�
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
	// common��24���ֽ�
	CIPHEADER cipHeader;

	// �����ض�����
	ACE_UINT32 PACKED	ulInterfaceHandle;	// 0(CIP)���ӿھ��
	ACE_UINT16 PACKED	uOpTimeOut;			// ���г�ʱ

	// Item (address item and data item)�������Ƿ�װ���ݰ����μ��������ݰ���ʽ�淶��8���ֽڣ���
	ACE_UINT16 PACKED	uItemCount;
	ACE_UINT16 PACKED	uAddressType;
	ACE_UINT16 PACKED	uAddressLen;
	ACE_UINT16 PACKED	uDataType;
	ACE_UINT16 PACKED	uDataItemLen;

	tagSendRRDataStruct()
	{
		cipHeader.uCommand = COMMAND_SENDRRDATA;//0x6F;
		cipHeader.uCommandDataLen = 0x10; // 16���ֽ�

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
// ǰ�滹��40���ֽ�
typedef struct tagFwdOpenRequestStruct
{
	SendRRDataStruct	sendRRData;			// ����ͷ����40���ֽ�
	// Message Router Header
	ACE_UINT8 PACKED	byServiceCode;		// �������
	ACE_UINT8 PACKED	byRequestPathSize;	// ����·���ߴ�
	ACE_UINT32 PACKED	ulEPath;			// ??

	// Connection Parameters
	ACE_UINT8 PACKED	byTicks;			// ?
	ACE_UINT8 PACKED	byTimeOutTicks;		// ��ʱ
	ACE_UINT32 PACKED	ulO2TConnID;		// Դ->Ŀ�������ID��������ţ�
	ACE_UINT32 PACKED	ulT2OConnID;		// Ŀ���豸���صĵ�Դ������ID��������Ϊ0
	ACE_UINT16 PACKED	uT2OConnSN;			// Ŀ���豸���ص��������к�
	ACE_UINT16 PACKED	uOVendorID;			// Ŀ���豸�ĳ���ID
	ACE_UINT32 PACKED	ulOSN;				// ���к�
	ACE_UINT8 PACKED	byConnTimeOutMultiplier; // ���ӳ�ʱ��������
	ACE_UINT8 PACKED	byReserved[3];		// ����3���ֽ�
	ACE_UINT32 PACKED	ulO2TRPI;			// ����
	ACE_UINT16 PACKED	uO2TConnParams;		// Դ��Ŀ������Ӳ���
	ACE_UINT32 PACKED	ulT2ORPI;			// Ŀ�굽Դ�ģ�
	ACE_UINT16 PACKED	uT2OConnParams;		// Ŀ���豸���ص����Ӳ���
	ACE_UINT8 PACKED	byTransportClassTrigger;	// ���䴥��ʱ������ѯ����ʱ�������ϱ�������APPLICATION, 0xA3
	ACE_UINT8 PACKED	byConnPathSize;		// ����·����С. Number of  words(16 bit) in Connection Path
	ACE_UINT8 PACKED	byPaddedEPath[8];
	tagFwdOpenRequestStruct()
	{
		// memset(this, 0, sizeof(tagFwdOpenRequestStruct)); �Ḳ��cipheader������˲��ܵ���
		byServiceCode = SERVICE_FWDOPEN;// 0x54;
		byRequestPathSize = 0x02;	// 2��words4���ֽ�
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
		byPaddedEPath[1] = 0x00;/*   CUPģ�����ڲۺţ�һ��Ĭ��Ϊ0��
								ʹ��softlogixģ����������Ҫ
								�޸�ǰ̨�ۺ����� hm 20130904*/
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
	ACE_UINT8 PACKED	byReserved3; // ����70���ֽڣ�ƴ����
	ACE_UINT8 PACKED	byReserved4; // ����70���ֽڣ�ƴ�������õ��ַ�

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

/////////////�����Ƕ�д�������󣬶�д���ݶ���װ�ڷ���SendUnitData��//////////////////////////////
// SENDRRDATA Command common packet format
typedef struct tagSendUnitDataStruct
{
	// ͨ��������
	CIPHEADER cipHeader;

	ACE_UINT32 PACKED	ulInterfaceHandle;
	ACE_UINT16 PACKED	uOpTimeOut;

	// ��ַ�������Ϊ�˷������Ǿͷ�������
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

		// ��ַ�������Ϊ�˷������Ǿͷ�������
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

// ����������ϲ������е�һ�ε��ζ�
// SingleRead-MultiRequest Struct
typedef struct tagSingleReadMultiRequestStruct
{
	// ��MultiRequest�в��뵥������
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

// ����������ϲ������е�һ�ε���д
// SingleWrite-MultiRequest Struct
typedef struct tagSingleWriteMultiRequestStruct
{
	// ��MultiRequest�в��뵥������
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



// CIP Connection Manager Service = Unconnected Send�� Request
typedef struct tagUnconnectedSendServiceRequestStruct
{
	// ��MultiRequest�в��뵥������
	ACE_UINT8 PACKED	byServiceCode;
	ACE_UINT8 PACKED	byWordsOfEPath;	// IOI EPath�ĳ��ȣ�wordsΪ��λ��
	ACE_UINT32 PACKED	ulEPath;
	ACE_UINT8 PACKED	byTricks; // Priority / Ticks multiplier, 0x0A means normal priority, multiple by about 1 second
	ACE_UINT8 PACKED	byTimeAmplifier; // Given above multiplier, this means Time out in 9 seconds
	ACE_UINT16 PACKED	uDataLen; // Length of CIP message we are ��unconnected sending�� (in BYTES)
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
//#pragma pack(pop)	// �ָ�Ĭ�ϡ�����ϵ����
//#endif
#pragma pack()// �ָ�Ĭ�ϡ�����ϵ����
#endif //_ABCIP_DRIVER_H_ 
