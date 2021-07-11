// Modbus 驱动。设备参数1为0表示ModbusRTU驱动，为1表示ModbusTCP驱动
// 数据块参数1表示站号。站号必须大于等于1
// 参数2：读写用哪些指令；
// 参数3：每个块的最大字节数
// 变量也具有一个参数：表示站号（如果一个设备接入了多个不同的组，每个组的站号可能是不同的）
// DRVTAG的nData1是起始地址位（相对于AI的0地址），nData2是结束地址位，nData3是在该块内的起始位数
#include "104drv.h"
#include "math.h"
#include "AutoGroup_BlkDev.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"

#define EC_ICV_INVALID_PARAMETER                    100
#define EC_ICV_DRIVER_DATABLOCK_TYPECANNOTWRITE		101
#define EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE			103
#define EC_ICV_DRIVER_DATABLOCK_INVALIDCMDLENGTH	104
#define EC_ICV_BUFFER_TOO_SMALL                     105
#define	INT_MAX										2147483647

#define DEVPARAM_TRANSID							0	// 事务号
#define DEVPARAM_STATION_NO							1	// 站号
#define DEVPARAM_RWCMD								2	// 读写用什么指令
#define DEVPARAM_MAXBYTES_ONEBLOCK					3	// 每个块的最大ID
#define DEVPARAM_CLEARFLAG							4	// 是否需要先清除缓冲区
#define PK_TAGDATA_MAXLEN							4096
/**
*  是否需要高低字节转换.
*
*
*  @version     08/21/2008  shijunpu  Initial Version.
*/
bool NeedSwap(char *szBlockType)
{
	if (!is_little_endian())
		return false;

	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AI) == 0 || PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0)
		return true;
	else
		return false;
}

int GetBlockTypeId(char *szBlockType)
{
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AI) == 0)
		return BLOCK_TYPE_ID_AI;
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0)
		return BLOCK_TYPE_ID_AO;
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DI) == 0)
		return BLOCK_TYPE_ID_DI;
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) == 0)
		return BLOCK_TYPE_ID_DO;
	return BLOCK_TYPE_ID_UNDEFINED;
}

/******************************************************************************************************
* FunName : CRC16
* Function : The function returns the CRC16 as a unsigned short type
* Input    : puchMsg - message to calculate CRC upon; usDataLen - quantity of bytes in message
* Output   : CRC value
******************************************************************************************************/
unsigned short CRC16(unsigned char * puchMsg, unsigned short usDataLen)
{
	unsigned char uchCRCHi = 0xFF; /* high byte of CRC initialized */
	unsigned char uchCRCLo = 0xFF; /* low byte of CRC initialized */
	unsigned uIndex; /* will index into CRC lookup table */
	while (usDataLen--) /* pass through message buffer */
	{
		uIndex = uchCRCLo ^ *puchMsg++; /* calculate the CRC */
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
}

bool IsValidFunctionCode(unsigned char nFuncCode)
{
	//modbus异常包返回的功能码为请求的功能码+0x80(见标准modbus协议介绍)
	if (nFuncCode > MODBUS_BASE_EXCEPTION_FUNC_CODE)
		nFuncCode -= MODBUS_BASE_EXCEPTION_FUNC_CODE;

	if (nFuncCode == FUNCCODE_READ_DO || nFuncCode == FUNCCODE_READ_DI
		|| nFuncCode == FUNCCODE_READ_AO || nFuncCode == FUNCCODE_READ_AI
		|| nFuncCode == FUNCCODE_WRITE_SGLDO || nFuncCode == FUNCCODE_WRITE_SGLAO
		|| nFuncCode == FUNCCODE_WRITE_MULTIDO || nFuncCode == FUNCCODE_WRITE_MULTIAO)
		return true;

	return false;
}

/**
*  $(Desp) .
*  $(Detail) .
*
*  @param		-[in,out]  char * pPackageHead : 包头指针
*  @param		-[in,out]  bool bModbusTcp : 是否modbus tcp
*  @return		int.
*
*  @version	12/24/2012  shijunpu  Initial Version.
*/
int GetTotalPackageLen(char *pPackageHead, int nPackLen, bool bModbusTcp)
{
	int nTotalPackageLen = 0;
	if (NULL == pPackageHead ||
		(bModbusTcp && nPackLen < PROTOCOL_TCP_PACKAGE_HEADER_LEN) ||
		(!bModbusTcp && nPackLen < PROTOCOL_RTU_PACKAGE_HEADER_LEN))
		return INT_MAX; // 包头都不够，则返回一个最大值，说明需要继续接受数据

	if (bModbusTcp)
	{
		char *pPackageLenBegin = pPackageHead + 4;
		nTotalPackageLen = (((unsigned char)*pPackageLenBegin) << BITS_PER_BYTE) + (unsigned char)*(pPackageLenBegin + 1);
		nTotalPackageLen += PROTOCOL_TCP_PACKAGE_HEADER_LEN;
	}
	else
	{
		char *pPackageLenBegin = pPackageHead + 2;
		nTotalPackageLen = (unsigned char)*(pPackageLenBegin)+PROTOCOL_RTU_PACKAGE_HEADER_LEN;
	}

	return nTotalPackageLen;
}

bool IsModbusTCP(PKDEVICE *pDevice)
{
#ifdef MODBUSTYPE_TCP
	return true;
#else
	return false;
#endif
}

// ModbusRTU时，设备ID不能为0，至少是1
// 先取tag点的第三个参数，如果没有则取设备的第三个参数。如果都没有则缺省为1
// 控制时处理了站号，但是读写时未处理站号？？？？？？？？？？？？？？？？？？？
unsigned char GetStationID(PKDEVICE *pDevice, PKTAG *pTag)
{
	unsigned char nStationNo = pDevice->nUserData[DEVPARAM_STATION_NO];
	if (pTag && strlen(pTag->szParam) > 0 && ::atoi(pTag->szParam) > 0)
		nStationNo = ::atoi(pTag->szParam);

	return nStationNo;
}

// 是否需要清楚标志位
void SetClearRecvBufferFlag(PKDEVICE *pDevice)
{
	pDevice->nUserData[DEVPARAM_CLEARFLAG] = 1;
}

bool GetClearRecvBufferFlag(PKDEVICE *pDevice)
{
	return pDevice->nUserData[DEVPARAM_CLEARFLAG] != 0;
}


void CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess)
{
	if (lSuccess == PK_SUCCESS)
		pTagGroup->nFailCountRecent = 0;
	else
	{
		if (pTagGroup->nFailCountRecent > 3)	// 最近失败次数
		{
			char szTip[1024] = { 0 };
			sprintf(szTip, "read failcount:%d", pTagGroup->nFailCountRecent);
			UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, szTip);
			pTagGroup->nFailCountRecent = 0; // 避免计数太大导致循环
		}
		else
			pTagGroup->nFailCountRecent += 1;
	}
}

long ParsePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *pszRecvBuf, long lRecvBufLen,
	unsigned short nRequestTransID, time_t tmRequest)
{
	bool bModbusTCP = IsModbusTCP(pDevice);
	if (bModbusTCP &&  lRecvBufLen < PROTOCOL_TCP_PACKAGE_HEADER_LEN || !bModbusTCP && lRecvBufLen < PROTOCOL_RTU_PACKAGE_HEADER_LEN)
	{
		UpdateGroupQuality(pDevice, pTagGroup, -100, "parsing,packlen:%d < headlen:%d,discard", lRecvBufLen, PROTOCOL_TCP_PACKAGE_HEADER_LEN);
		return -100;
	}

	int nStartBytes = pTagGroup->nStartAddrBit / 8; // 从0开始的第N个字节
	int nEndBytes = pTagGroup->nEndAddrBit / 8; // 从0开始的第N个字节
	int nLenBytes = nEndBytes - nStartBytes + 1;
	int nBlockSize = nLenBytes; //(int)ceil((pTagGroup->nEndBit - pTagGroup->nStartAddrBit + 1)/8.0f);
	// 循环解析收到的所有可能数据包
	// 2表示站号（1个字节）和功能码（1个字节）。
	// 任何请求应答的头6个字节都是一样的意义，第7和第8个字节也是一样的意义（站号和功能码）
	long nRecvBytes = lRecvBufLen;
	bool bFoundThisResponse = false;
	char *pCurBuf = (char *)pszRecvBuf;

	while ((bModbusTCP && nRecvBytes >= PROTOCOL_TCP_PACKAGE_HEADER_LEN) ||
		(!bModbusTCP && nRecvBytes >= PROTOCOL_RTU_PACKAGE_HEADER_LEN))
	{
		char *pThisPackHeader = pCurBuf;
		unsigned short uTransID = 0; // 仅对ModbusTCP有效
		unsigned short uPackBodyLen = 0; // 仅对ModbusTCP有效
		if (bModbusTCP)
		{
			memcpy(&uTransID, pCurBuf, 2);	// 事物号
			if (uTransID < 0) // 事务ID必须不能为0
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "解析包时, 发现事务ID为 %d<0, 缓冲区长度为 %d, 抛弃设备%s本次接收的其他数据!",
					uTransID, nRecvBytes, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -101, "parsing,packlen:%d < utransId:%d<0,discard", nRecvBytes, uTransID);
				return -101;
			}
			if (uTransID == nRequestTransID)
				bFoundThisResponse = true;

			pCurBuf += 2;	// first 2 bytes

			// 跳过协1d议号。永远是0
			unsigned short uProtocolID = 0;
			memcpy(&uProtocolID, pCurBuf, 2);	// 协议号
			if (uProtocolID < 0) // 协议ID必须不能为0
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "解析包时, 发现事务协议号为 %d, 必须是0! 剩余缓冲区长度为 %d, 抛弃设备%s本次接收的其他数据!",
					uProtocolID, lRecvBufLen, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -102, "parsing,packlen:%d < protocolId:%d<0,discard", nRecvBytes, uProtocolID);
				return -102;
			}
			pCurBuf += 2;  // No.3-4 bytes

			// 整个数据包长度
			uPackBodyLen = (((unsigned char)*pCurBuf) << BITS_PER_BYTE) + (unsigned char)*(pCurBuf + 1);
			// 长度非法，不可能为0
			if (uPackBodyLen <= 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "解析包时, 发现包长度为 %d, 剩余缓冲区长度为 %d,  抛弃设备%s本次接收的其他数据!",
					uPackBodyLen, lRecvBufLen, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -102, "parsing,packlen:%d < uPackBodyLen:%d<0,discard", nRecvBytes, uPackBodyLen);
				return -1;
			}
			pCurBuf += 2; // No.3 5-6 bytes
		} // if(bModbusTCP)

		// 站号
		unsigned char uStationID = (unsigned char)*pCurBuf; // 第7字节
		unsigned char nFuncCode = (unsigned char)* (pCurBuf + 1); // No.8 bytes

		//modbus异常包的工程码为请求的功能码+0x80，我们认为是合法的功能码
		if (!IsValidFunctionCode(nFuncCode))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Modbus应答包(事务号:%d)的功能号为 %d，不正确的请求应答功能号！抛弃设备 %s 本次接收的其余数据！",
				uTransID, nFuncCode, pDevice->szName);
			UpdateGroupQuality(pDevice, pTagGroup, -103, "parsing,packlen:%d < invalid funcCode:%d<0", nRecvBytes, nFuncCode);
			return -103;
		}

		// 如果包体长度+长度前的头6个字节，大于剩余缓冲区长度，说明不够一个包，返回
		if (bModbusTCP && (uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN > lRecvBufLen))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "设备 %s Modbus应答包(事务号:%d)的功能号为 %d，事务号:%d, 有不完整的包！",
				pDevice->szName, nFuncCode, uTransID);
			UpdateGroupQuality(pDevice, pTagGroup, -104, "parsing,recvbuf:%d < packlen(should):%d", lRecvBufLen, uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN);
			return -104;
		}

		//通过位操作，判断该功能码为异常包功能码,如果是异常包直接跳过该包
		if ((nFuncCode & MODBUS_BASE_EXCEPTION_FUNC_CODE) == MODBUS_BASE_EXCEPTION_FUNC_CODE)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Modbus请求(事务号:%d)返回异常包，异常包的功能号为 %d！", uTransID, nFuncCode);
			if (bModbusTCP)
			{
				nRecvBytes -= (uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN);
				pCurBuf += uPackBodyLen;
				UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, "recv exception funcid:%d", nFuncCode);
				continue;
			}
			else
			{
				UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, "recv exception funcid:%d", nFuncCode);
				continue;
			}
		}

		// 5/6/15/16 是写请求的应答, 读取请求的响应包 1/2/3/4是读请求
		bool bReadResponseFromDev = (nFuncCode <= 4);
		if (!bReadResponseFromDev)
		{
			return TAG_QUALITY_GOOD; // 驱动框架会自动设置控制的状态
		}

		// 注意：当申请的数据块长度超过256时（modbus模拟器最长为232字节），modbus模拟器返回的包头为请求字节数+3，但uReturnDataLen为232
		// 此时就不要查找事物号了，直接清空内存退出
		unsigned short uReturnDataLen = (unsigned char)*(pCurBuf + 2);	// 返回的数据的长度.No.8 byte

		// 发现Modbus模拟器在请求数据超过232字节时，uPackBody为请求长度+3，但uReturnDataLen为232
		// 因此，在进行前面所述的合法性验证之后，只将部分数据写入DIT中去？？？
		if (bModbusTCP && (uPackBodyLen & 0x00FF) != uReturnDataLen + 3)//重新理解了返回数据长度，该长度为body的低字节-3
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "设备 %s 解析包时, 解析出包区（包头+数据区）长度为 %d, 解析出数据区长度为 %d, 两者不匹配(前者长度应=后者+3）！（总缓冲区长度为 %d），抛弃该包!",
				pDevice->szName, uPackBodyLen, uReturnDataLen, lRecvBufLen);
			UpdateGroupQuality(pDevice, pTagGroup, -106, "recv bodylen(%d)!=retdatalen(%d)+3:%d", uPackBodyLen, uReturnDataLen);
			return -106;
		}

		// 一次读取到的块小于DIT长度也是要更新的
		if (nBlockSize >= uReturnDataLen)
		{
			char *szBlockType = pTagGroup->szHWBlockName; // AI/DI/AO/DO
			// 设置数据块的数据
			if (NeedSwap(szBlockType))
			{
				// 每个寄存器的长度（位）
				int nElemBits = 1;
				if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0 || PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AI) == 0)
					nElemBits = 16;

				// pCurBuf指向第6个字节，应该再跳过站号、功能码和长度共三个字节
				SwapByteOrder(pCurBuf + 3, uReturnDataLen, nElemBits / BITS_PER_BYTE);
			}

			// 更新数据块的数据。当在OnDatablockTimer的定时器中读取到数据后，调用该接口更新数据以便过程数据库读取
			UpdateGroupData(pDevice, pTagGroup, pCurBuf + 3, uReturnDataLen, TAG_QUALITY_GOOD);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "dev:%s, grp:%s, updata:%d bytes", pDevice->szName, pTagGroup->szAutoGroupName, uReturnDataLen);
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "数据块(%s)收到的读请求应答消息体长度(%d)和内存中长度(%d)不一致, 跳过",
				pTagGroup->szAutoGroupName, uReturnDataLen, nBlockSize);
			UpdateGroupQuality(pDevice, pTagGroup, -105, "recv packet datalen:%d too small", uReturnDataLen);
			return -105;
		}

		if (bModbusTCP)
		{
			pCurBuf += uPackBodyLen;
			nRecvBytes -= (uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN);
		}
		else
		{
			pCurBuf += (uReturnDataLen + 2); // CRC 01 04      14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     95 81 
			nRecvBytes -= (uReturnDataLen + 4);
		}

		Drv_LogMessage(PK_LOGLEVEL_INFO, "收到请求响应包（设备名:%s,块名：%s, 事务号:%d）", pDevice->szName, pTagGroup->szAutoGroupName, uTransID);
		//DRVTIME tvNow;
		//Drv_GetCurrentTime(&tvNow);
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "收到请求响应包（设备名:%s,块名：%s, 事务号:%d），包请求时间(%u.%u),包接收时间(%u.%u),时间差:%u毫秒",
		//	pDevice->szName, pTagGroup->szAutoGroupName, uTransID, (unsigned long)tmRequest.tv_sec, (unsigned long)tmRequest.tv_usec,
		//	(unsigned long)tvNow.tv_sec, (unsigned long)tvNow.tv_usec, abs((long)((tvNow.tv_sec - tmRequest.tv_sec) * 1000 +  (tvNow.tv_usec - tmRequest.tv_usec)/1000)));
	}//while(m_nRemainBufferLen > PROTOCOL_PACKAGE_HEADER_LEN + 2)

	return PK_SUCCESS;
}

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备param1:站号,缺省为1. 参数2:0表示正常指令读写,1表示多寄存器读写,缺省为0.  参数3:每个包最大字节数,缺省不限制.  参数4:未定义");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s)", pDevice->szName);
	pDevice->nUserData[DEVPARAM_TRANSID] = 0;				// 事务号

	// 参数1：站号
	pDevice->nUserData[DEVPARAM_STATION_NO] = 1;			// 站号，缺省为1
	if (pDevice->szParam1 != NULL && atoi(pDevice->szParam1) != 0)
		pDevice->nUserData[DEVPARAM_STATION_NO] = atoi(pDevice->szParam1);

	// 参数2：采用什么指令
	pDevice->nUserData[DEVPARAM_RWCMD] = 0;				// 读写用什么指令,0表示正常的指令，1表示多寄存器指令
	if (pDevice->szParam2 != NULL && atoi(pDevice->szParam2) != 0)
		pDevice->nUserData[DEVPARAM_RWCMD] = atoi(pDevice->szParam2);

	// 参数3：每个块最大字节数  modbus每种设备都有所不同，应该是作为参数传过来比较合适
	if (strlen(pDevice->szParam3) > 0)
	{
		int nCfgMaxBytes = ::atoi(pDevice->szParam3);
		if (nCfgMaxBytes > 0)
			pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK] = nCfgMaxBytes;
	}

	// 获取到所有的tag点。需要在tag点存储块内偏移（位）、长度（位），组包含的tag点对象列表（以便计算）

	// 进行自组块处理，将所有的tag点自组块成BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK]; // modbus每种设备都有所不同，应该是作为参数传过来比较合适
	if (groupOption.nMaxBytesOneGroup <= 0) //如果没有输入，则取缺省230个字节
		groupOption.nMaxBytesOneGroup = 230;

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();

	unsigned int i = 0;
	for (; i < vecTagGroup.size(); i++)
	{
		DRVGROUP *pTagGrp = vecTagGroup[i];

		int nBlockTypeId = GetBlockTypeId(pTagGrp->szHWBlockName);
		int nRegisterLenBits = 1;
		if (PKStringHelper::StriCmp(pTagGrp->szHWBlockName, BLOCK_TYPE_AO) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, BLOCK_TYPE_AI) == 0)
			nRegisterLenBits = 16;
		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // AI,AO按照字组块，DI、DO按照位组块


		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[0] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "modbustcp, device:%s, autogroup:%s,tagcount:%d, cycle:%d ms,registertype:%s,startregno:%d,endregno:%d",
			pDevice->szName, pTagGrp->szAutoGroupName, pTagGrp->vecTags.size(), pTagGrp->nPollRate, pTagGrp->szHWBlockName, pTagGrp->nBeginRegister, pTagGrp->nEndRegister);
		string strTags = "";
		for (int iTag = 0; iTag < pTagGrp->vecTags.size(); iTag++)
		{
			PKTAG *pTag = pTagGrp->vecTags[iTag];
			if (strTags.empty())
				strTags = pTag->szName;
			else
				strTags = strTags + "," + pTag->szName;
		}
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "modbustcp, device:%s, groupname:%s, tagnames:%s", pDevice->szName, pTagGrp->szAutoGroupName, strTags.c_str());
	}
	return 0;
}

