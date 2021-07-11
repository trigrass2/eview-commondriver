#include "CIPDevice.h"

#define	INT_MAX										2147483647


// 构建发送RRComand命令数据包
// 
int CIPDevice::Build_Command_SendRRData_Packet(PKDEVICE *pDevice, char *szInDataBuff, int nInDataBufLen, char *szOutRequestBuffer, int & nOutBuffLen)
{
	char *pBuff = szOutRequestBuffer;
	
	SendRRDataStruct sendRRData;
	sendRRData.cipHeader.ulSessionHandle = m_nSessionId;
	unsigned short nTransId = this->m_nTransId;
	memcpy(sendRRData.cipHeader.bySendContext, &nTransId, sizeof(nTransId)); // 事务号, 在RRDataCommand是有效的

	nOutBuffLen = sizeof(SendRRDataStruct) + nInDataBufLen;
	int nCIPDataLen = nOutBuffLen - sizeof(CIPHEADER); // 减去头部长度24
	sendRRData.cipHeader.uCommandDataLen = nCIPDataLen; // 整个cip数据包体的长度
	sendRRData.uDataItemLen = nInDataBufLen; // 整个RRData的数据区长度
	memcpy(pBuff, &sendRRData, sizeof(SendRRDataStruct));
	pBuff += sizeof(SendRRDataStruct);

	memcpy(pBuff, szInDataBuff, nInDataBufLen);
	pBuff += nInDataBufLen;
	return 0;
}

// 构建SendUnitData命令，这个命令必须先打开ForwardOpen的连接
// 70 00 44 00 14 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 a1 00 04 00 3e 00 00 80 b1 00 30 00 22 00 0a 02 20 02 24 01 01 00 04 00 52 0e 91 0e 45 78 69 74 50 61 6c 6c 65 74 44 61 74 61 91 0a 53 74 61 74 75 73 43 6f 64 65 01 00 00 00 00 00
// tag1
// reqst:70 00 2E 00 4C 00 00 00 00 00 00 00 32 16 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 68 00 00 80 B1 00 1A 00 32 16 0A 02 20 02 24 01 01 00 04 00 52 03 91 04 74 61 67 31 01 00 00 00 00 00
// reply:70 00 1A 00 4C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 1E 01 00 00 B1 00 06 00 5D 16 8A 00 08 00
int CIPDevice::Build_Command_SendUnitData_Packet(PKDEVICE *pDevice, char *szInDataBuff, int nInDataLen, char *szOutRequestBuffer, int &nOutRequestBufLen)
{
	char *pBuff = szOutRequestBuffer;

	SendUnitDataStruct sendUnitDataStruct;
	nOutRequestBufLen = sizeof(SendUnitDataStruct) + nInDataLen;
	sendUnitDataStruct.cipHeader.uCommandDataLen = nOutRequestBufLen - sizeof(CIPHEADER);
	sendUnitDataStruct.cipHeader.ulSessionHandle = this->m_nSessionId;
	sendUnitDataStruct.uDataItemLen = nInDataLen;
	unsigned short nTransId = 0; // m_nTransId, SendUnitData的事务号总是为0!????
	memset(sendUnitDataStruct.cipHeader.bySendContext, 0, sizeof(sendUnitDataStruct.cipHeader.bySendContext));
	memcpy(sendUnitDataStruct.cipHeader.bySendContext, &nTransId, sizeof(nTransId)); // 事务号. SendUnitData的事务号是不能返回的！！！总是返回0！！！
	sendUnitDataStruct.ulO2TConnID = this->m_nO2TConnID;
	//sendUnitDataStruct.bySendContext[0] = SENDCONTEXT_BYTE1_READ;

	// 拷贝该部分缓冲区
	sendUnitDataStruct.to_little_endian();
	memcpy(pBuff, &sendUnitDataStruct, sizeof(SendUnitDataStruct));
	pBuff += sizeof(SendUnitDataStruct);

	memcpy(pBuff, szInDataBuff, nInDataLen);
	pBuff += nInDataLen;

	nOutRequestBufLen = sizeof(SendUnitDataStruct) + nInDataLen;
	return 0;
}

void CIPDevice::ResetAttributes()
{
	m_nTransId = 0;		// 事务号，填充与CIP头部的8个用户自定义字节中
	m_nSessionId = 0;	// Register
	// 由Origninator产生的，Target使用.
	m_nT2OConnID = 0;
	m_nT2OConnSN = 0;
	m_nOVendorID = 0;
	m_nOSN = 0;
	m_nT2OAPI = 0;

	// 由Target产生，Originator使用
	m_nO2TConnID = 0;
	m_nO2TConnSN = 0;
	m_nO2TAPI = 0;
}

CIPDevice::CIPDevice(PKDEVICE *pDevice)
{
	m_pDevice = pDevice;
	string strPLCMOdel = pDevice->szParam1;
	string strModelName;

	if (strPLCMOdel.find("5000") != string::npos || strPLCMOdel.empty())
	{
		m_nPLCModel = ABPLC_MODEL_LOGIX5000;
		strModelName = "Logix5000";
	}
	else if (strPLCMOdel.find("500") != string::npos)
	{
		m_nPLCModel = ABPLC_MODEL_SLC500;
		strModelName = "SLC5000";
	}
	else
	{
		strModelName = "UnknownModel";
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, param1[model]:%s, not supported! must be:Logix5000/SLC500", pDevice->szName, strPLCMOdel.c_str());
	}

	m_nSlotOfCPU = ::atoi(pDevice->szParam2);
	
	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, model:%s, CPU Slot:%d[default is 0]", pDevice->szName, strModelName.c_str(), m_nSlotOfCPU);

	ResetAttributes();
}
CIPDevice::~CIPDevice()
{

}

// 构建注册会话未连接数据包消息
// fameview:	65 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00 00 00 00 00 00 00 01 00 00 00
//  65 00[cmd] 04 00[data len] 00 00 00 00[session handle]	 00 00 00 00[status]	 00 00 00 00 03 00 00 00[sendcontext] 00 00 00 00[options]	01 00[protocal version]		00 00[option flag]
// 发送：[0028] 65 00 04 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00
// 应答：[0028] 65 00 04 00 73 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00
// 一个连接只有一个会话。如果不断开连接，那么重复发送的registerSession将会返回句柄为1、status==1.此时必须断开连接或者注销连接
int CIPDevice::Build_Command_RegisterSession_Packet(PKDEVICE *pDevice, char *szReadMsg, unsigned short nTransId)
{
	char *pMsg = szReadMsg;
	RegisterSessionStruct registerSession;
	memcpy(registerSession.cipHeader.bySendContext, &nTransId, sizeof(nTransId)); // 事务号
	registerSession.cipHeader.uCommandDataLen = sizeof(RegisterSessionStruct) - sizeof(CIPHEADER);
	memcpy(szReadMsg, &registerSession, sizeof(RegisterSessionStruct));
	return sizeof(RegisterSessionStruct);
}

// 构建注册会话未连接数据包消息
// 发送：66 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 1A 00 00 00 01 00 00 00
int CIPDevice::Build_Command_UnRegisterSession_Packet(PKDEVICE *pDevice, char *szReadMsg, unsigned short nTransId)
{
	char *pMsg = szReadMsg;
	UnRegisterSessionStruct unRegisterSession;
	unRegisterSession.cipHeader.ulSessionHandle = m_nSessionId;
	memset(unRegisterSession.cipHeader.bySendContext, 0, sizeof(unRegisterSession.cipHeader.bySendContext));
	memcpy(unRegisterSession.cipHeader.bySendContext, &nTransId, sizeof(nTransId)); // 事务号
	memcpy(szReadMsg, &unRegisterSession, sizeof(UnRegisterSessionStruct));
	return sizeof(UnRegisterSessionStruct);
}

// 构建注册会话未连接数据包消息
// request[90bytes]:6f 00 42 00 08 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 32 00 54 02 20 06 24 01 05 f7 02 00 00 00 40 00 00 00 41 00 4b 57 53 45 00 00 02 00 00 00 80 84 1e 00 f4 43 80 84 1e 00 f4 43 a3 04 01 00 20 02 24 01 2c 01
// reply[70bytes]：6f 00 2e 00 08 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 00 00 00 00 1e 00 02 00 00 00 00 00 b2 00 1e 00 d4 00 00 00 20 00 00 80 40 00 00 00 41 00 4b 57 53 45 00 00 80 84 1e 00 80 84 1e 00 00 00
int CIPDevice::Build_SendRRData_ForwardOpenService_Request_Packet(PKDEVICE *pDevice, char *szMsg, unsigned short nTransId)
{
	FwdOpenRequestStruct fwdOpenReqStruct;
	// SendRRData Command Header and Item (Address and data Item)
	// assign SessionID
	fwdOpenReqStruct.sendRRData.cipHeader.ulSessionHandle = this->m_nSessionId;
	memcpy(fwdOpenReqStruct.sendRRData.cipHeader.bySendContext, &nTransId, sizeof(nTransId)); // 事务号

	// FwdOpenRequest Struct	
	ACE_OS::srand((int)this->m_nSessionId); // 随机数
	this->m_nT2OConnID = ACE_OS::rand(); // 这个应该是返回的，这里随机给个数字即可
	this->m_nT2OConnSN = this->m_nT2OConnID + 1; // 这个应该是返回的，这里随机给个数字即可
	fwdOpenReqStruct.ulT2OConnID = this->m_nT2OConnID;  // 
	fwdOpenReqStruct.uT2OConnSN = this->m_nT2OConnSN; 
	fwdOpenReqStruct.byPaddedEPath[1] = this->m_nSlotOfCPU;//slot hm 20130904
	fwdOpenReqStruct.to_little_endian();
	fwdOpenReqStruct.sendRRData.cipHeader.uCommandDataLen = sizeof(FwdOpenRequestStruct) - sizeof(CIPHEADER);
	fwdOpenReqStruct.sendRRData.uDataType = ITEMNO_DATA_UNCONNECTED_MESSAGE; // 0xB2
	fwdOpenReqStruct.sendRRData.uDataItemLen = sizeof(FwdOpenRequestStruct) - sizeof(SendRRDataStruct);
	memcpy(szMsg, &fwdOpenReqStruct, sizeof(FwdOpenRequestStruct));
	int nPackLen = sizeof(FwdOpenRequestStruct); // 应该是40+50==90

	return nPackLen;
}

