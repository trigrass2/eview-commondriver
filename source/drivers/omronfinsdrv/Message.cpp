// OmronMessage.cpp: implementation of the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "Message.h"
#include "AutoGroup_BlkDev.h"
#include <algorithm>
#include "math.h"
using namespace std;

#define DEVICE_USERDATA_TRANSID_INDEX		0	// 事务号
#define	DEFAULT_RESPONSE_MAXLEN			4096

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessage::CMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	m_pDevice = pDevice;
	m_pTagGroup = pTagGroup;
	m_pMsgBuffer = new char[MAX_MESSAGE_BUFFER_LEN];
	m_nMsgBufferLen = 0;
}

CMessage::~CMessage()
{
	delete[] m_pMsgBuffer;
	m_nMsgBufferLen = 0;
}

// 填充16个字节的TCP头部
int CMessage::BuildTcpHeader(char *szBuffer, int nBufferLen, int nCmdNo, int nMsgBodyLen)
{
	if (nBufferLen < TCP_HEADER_LENGTH)
		return -1;

	int nTotalLen = COMMAND_START_LENGTH + FINS_HEADER_LENGTH + nMsgBodyLen;

	char *szTcpHeader = szBuffer;
	szTcpHeader[0] = 0x46;	//fins
	szTcpHeader[1] = 0x49;
	szTcpHeader[2] = 0x4E;
	szTcpHeader[3] = 0x53;
	szTcpHeader[4] = 0x00;	//length，4bytes
	szTcpHeader[5] = 0x00;
	szTcpHeader[6] = 0x00;
	szTcpHeader[7] = 0x0E;
	szTcpHeader[8] = 0x00;	//command, 4bytes
	szTcpHeader[9] = 0x00;
	szTcpHeader[10] = 0x00;
	szTcpHeader[11] = nCmdNo; // 0x02;
	szTcpHeader[12] = 0x00;	//error code
	szTcpHeader[13] = 0x00;
	szTcpHeader[14] = 0x00;
	szTcpHeader[15] = 0x00;

	//转换为16进制
	szTcpHeader[6] = nTotalLen / 256;
	szTcpHeader[7] = nTotalLen % 256;

	return 0;
}

// 填充16个字节的TCP头部
int CMessage::BuildFinsHeader(char *szBuffer, int nBufferLen)
{
	if (nBufferLen < FINS_HEADER_LENGTH)
		return -1;

	m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX] = m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX] ++;
	m_nTransId = m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX];
	m_nTransId = m_nTransId % 256;

	char nServerNodeNo = ::atoi(m_pDevice->szParam1); // 设备参数1存放服务端节点DNA1
	char nClientNodeNo = ::atoi(m_pDevice->szParam2); // 设备参数2存放客户端节点SNA1

	char *szFinsHeader = szBuffer;
	szFinsHeader[0] = (char)0x80;			//ICF
	szFinsHeader[1] = 0x00;			//RSV
	szFinsHeader[2] = 0x02;			//GCT
	szFinsHeader[3] = 0x00;			//DNA
	szFinsHeader[4] = nServerNodeNo;	//DNA1
	szFinsHeader[5] = 0x00;			//DNA2
	szFinsHeader[6] = 0x00;			//SNA
	szFinsHeader[7] = nClientNodeNo;	//SNA1
	szFinsHeader[8] = 0x00;			//SNA2
	szFinsHeader[9] = (char)m_nTransId;		//SID, 事务号，不超过256的一个数字

	return 0;
}