/*
反初始化设备
*/
//#include "windows.h"
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	//Sleep(100000);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	int nTimerNum = 0;
	//PKTIMER * pTimers = Drv_GetTimers(pDevice, &nTimerNum);
	//int i = 0;
	//for (i = 0; i < nTimerNum; i++)
	//{
	//	PKTIMER *pTimerInfo = &pTimers[i];
	//	DRVGROUP *pTagGroup = (DRVGROUP *)pTimerInfo->pUserData[0];
	//	delete pTagGroup;
	//	pTagGroup = NULL;
	//	Drv_DestroyTimer(pDevice, pTimerInfo);
	//}
	return 0;
}

PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDriver(driver:%s)", pDriver->szName);
	return 0;
}
/*
int Drv_SetDeviceCommStatus(PKDEVICE *pDevice, int bDevConnected, int nMaxToleranceSeconds)
{
bool bChangedStatus = false;
int nLastConnStatus = pDevice->nUserData[8];
if(!bDevConnected)
{
time_t tmNow;
time(&tmNow);
if(nLastConnStatus == 1)// 以前连上了,现在没连上,1--->0
{
if(pDevice->nUserData[9] == 0) // 以前没有断开
pDevice->nUserData[9] = tmNow; // 初次发现断开时间
else // 以前记录有时间，已经做了断开标识
{
int nDiscTimeSpan = tmNow - pDevice->nUserData[9]; // 和上次为连接上的差值
if(nDiscTimeSpan > nMaxToleranceSeconds)
bChangedStatus = true;
}
}
}
else
{
if(nLastConnStatus == 1)   // 以前没连上，现在连上了
{
bChangedStatus = true;
pDevice->nUserData[8] = 1; // 上次连接状态
}
pDevice->nUserData[9] = 0; // 标识之前一直处于连接状态
}

//if(bChangedStatus)
{
vector<PKTAG *> vecTags;
for(int i = 0; i < pDevice->nTagNum; i ++)
{
PKTAG *pTag = pDevice->ppTags[i];
vecTags.push_back(pTag);
if(bDevConnected)
{
pTag->nQuality = 0; // 值保留之前的值不变!!
pTag->nTimeSec = pTag->nTimeMilSec = 0; // 时间戳置为最新的
}
else
{
pTag->nQuality = 1;
pTag->nTimeSec = pTag->nTimeMilSec = 0; // 时间戳置为最新的
}
}
Drv_UpdateTagsData(pDevice, vecTags);
vecTags.clear();
}
return 0;
}
*/
/**
*  设定的数据块定时周期到达时该函数被调用.
*
*  可以在该函数中向设备发送请求读取实时数据或向设备发送控制指令.
*  1. 从参数HANDLE_BLOCK中可以获取到数据块信息和设备信息.

*  2. 该函数中需要做如下处理：
a) 如果需要定时扫描，则向设备发送请求读取实时数据
b) 如果需要定时控制，则向设备发送控制命令
c) 如果是同步方式接收设备数据，则需要在该函数中等待设备数据,直到数据到达或超时返回；
d) 如果异步方式接收设备数据，则需要提供一个接收数据的回调函数，在回调函数中处理设备返回数据。

*  3. 当接收到设备返回数据时，解析设备返回数据：
a) 如果是实时数据，则需要更新数据块的值
可以调用驱动EXE提供的回调函数g_drvCBFuncs.pfn_Drv_UpdateBlock或g_drvCBFuncs.pfn_Drv_UpdateBlockEx
b) 如果是控制命令反馈，则根据需求做相应处理
*
*  @param  -[in]  HANDLE_BLOCK  hBlk: [数据块句柄]
*
*  @return PK_SUCCESS: 执行成功
*
*  @version     12/11/2008    Initial Version.
*  @version	10/26/2012  shijunpu  修改由于包计数错误造成启动后一直无法获取块的数据和内存异常问题.
*  @version	11/2/2012  shijunpu  修改对于ModbusRTU协议，由于可能收到另外站号的包造成解析失败丢弃的问题.
*/
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	bool bModbusTCP = IsModbusTCP(pDevice);

	// 组织读取消息
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];
	unsigned short uTransID = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];
	char szRequestBuff[DEFAULT_REQUEST_MAXLEN];
	memset(szRequestBuff, 0, sizeof(szRequestBuff));
	char* pTransmit = szRequestBuff;

	// 起始地址，ModbusTCP才有
	if (bModbusTCP)
	{
		// 事物号为2个字节
		memcpy(pTransmit, &uTransID, 2);
		pTransmit += 2;

		// 协议标识符，先高字节，后低字节，都是0
		memset(pTransmit, 0, 2);
		pTransmit += 2;

		// 长度字段（后续字节的数量），高字节
		*pTransmit = 0;
		pTransmit++;
		//  长度字段（后续字节的数量），低字节
		*pTransmit = 0x06; // 后续请求头部的长度，固定为6个字节
		pTransmit++;
	}

	// 下面开始时Modbus共有的
	unsigned char nStationNo = GetStationID(pDevice, NULL);
	// 站号
	memcpy(pTransmit, &nStationNo, 1);
	pTransmit++;

	// 下面是Modbus请求头部，5个字节
	// 功能码
	char *szBlockType = pTagGroup->szHWBlockName;
	int nBlockType = GetBlockTypeId(szBlockType);
	memcpy(pTransmit, &nBlockType, 1);// 功能码和类型数值相同
	pTransmit++;

	//nStartAddress += 1; // 起始地址从0开始.AO:8的nBeginRegister为7
	// 起始地址高字节
	*pTransmit = (pTagGroup->nBeginRegister) >> BITS_PER_BYTE; // 地址是字，寄存器？？？？？？？？？？？？？？？？
	pTransmit++;
	// 起始地址低字节
	*pTransmit = (pTagGroup->nBeginRegister) & 0xFF;
	pTransmit++;

	// 读取bit/寄存器个数，高字节
	*pTransmit = pTagGroup->nRegisterNum >> BITS_PER_BYTE;
	pTransmit++;
	// 读取bit/寄存器个数，低字节
	*pTransmit = pTagGroup->nRegisterNum & 0xFF;
	pTransmit++;

	int nRequestBufLen = pTransmit - szRequestBuff;
	if (!bModbusTCP) // ModbusRTU
	{
		// 计算CRC
		unsigned short nCRCValue = CRC16((unsigned char *)szRequestBuff, nRequestBufLen);
		memcpy(pTransmit, &nCRCValue, sizeof(unsigned short));
		pTransmit += 2;
	}
	nRequestBufLen = pTransmit - szRequestBuff;

	// 先清空接收缓冲区，避免以前发的请求数据没有来得及接收还放在缓冲区，影响本次请求
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	// 生成的读消息长度，应该是固定的12个字节
	time_t tmRequest;
	time(&tmRequest);
	long lSentBytes = Drv_Send(pDevice, szRequestBuff, nRequestBufLen, 100);
	if (lSentBytes != nRequestBufLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device(%s), fail to send request for datablock(%s) (need send:%d, sent:%d), transaction:%d",
			pDevice->szName, pTagGroup->szAutoGroupName, nRequestBufLen, lSentBytes, uTransID);
		CheckBlockStatus(pDevice, pTagGroup, -1);
		//Drv_SetDeviceCommStatus(pDevice, 0, 3);

		return -1;
	}

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "device(%s), success to send request for datablock(%s) (sent:%d), transaction:%d",
		pDevice->szName, pTagGroup->szAutoGroupName, lSentBytes, uTransID);

	// 从设备接收应答消息
	long lRet = 0;
	long lCurRecvLen = 0;
	char szResponse[DEFAULT_RESPONSE_MAXLEN] = { 0 };
	while (true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + lCurRecvLen, sizeof(szResponse) - lCurRecvLen, pDevice->nRecvTimeout);
		if (lRecvBytes <= 0)
		{
			break;
		}

		lCurRecvLen += lRecvBytes;
		if ((bModbusTCP && lCurRecvLen >= PROTOCOL_TCP_PACKAGE_HEADER_LEN) ||
			(!bModbusTCP && lCurRecvLen >= PROTOCOL_RTU_PACKAGE_HEADER_LEN))
		{
			if (lCurRecvLen >= GetTotalPackageLen(szResponse, lCurRecvLen, bModbusTCP))
				break;
		}
	}

	lRet = -1;
	int nConnStatus = 0;
	if (lCurRecvLen <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "send to device %d bytes success,but receive no bytes", lSentBytes);
		nConnStatus = -1;
		//Drv_SetDeviceCommStatus(pDevice, 0, 3);
	}
	else
	{
		lRet = ParsePackage(pDevice, pTagGroup, szResponse, lCurRecvLen, uTransID, tmRequest);
		nConnStatus = 0;
		//Drv_SetDeviceCommStatus(pDevice, 1, 3);
	}

	if (lRet != PK_SUCCESS)
	{
		CheckBlockStatus(pDevice, pTagGroup, -1);
		SetClearRecvBufferFlag(pDevice);	// 设置下次发送前清除标志位标记
	}
	else
	{
		// 设置失败请求计数
		CheckBlockStatus(pDevice, pTagGroup, 0);
	}

	return nConnStatus;
}