// 在接收到的数据缓冲区中，找到一个合法的应答包
// 合法包定义：大于最小长度24；命令类型合法；命令长度合法；会话句柄合法。
// 如果找到了合法包，且包中事务号和期望相等，则返回这个包
int CIPDevice::FindPackage(PKDEVICE *pDevice, char *szReadMsg, int nReadMsgLen, int nHopeCmd, unsigned short nHopeTransId, char *szHopePackBuf, int &nHopePackLen, char *&pCurBuf)
{
	bool bFoundValidPack = false;
	nHopePackLen = 0;
	char *pOnePackHead = szReadMsg;
	int nOnePackLen = 0;
	char *pPackEnd = NULL;
	pCurBuf = szReadMsg;
	char *pBufEnd = szReadMsg + nReadMsgLen;
	while (pCurBuf + sizeof(CIPHEADER) <= pBufEnd) // 至少需要一个头部==24个字节
	{
		bool bFoundHopePack = false;
		CIPHEADER cipHeader;
		memcpy(&cipHeader, pCurBuf, sizeof(CIPHEADER));
		// 判断是否是合法的包
		if (cipHeader.uCommand != COMMAND_REGISTERSESSION && cipHeader.uCommand != COMMAND_SENDRRDATA && cipHeader.uCommand != COMMAND_SENDUNITDATA)
		{
			pCurBuf += 1; // 不合法的包继续检查下一个字节
			continue;
		}

		if (cipHeader.uCommand != COMMAND_REGISTERSESSION) // 注册时会话句柄是不相等的
		{
			if (cipHeader.ulSessionHandle != this->m_nSessionId) // 会话句柄不等则继续！
			{
				pCurBuf++;
				continue;
			}
		}

		if (cipHeader.uCommand == COMMAND_SENDRRDATA)
		{

		}
		else if (cipHeader.uCommand == COMMAND_SENDUNITDATA)
		{
			//printf("COMMAND_SENDUNITDATA\n");
		}

		// 剩余的长度
		int nLeftBufLen = pBufEnd - pCurBuf;
		if (cipHeader.uCommandDataLen + sizeof(CIPHEADER) > nLeftBufLen) // 长度不够，不合法
		{
			pCurBuf++;
			continue;
		}

		if (cipHeader.ulOptions != 0)
		{
			pCurBuf++;
			continue;
		}
		if (cipHeader.uCommandDataLen > 1000)
		{
			pCurBuf++;
			continue;
		}

		// 下面是一个完整的数据包文
		nOnePackLen = cipHeader.uCommandDataLen + sizeof(CIPHEADER);
		nHopePackLen = nOnePackLen;
		memcpy(szHopePackBuf, pCurBuf, nOnePackLen);
		pCurBuf += cipHeader.uCommandDataLen + sizeof(CIPHEADER);

		unsigned short nTransIdInPack;
		memcpy(&nTransIdInPack, cipHeader.bySendContext, sizeof(nTransIdInPack));
		if (nHopeCmd == cipHeader.uCommand) // 命令号必须相同！
		{
			if (nTransIdInPack == nHopeTransId || (nHopeCmd == COMMAND_SENDUNITDATA))  // 事务号必须相同，或者SendUnitData事务号总返回0此时不需要相同！ 
			{
				// 找到了想要的一个应答数据包
				bFoundHopePack = true;
				char szHex[1024];
				unsigned int nHexStringLen = 0;
				PKStringHelper::HexDumpBuf(szHopePackBuf, nOnePackLen, szHex, sizeof(szHex), &nHexStringLen);
				Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s,recv a response package:%s, len:%d", pDevice->szName, szHex, nOnePackLen);
				return 0;
			}
		}
	}

	return -1; // 未找到合法的数据包
}

// 得到一个可用的会话ID
unsigned short CIPDevice::GetTransactionId(PKDEVICE *pDevice)
{
	// 事务号
	m_nTransId++;
	return m_nTransId;
}

// 接受和处理应答包, 如果找到事务号为nTransId的包则返回0，否则返回非0
int CIPDevice::RecvAndGetHopedReplyPacket(PKDEVICE *pDevice, int nHopeCmdNo, unsigned short nTransId, int nRecvTimeoutMS, char *szHopePackBuf, int &nHopePackLen)
{
	char szRecvBuff[1024] = { 0 };
	ACE_Time_Value tvBegin = ACE_OS::gettimeofday();

	// 接收的包中需要包含10 02 .... 10 03 BCC,并且事务号等于nTrans
	int nRecvBytes = 0;
	bool bFoundTagDataPack = false; // 是否已经有该点的数据包
	while (true && !bFoundTagDataPack)
	{
		ACE_Time_Value tvNow = ACE_OS::gettimeofday();
		ACE_Time_Value tvSpan = tvNow - tvBegin; // 已经过得时间
		int nTimeoutMS = 8000 - tvSpan.msec(); // 每次最多等待8秒. PLC-5通过网络转之后，2秒都收不到数据
		if (nTimeoutMS < 0 || nTimeoutMS > 100 * 1000) // 超时用尽还没有满足条件则跳过, 或者修改了系统时间
			break;

		if (nRecvBytes >= 1024)
			break;

		int nRecvOnce = Drv_Recv(pDevice, (char*)szRecvBuff, 1024 - nRecvBytes, nTimeoutMS);
		if (nRecvOnce <= 0)
			continue;

		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device:[%s] transId %d recv the requst(part data) len: %d", pDevice->szName, nTransId, nRecvOnce);

		nRecvBytes += nRecvOnce;
		char *pCurBuff = (char *)szRecvBuff;
		while (pCurBuff < szRecvBuff + nRecvBytes) // 找到所有的数据包
		{
			nHopePackLen = 0;
			char *pNextBuff = NULL;
			int nRet = FindPackage(pDevice, pCurBuff, nRecvBytes, nHopeCmdNo, nTransId, szHopePackBuf, nHopePackLen, pNextBuff);
			if (nRet == 0) // 找到想要的包了！10 02 00 01 4f 00 00 01 18 00 10 03 97
			{
				//char *szData = szHopePackBuf + 8; // 18 00 是数据，2
				//Drv_SetTagData_Binary(pTag, szData, 2); // tag点必须配置为short或者unsigned short类型
				//unsigned short nData;
				//memcpy(&nData, szData, 2);
				//int nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
				//if (nRet == 0)
				//	Drv_LogMessage(PK_LOGLEVEL_INFO, "!!!device:[%s],tag:%s[No.%d of %d], recv a data response package, packlen:%d, data:%02x %02x, to int16:%d",
				//	pDevice->szName, pTag->szName, iTag, pDevice->nTagNum, nHopePackLen, *szData, *(szData + 1), nData);
				//else
				//	Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[No.%d of %d], recv a data response package, packlen:%d, data:%02x %02x, to int16:%d, but Drv_UpdateTagsData return:%d",
				//	pDevice->szName, pTag->szName, iTag, pDevice->nTagNum, nHopePackLen, *szData, *(szData + 1), nData, nRet);

				// 更新数据
				bFoundTagDataPack = true;
				break;
			}

			pCurBuff = pNextBuff;
		} // while (pCurBuff < szRecvBuff + nRecvBytes) // 找到所有的数据包
	} // while (true)

	if (bFoundTagDataPack)
		return 0;
	return -1; // 未找到
}