// 填充8个字节的协议头部,再加上写入数据的内容
int CMessage::BuildFinsBody_WriteMessage(char *szBodyBuffer, int nBufferLen, PKTAG *pTag, char *szBinValue, int nBinValueLen)
{
	if (nBufferLen < MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinValueLen)
		return -1;

	int nStartBits = pTag->nStartBit + m_pTagGroup->nStartAddrBit;// 相对于块内的起始地址位（如AI/DI)+块的起始地址位 ？？？
	int nEndBits = pTag->nEndBit + m_pTagGroup->nStartAddrBit; // 相对于块内的结束地址位（如AI/DI)+块的起始地址位 ？？？

	// 起始地址和要控制的地址。按照字为单位考虑！！！！！！
	int nStartRegisterNo = nStartBits / 16; // 起始寄存器, WORD。 如10bit返回1
	char nStartRegisterHighByte = nStartRegisterNo / 256;  // 字为单位的寄存器起始地址，高字节
	char nStartRegisterLowByte = nStartRegisterNo % 256;  // 字为单位的寄存器起始地址，低字节
	char nStartRegisterBitOffset = nStartBits % 16;    // 字内的偏移 bit offset

	int nEndRegisterNo = ceil(m_pTagGroup->nEndAddrBit / 16.0f); // 起始寄存器, WORD。 取上限ceil，避免第一个字节读取不到, 如10bit返回1
	int nRegisterNum = nEndRegisterNo - nStartRegisterNo;

	char *szBody = szBodyBuffer;
	szBody[0] = 0x01;
	szBody[1] = 0x02;
	szBody[2] = GetAreaCode();
	szBody[3] = nStartRegisterHighByte;
	szBody[4] = nStartRegisterLowByte;
	szBody[5] = nStartRegisterBitOffset;
	szBody[6] = nRegisterNum / 256;
	szBody[7] = nRegisterNum % 256;
	if (nBinValueLen > 0)
	{
		memcpy(szBody + MESSAGE_CMD_ADDRESS_REGNUM_LEN, szBinValue, nBinValueLen);
	}

	return 0;
}

// 填充8个字节的协议头部;
int CMessage::BuildFinsBody_ReadMessage(char *szBodyBuffer, int nBufferLen)
{
	if (nBufferLen < MESSAGE_CMD_ADDRESS_REGNUM_LEN)
		return -1;

	int nStartRegisterNo = m_pTagGroup->nStartAddrBit / 16;				 // 起始寄存器, WORD。 如10bit返回1;
	char nStartRegisterHighByte = nStartRegisterNo / 256;				 // 字为单位的寄存器起始地址，高字节;
	char nStartRegisterLowByte = nStartRegisterNo % 256;				 // 字为单位的寄存器起始地址，低字节;
	char nStartRegisterBitOffset = m_pTagGroup->nStartAddrBit % 16;      // 字内的偏移 bit offset;
	int nEndRegisterNo = ceil(m_pTagGroup->nEndAddrBit / 16.0f);		 // 起始寄存器, WORD。 取上限ceil，避免第一个字节读取不到, 如10bit返回1;
	int nRegisterNum = nEndRegisterNo - nStartRegisterNo;

	char *szBody = szBodyBuffer;
	szBody[0] = 0x01;
	szBody[1] = 0x01;
	szBody[2] = GetAreaCode();
	szBody[3] = nStartRegisterHighByte;
	szBody[4] = nStartRegisterLowByte;
	szBody[5] = nStartRegisterBitOffset;
	szBody[6] = nRegisterNum / 256;
	szBody[7] = nRegisterNum % 256;
	return 0;
}

long CMessage::BuildHeartBeatMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	//构建cmd. 长度为8个字节+4个字节（客户端号）
	char *pMsgBuf = m_pMsgBuffer;
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_HEARTBEAT, MESSAGE_CMD_ADDRESS_REGNUM_LEN + 4); // 16个字节
	pMsgBuf += TCP_HEADER_LENGTH;

	int nClientNo = ::atoi(pDevice->szParam1);
	memcpy(pMsgBuf, &nClientNo, 4);
	pMsgBuf += 4;

	m_nMsgBufferLen = TCP_HEADER_LENGTH + 4; // 共20个字节
	return PK_SUCCESS;
}


