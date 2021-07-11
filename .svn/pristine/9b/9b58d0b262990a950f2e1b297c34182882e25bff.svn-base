// OmronMessage.h: interface for the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "pkdriver/pkdrvcmn.h"
#include "AutoGroup_BlkDev.h"

// CIO,WR,HR,AR,DM,T/C-PV,EM,EM(Current Bank).除了EM外，其他读取单位都是WORD，EM可以是BYTE,WORD,DWORD
// 每一种类型都有按位控制的方法！
#define AREA_NAME_CIO_WORD		"CIO"		// WORD
#define AREA_NAME_WR_WORD		"WR"		// WORD, Work Area
#define AREA_NAME_HR_WORD		"HR"		// WORD, holding Area
#define AREA_NAME_AR_WORD		"AR"		// WORD, Auxiliary Area
#define AREA_NAME_TIMER_PV		"TPV"		// TIME,WORD, Timer Area PV
#define AREA_NAME_COUNTER_PV	"CPV"		// COUNT,WORD,Counter Area PV
#define AREA_NAME_TIMER_CF		"TCF"		// TIME,WORD,Timer Area CF
#define AREA_NAME_COUNTER_CF	"CCF"		// COUNT,WORD,Counter Area CF
#define AREA_NAME_DM_WORD		"DM"		// WORD,DM Area
#define AREA_NAME_IR_DWORD		"IR"		// Index Register,DWORD
#define AREA_NAME_DR_WORD		"DR"		// Data Register	WORD
#define AREA_NAME_EM_WORD		"EM"		// EM WORD
#define AREA_NAME_EM_CURRENT_WORD	"EMC"		// EM Current WORD
//#define AREA_NAME_EM			"EMB"		// BYTE
//#define AREA_NAME_EM			"EMDW"		// DWORD


#define TCP_HEADER_LENGTH					16
#define TCP_SHAKE_LENGTH					20
#define FINS_HEADER_LENGTH					10

#define MAX_MESSAGE_BUFFER_LEN				2048	// 一个读或写消息的最大长度
#define MESSAGE_CMD_ADDRESS_REGNUM_LEN		8		// 读消息固定为8个字节

#define READ_REPONSE_HEADER_LENGTH			30		// 读取消息应答的头部长度
#define TRANSID_POSITON						26		// 第26个字节是事务号

#define COMMAND_READ						2
#define COMMAND_WRITE						2
#define COMMAND_HEARTBEAT					0
//计算报文长度，从command之后算起
#define COMMAND_START_LENGTH				8		

class CMessage
{
public:
	CMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
	virtual ~CMessage();

protected:
	//获取AreaCode
	char GetAreaCode();

	// 是否需要高低字节转换
	bool NeedSwap();
	int BuildTcpHeader(char *szBuffer, int nBufferLen, int nCmdNo, int nBodyLen);
	int BuildFinsHeader(char *szBuffer, int nBufferLen);
	int BuildFinsBody_WriteMessage(char *szBodyBuffer, int nBufferLen, PKTAG *pTag, char *szBinValue, int nBinValueLen);
	int BuildFinsBody_ReadMessage(char *szBodyBuffer, int nBufferLen);
	char hextostring(char hex);
	int m_nTransId; // 这个消息包的事务号，发送时生成的
public:

	int GetTransId(){ return m_nTransId; };
public:
	long BuildReadMessage(DRVGROUP *pTagGroup);
	long BuildWriteMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *szBinBuf, int nBinBufLen);
	long ParsePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *pszRecvBuf, long lRecvBufLen, unsigned short nRequestTransID, time_t tmRequest);
	long BuildHeartBeatMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
	bool ParseOnePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *pOnePackBuf, int nOnePackLen, unsigned short nRequestTransID, time_t tmRequest, bool bReadResponse);
	bool ProcessRecvData(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nTransId, bool bReadResponse, time_t tmRequest);
	bool GetOnePackage(char *& pCurBuf, int &lCurRecvLen, char *&pOnePackBuf, int nOnePackLen);
public:
	PKDEVICE *m_pDevice;
	DRVGROUP *m_pTagGroup;
	char *m_pMsgBuffer;
	int m_nMsgBufferLen;

	int m_nStartAddress;
	int m_nNoOfPoints;

};