// 构建读取一组tag点的消息
// SendUnitData:ExitPalletData.StatusCode
// 70 00 44 00 14 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 a1 00 04 00 3e 00 00 80 b1 00 30 00 22 00 0a 02 20 02 24 01 01 00 04 00 52 0e 91 0e 45 78 69 74 50 61 6c 6c 65 74 44 61 74 61 91 0a 53 74 61 74 75 73 43 6f 64 65 01 00 00 00 00 00
// tag1
// reqst:70 00 2E 00 4C 00 00 00 00 00 00 00 32 16 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 68 00 00 80 B1 00 1A 00 32 16 0A 02 20 02 24 01 01 00 04 00 52 03 91 04 74 61 67 31 01 00 00 00 00 00
// reply:70 00 1A 00 4C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 1E 01 00 00 B1 00 06 00 5D 16 8A 00 08 00
int CIPDevice::Build_Service_MultiRequest_Packet(PKDEVICE *pDevice, vector<string> &vecMultiServiceBuffer, char *szRequestBuffer, int &nAllServiceBuffLen)
{
	char *pBuff = szRequestBuffer;
	char * pBuffItemDataLenBegin = pBuff;

	// Sequence计数内容, 用事务号替用
	unsigned short usTransId = m_nTransId;
	memcpy(pBuff, &usTransId, sizeof(ACE_UINT16));
	to_little_endian(pBuff, sizeof(ACE_UINT16));
	pBuff += sizeof(ACE_UINT16);

	// 0A，Multi-Request Service
	ACE_UINT8 byServiceCode = SERVICE_MULTIREQUEST; // 0x0A
	memcpy(pBuff, &byServiceCode, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	// unknown field, Guess words count of EPath
	ACE_UINT8 byUnknown = 0x02;
	memcpy(pBuff, &byUnknown, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	// EPath, MessageRouter Object, Instance1
	ACE_UINT32	ulEPath = 0x01240220;	// 20 Class Segment, 02 MessageRouter Object; 24 Instant Segement, 01 InstantNo.
	memcpy(pBuff, &ulEPath, sizeof(ACE_UINT32));
	to_little_endian(pBuff, sizeof(ACE_UINT32));
	pBuff += sizeof(ACE_UINT32);

	char *pServiceOffsetBegin = pBuff; // 第一个service的偏移，从servicecount算起，servicecount（2B）+offset1（2B）
	// Service Count - Because is multiService
	ACE_UINT16 uServiceCount = vecMultiServiceBuffer.size(); // 服务个数， 每个点是一个SingleRead服务
	memcpy(pBuff, &uServiceCount, sizeof(uServiceCount));
	to_little_endian(pBuff, sizeof(uServiceCount));
	pBuff += sizeof(uServiceCount);

	// Offset, skip and fill at last
	
	// Give Offset an initial value, all services
	int nTotalServiceBufLen = 0;	// 所有服务的内容长度和
	int nCurOffsetTotal = sizeof(ACE_UINT16) * uServiceCount; // 第一个service的偏移，从servicecount算起，servicecount（2B）+offset1（2B）
	for (int i = 0; i < uServiceCount; ++i)
	{
		string &strOneServiceBuffer = vecMultiServiceBuffer[i];
		int nOneServiceBuffLen = strOneServiceBuffer.size();

		char *pCurServiceOffset = pServiceOffsetBegin + 2 + sizeof(ACE_UINT16) * i;	// 当前服务偏移量2个字节的首地址
		ACE_UINT16 nCurServiceOffset = 2 + sizeof(ACE_UINT16) * (i + 1) + nTotalServiceBufLen; // 当前服务偏移量多少

		// 把偏移长度写入到2个字节中
		memcpy(pCurServiceOffset, &nCurServiceOffset, sizeof(ACE_UINT16));
		to_little_endian(pCurServiceOffset, sizeof(ACE_UINT16));
		pBuff += sizeof(ACE_UINT16);

		// 把服务的内容写入到偏移开始位置去
		char *pCurServiceBuffPos = pServiceOffsetBegin + 2 + sizeof(ACE_UINT16) * (i + 1) + nTotalServiceBufLen;
		memcpy(pCurServiceBuffPos, strOneServiceBuffer.c_str(), nOneServiceBuffLen);
		nTotalServiceBufLen += nOneServiceBuffLen;
		pBuff += nOneServiceBuffLen;
	}
	
	// pBuff = pServiceOffsetBegin + sizeof(ACE_UINT16) + sizeof(ACE_UINT16) * uServiceCount + nTotalServiceBufLen; // 指向最后1个字节的位置
	nAllServiceBuffLen = pBuff - szRequestBuffer;
	return 0;
}

// 组建0x52命令, SERVICE_READFRAGDATA
int CIPDevice::Build_Service_ReadFragData_Packet(PKDEVICE *pDevice, PKTAG *pTag, char *szOutRequestBuff, int &nOutServiceBuffLen)
{
	char *pBuff = szOutRequestBuff;

	// 插入服务代码
	ACE_UINT8 byChildSerCode = SERVICE_READFRAGDATA;
	memcpy(pBuff, &byChildSerCode, sizeof(byChildSerCode));
	pBuff += sizeof(byChildSerCode);

	CIOI * pSymbolAddr = (CIOI *)pTag->pData1;
	// 获取地址缓冲区并插入
	long nAddrLen;
	char *pAdrBuf = pSymbolAddr->GetIOIBuff(&nAddrLen);
	if (!pAdrBuf || nAddrLen <= 0)
	{
		nOutServiceBuffLen = 0;
		return -1;
	}

	memcpy(pBuff, pAdrBuf, nAddrLen);
	pBuff += nAddrLen;

	//size, unit:word.读取几个元素, Number of elements to read. 字符串如何操作？
	ACE_UINT16 uCount = pSymbolAddr->m_uCount;
	memcpy(pBuff, &uCount, sizeof(uCount));
	to_little_endian(pBuff, sizeof(uCount));
	pBuff += sizeof(uCount);

	//padded byte, offset
	ACE_UINT32 ulPad = 0x00;
	memcpy(pBuff, &ulPad, sizeof(ulPad));
	to_little_endian(pBuff, sizeof(ulPad));
	pBuff += sizeof(ulPad);

	nOutServiceBuffLen = pBuff - szOutRequestBuff;
	return 0;
}


// 构建一个UnConnectedSend的Service，这个服务里面必然还要包装其他的子服务
// SendUnitData:ExitPalletData.StatusCode
// 70 00 44 00 14 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 a1 00 04 00 3e 00 00 80 b1 00 30 00 22 00 0a 02 20 02 24 01 01 00 04 00 52 0e 91 0e 45 78 69 74 50 61 6c 6c 65 74 44 61 74 61 91 0a 53 74 61 74 75 73 43 6f 64 65 01 00 00 00 00 00
// tag1
// reqst:70 00 2E 00 4C 00 00 00 00 00 00 00 32 16 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 68 00 00 80 B1 00 1A 00 32 16 0A 02 20 02 24 01 01 00 04 00 52 03 91 04 74 61 67 31 01 00 00 00 00 00
// reply:70 00 1A 00 4C 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 1E 01 00 00 B1 00 06 00 5D 16 8A 00 08 00
int CIPDevice::Build_Service_UnconnectedSend_Packet(PKDEVICE *pDevice, char *szInContentBuff, int nInContentBuffLen,  char *szOutRequestBuffer, int &nOutBuffLen)
{
	char *pBuff = szOutRequestBuffer;

	UnconnectedSendServiceRequestStruct unConnectedSendService;
	unConnectedSendService.uDataLen = nInContentBuffLen;
	memcpy(pBuff, &unConnectedSendService, sizeof(UnconnectedSendServiceRequestStruct));
	pBuff += sizeof(UnconnectedSendServiceRequestStruct);

	memcpy(pBuff, szInContentBuff, nInContentBuffLen);
	pBuff += nInContentBuffLen;

	nOutBuffLen = pBuff - szOutRequestBuffer;
	return 0;
}

// N10:25, N10:26的一个组，将返回：FileType：0x89，FileNumber：10，nStartAddr：26
int CIPDevice::GetFileType_SLC500(PKDEVICE *pDevice, DRVGROUP *pTagGroup, ACE_UINT8 &nFileType, ACE_UINT8 &nFileNumber, ACE_UINT16 &nStartAddrInFile, ACE_UINT8 &nBytesToRead)
{
	int nRegisterNum = pTagGroup->nRegisterNum;
	if (strcmp(pTagGroup->szHWBlockName, "N") == 0) //整数
	{
		nFileType = 0x89;
		nBytesToRead = nRegisterNum * 2;
	}
	if (strcmp(pTagGroup->szHWBlockName, "C") == 0) //计数
	{
		nFileType = 0x87;
		nBytesToRead = nRegisterNum * 2;
	}
	if (strcmp(pTagGroup->szHWBlockName, "B") == 0) //位
	{
		nFileType = 0x85;
		nBytesToRead = nRegisterNum;
	}
	else
	{
		nFileType = 0x89;	// default:N
		nBytesToRead = nRegisterNum * 2;
	}

	PKTAG *pFirstTag = pTagGroup->vecTags[0]; // 第一个就是地址最小的
	string strTagAddress = pFirstTag->szAddress;
	int pos = strTagAddress.find(":");
	if (pos == -1)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, addr:%s, address may be wrong due to lack of ':'", 
			pDevice->szName, pFirstTag->szName, pFirstTag->szAddress);
		return -1;
	}
	string strFileNumber = strTagAddress.substr(1, pos); // 去掉第一个块类型前缀
	string strStartAddr = strTagAddress.substr(pos + 1);
	nFileNumber = ::atoi(strFileNumber.c_str());
	nStartAddrInFile = ::atoi(strStartAddr.c_str());
	// nStartAddrInFile -= 1; // N10:35,算35，从1开始!!!参考FameView
	return 0;
}