/**
*  当有控制命令时该函数被调用.
*  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
*
*  @version     12/11/2008    Initial Version.
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	char szBinValue[4096] = { 0 };
	int nBinValueLen = 0;
	Drv_TagValStr2Bin(pTag, szStrValue, szBinValue, sizeof(szBinValue), &nBinValueLen);

	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;

	bool bModbusTCP = IsModbusTCP(pDevice);

	char szRequestBuff[DEFAULT_REQUEST_MAXLEN];
	memset(szRequestBuff, 0, sizeof(szRequestBuff));
	long nStatus = PK_SUCCESS;
	int  i = 0;

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@收到控制命令：向设备(%s)的tag(%s)进行控制，地址:%s",
		pDevice->szName, szTagName, szAddress);

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	char *szBlockType = pTagGroup->szHWBlockName;
	int nBlockType = GetBlockTypeId(szBlockType);
	int nStartBits = pTag->nStartBit;// 相对于块内的起始地址位（如AI/DI)
	int nEndBits = pTag->nEndBit; // 相对于块内的结束地址位（如AI/DI)
	//int nBitNo = 0;
	//int nRet = ParseTagAddrInfo(pTag->szAddress,pTag->nDataType, &nBlockType, &nStartBits, &nEndBits, &nBitNo);

	// AI/DI类型，不能写入
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) != 0 && PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "控制时数据块类型为 %s,不正确, 不是AO或DO！", szBlockType);
		return EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE;
	}

	// 起始地址和要控制的地址
	long lStartAddress = nStartBits;	// 以0开始的地址// 要控制的地址长度,是以寄存器长度为单位的
	int nLenBits = nEndBits - nStartBits + 1; // 长度（位表示）
	int  nRegisterNum = nLenBits;
	int  nByteCountToWrite = (int)ceil((nLenBits) / 8.0f);	// 写入数据的字节数,协议中要求写入是以字节数为单位的

	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0){
		lStartAddress = (int)ceil(nStartBits / 16.0f); // AO是2个字节为单位
		nRegisterNum = (int)ceil(nRegisterNum / 16.0f); // AO是2个字节为单位
	}
	else // DO
	{
		lStartAddress = nStartBits;	// 以0开始的地址// 要控制的地址长度,是以寄存器长度为单位的
		nRegisterNum = nLenBits;

	}

	int nFunctionCode = 0; // 功能码、起始地址和读取个数,因为在头信息中需要用到
	if (pDevice->nUserData[DEVPARAM_RWCMD] != 0)
		nFunctionCode = (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) == 0 ? FUNCCODE_WRITE_MULTIDO : FUNCCODE_WRITE_MULTIAO);
	else
	{
		if (nRegisterNum == 1)
			nFunctionCode = (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) == 0 ? FUNCCODE_WRITE_SGLDO : FUNCCODE_WRITE_SGLAO);
		else
			nFunctionCode = (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) == 0 ? FUNCCODE_WRITE_MULTIDO : FUNCCODE_WRITE_MULTIAO);
		(nRegisterNum == 1 ? FUNCCODE_WRITE_SGLDO : FUNCCODE_WRITE_MULTIDO);
	}

	char* pTransmit = szRequestBuff;
	unsigned short uTransID = 0;
	if (bModbusTCP)
	{
		// 事务处理号
		unsigned short uTransID = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];

		// 按照协议组织写消息
		// 事务处理号，高字节
		*pTransmit = uTransID >> BITS_PER_BYTE;
		pTransmit++;

		// 事务处理号，低字节
		*pTransmit = uTransID & 0xFF;
		pTransmit++;

		// 协议标识符，先高字节，后低字节
		*pTransmit = 0;
		pTransmit++;
		*pTransmit = 0;
		pTransmit++;

		// 长度字段，高字节
		*pTransmit = (7 + nByteCountToWrite) >> BITS_PER_BYTE;
		pTransmit++;

		// 长度字段，低字节
		*pTransmit = (nFunctionCode == FUNCCODE_WRITE_SGLAO || nFunctionCode == FUNCCODE_WRITE_SGLDO) ? MB_WRITEPDU_LENGTH : ((7 + nByteCountToWrite) & 0xFF);   // 后面字节的数量
		pTransmit++;
	} // bModbusTCP

	// 站号
	*pTransmit = GetStationID(pDevice, pTag);
	pTransmit++;

	// 功能码
	*pTransmit = nFunctionCode;
	pTransmit++;

	// 起始地址高字节
	*pTransmit = lStartAddress >> BITS_PER_BYTE;
	pTransmit++;

	// 起始地址低字节
	*pTransmit = lStartAddress & 0xFF;
	pTransmit++;

	// 功能码为06时不需要这几个字段。写多个线圈或者寄存器请求
	if (nFunctionCode != FUNCCODE_WRITE_SGLAO && nFunctionCode != FUNCCODE_WRITE_SGLDO)
	{
		// 写入线圈数/寄存器个数，高字节
		*pTransmit = nRegisterNum >> BITS_PER_BYTE;
		pTransmit++;

		// 写入线圈数/寄存器个数，低字节
		*pTransmit = nRegisterNum & 0xFF;
		pTransmit++;

		// 写入数据的字节数
		*pTransmit = nByteCountToWrite & 0xFF;
		pTransmit++;
	}

	//////////////////////////////////////
	if (PKStringHelper::StriCmp(BLOCK_TYPE_DO, szBlockType) == 0)
	{
		// 一个寄存器即只有1个比特，为1时将所有bit全部置为1
		if (nRegisterNum == 1 && nFunctionCode != FUNCCODE_WRITE_MULTIDO) // 比特值：0或1
		{
			if (szBinValue[0] == 0)
				*pTransmit = 0;
			else
				*pTransmit = (char)0xFF;

			pTransmit++;
			*pTransmit = 0;
			pTransmit++;
		}
		else
		{
			// edit by shijunpu,强制写多个线圈时不一定是写8个位的整数倍，因此需要考虑非8的整数倍的情况 
			for (int i = 0; i < nByteCountToWrite; i++)
			{
				*pTransmit = szBinValue[i];
				pTransmit++;
			}
		}
	}
	else // BLOCK_TYPE_AO
	{
		// 先写整个寄存器
		// 可能该次请求要控制的缓冲区的头部、尾部都有不是完整寄存器长度（如4个字节）位，要先判断出来并写入
		if (nLenBits == 1 || (nLenBits % 8 != 0)) // 比特值：0或1
		{
			// 待写入数据
			//int lValueStatus = 0;
			//unsigned short nData = 0;
			//PKTAGDATA tagData;
			//strncpy(tagData.szName, pTag->szName,sizeof(tagData.szName) - 1);
			//int lRet = Drv_GetTagData(pDevice,&tagData); // 获取到上次的数据
			//if(lRet != PK_SUCCESS)
			//	return lRet;

			//if(lValueStatus != TAG_QUALITY_GOOD)
			//	return -1;

			//memcpy(&nData, &tagData, 2); // AO or AI
			//// 设置指定bit处的值
			//int nBitNo = nStartBits % 16; // AO:W5.13, 一定是某个Word的余数
			//nData = nData & ~(1 << nBitNo);
			//nData = nData | (szBinValue[0] << nBitNo);
			//
			//// 待写入数据
			//*pTransmit = nData >> BITS_PER_BYTE;
			//pTransmit++;
			//*pTransmit = (nData & 0xFF);
			//pTransmit++;
		}
		else // TAG_DATATYPE_ANALOG、TAG_DATATYPE_BLOB、TAG_DATATYPE_TEXT
		{
			// 待写入数据
			if (NeedSwap(szBlockType))
				SwapByteOrder(szBinValue, nBinValueLen, 2);

			// 写入字节数刚好等于寄存器字节时
			if (nByteCountToWrite == nBinValueLen)
			{
				for (i = 0; i < nBinValueLen; i++)
				{
					*pTransmit = szBinValue[i];
					pTransmit++;
				}
			}
			else
			{
				// 写入字节数不等于寄存器字节数时
				// 第一部分：已经swap过的值，由高到低
				for (i = 0; i < (int)(nRegisterNum - 1) * 2; i++)
				{
					*pTransmit = szBinValue[i];
					pTransmit++;
				}

				// 第二部分：最后一个寄存器的高字节
				int nToAdd = nByteCountToWrite - nBinValueLen;
				for (int j = 0; j < nToAdd; j++)
				{
					*pTransmit = 0;
					pTransmit++;
				}

				// 第三部分：最后一个寄存器的低字节，由高到低
				for (; i < nBinValueLen; i++)
				{
					*pTransmit = szBinValue[i];
					pTransmit++;
				}
			}
		}
	}

	// 生成的写消息长度
	int nRequestBufLen = pTransmit - szRequestBuff;
	if (!bModbusTCP)
	{
		unsigned short nCRCValue = CRC16((unsigned char *)szRequestBuff, nRequestBufLen);
		memcpy(pTransmit, &nCRCValue, sizeof(unsigned short));
		pTransmit += 2;
		nRequestBufLen += sizeof(unsigned short);
	}

	// 先判断是否要清除标志位
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	long lSentBytes = Drv_Send(pDevice, szRequestBuff, nRequestBufLen, 100);
	if (lSentBytes != nRequestBufLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "向设备(%s)发送读写tag(%s)请求失败(发送%d个字节，实际发送%d个)，事务号：%d",
			pDevice->szName, szTagName, nRequestBufLen, lSentBytes, uTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -201, "control, sentlen(%d)!=reqlen(%d)", lSentBytes, nRequestBufLen);
		return -201;
	}

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!控制命令：向设备(%s)发送写tag(%s)请求成功，功能码:%d，实际发送%d个, 事务号：%d",
		pDevice->szName, pTag->szName, nFunctionCode, lSentBytes, uTransID);

	// 从设备接收应答消息
	char szResponse[DEFAULT_RESPONSE_MAXLEN];
	long lRecvBytes = Drv_Recv(pDevice, szResponse, sizeof(szResponse), pDevice->nRecvTimeout);
	if (lRecvBytes <= 0)
	{
		//修改由于参数不匹配造成程序异常问题，edit by shijunpu @20121026
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "从设备(%s)接收写tag(%s)请求失败(实际收到%d个)，事务号：%d",
			pDevice->szName, pTag->szName, lRecvBytes, uTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -202, "control, recvlen(%d)<=0", lRecvBytes);
		return -202;
	}
	/*
	long lRet = ParsePackage(pDevice, pTagGroup, szResponse, lRecvBytes, uTransID);
	if(lRet != PK_SUCCESS)
	SetClearRecvBufferFlag(pDevice);	// 设置下次发送前清楚标志位标记
	else*/
	{
		// 对于设备为AO，但上位配置为DO（取AO的某一个位）；如果上位连续对某个AO（2个字节），
		// 在一个读写周期内，两个不同的位连续控制，会出现后面会覆盖前面的控制值
		if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0 && nLenBits == 1)
		{
			vector<PKTAG *> vecTags;
			vecTags.push_back(pTag);
			Drv_SetTagData_Text(pTag, szBinValue);
			Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());
			//OnTimer(pDevice, "block",pTagGroup,NULL);
		}
	}
	return 0;
}