long CMessage::BuildReadMessage(DRVGROUP *pTagGroup)
{
	//构建cmd
	char *pMsgBuf = m_pMsgBuffer;
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_READ, MESSAGE_CMD_ADDRESS_REGNUM_LEN); // 16个字节;
	pMsgBuf += TCP_HEADER_LENGTH;

	BuildFinsHeader(pMsgBuf, FINS_HEADER_LENGTH); // 10个字节
	pMsgBuf += FINS_HEADER_LENGTH;

	BuildFinsBody_ReadMessage(pMsgBuf, MESSAGE_CMD_ADDRESS_REGNUM_LEN);  // 8个字节

	m_nMsgBufferLen = TCP_HEADER_LENGTH + FINS_HEADER_LENGTH + MESSAGE_CMD_ADDRESS_REGNUM_LEN; // 共34个字节
	return PK_SUCCESS;
}

// 组织写消息;
long CMessage::BuildWriteMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *szBinBuf, int nBinBufLen)
{
	char *pMsgBuf = m_pMsgBuffer;
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_WRITE, MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinBufLen); // 16个字节
	pMsgBuf += TCP_HEADER_LENGTH;

	BuildFinsHeader(pMsgBuf, FINS_HEADER_LENGTH); // 10个字节
	pMsgBuf += FINS_HEADER_LENGTH;

	BuildFinsBody_WriteMessage(pMsgBuf, MESSAGE_CMD_ADDRESS_REGNUM_LEN, pTag, szBinBuf, nBinBufLen); // 8个字节 + 要写入的内容的长度;
	
	m_nMsgBufferLen = TCP_HEADER_LENGTH + FINS_HEADER_LENGTH + MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinBufLen; // 共34+写入的实际长度  个 字节;

	return PK_SUCCESS;
}


//获得区域码;
char CMessage::GetAreaCode()
{
	char areaCode = '0';
	string strAreaType = m_pTagGroup->szHWBlockName;
	std::transform(strAreaType.begin(), strAreaType.end(), strAreaType.begin(), ::toupper); // 变为大写字母;

	//获取数据类型:1121;
	strAreaType = strAreaType.substr(0, strAreaType.find(":"));
	if (strAreaType.compare(AREA_NAME_CIO_WORD) == 0)
	{
		areaCode = (char)0xB0;
	}
	else if (strAreaType.compare(AREA_NAME_WR_WORD) == 0)
	{
		areaCode = (char)0xB1;
	}
	else if (strAreaType.compare(AREA_NAME_HR_WORD) == 0)
	{
		areaCode = (char)0xB2;
	}
	else if (strAreaType.compare(AREA_NAME_AR_WORD) == 0)
	{
		areaCode = (char)0xB3;
	}
	else if (strAreaType.compare(AREA_NAME_TIMER_PV) == 0 || strAreaType.compare(AREA_NAME_COUNTER_PV) == 0)
	{
		areaCode = (char)0x89;
	}
	else if (strAreaType.compare(AREA_NAME_TIMER_CF) == 0 || strAreaType.compare(AREA_NAME_COUNTER_CF) == 0)
	{
		areaCode = (char)0x09;
	}
	else if (strAreaType.compare(AREA_NAME_DM_WORD) == 0)
	{
		areaCode = (char)0x82;
	}
	else if (strAreaType.compare(AREA_NAME_IR_DWORD) == 0)
	{
		areaCode = (char)0xDC;
	}
	else if (strAreaType.compare(AREA_NAME_DR_WORD) == 0)
	{
		areaCode = (char)0xBC;
	}
	else if (strAreaType.compare(AREA_NAME_EM_CURRENT_WORD) == 0)
	{
		areaCode = (char)0x98;
	}
	else if (strAreaType.compare(AREA_NAME_EM_WORD) == 0)
	{
		areaCode = (char)0xA0;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "设备:%s, 欧姆龙PLC的区域类型不对:%s", m_pDevice->szName, strAreaType.c_str());
	}

	return areaCode;
}