int CIPDevice::Build_SLC500_Service_PCCCReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen)
{
	char *pBuff = szOutRequestBuffer;

	// 插入服务代码
	ACE_UINT8 byServiceCode = 0x4C; // PCCC Object Service Code
	memcpy(pBuff, &byServiceCode, sizeof(byServiceCode));
	pBuff += sizeof(byServiceCode);

	ACE_UINT8 byEPathWords = 0x02; // Length (in WORDS) of CIP IOI Path
	memcpy(pBuff, &byEPathWords, sizeof(byEPathWords));
	pBuff += sizeof(byEPathWords);

	ACE_UINT32 ulEPath = 0x01240620;	// 20 Class Segment, Class=0x06 (Connection Manager); 24 Instant Segement, 01 InstantNo.;
	to_little_endian(&ulEPath, sizeof(ulEPath));
	memcpy(pBuff, &ulEPath, sizeof(ulEPath));
	pBuff += sizeof(ulEPath);

	ACE_UINT8 byReserved[8]; // PCCC addressing info – just use these defaults
	memset(byReserved, 0, sizeof(byReserved));
	byReserved[1] = 0x01;
	memcpy(pBuff, byReserved, sizeof(byReserved));
	pBuff += sizeof(byReserved);

	ACE_UINT8 byCommand = 0x0F; // PCCC CMD / Command(see DF1 manual #17706516 at www.ab.com)
	memcpy(pBuff, &byCommand, sizeof(byCommand));
	pBuff += sizeof(byCommand);

	ACE_UINT8 byStatus = 0x00; // PCCC STS / Status – send as zero
	memcpy(pBuff, &byStatus, sizeof(byStatus));
	pBuff += sizeof(byStatus);

	ACE_UINT16 uTransNo = m_nTransId; // PCCC TNS / Transaction Number – should vary between each poll
	to_little_endian(&uTransNo, sizeof(uTransNo));
	memcpy(pBuff, &uTransNo, sizeof(uTransNo));
	pBuff += sizeof(uTransNo);

	ACE_UINT8 byFuncNo = 0xA2; // PCCC FNC = Protected Typed Logical Read with 3 address fields
	memcpy(pBuff, &byFuncNo, sizeof(byFuncNo));
	pBuff += sizeof(byFuncNo);

	ACE_UINT8 byByteCountToRead = 0;
	ACE_UINT8 byFileType = 0x89; // File Type = 16-bit INTEGER – see DF1 manual for other types===================
	ACE_UINT8 byFileNumber = 0x00; // File Number defined in ControlLogix – limited 0 to 255===================
	ACE_UINT16 uStartRegisterNo = 0x00;
	int nRet = GetFileType_SLC500(pDevice, pTagGroup, byFileType, byFileNumber, uStartRegisterNo, byByteCountToRead);
	if (nRet != 0)
		return nRet;

	memcpy(pBuff, &byByteCountToRead, sizeof(byByteCountToRead));
	pBuff += sizeof(byByteCountToRead);

	memcpy(pBuff, &byFileNumber, sizeof(byFileNumber)); // 文件号，N10就是10
	pBuff += sizeof(byFileNumber);

	memcpy(pBuff, &byFileType, sizeof(byFileType));
	pBuff += sizeof(byFileType);

	ACE_UINT8 byStartRegisterNo = uStartRegisterNo; // Read starting at this element , WARNING – must PAD if byte 48 is ODD – but NO PAD in this example!!
	memcpy(pBuff, &byStartRegisterNo, sizeof(byStartRegisterNo));
	pBuff += sizeof(byStartRegisterNo);

	ACE_UINT8 bySubElement = 0x00; // Sub-Element – only used for bit or structured data file types
	memcpy(pBuff, &bySubElement, sizeof(bySubElement));
	pBuff += sizeof(bySubElement);

	if (byStartRegisterNo % 2 == 1)
	{
		ACE_UINT8 byPad = 0x00; // WARNING – must PAD if byte 48 is ODD – but NO PAD in this example!!
		memcpy(pBuff, &byPad, sizeof(byPad));
		pBuff += sizeof(byPad);
	}

	ACE_UINT8 byConnPathLength = 0x01; // Length of “Connection Path” – route through ControlLogix Backplane
	memcpy(pBuff, &byConnPathLength, sizeof(byConnPathLength));
	pBuff += sizeof(byConnPathLength);

	ACE_UINT8 byReserved2 = 0x00; // Reserved – must be zero
	memcpy(pBuff, &byReserved2, sizeof(byReserved2));
	pBuff += sizeof(byReserved2);

	ACE_UINT8 byRouter = 0x01; // Route to ControlLogix Backplane – MUST be one (1)
	memcpy(pBuff, &byRouter, sizeof(byRouter));
	pBuff += sizeof(byRouter);

	ACE_UINT8 bySlotNo = m_nSlotOfCPU; // Which SLOT – normally ControlLogix CPU is in slot 0
	memcpy(pBuff, &bySlotNo, sizeof(bySlotNo));
	pBuff += sizeof(bySlotNo);

	nOutBuffLen = pBuff - szOutRequestBuffer;
	return 0;
}