// 支持的地址格式：按位类型包括：DI、DO或者首地址为0、1。按字类型包括：AI、AO或者首地址为3、4
// 输入：szAddressInDevice：变量地址，按字类型：[AI|AO|DI|DO|0|1|2|3][.|#|:]20[.5],AO3,3001,3004,AI20.2,3003.3。按位类型：DI1,DO2,1001,0001。支持以:#分割块和地址
// 输入：nTagLenBits，根据tag类型计算得到的tag值位数
// 输出：*pnStartBits、*pnEndBits, 相对于相对整个物理块（如AI、DO、D、DB1，而不是重组的某个AI内的某个Group）内的起始位、结束位（含结束位，如16则起始位0，结束位15），以位为单位
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// 复制地址，并去掉#和：号。该方法可能会被多线程调用，不能用静态变量
	char szAddress[PK_IOADDR_MAXLEN + 1] = { 0 };
	char *pDest = szAddress;
	char *pTmp = szAddressInDevice;
	while (*pTmp != '\0')
	{
		if (*pTmp == '#' || *pTmp == ':') // 仅仅复制非: 和非#
			*pDest = '.';
		else
			*pDest = *pTmp;
		pTmp++;
		pDest++;
	}

	pTmp = szAddress; // 获取字符串的首地址

	// 查找块名。找到第一个是阿拉伯数字的字符，之前的就是块名
	memset(szBlockName, 0, nBlockNameLen);
	pDest = szBlockName;
	while (*pTmp != '\0')
	{
		if ((*pTmp >= '0' && *pTmp <= '9') || *pTmp == '.') // 找到第一个是阿拉伯数字的字符，之前的就是块名
			break;
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}
	if (strlen(szBlockName) == 0)//如果第一个字符是阿拉伯数字，那么根据3、4、1、2来区分
	{
		if (*pTmp == '3')
			strcpy(szBlockName, "AI");
		else if (*pTmp == '4')
			strcpy(szBlockName, "AO");
		else if (*pTmp == '0')
			strcpy(szBlockName, "DI");
		else if (*pTmp == '1')
			strcpy(szBlockName, "DO");
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no block type name! must:AI/AO/DI/DO/0/1/2/3", szAddressInDevice);
			return -1;
		}
		pTmp++; // 跳过0、1、3、4这个字符
	}
	if (*pTmp == '.') // 跳过.号
		pTmp++;

	// 找到块名后面的地址，直到结束或者有.号
	char szRegisterNo[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szRegisterNo;
	while (*pTmp != '\0' && *pTmp != '.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}
	if (strlen(szRegisterNo) == 0)//如果第一个字符是阿拉伯数字，认为是异常
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, 寄存器地址位配置, 异常", szAddressInDevice);
		return -2;
	}
	if (*pTmp == '.') // 跳过.号
		pTmp++;

	// 找到地址后面的位号，有.号
	char szBitNo[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szBitNo;
	while (*pTmp != '\0' && *pTmp != '.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}

	bool bBlockByBit = false;
	bool bBlockByWord = false;
	if (PKStringHelper::StriCmp("AI", szBlockName) == 0 || PKStringHelper::StriCmp("AO", szBlockName) == 0)
	{
		bBlockByWord = true;
	}
	else if (PKStringHelper::StriCmp("DI", szBlockName) == 0 || PKStringHelper::StriCmp("DO", szBlockName) == 0)
	{
		bBlockByBit = true;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, 数据块:%s不支持, 异常", szAddressInDevice, szBlockName);
		return -2;
	}

	// 计算起始地址.Modbus的地址都是从1开始的，因此要减去1
	int	 nStartAddr = ::atoi(szRegisterNo);
	if (bBlockByBit) // bit
		*pnStartBits = nStartAddr - 1;
	else
		*pnStartBits = (nStartAddr - 1) * 16;

	// 计算起始位，对于字地址有效
	if (bBlockByWord && strlen(szBitNo) > 0)
	{
		int nBitNo = 0;
		nBitNo = ::atoi(szBitNo);
		if (nBitNo >= 0)
			*pnStartBits += nBitNo;
	}

	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}