// 解析一个缓冲区, 可能包含1个、多个、半个数据包;
// nRequestTransID, 读写请求的事务号,便于判断是不是收到了该次的请求应答;
bool CMessage::ParseOnePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *pOnePackBuf, int nOnePackLen, unsigned short nRequestTransID, time_t tmRequest, bool bReadResponse)
{
	Drv_LogMessage(PK_LOGLEVEL_INFO, "收到请求响应包（设备名:%s,块名：%s, 事务号:%d）", pDevice->szName, pTagGroup->szAutoGroupName, nRequestTransID);

	// 更新数据块的数据。当在OnDatablockTimer的定时器中读取到数据后，调用该接口更新数据以便过程数据库读取;
	if (bReadResponse)
		UpdateGroupData(pDevice, pTagGroup, pOnePackBuf + READ_REPONSE_HEADER_LENGTH, nOnePackLen - READ_REPONSE_HEADER_LENGTH, TAG_QUALITY_GOOD);

	return true;
}

// 返回是否收到应答结果报;
bool CMessage::ProcessRecvData(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nTransId, bool bReadResponse, time_t tmRequest)
{
	bool bFoundPackWithTransId = false; // 是否找到了该事务号的应答包;
	// 从设备接收应答消息, 每次收到没有数据可以接收了就完毕;
	int nCurRecvLen = 0;
	char szResponse[DEFAULT_RESPONSE_MAXLEN] = { 0 };
	while (true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + nCurRecvLen, sizeof(szResponse) - nCurRecvLen, pDevice->nRecvTimeout);
		if (lRecvBytes <= 0)
		{
			break;
		}

		char *pCurBuf = szResponse;
		nCurRecvLen += lRecvBytes;
		bool bGetOne = true;
		while (bGetOne) // 循环处理这个缓冲区中的所有的包;
		{
			char *pOnePackBuf = NULL; // 找到的一个包的包头指针;

			//1127
			//int nOnePackLen = 0; // 找到的包的长度;
			int nOnePackLen = lRecvBytes * 2; // 找到的包的长度;
			bGetOne = GetOnePackage(pCurBuf, nCurRecvLen, pOnePackBuf, nOnePackLen); // 从头部开始找到一个包;
			if (!bGetOne) // 一个包也没有找到;
			{
				break;
			}

			bool bFoundPackThisTime = ParseOnePackage(pDevice, pTagGroup, pOnePackBuf, nOnePackLen, nTransId, tmRequest, bReadResponse); // 一定有一个包，进行解析处理;
			if (bFoundPackThisTime)
				bFoundPackWithTransId = true;
		}

		if (bFoundPackWithTransId) // 如果已经找到这个包的应答，那么就不需要继续接收数据了;
			break;
	}

	return bFoundPackWithTransId;
}

// 从头部开始找到一个包, 同时指针前移，长度缩小;
// 如果找到，则返回一个包的指针和长度;
bool CMessage::GetOnePackage(char *& pCurBuf, int &lCurRecvLen, char *&pOnePackBuf, int nOnePackLen)
{
	while (true)
	{
		if (lCurRecvLen < TCP_HEADER_LENGTH) // 小于最小长度，直接返回失败;
			return false;

		unsigned short nBodyLen = 0;
		memcpy(&nBodyLen, pCurBuf + 6, sizeof(short));
		int nTotalPackLen = nBodyLen + 8;
		//1126
		if (pCurBuf[0] == 0x46 && pCurBuf[1] == 0x49 && pCurBuf[2] == 0x4E && pCurBuf[3] == 0x53)
		//if (pCurBuf[0] == 0x46 && pCurBuf[1] == 0x49 && pCurBuf[2] == 0x49 && pCurBuf[3] == 0x53)
		{
			//注释掉,1126
			//if (nTotalPackLen > nOnePackLen) // 这个里面不包含任何一个包，包数据还不完整;
			//	return false;

			// 包含1个完整的符合规则的包;
			pOnePackBuf = pCurBuf;
			nOnePackLen = nTotalPackLen;

			pCurBuf += nTotalPackLen;
			lCurRecvLen -= nTotalPackLen;
			return true;
		}
		else // 未找到一个完整的包, 向前滑动1个字节;
		{
			pCurBuf += 1;
			lCurRecvLen -= 1;
		}
	}
	return false;
}