int CIPDevice::Build_SLC500_Service_UnConnectedSend_RSLinuxUnknown_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen)
{
	char *pBuff = szOutRequestBuffer;

	// slc500 未知的服务
	ACE_UINT8 unkownSerivce = 0x4b;
	::memcpy(pBuff, &unkownSerivce, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	ACE_UINT8 requestPathSize = 0x02; //2 word = 10 byte
	::memcpy(pBuff, &requestPathSize, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	ACE_UINT32 requestPath = 0x01246720; // 20(Class Segment):Logical Segment, Class=0x67 (Rockwell Specific, PCCC Object), 24(Instance Segment):No.1
	::memcpy(pBuff, &requestPath, sizeof(ACE_UINT32));
	pBuff += sizeof(ACE_UINT32);

	ACE_UINT16 fixHead = 0x0107; // ??????
	::memcpy(pBuff, &fixHead, sizeof(ACE_UINT16));
	pBuff += sizeof(ACE_UINT16);

	ACE_UINT8 un = 0; // GUESS  STS / Status – send as zero
	::memcpy(pBuff, &un, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	ACE_UINT16 trans = m_nTransId;  //剩余数据事务号, 不知道有啥用
	to_little_endian(&trans, sizeof(ACE_UINT16));
	::memcpy(pBuff, &trans, sizeof(ACE_UINT16));
	pBuff += sizeof(ACE_UINT16);

	ACE_UINT32 unkown1 = 0x0f0000; // ????????
	::memcpy(pBuff, &unkown1, sizeof(ACE_UINT32));
	pBuff += sizeof(ACE_UINT32);

	ACE_UINT8 u1 = 0x12;// ????????
	::memcpy(pBuff, &u1, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	u1 = 0x11;// ????????
	::memcpy(pBuff, &u1, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	u1 = 0xa2; // PCCC FNC = Protected Typed Logical Read with 3 address field
	::memcpy(pBuff, &u1, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	ACE_UINT8 byByteCountToRead = 0;
	ACE_UINT8 byFileType = 0x89; // File Type = 16-bit INTEGER – see DF1 manual for other types===================
	ACE_UINT8 byFileNumber = 0x00; // File Number defined in ControlLogix – limited 0 to 255===================
	ACE_UINT16 uStartRegisterNo = 0x00;
	int nRet = GetFileType_SLC500(pDevice, pTagGroup, byFileType, byFileNumber, uStartRegisterNo, byByteCountToRead);
	if (nRet != 0)
		return nRet;

	::memcpy(pBuff, &byByteCountToRead, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	::memcpy(pBuff, &byFileNumber, sizeof(ACE_UINT8));//请求的文件号，N10:31，文件号就是10
	pBuff += sizeof(ACE_UINT8);

	::memcpy(pBuff, &byFileType, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	ACE_UINT8 byStartRegisterNo = uStartRegisterNo; // Read starting at this element , WARNING – must PAD if byte 48 is ODD – but NO PAD in this example!!
	::memcpy(pBuff, &byStartRegisterNo, sizeof(ACE_UINT8)); // zero-based
	pBuff += sizeof(ACE_UINT8);

	u1 = 0;
	::memcpy(pBuff, &u1, sizeof(ACE_UINT8));
	pBuff += sizeof(ACE_UINT8);

	nOutBuffLen = pBuff - szOutRequestBuffer;
	return 0; 
}

// 内容应该是：
// [63Bytes] 6f 00 27 00 00 00 00 00 00 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 01 00 00 00 0f 00 12 11 a2 02 0a 89 0f 00

// 下面是FameView
// Request,N10:25,2长度:	[63Bytes] 6f 00 27 00 00 4f 02 2f 00 00 00 00 00 00 00 00 1d 01 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 2e 01 00 00 0f 00 12 11 a2 04 0a 89 19 00
// Reply:C7 00 e8 03		[59Bytes] 6f 00 23 00 00 4f 02 2f 00 00 00 00 00 00 00 00 1d 01 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 13 00 cb 00 00 00 07 01 00 2e 01 00 00 4f 00 12 11 c7 00 e8 03
// Request,N10:15,10长度:	[63Bytes] 6f 00 27 00 00 50 02 2f 00 00 00 00 00 00 00 00 13 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 39 00 00 00 0f 00 12 11 a2 14 0a 89 0f 00
// Reply,					[75Bytes] 6f 00 33 00 00 50 02 2f 00 00 00 00 00 00 00 00 13 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 23 00 cb 00 00 00 07 01 00 39  00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01
// Request,N10:15,12长度:	[63Bytes] 6f 00 27 00 00 51 02 2f 00 00 00 00 00 00 00 00 0f 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 19 00 00 00 0f 00 12 11 a2 18 0a 89 0f 00
// Reply,					[79Bytes] 6f 00 37 00 00 51 02 2f 00 00 00 00 00 00 00 00 0f 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 27 00 cb 00 00 00 07 01 00 19 00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01 29 01 e8 03
int CIPDevice::Build_SLC500_SendRRData_RSLinuxLike_ReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen)
{
	char szUnConnSendReadBuffer[1024];
	int nUnConnSendReadBuffLen = 0;
	int nRet = Build_SLC500_Service_UnConnectedSend_RSLinuxUnknown_Packet(pDevice, pTagGroup, szUnConnSendReadBuffer, nUnConnSendReadBuffLen);
	if (nRet != 0)
		return nRet;

	nRet = Build_Command_SendRRData_Packet(pDevice, szUnConnSendReadBuffer, nUnConnSendReadBuffLen, szOutRequestBuffer, nOutBuffLen);
	return nRet;
}


int CIPDevice::Build_SLC500_SendRRData_UnConnectedSend_ReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen)
{
	char szPCCCReadBuffer[512];
	int nPCCCReadBuffLen = 0;
	int nRet = Build_SLC500_Service_PCCCReadData_Packet(pDevice, pTagGroup, szPCCCReadBuffer, nPCCCReadBuffLen);
	if (nRet != 0)
		return nRet;

	char szUnConnSendReadBuffer[512];
	int nUnConnSendReadBuffLen = 0;
	nRet = Build_Service_UnconnectedSend_Packet(pDevice, szPCCCReadBuffer, nPCCCReadBuffLen, szUnConnSendReadBuffer, nUnConnSendReadBuffLen);
	if (nRet != 0)
		return nRet;
	
	nRet = Build_Command_SendRRData_Packet(pDevice, szUnConnSendReadBuffer, nUnConnSendReadBuffLen, szOutRequestBuffer, nOutBuffLen);
	szOutRequestBuffer[48] = 0x18; // Length of CIP message we are ‘unconnected sending’ (in BYTES)
	return nRet;
}

// 构建读取一组tag点的消息.SendUnitData-->MultiRequest-->ReadFragmentData
// ExitPalletData.PalletNumber
// reqst:[0094] 70 00 46 00 D8 DD 02 17 00 00 00 00 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 0F C7 B3 00 B1 00 32 00 14 00 0A 02 20 02 24 01 01 00 04 00 52 0F 91 0E 45 78 69 74 50 61 6C 6C 65 74 44 61 74 61 91 0C 50 61 6C 6C 65 74 4E 75 6D 62 65 72 01 00 00 00 00 00
// reply:[0064] 70 00 28 00 D8 DD 02 17 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 93 17 00 00 B1 00 14 00 0E 00 8A 00 00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
int CIPDevice::Build_SendUnitData_Cmd_MultiRequest_ReadData_Service_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutMultiReadBuffer, int &nOutMultiReadBufLen)
{
	int nRet = 0; 
	vector<string> vecReadTagBuffer;
	for (int i = 0; i < pTagGroup->vecTags.size(); i++)
	{
		PKTAG *pTag = pTagGroup->vecTags[i];
		char szOneTagBuff[512];
		int nOneTagBuffLen = 0;
		nRet = Build_Service_ReadFragData_Packet(pDevice, pTag, szOneTagBuff, nOneTagBuffLen);
		if (nRet != 0)
			continue;
		string strOneService;
		strOneService.assign(szOneTagBuff, nOneTagBuffLen);
		//strOneService.reserve(nOneTagBuffLen);
		//memcpy((void *)strOneService.data(), szOneTagBuff, nOneTagBuffLen);
		vecReadTagBuffer.push_back(strOneService);
	}

	char szOutMultiReqBuffer[1024];
	int nOutMultiReqBuffLen = 0;
	nRet = Build_Service_MultiRequest_Packet(pDevice, vecReadTagBuffer, szOutMultiReqBuffer, nOutMultiReqBuffLen);
	if (nRet != 0)
		return -1;

	nRet = Build_Command_SendUnitData_Packet(pDevice, szOutMultiReqBuffer, nOutMultiReqBuffLen, szOutMultiReadBuffer, nOutMultiReadBufLen);

	return 0;
}

// 尝试注册session
int CIPDevice::CheckRegisterSession(PKDEVICE *pDevice)
{
	if (m_nSessionId != 0) // 完成注册会话
		return 0;

	// 需要先注册会话
	char szRequestBuffer[1024];
	int nTransId = GetTransactionId(pDevice);
	int nPackLen = Build_Command_RegisterSession_Packet(pDevice, szRequestBuffer, nTransId);
	// 一个连接只有一个会话。如果不断开连接，那么重复发送的registerSession将会返回句柄为1、status==1.此时必须断开连接或者注销连接
	Drv_Disconnect(pDevice);
	Drv_Connect(pDevice, 500);

	int nSentBytes = Drv_Send(pDevice, szRequestBuffer, nPackLen, 500);
	if (nSentBytes != nPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, send RegisterSession request failed, Need to Send:%d, Sent:%d", pDevice->szName, nPackLen, nSentBytes);
		return -1;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s,send RegisterSession request success, Sent:%d", pDevice->szName, nSentBytes);
	}

	char szHopePackBuf[1024];
	int nHopePackLen = 0;
	int nRet = RecvAndGetHopedReplyPacket(pDevice, COMMAND_REGISTERSESSION, nTransId, 3000, szHopePackBuf, nHopePackLen);
	if (nRet != 0)
	{
		m_nSessionId = 0;
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, fail to recv a RegisterSession reply, transId:%d", pDevice->szName, nTransId);
		return 0;
	}

	if (nHopePackLen < sizeof(CIPHEADER)) // 返回这个说明该连接前面已经注册过了！需要先注销会话再重新开始.此时长度为24，只有一个头部长度。否则返回28，包含注册会话的4个返回字节！
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv RegisterSession reply, but recv len:%d < 24(CIP HEADERSIZE:24), transId:%d",
			pDevice->szName, nHopePackLen, nTransId);
		return -2;
	}

	CIPHEADER cipHeader;
	memcpy(&cipHeader, szHopePackBuf, sizeof(cipHeader));
	if (cipHeader.ulStatus == 1) // 重复注册了！要想再注册需要断开再开始！这时的SessionId为0
	{
		m_nSessionId = cipHeader.ulSessionHandle;
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s RegisterSession Reply:Status:%d == 1 <> 0, sessionId:%d, need to UnRegister First!!", pDevice->szName, cipHeader.ulStatus, m_nSessionId);
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "!!!device:%s, success to recv RegisterSession reply, recvLen:%d, but replay status:%d <> 0, SESSION CREATED BEFORE, MUST DISCONNECT NETWORK first..., sessionHandle:%d, transId:%d",
		//	pDevice->szName, nHopePackLen, registerSessionReply.cipHeader.ulStatus, registerSessionReply.cipHeader.ulSessionHandle, nTransId);
		UnRegisterSession(pDevice);
		return 0;
	}
		
	if (cipHeader.ulStatus == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, success to recv RegisterSession reply, recvLen:%d, sessionHandle:%d,transId:%d",
			pDevice->szName, nHopePackLen, cipHeader.ulSessionHandle, nTransId);
		m_nSessionId = cipHeader.ulSessionHandle;
		return 0;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv RegisterSession reply, but replay status:%d <> 0, sessionHandle:%d, recvLen:%d, transId:%d",
			pDevice->szName, cipHeader.ulStatus, cipHeader.ulSessionHandle, nHopePackLen, nTransId);
		m_nSessionId = cipHeader.ulSessionHandle;
	}

	return 0;
}


// 尝试注册session
int CIPDevice::UnRegisterSession(PKDEVICE *pDevice)
{
	if (m_nSessionId == 0) // 未注册会话，不需要UnRegister
		return 0;

	// 需要先注册会话
	char szRequestBuffer[1024];
	int nTransId = GetTransactionId(pDevice);
	int nPackLen = Build_Command_UnRegisterSession_Packet(pDevice, szRequestBuffer, nTransId);
	int nSentBytes = Drv_Send(pDevice, szRequestBuffer, nPackLen, 500);
	if (nSentBytes != nPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, send UnRegisterSession request failed, Need to Send:%d, Sent:%d", pDevice->szName, nPackLen, nSentBytes);
		return -1;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s,send UnRegisterSession request success, Sent:%d", pDevice->szName, nSentBytes);
		m_nSessionId = 0; // 标志连接断开
		m_nT2OConnID = 0; // 标志ForwardOpen会话断开
	}
	return 0;
}

// 尝试注册session
int CIPDevice::CheckForwardOpenSuccess(PKDEVICE *pDevice)
{
	if (m_nO2TConnID != 0) // 已经执行了ForwardOpenSuccess
		return 0;

	char szRequestBuffer[1024];
	int nTransId = GetTransactionId(pDevice);
	int nPackLen = Build_SendRRData_ForwardOpenService_Request_Packet(pDevice, szRequestBuffer, nTransId);
	int nSentBytes = Drv_Send(pDevice, szRequestBuffer, nPackLen, 500);
	if (nSentBytes != nPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, send ForwardOpenService request failed, Need to Send:%d, Sent:%d", pDevice->szName, nPackLen, nSentBytes);
		return -1;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s,send ForwardOpenService request success, Sent:%d", pDevice->szName, nSentBytes);
	}

	char szHopePackBuf[1024];
	int nHopePackLen = 0;
	int nRet = RecvAndGetHopedReplyPacket(pDevice, COMMAND_SENDRRDATA, nTransId, 3000, szHopePackBuf, nHopePackLen);
	if (nRet == 0)
	{
		if (nHopePackLen != sizeof(FwdOpenReplyStruct))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv  Forward Open Reply, recvLen:%d <> NeedLen:%d, transId:%d",
				pDevice->szName, nHopePackLen, sizeof(FwdOpenReplyStruct), nTransId);
			return -1;
		}

		FwdOpenReplyStruct fwdOpenReply;
		memcpy(&fwdOpenReply, szHopePackBuf, sizeof(FwdOpenReplyStruct));
		if (fwdOpenReply.sendRRData.cipHeader.ulStatus == 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, success to recv Forward Open Reply, recvLen:%d, sessionHandle:%d,transId:%d",
				pDevice->szName, nHopePackLen, fwdOpenReply.sendRRData.cipHeader.ulSessionHandle, nTransId);

			if ((fwdOpenReply.byStatus != 0x00) || (fwdOpenReply.byAddStatus != 0x00) || (fwdOpenReply.ulT2OConnID != this->m_nT2OConnID))  // 打开链接失败
			{
				Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, success to recv  Forward Open Reply, recvLen:%d, sessionHandle:%d,transId:%d, reply.byStatus:%d(== 0), byAddStatus:%d(== 0),T2OConnID:%d,hope:%d",
					pDevice->szName, nHopePackLen, fwdOpenReply.sendRRData.cipHeader.ulSessionHandle, nTransId, fwdOpenReply.byStatus, fwdOpenReply.byAddStatus, fwdOpenReply.ulT2OConnID, m_nT2OConnID);
				return -1;
			}

			this->m_nO2TConnID = fwdOpenReply.ulO2TConnID; // 说明已经连接上了
			this->m_nO2TConnSN = fwdOpenReply.uO2TConnSN;
		}
		else if (fwdOpenReply.sendRRData.cipHeader.ulStatus == 1) // 不支持该服务！
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "!!!device:%s, success to recv  Forward Open Reply, recvLen:%d, but replay status:%d <> 0, SESSION CREATED BEFORE, MUST DISCONNECT NETWORK first..., sessionHandle:%d, transId:%d",
				pDevice->szName, nHopePackLen, fwdOpenReply.sendRRData.cipHeader.ulStatus, fwdOpenReply.sendRRData.cipHeader.ulSessionHandle, nTransId);
			this->m_nO2TConnID = 0;
			this->m_nO2TConnSN = 0;
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv  Forward Open Reply, but replay status:%d <> 0, sessionHandle:%d, recvLen:%d, transId:%d",
				pDevice->szName, fwdOpenReply.sendRRData.cipHeader.ulStatus, fwdOpenReply.sendRRData.cipHeader.ulSessionHandle, nHopePackLen, nTransId);
		}
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, fail to recv a Forward Open Reply, transId:%d", pDevice->szName, nTransId);
	}
	return 0;
}

// 定时查询tag组的数据
// ExitPalletData.PalletNumber
// reqst:[0094] 70 00 46 00 D8 DD 02 17 00 00 00 00 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 0F C7 B3 00 B1 00 32 00 [14 00 0A 02 20 02 24 01 01 00 04 00 52 0F 91 0E 45 78 69 74 50 61 6C 6C 65 74 44 61 74 61 91 0C 50 61 6C 6C 65 74 4E 75 6D 62 65 72 01 00 00 00 00 00
// reply:[0064] 70 00 28 00 D8 DD 02 17 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 93 17 00 00 B1 00 14 00 [0E 00 8A 00 00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
int CIPDevice::OnTimer_Logix5000_ReadTags(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	CIPDevice *pCIPDevice = (CIPDevice *)this;
	pCIPDevice->CheckRegisterSession(pDevice); // 检查会话连接情况
	if (pCIPDevice->m_nSessionId == 0) // 尚未注册会话，需要先注册会话
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, NOT RegisterSession !!! cannot send RR data.", pDevice->szName);
		return -1;
	}

	pCIPDevice->CheckForwardOpenSuccess(pDevice);
	if (pCIPDevice->m_nO2TConnID == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, NOT FORWARD OPEN !!! cannot send UnitData data.", pDevice->szName);
		return -1;
	}

	// 组织读取消息
	PKTAG *pTag = pTagGroup->vecTags[0];

	// 事务号
	int nTransId = pCIPDevice->GetTransactionId(pDevice);
	int nTotalPackLen = 0;
	char szRequestBuffer[1024];
	int nRet = Build_SendUnitData_Cmd_MultiRequest_ReadData_Service_Packet(pDevice, pTagGroup, szRequestBuffer, nTotalPackLen);
	int nSentBytes = Drv_Send(pDevice, szRequestBuffer, nTotalPackLen, 500);
	if (nSentBytes != nTotalPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, group:%s, tagnum:%d, firstag:%s,addr:%s, send ReadMessage request failed, Need to Send:%d, Sent:%d",
			pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->vecTags.size(), pTag->szName, pTag->szAddress, nTotalPackLen, nSentBytes);
		return -1;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, group:%s, tagnum:%d, firstag:%s,addr:%s, send SendUnitData_MultiRequest_Reads request success, Sent:%d",
			pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->vecTags.size(), pTag->szName, pTag->szAddress, nSentBytes);
	}

	char szHopePackBuf[1024];
	int nHopePackLen = 0;
	nRet = pCIPDevice->RecvAndGetHopedReplyPacket(pDevice, COMMAND_SENDUNITDATA, nTransId, 3000, szHopePackBuf, nHopePackLen);
	if (nRet == 0)
	{
		SendUnitDataStruct sendUnitDataReply_Header; // SendRRData返回的前一部分数据
		memcpy(&sendUnitDataReply_Header, szHopePackBuf, sizeof(SendUnitDataStruct));
		if (sendUnitDataReply_Header.cipHeader.ulStatus == 0) // 返回数据错误了
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, success to recv SendUnitData_MultiRequest_Reads reply, recvLen:%d, transId:%d", pDevice->szName, nHopePackLen, nTransId);

			// 前面只是个头部, 需要继续解析后续的数据
			char *pBuff = szHopePackBuf + sizeof(SendUnitDataStruct); // 44, 跳过头部44，从事务号03开始
			char *pPackEnd = szHopePackBuf + nHopePackLen;

			// 0E 00 8A 00 00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
			// 后面是：事务号（2B）、服务类型（8A） 0E 00
			unsigned short nTransId = 0;
			memcpy(&nTransId, pBuff, sizeof(unsigned short));
			pBuff += sizeof(unsigned short);

			// 事务号不等！
			if (nTransId != pCIPDevice->m_nTransId) // 0A
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but TransNo:%d != %d(Hoped)(multirequest response)",
					pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, nTransId, pCIPDevice->m_nTransId);
				return -100;
			}

			// 获取服务类型, 应该是0A+0x80 // 0E 00		8A 00 00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
			ACE_UINT16 uSerCode = 0x00;
			to_normal(pBuff, sizeof(ACE_UINT16));
			memcpy(&uSerCode, pBuff, sizeof(ACE_UINT16));
			pBuff += sizeof(ACE_UINT16);

			// 是否是获取多次服务命令的响应
			if (uSerCode != SERVICE_MULTIREQUESTREPLY) // 0A
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but ServiceCode:%d != 0x8A(multirequest response)",
					pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSerCode);
				return -100;
			}

			// 跳过保留的两个字节0x00 // 0E 00 8A 00		00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
			pBuff += sizeof(ACE_UINT16);

			char *pServiceOffsetBegin = pBuff; // 偏移从这里算起
			// 获取服务个数 // 0E 00 8A 00 00 00	01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
			ACE_UINT16 uSericeCnt = 0x00;
			to_normal(pBuff, sizeof(uSericeCnt));
			memcpy(&uSericeCnt, pBuff, sizeof(uSericeCnt));
			pBuff += sizeof(uSericeCnt);
			if (uSericeCnt <= 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but uSericeCnt:%d <= 0(multirequest response)",
					pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt);
				return -200;
			}

			Drv_LogMessage(PK_LOGLEVEL_INFO, "====device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, uSericeCnt:%d > 0(multirequest response)",
				pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt);

			vector<PKTAG *> vecTag2Update;
			// 0E 00 8A 00 00 00 01 00		04 00 D2 00 00 00 C4 00 0A 00 00 00
			// 每个服务的内容取出来
			for (int i = 0; i < uSericeCnt; i++)
			{
				if (i >= pTagGroup->vecTags.size())
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, uSericeCnt:%d > 0(multirequest response), reply No.%d >= tagcount:%d",
						pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt, i, pTagGroup->vecTags.size());
					continue;
				}

				PKTAG *pTag = pTagGroup->vecTags[i];
				ACE_UINT16 nOffsetByte = 0;
				memcpy(&nOffsetByte, pBuff, sizeof(ACE_UINT16));  // 0E 00 8A 00 00 00 01 00	04 00 D2 00 00 00 C4 00 0A 00 00 00
				to_normal(&nOffsetByte, sizeof(ACE_UINT16));
				pBuff += sizeof(ACE_UINT16);

				int nServiceBytes = 0;
				if (i == uSericeCnt - 1) // 最后1个，从pBuff到缓冲区结束
				{
					nServiceBytes = pPackEnd - pBuff;
				}
				else // 不是最后1个
				{
					ACE_UINT16 nNextOffsetByte = 0; // 需要下一个偏移以便计算该服务返回的长度
					memcpy(&nNextOffsetByte, pBuff, sizeof(ACE_UINT16));
					to_normal(&nNextOffsetByte, sizeof(ACE_UINT16));
					nServiceBytes = nNextOffsetByte - nOffsetByte;
				}
				if (nServiceBytes <= 0)
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, uSericeCnt:%d > 0(multirequest response), reply No.%d, nServiceBytes:%d <= 0",
						pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt, i, nServiceBytes);
					continue;
				}

				char szOneServiceBuff[1024];	// 一个服务返回的结果
				memcpy(szOneServiceBuff, pBuff, nServiceBytes);	// 0E 00 8A 00 00 00 01 00 04 00		D2 00 00 00 C4 00 0A 00 00 00
				char szHexBuf[512];
				unsigned int nRetHexBuf = 0;
				PKStringHelper::HexDumpBuf(szOneServiceBuff, nServiceBytes, szHexBuf, sizeof(szHexBuf), &nRetHexBuf);
				Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, tag:%s, OneReadService:%s", pDevice->szName, pTag->szName, szHexBuf);

				// D2 00 00 00 C4 00 0A 00 00 00
				char *pOneServiceBuff = szOneServiceBuff;	//		D2		00 00 00 C4 00 0A 00 00 00
				ACE_UINT8 byServiceCode = *(ACE_UINT8 *)pOneServiceBuff;
				if (byServiceCode != 0xD2)
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, uSericeCnt:%d > 0(multirequest response), reply No.%d, nServiceBytes:%d, ServiceCode:0x%02x != 0xD2",
						pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt, i, nServiceBytes, byServiceCode);
					continue;
				}
				pOneServiceBuff++;	// skip service no 0xD2
				pOneServiceBuff++; // skip 0x00 // D2 [00] 00 00 C4 00 0A 00 00 00
				ACE_UINT16 uOneServiceStatus = 0;
				memcpy(&uOneServiceStatus, pOneServiceBuff, sizeof(ACE_INT16)); // 状态, D2 00 [00 00] C4 00 0A 00 00 00
				if (uOneServiceStatus != 0)
				{
					CIOI * pSymbolAddr = (CIOI *)pTag->pData1;
					// 获取地址缓冲区并插入
					long nAddrLen;
					char *pAdrBuf = pSymbolAddr->GetIOIBuff(&nAddrLen);
					char szHexBuf[512];
					unsigned int nRetHexBuf = 0;
					PKStringHelper::HexDumpBuf(pAdrBuf, nAddrLen, szHexBuf, sizeof(szHexBuf), &nRetHexBuf);

					Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:%s, multirequest read of group:%s, tag:%s, reply service STATUS:0x%04x <>0, TAGADDRESS:%s, TAG IOI BUFFER:%s", pDevice->szName, pTagGroup->szAutoGroupName, pTag->szName, uOneServiceStatus, pTag->szAddress, szHexBuf);
					continue;
				}
				pOneServiceBuff += sizeof(ACE_INT16);

				// D2 00 00 00 [C4 00 0A 00 00 00]
				ACE_UINT16 uDataType = 0;
				memcpy(&uDataType, pOneServiceBuff, sizeof(ACE_UINT16)); // ????datatype? // D2 00 00 00 [C4 00] 0A 00 00 00
				to_normal(&uDataType, sizeof(ACE_UINT16));
				pOneServiceBuff += sizeof(ACE_INT16);

				// data // D2 00 00 00 C4 00 [0A 00 00 00]
				char *pOneServiceBufEnd = szOneServiceBuff + nServiceBytes;
				int nOneServiceDataLen = pOneServiceBufEnd - pOneServiceBuff;
				to_normal(pOneServiceBuff, nOneServiceDataLen);
				unsigned short uTagValue;
				memcpy(&uTagValue, pOneServiceBuff, 2);
				Drv_SetTagData_Binary(pTag, pOneServiceBuff, nOneServiceDataLen); // tag点必须配置为short或者unsigned short类型
				vecTag2Update.push_back(pTag);

				if (nRet == 0)
					Drv_LogMessage(PK_LOGLEVEL_INFO, "===device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d, tagValue(uint16):%d",
					pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pOneServiceBuff, *(pOneServiceBuff + 1), nOneServiceDataLen, uTagValue);
				else
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d, but Drv_UpdateTagsData return:%d",
					pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pOneServiceBuff, *(pOneServiceBuff + 1), nOneServiceDataLen, nRet);
			}

			// 更新所有的数据
			int nRet = Drv_UpdateTagsData(pDevice, vecTag2Update.data(), vecTag2Update.size());

			vecTag2Update.clear();
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv SendUnitData_MultiRequest_Reads reply, but replay status:%d <> 0, sessionHandle:%d, recvLen:%d, transId:%d",
				pDevice->szName, sendUnitDataReply_Header.cipHeader.ulStatus, sendUnitDataReply_Header.cipHeader.ulSessionHandle, nHopePackLen, nTransId);
			pCIPDevice->ResetAttributes(); // 收到数据错误，认为会话异常，需要重新建立会话！应该判断是否是连接断开才更准确
		}
	}
	else
	{
		pCIPDevice->ResetAttributes();
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, fail to recv a SendRRData reply of tag:%s, address:%s, transId:%d", pDevice->szName, pTag->szName, pTag->szAddress, nTransId);
	}
	return 0;
}

// 定时查询tag组的数据 N10:25
// reqst:[0094] 70 00 46 00 D8 DD 02 17 00 00 00 00 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 0F C7 B3 00 B1 00 32 00 [14 00 0A 02 20 02 24 01 01 00 04 00 52 0F 91 0E 45 78 69 74 50 61 6C 6C 65 74 44 61 74 61 91 0C 50 61 6C 6C 65 74 4E 75 6D 62 65 72 01 00 00 00 00 00
// reply:[0064] 70 00 28 00 D8 DD 02 17 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 A1 00 04 00 93 17 00 00 B1 00 14 00 [0E 00 8A 00 00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
int CIPDevice::OnTimer_SLC500_ReadTags(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	CIPDevice *pCIPDevice = (CIPDevice *)this;
	pCIPDevice->CheckRegisterSession(pDevice); // 检查会话连接情况
	if (pCIPDevice->m_nSessionId == 0) // 尚未注册会话，需要先注册会话
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s SLC500, NOT RegisterSession !!! cannot read data.", pDevice->szName);
		return -1;
	}
	PKTAG *pTag = pTagGroup->vecTags[0];
	// 事务号
	unsigned short nTransId = pCIPDevice->GetTransactionId(pDevice);
	int nTotalPackLen = 0;
	char szRequestBuffer[1024];
	//int nRet = Build_SLC500_SendRRData_UnConnectedSend_ReadData_Packet(pDevice, pTagGroup, szRequestBuffer, nTotalPackLen);
	int nRet = Build_SLC500_SendRRData_RSLinuxLike_ReadData_Packet(pDevice, pTagGroup, szRequestBuffer, nTotalPackLen);
	int nSentBytes = Drv_Send(pDevice, szRequestBuffer, nTotalPackLen, 500);
	if (nSentBytes != nTotalPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s SLC500, group:%s, tagnum:%d, firstag:%s,addr:%s, send ReadMessage request failed, Need to Send:%d, Sent:%d",
			pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->vecTags.size(), pTag->szName, pTag->szAddress, nTotalPackLen, nSentBytes);
		return -1;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s SLC500, group:%s, tagnum:%d, firstag:%s,addr:%s, send SendUnitData_MultiRequest_Reads request success, Sent:%d",
			pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->vecTags.size(), pTag->szName, pTag->szAddress, nSentBytes);
	}

	char szHopePackBuf[1024];
	int nHopePackLen = 0;
	nRet = pCIPDevice->RecvAndGetHopedReplyPacket(pDevice, COMMAND_SENDRRDATA, nTransId, 3000, szHopePackBuf, nHopePackLen);
	if (nRet != 0)
	{
		pCIPDevice->ResetAttributes();
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, fail to recv a SendRRData reply of tag:%s, address:%s, transId:%d", pDevice->szName, pTag->szName, pTag->szAddress, nTransId);
		return nRet;
	}

	SendRRDataStruct sendDataReply_Header; // SendRRData返回的前一部分数据
	memcpy(&sendDataReply_Header, szHopePackBuf, sizeof(SendUnitDataStruct));
	if (sendDataReply_Header.cipHeader.ulStatus != 0) // 返回数据错误了
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, success to recv SendUnitData_MultiRequest_Reads reply, recvLen:%d, transId:%d, status:%d != 0", pDevice->szName, nHopePackLen, nTransId, sendDataReply_Header.cipHeader.ulStatus);
		return -100;
	}
	
	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, success to recv SendUnitData_MultiRequest_Reads reply, recvLen:%d, transId:%d", pDevice->szName, nHopePackLen, nTransId);

	// 前面只是个头部, 需要继续解析后续的数据
	char *pBuff = szHopePackBuf + sizeof(SendRRDataStruct); // 44, 跳过头部44，从事务号03开始
	char *pPackEnd = szHopePackBuf + nHopePackLen;

	// PCCC Object Service Code or’d with 0x80 to indicate REPLY
	// 获取服务类型, 应该是4C+0x80 
	ACE_UINT16 uSerCode = 0x00;
	to_normal(pBuff, sizeof(ACE_UINT16));
	memcpy(&uSerCode, pBuff, sizeof(ACE_UINT16));
	// pBuff += sizeof(ACE_UINT16);

	if (uSerCode == 0xCC) // ReadPCCCData的返回
	{
		int nSendDataRelyDataLen = sendDataReply_Header.uDataItemLen;
		OnParsePCCCReadDataReply(pDevice, pTagGroup, pTag, pBuff, nHopePackLen, nSendDataRelyDataLen);
	}
	else if (uSerCode == 0xCB) // 0x4B  1个长度：CB 00 00 00 07 01 00 8E 02 00 00 4F 00 12 11 00 00， 2个长度：cb 00 00 00 07 01 00 2e 01 00 00 4f 00 12 11 c7 00 e8 03
	{
		int nSendDataRelyDataLen = sendDataReply_Header.uDataItemLen;
		OnParse4BReadDataReply(pDevice, pTagGroup, pTag, pBuff, nHopePackLen, nSendDataRelyDataLen);

	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but ServiceCode:0x%02x != 0x8A(multirequest response)",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSerCode);
		return -100;
	}
	return 0;
}

int CIPDevice::OnParsePCCCReadDataReply(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *&pBuff, int nHopePackLen, int nSendDataRelyDataLen)
{
	pBuff += 2; // 跳过0xCC 00
	int nRet = 0;
	// 跳过保留的两个字节0x00 // CC 00		0E 00 8A 00		00 00 01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
	pBuff += sizeof(ACE_UINT16);

	char *pServiceOffsetBegin = pBuff; // 偏移从这里算起
	// 获取服务个数 // 0E 00 8A 00 00 00	01 00 04 00 D2 00 00 00 C4 00 0A 00 00 00
	ACE_UINT16 uSericeCnt = 0x00;
	to_normal(pBuff, sizeof(uSericeCnt));
	memcpy(&uSericeCnt, pBuff, sizeof(uSericeCnt));
	pBuff += sizeof(uSericeCnt);
	if (uSericeCnt <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but uSericeCnt:%d <= 0(multirequest response)",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt);
		return -200;
	}

	Drv_LogMessage(PK_LOGLEVEL_INFO, "====device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, uSericeCnt:%d > 0(multirequest response)",
		pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uSericeCnt);


	ACE_UINT8 byStatus[3]; // Response codes – should all be ZERO. If byte 42 is NOT zero, an error occurred.	Byte 42 will be the GRC or General Response Code.See ODVA manual for how to decode bytes following GRC
	to_normal(pBuff, sizeof(byStatus));
	memcpy(byStatus, pBuff, sizeof(byStatus));
	pBuff += sizeof(byStatus);
	if (byStatus[1] != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but status:%d != 0",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, byStatus[1]);
		return -200;
	}

	ACE_UINT8 byAddressInfo[8]; // PCCC addressing info – notice 4-byte pieces swapped
	to_normal(pBuff, sizeof(byAddressInfo));
	memcpy(byAddressInfo, pBuff, sizeof(byAddressInfo));
	pBuff += sizeof(byAddressInfo);

	ACE_UINT8 chCmdReply; // PCCC addressing info – notice 4-byte pieces swapped
	to_normal(pBuff, sizeof(chCmdReply));
	memcpy(&chCmdReply, pBuff, sizeof(chCmdReply));
	pBuff += sizeof(ACE_UINT8);
	if (chCmdReply != 0x4F)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but chCmdReply:0x%02x != 0x4F",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, chCmdReply);
		return -200;
	}

	ACE_UINT8 byPCCCStatus; // PCCC addressing info – notice 4-byte pieces swapped
	to_normal(pBuff, sizeof(byPCCCStatus));
	memcpy(&byPCCCStatus, pBuff, sizeof(byPCCCStatus));
	pBuff += sizeof(ACE_UINT8);
	if (byPCCCStatus != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but byPCCCStatus:0x%02x != 0",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, byPCCCStatus);
		return -200;
	}

	ACE_UINT16 uPCCCTransNo; // PCCC addressing info – notice 4-byte pieces swapped
	to_normal(pBuff, sizeof(uPCCCTransNo));
	memcpy(&uPCCCTransNo, pBuff, sizeof(uPCCCTransNo));
	pBuff += sizeof(ACE_UINT16);
	if (uPCCCTransNo != m_nTransId)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but uPCCCTransNo:%d != Request TransNo:%d",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, uPCCCTransNo, m_nTransId);
		return -200;
	}

	int nRRDataLen = nSendDataRelyDataLen;// sendDataReply_Header.uDataItemLen;
	int nTagDataLen = nRRDataLen - 16;

	vector<PKTAG *> vecTag2Update;
	Drv_SetTagData_Binary(pTag, pBuff, nTagDataLen); // tag点必须配置为short或者unsigned short类型
	vecTag2Update.push_back(pTag);

	if (nRet == 0)
		Drv_LogMessage(PK_LOGLEVEL_INFO, "===device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d",
		pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pBuff, *(pBuff + 1), nTagDataLen);
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d, but Drv_UpdateTagsData return:%d",
		pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pBuff, *(pBuff + 1), nTagDataLen, nRet);

	// 更新所有的数据
	nRet = Drv_UpdateTagsData(pDevice, vecTag2Update.data(), vecTag2Update.size());

	vecTag2Update.clear();
	return 0;
}

// FameView模拟
// Request,N10:15,10长度:	[63Bytes] 6f 00 27 00 00 50 02 2f 00 00 00 00 00 00 00 00 13 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 39 00 00 00 0f 00 12 11 a2 14 0a 89 0f 00
// Reply,					[75Bytes] 6f 00 33 00 00 50 02 2f 00 00 00 00 00 00 00 00 13 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 23 00 cb 00 00 00 07 01 00 39  00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01
// Request,N10:15,12长度:	[63Bytes] 6f 00 27 00 00 51 02 2f 00 00 00 00 00 00 00 00 0f 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 17 00 4b 02 20 67 24 01 07 01 00 19 00 00 00 0f 00 12 11 a2 18 0a 89 0f 00
// Reply,					[79Bytes] 6f 00 37 00 00 51 02 2f 00 00 00 00 00 00 00 00 0f 00 00 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 00 00 b2 00 27 00 cb 00 00 00 07 01 00 19 00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01 29 01 e8 03

// 0x4B命令的应答
// 01个长度：CB 00 00 00 07 01 00 8E 02 00 00 4F 00 12 11 00 00
// 02个长度：cb 00 00 00 07 01 00 2e 01 00 00 4f 00 12 11 c7 00 e8 03
// 10个长度：cb 00 00 00 07 01 00 39 00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01
// 12个长度：cb 00 00 00 07 01 00 19 00 00 00 4f 00 12 11 30 75 45 00 96 00 c8 00 2c 01 2c 01 28 23 64 00 fa 00 2c 01 29 01 e8 03
int CIPDevice::OnParse4BReadDataReply(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *&pBuff, int nHopePackLen, int nSendDataRelyDataLen)
{
	char *pPackBegin = pBuff;
	char szHexLog[512];
	unsigned int nHexLogLen = 0;
	PKStringHelper::HexDumpBuf(pPackBegin, nSendDataRelyDataLen, szHexLog, sizeof(szHexLog), &nHexLogLen);
	if (nSendDataRelyDataLen < 15) // 头部就有15个字节
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d<15, INVALID, content:%",
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nSendDataRelyDataLen, szHexLog);
		return -101;
	}

	ACE_UINT16 uSerCode = 0x00;
	to_normal(pBuff, sizeof(ACE_UINT16));
	memcpy(&uSerCode, pBuff, sizeof(ACE_UINT16));
	pBuff += sizeof(ACE_UINT16); //  [CB 00] 00 00 07 01 00 8E 02 00 00 4F 00 12 11] 00 00 .至少14个字节
	pBuff += 2; // CB 00 	[00 00] 07 01 00 8E 02 00 00 4F 00 12 11 00 00

	ACE_UINT16 n71 = 0;
	memcpy(&n71, pBuff, 2);
	pBuff += 2; // 跳过07 01， CB 00 00 00	[07 01] 00 8E 02 00 00 4F 00 12 11 00 00

	pBuff += 5; // 跳过 CB 00 00 00 07 01	【00 8E 02 00 00】 4F 00 12 11 00 00
	unsigned char n4F = *pBuff;
	pBuff++; // 跳过0x4F  CB 00 00 00 07 01	00 8E 02 00 00 【4F】 00 12 11 00 00
	
	ACE_UINT8 nStatus = *pBuff; //0x4F后面是状态
	pBuff++; // 跳过0x00  CB 00 00 00 07 01	00 8E 02 00 00 4F【 00】 12 11 00 00

	ACE_UINT16 n1211;
	memcpy(&n1211, pBuff, 2);
	pBuff += 2; // 跳过12 11， CB 00 00 00 07 01 00 8E 02 00 00 4F 00	[12 11] 00 00

	if (n71 != 0x0107 || n4F != 0x4F || nStatus != 0 || n1211 != 0x1112)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, but check invalid!n71:0x%x, n4F:0x%02x, nStatus:%d, n1211:0x%x, content:%s", 
			pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, n71, n4F, nStatus, n1211, szHexLog);
		return -100;
	}

	// 后面是数据区
	int nTagDataLen = pPackBegin + nSendDataRelyDataLen - pBuff;

	vector<PKTAG *> vecTag2Update;
	int nRet = Drv_SetTagData_Binary(pTag, pBuff, nTagDataLen); // tag点必须配置为short或者unsigned short类型
	vecTag2Update.push_back(pTag);

	if (nRet == 0)
		Drv_LogMessage(PK_LOGLEVEL_INFO, "===device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d",
		pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pBuff, *(pBuff + 1), nTagDataLen);
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[group tagnum:%d, device tagnum: %d], recv a data response package, packlen:%d, data:%02x %02x, datalen:%d, but Drv_UpdateTagsData return:%d",
		pDevice->szName, pTag->szName, pTagGroup->vecTags.size(), pDevice->nTagNum, nHopePackLen, *pBuff, *(pBuff + 1), nTagDataLen, nRet);

	// 更新所有的数据
	nRet = Drv_UpdateTagsData(pDevice, vecTag2Update.data(), vecTag2Update.size());

	vecTag2Update.clear();
	return 0;
}
