// DRVTAG的nData1是起始地址位（相对于AI的0地址），nData2是结束地址位，nData3是在该块内的起始位数
// 三菱的PLC和intel的字节序正好是一模一样的，这样读出来的float、int32、int16、double都可以直接转换，不需要做字节序处理了！
/*
	X/Y/M/D

	#define  BLOCK_TYPE_NAME_X				"X"		// 输入继电器,位软元器件
	#define  BLOCK_TYPE_NAME_Y				"Y"		// 输出继电器,位软元器件
	#define  BLOCK_TYPE_NAME_M				"M"		// 辅助继电器,位软元器件
	#define  BLOCK_TYPE_NAME_S				"S"		// 状态,位软元器件
	#define  BLOCK_TYPE_NAME_L				"L"		// 锁存继电器,位软元器件
	#define  BLOCK_TYPE_NAME_F				"F"		// 报警器,位软元器件
	#define  BLOCK_TYPE_NAME_B				"B"		// 链接继电器,位软元器件
	#define  BLOCK_TYPE_NAME_V				"V"		// 边沿继电器,位软元器件
	#define  BLOCK_TYPE_NAME_CS				"CS"	// 计数器和高速计数器,触点,位软元器件
	#define  BLOCK_TYPE_NAME_CC				"CC"	// 计数器和高速计数器,线圈,位软元器件
	#define  BLOCK_TYPE_NAME_TS				"TS"	// 定时器,触点，位软元器件
	#define  BLOCK_TYPE_NAME_TC				"TC"	// 定时器,线圈，位软元器件
	#define  BLOCK_TYPE_NAME_SS				"SS"	// 累计定时器,触点，位软元器件
	#define  BLOCK_TYPE_NAME_SC				"SC"	// 累计定时器,线圈，位软元器件
	#define  BLOCK_TYPE_NAME_SB				"SB"	// 链接特殊继电器,线圈，位软元器件
	#define  BLOCK_TYPE_NAME_S				"S"		// 步进继电器,线圈，位软元器件
	#define  BLOCK_TYPE_NAME_DX				"DX"	// 直接输入继电器,线圈，位软元器件
	#define  BLOCK_TYPE_NAME_DY				"DY"	// 直接输出继电器,线圈，位软元器件

	#define  BLOCK_TYPE_NAME_TN				"TN"	// 定时器,当前值，字软元器件
	#define  BLOCK_TYPE_NAME_SN				"SN"	// 累计寄存器,当前值,字软元器件
	#define  BLOCK_TYPE_NAME_CN				"CN"	// 计数器和高速计数器,当前值,字软元器件
	#define  BLOCK_TYPE_NAME_D				"D"		// 数据寄存器,字软元器件
	#define  BLOCK_TYPE_NAME_Z				"Z"		// 变址寄存器,字软元器件
	#define  BLOCK_TYPE_NAME_W				"W"		// 链接寄存器,字软元器件
	#define  BLOCK_TYPE_NAME_SW				"SW"	// 链接特殊继电器,线圈，字软元器件
*/

#include "mitubishifxdrv.h"
#include "math.h"
#include "AutoGroup_BlkDev.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include <map>
#include <string>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "eviewcomm/eviewcomm.h"
using namespace std;

#define TIMERINFO_PUSERDATA_INDEX_TAGGROUP		0
#define DEVICE_NUSERDATA_INDEX_TRANSID			0
#define DEVICE_NUSERDATA_INDEX_NEEDCLEAR		1
#define TAGGROUP_NUSERDATA_INDEX_COMPCODE		1
#define TAGGROUP_NUSERDATA_INDEX_BITORWORD		2
#define PK_TAGDATA_MAXLEN							4096

map<string,int> g_mapTypeName2Code; // 存储块类型名称到内部通信代码

// 以位为单位的软元件代码数组
int g_arrTypeCodeByBit[] = {BLOCK_TYPE_CODE_X,BLOCK_TYPE_CODE_Y,BLOCK_TYPE_CODE_M,BLOCK_TYPE_CODE_S,BLOCK_TYPE_CODE_L,BLOCK_TYPE_CODE_F,BLOCK_TYPE_CODE_B,BLOCK_TYPE_CODE_V,BLOCK_TYPE_CODE_TS,
BLOCK_TYPE_CODE_TC,BLOCK_TYPE_CODE_CS,BLOCK_TYPE_CODE_CC,BLOCK_TYPE_CODE_SS,BLOCK_TYPE_CODE_SC,BLOCK_TYPE_CODE_SB,BLOCK_TYPE_CODE_S,BLOCK_TYPE_CODE_DX,BLOCK_TYPE_CODE_DY};
// 以字为单位的软元件代码数组
int g_arrTypeCodeByWord[] = {BLOCK_TYPE_CODE_D,BLOCK_TYPE_CODE_TN,BLOCK_TYPE_CODE_SN,BLOCK_TYPE_CODE_Z,BLOCK_TYPE_CODE_W,BLOCK_TYPE_CODE_SW};

// 以位为单位的软元件名称数组
char g_arrTypeNameByBit[][PK_NAME_MAXLEN] = {BLOCK_TYPE_NAME_X,BLOCK_TYPE_NAME_Y,BLOCK_TYPE_NAME_M,BLOCK_TYPE_NAME_S,BLOCK_TYPE_NAME_L,BLOCK_TYPE_NAME_F,BLOCK_TYPE_NAME_B,BLOCK_TYPE_NAME_V,BLOCK_TYPE_NAME_TS,
BLOCK_TYPE_NAME_TC,BLOCK_TYPE_NAME_CS,BLOCK_TYPE_NAME_CC,BLOCK_TYPE_NAME_SS,BLOCK_TYPE_NAME_SC,BLOCK_TYPE_NAME_SB,BLOCK_TYPE_NAME_S,BLOCK_TYPE_NAME_DX,BLOCK_TYPE_NAME_DY};
// 以字为单位的软元件名称数组
char g_arrTypeNameByWord[][PK_NAME_MAXLEN] = {BLOCK_TYPE_NAME_D,BLOCK_TYPE_NAME_TN,BLOCK_TYPE_NAME_SN,BLOCK_TYPE_NAME_Z,BLOCK_TYPE_NAME_W,BLOCK_TYPE_NAME_SW};

// 寄存器类型，是按位还是按字
int GetCompCode(char *szBlockType) {
	map<string,int>::iterator it = g_mapTypeName2Code.find(szBlockType);
	if(it == g_mapTypeName2Code.end())
		return BLOCK_TYPE_CODE_UNKNOWN;
	return it->second;
}

int GetRegisterLenType(int nCompCode) {
	for(int i = 0; i < sizeof(g_arrTypeCodeByWord)/sizeof(int); i++)
	{
		if(nCompCode == g_arrTypeCodeByWord[i])
			return BLOCK_LENTYPE_ID_WORD;
	}

	for(int i = 0; i < sizeof(g_arrTypeCodeByBit)/sizeof(int); i++)
	{
		if(nCompCode == g_arrTypeCodeByBit[i])
			return BLOCK_LENTYPE_ID_BIT;
	}
	return BLOCK_LENTYPE_ID_UNDEFINED;
}

 // 是否需要清楚标志位
 void SetClearRecvBufferFlag(PKDEVICE *pDevice)
 {
	 pDevice->nUserData[DEVICE_NUSERDATA_INDEX_NEEDCLEAR] = 1;
 }

 bool GetClearRecvBufferFlag(PKDEVICE *pDevice)
 {
	 return pDevice->nUserData[DEVICE_NUSERDATA_INDEX_NEEDCLEAR] != 0;
 }


 void CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess)
 {
	 if(lSuccess == PK_SUCCESS)
		 pTagGroup->nFailCountRecent = 0;
	 else
	 {
		 if(pTagGroup->nFailCountRecent > 3)	// 最近失败次数
		 {
			 UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, "read failcount:%d", pTagGroup->nFailCountRecent);
			 pTagGroup->nFailCountRecent = 0; // 避免计数太大导致循环
		 }
		 else
			 pTagGroup->nFailCountRecent += 1;
	 }
 }

 long ParseOnePacket(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *pPacket, long nTotalPacketLen, time_t tmRequest)
 {
	 PACKET_HEAD packHeader;
	 memcpy(&packHeader, pPacket, sizeof(PACKET_HEAD));
	 char *pDataBufP = pPacket+ sizeof(PACKET_HEAD);
	 int nDataLen = packHeader.nBodyDataLen;

	 // 获取结束代码
	 unsigned short uExitCode = 0;
	 memcpy(&uExitCode, pDataBufP, sizeof(unsigned short));
	 if(uExitCode != 0) //发生了数据错误
	 {
		 Drv_LogMessage(PK_LOGLEVEL_ERROR, "[%s][%s]解析包时, 发现包长度: %d, 返回码：%d, 设置数据质量为BAD!",
			 pDevice->szName, pTagGroup->szAutoGroupName, nTotalPacketLen, uExitCode);
		 UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, "return packet err,exitcode:%d", uExitCode);
		 return -1;
	 }
	 pDataBufP += sizeof(unsigned short);
	 nDataLen -= sizeof(unsigned short);
	 UpdateGroupData(pDevice, pTagGroup, pDataBufP, nDataLen, 0);
	 return 0;
 }

long ParsePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *pszRecvBuf, long lRecvBufLen,time_t tmRequest)
{
	if(lRecvBufLen <=  sizeof(PACKET_HEAD))
	{
		UpdateGroupQuality(pDevice, pTagGroup, -200, "parsing,packlen:%d < headlen:%d,discard", lRecvBufLen, sizeof(PACKET_HEAD));
		return -200;
	}

	int nRet = 0;
	int nStartBytes = pTagGroup->nStartAddrBit / 8;
	int nEndBytes = pTagGroup->nEndAddrBit / 8;
	int nLenBytes = nEndBytes - nStartBytes + 1;
	int nBlockSize = nLenBytes; //(int)ceil((pTagGroup->nEndBit - pTagGroup->nStartAddrBit + 1)/8.0f);
	// 循环解析收到的所有可能数据包
	// 2表示站号（1个字节）和功能码（1个字节）。
	// 任何请求应答的头6个字节都是一样的意义，第7和第8个字节也是一样的意义（站号和功能码）
	long nRecvBytes = lRecvBufLen;
	bool bFoundThisResponse = false;
	char *pCurBuf = (char *)pszRecvBuf;
	while(nRecvBytes > sizeof(PACKET_HEAD))
	{
		PACKET_HEAD packHeader;
		memcpy(&packHeader, pszRecvBuf, sizeof(PACKET_HEAD));
		int nBodyDataLen = packHeader.nBodyDataLen;
		int nTotalPacketLen = nBodyDataLen + sizeof(PACKET_HEAD);
		if(nRecvBytes < nTotalPacketLen) // 长度不足一个包
		{
			nRet = -20;
			UpdateGroupQuality(pDevice, pTagGroup, nRet, "parsing,packlen:%d < packlen of head:%d,discard", nRecvBytes, nTotalPacketLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,group:%s,parsing,packlen:%d < packlen of head:%d,discard", 
				pDevice->szName, pTagGroup->szAutoGroupName, nRecvBytes, nTotalPacketLen);
			break;
		}
		if(nTotalPacketLen > sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "解析包时, 发现包长度: %d, 超过最大的包长度:%d, 抛弃设备%s本次接收的其他数据!",
				nTotalPacketLen, sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN, pDevice->szName);
			nRet = -10;
			UpdateGroupQuality(pDevice, pTagGroup, nRet, "parsing,packlen:%d,maxpacklen:%d,discard", nTotalPacketLen, sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN);
			break;
		}

		//char *pThisPackData = pszRecvBuf + sizeof(PACKET_HEAD); // 指向当前这个包
		ParseOnePacket(pDevice, pTagGroup, pCurBuf, nTotalPacketLen, tmRequest);
		nRecvBytes -= nTotalPacketLen;
		pCurBuf += nTotalPacketLen;
	}//while(m_nRemainBufferLen > PROTOCOL_PACKAGE_HEADER_LEN + 2)
	
	return nRet;
}

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDrviver)
{
	// bit
	g_mapTypeName2Code[BLOCK_TYPE_NAME_D] = BLOCK_TYPE_CODE_D;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_X] = BLOCK_TYPE_CODE_X;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_Y] = BLOCK_TYPE_CODE_Y;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_M] = BLOCK_TYPE_CODE_M;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_S] = BLOCK_TYPE_CODE_S;

	g_mapTypeName2Code[BLOCK_TYPE_NAME_L] = BLOCK_TYPE_CODE_L;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_F] = BLOCK_TYPE_CODE_F;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_B] = BLOCK_TYPE_CODE_B;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_V] = BLOCK_TYPE_CODE_V;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_CS] = BLOCK_TYPE_CODE_CS;

	g_mapTypeName2Code[BLOCK_TYPE_NAME_CC] = BLOCK_TYPE_CODE_CC;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_TS] = BLOCK_TYPE_CODE_TS;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_TC] = BLOCK_TYPE_CODE_TC;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_SS] = BLOCK_TYPE_CODE_SS;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_SC] = BLOCK_TYPE_CODE_SC;

	g_mapTypeName2Code[BLOCK_TYPE_NAME_SB] = BLOCK_TYPE_CODE_SB;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_S] = BLOCK_TYPE_CODE_S;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_DX] = BLOCK_TYPE_CODE_DX;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_DY] = BLOCK_TYPE_CODE_DY;

	// word
	g_mapTypeName2Code[BLOCK_TYPE_NAME_TN] = BLOCK_TYPE_CODE_TN;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_SN] = BLOCK_TYPE_CODE_SN;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_CN] = BLOCK_TYPE_CODE_CN;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_D] = BLOCK_TYPE_CODE_D;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_Z] = BLOCK_TYPE_CODE_Z;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_W] = BLOCK_TYPE_CODE_W;
	g_mapTypeName2Code[BLOCK_TYPE_NAME_SW] = BLOCK_TYPE_CODE_SW;

	return 0;
}

// 传入的设备信息，有用的包括：
// 参数1，表示通信类型，如二进制（E71）、ASCII（串口，协议格式1）、ASCII（串口，协议格式2）
// TagGroup的nUserData[1]--软元件代码，[2]--是按位还是按块
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// 获取到所有的tag点。需要在tag点存储块内偏移（位）、长度（位），组包含的tag点对象列表（以便计算）
	// 进行自组块处理，将所有的tag点自组块成BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = 960; // 根据pdf文档判断，每次可以读取480个字的大小

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();

	unsigned int i = 0;
	for(; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGrp = vecTagGroup[i];
		// 计算式按位还是按字的软元件
		pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE] = GetCompCode(pTagGrp->szHWBlockName);
		pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD] = GetRegisterLenType(pTagGrp->nUserData[1] );
		if(pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD] == BLOCK_LENTYPE_ID_UNDEFINED)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "不支持的三菱PLC软元件类型：%s",pTagGrp->szHWBlockName);
			UpdateGroupQuality(pDevice, pTagGrp, -1, "unsupported comptype:%s",pTagGrp->szHWBlockName);
			continue;
		}

		// 计算块的开始寄存器和个数、无用位数。无论字还是位寄存器，都按照字来读取，所以均赋值16
		int nRegisterLenBits = 16;
		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // 无论字还是位寄存器，都按照字来读取，所以均赋值16

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[TIMERINFO_PUSERDATA_INDEX_TAGGROUP] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器
	}
	return 0;
}

/*
	反初始化设备
*/

PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	int nTimerNum = 0;
	//PKTIMER * pTimers = Drv_GetTimers(pDevice, &nTimerNum);
	//int i = 0;
	//for(i = 0; i < nTimerNum; i ++)
	//{
	//	PKTIMER *pTimerInfo = &pTimers[i];
	//	DRVGROUP *pTagGroup = (DRVGROUP *)pTimerInfo->pUserData[TIMERINFO_PUSERDATA_INDEX_TAGGROUP];
	//	delete pTagGroup;
	//	pTagGroup = NULL;
	//	Drv_DestroyTimer(pDevice, pTimerInfo);
	//}
	return 0;
}

bool IsPacketValidResponse(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PACKET_HEAD *pPackHead)
{
	if(pPackHead->nStartFlag != PACKET_RESP_HEADFLAG)
		return false;
	if(pPackHead->nNetworkNo != PACKET_NETWORK_NO || pPackHead->nPLCNo != PACKET_PLC_NO || pPackHead->nDestModuleIONo != PACKET_DESTMODULE_IONO || pPackHead->nDestModuleStationNo != PACKET_DESTMODULE_STATIONNO)
		return false;

	return true;
}
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
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	char szTip[PK_NAME_MAXLEN] = {0};
	// 组织读取消息
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[TIMERINFO_PUSERDATA_INDEX_TAGGROUP];
	unsigned short uTransID = (unsigned short) ++ pDevice->nUserData[DEVICE_NUSERDATA_INDEX_TRANSID];
	REQ_PACKET reqPack;
	unsigned char nCompCode = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE];
	int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
	if(nBitOrWord == BLOCK_LENTYPE_ID_UNDEFINED){
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "不支持的三菱PLC软元件类型：%s",pTagGroup->szHWBlockName);
		UpdateGroupQuality(pDevice, pTagGroup, -100, "unspported plc comp type:%s", pTagGroup->szHWBlockName);
		return -1;
	}
	reqPack.nCommand = COMMAND_READ_BULK;
	char *pCurData = reqPack.szReqData;
	// 需要计算：按位、数据长度、数据内容
	// 请求数据区：起始软元件（2B）、软元件代码（1B）、软元件点数（2B）
	//unsigned short nStartRegisterNo = 0;
	//unsigned short nWordNum = 0;
	// 为何不按位取？因为按位取9个位的数据：10 10 00 00 00，比较奇怪，似乎是每4位表示1位，不方便取用
	//if(nBitOrWord == BLOCK_LENTYPE_ID_BIT)
	//{
	//	reqPack.nSubCommand = SUBCOMMAND_BIT;
	//	nStartRegisterNo = pTagGroup->nStartAddrBit;
	//	nRegisterNum = pTagGroup->nLenBits;
	//}
	//else
	//{
	reqPack.nSubCommand = SUBCOMMAND_WORD;
	//}
	StartAddr2Packet(&reqPack, pTagGroup->nBeginRegister); // 寄存器从0开始，不需要+1
	reqPack.ucCompCode =nCompCode; // 软元件
	reqPack.uCompNum = pTagGroup->nRegisterNum;// 软元件数量
	reqPack.header.nBodyDataLen = (char *)reqPack.szReqData - (char *)&reqPack.nCPUTimer; // 数据区的长度
	reqPack.nTotalPackLen = (char *)reqPack.szReqData - (char *)&reqPack.header.nStartFlag; // 包的总长度

	// 先清空接收缓冲区，避免以前发的请求数据没有来得及接收还放在缓冲区，影响本次请求
	if(GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);	

	// 生成的读消息长度，应该是固定的12个字节
	char *pSendBuf = (char *)&reqPack;
	time_t tmRequest;
	time(&tmRequest);
	long lSentBytes = Drv_Send(pDevice, pSendBuf, reqPack.nTotalPackLen, 100);
	if(lSentBytes != reqPack.nTotalPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device(%s), fail to send request for datablock(%s) (need send:%d, sent:%d), transaction:%d",
			pDevice->szName, pTagGroup->szAutoGroupName,reqPack.nTotalPackLen, lSentBytes, uTransID);
		CheckBlockStatus(pDevice, pTagGroup, -1);
		return -1;
	}

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "device(%s), success to send request for datablock(%s) (sent:%d), transaction:%d",
		pDevice->szName, pTagGroup->szAutoGroupName, lSentBytes, uTransID);

	// 从设备接收应答消息
	long lRet = 0;
	long lCurRecvLen = 0;
	char szResponse[MAX_RESPONSE_BUFLEN] = {0};
	while(true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + lCurRecvLen, sizeof(szResponse) - lCurRecvLen, pDevice->nRecvTimeout);
		if(lRecvBytes <= 0)
		{
			break;
		}

		//Drv_LogHex(szResponse + lCurRecvLen, lRecvBytes);
		lCurRecvLen += lRecvBytes;
		if(lCurRecvLen >= sizeof(PACKET_HEAD)) // 至少要是1个包头的大小，才能得到长度
		{
			PACKET_HEAD packHead;
			memcpy(&packHead, szResponse, sizeof(PACKET_HEAD));
			if(!IsPacketValidResponse(pDevice, pTagGroup, &packHead)) // 包头都不对，那么肯定抛出错误信息
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "从设备(%s)接收到的读数据块(%s)请求，头部格式非法(长度：%d)，事务号：%d",
					pDevice->szName, pTagGroup->szAutoGroupName,reqPack.nTotalPackLen, uTransID);
				CheckBlockStatus(pDevice, pTagGroup, -100); // 设置状态
				return -1;
			}

			if (lCurRecvLen >= sizeof(PACKET_HEAD) + packHead.nBodyDataLen)	// 大于1个包的长度，则直接返回
				break;
		}
	}
	if(lCurRecvLen <= 0){
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"send to device %d bytes success,but receive no bytes",lSentBytes);
		CheckBlockStatus(pDevice, pTagGroup, -200);
	}

	lRet = ParsePackage(pDevice, pTagGroup, szResponse, lCurRecvLen, tmRequest);
	if(lRet != PK_SUCCESS)
	{
		CheckBlockStatus(pDevice, pTagGroup, -1);
		SetClearRecvBufferFlag(pDevice);	// 设置下次发送前清除标志位标记
	}
	else
	{
		// 设置失败请求计数
		CheckBlockStatus(pDevice, pTagGroup, 0);
	}
	
	return lRet;
}

/**
 *  当有控制命令时该函数被调用.
 *  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
 *
 *  @version     12/11/2008    Initial Version.
 */
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	char szBinValue[PK_TAGDATA_MAXLEN] = {0};
	int nBinValueLen = 0;
	Drv_TagValStr2Bin(pTag, szStrValue, szBinValue, sizeof(szBinValue), &nBinValueLen);

	char szTip[PK_TAGDATA_MAXLEN] = {0};
	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@收到控制命令：向设备(%s)的tag(%s)进行控制，地址:%s",
		pDevice->szName, szTagName, szAddress);

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	REQ_PACKET reqPack;
	unsigned char nCompCode = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE];
	int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
	if(nCompCode == BLOCK_LENTYPE_ID_UNDEFINED){
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "不支持的三菱PLC软元件类型：%s",pTagGroup->szHWBlockName);
		sprintf(szTip, "unspported plc comp type:%s", pTagGroup->szHWBlockName);
		//Drv_SetCtrlResult(lCmdId, -100, szTip);
		return -1;
	}

	reqPack.nCommand = COMMAND_WRITE_BULK;
	// 需要计算：按位、数据长度、数据内容
	// 写数据区：起始软元件（2B）、软元件代码（1B）、软元件点数（2B）
	int nStartBits = pTag->nStartBit;// 相对于块内的起始地址位（如AI/DI)
	int nEndBits = pTag->nEndBit; // 相对于块内的结束地址位（如AI/DI)
	int nLenBits = nEndBits - nStartBits + 1; // 长度（位表示）
	int  nRegisterNum = 0; // 寄存器数目
	unsigned short nStartRegisterNo = 0; // 起始寄存器地址
	int nBytes2Write = 0;
	// D块（字为单位）也可以按照位读写，所以这里不能依据块的类型来判断，而是必须依据点的类型来判断
	if(nBitOrWord == BLOCK_LENTYPE_ID_BIT) // 按位操作的块
	{
		reqPack.nSubCommand = SUBCOMMAND_BIT;
		if(nCompCode == BLOCK_TYPE_CODE_D)
		{
			nStartRegisterNo = (int)(nStartBits / 16.0f);
			nRegisterNum = (int)ceil(nLenBits / 16.0f);
			nBytes2Write = 1;//nRegisterNum * 2;
		}
		else
		{
			nStartRegisterNo = nStartBits;
			nRegisterNum = nLenBits;
			nBytes2Write = (int)ceil(nRegisterNum / 8.0f);
		}
	}
	else // nBitOrWord == 0
	{
		reqPack.nSubCommand = SUBCOMMAND_WORD;
		nStartRegisterNo = (int)(nStartBits / 16.0f);
		nRegisterNum = (int)ceil(nLenBits / 16.0f);
		nBytes2Write = nRegisterNum * 2;
	}

	// 将要控制的起始地址、元件个数转换为包
	StartAddr2Packet(&reqPack, nStartRegisterNo);
	reqPack.ucCompCode =nCompCode;
	reqPack.uCompNum = nRegisterNum;
	reqPack.header.nBodyDataLen = (char *)reqPack.szReqData + nBytes2Write - (char *)&reqPack.nCPUTimer;
	reqPack.nTotalPackLen = (char *)reqPack.szReqData + nBytes2Write - (char *)&reqPack.header.nStartFlag; // 计算总长度

	// 写入要控制的各个数据
	char *pData = reqPack.szReqData;
	if(nLenBits == 1) //一次只允许控制1个位（一个寄存器）
	{
		/*
		if(nBytes2Write > nBinValueLen)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??从设备(%s)控制数据块(%s)的tag:%s请求，数据长度(%d)，应为长度:%d",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, nBinValueLen, nBytes2Write);
			sprintf(szTip, "tag:%s, towrite bytes:%d,supplied bytes:%d too small", pTag->szName, nBytes2Write, nBinValueLen);
			//Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -1;
		}
		*/
		// 三菱PLC的Y写1很特殊，每个字节表示两个位的控制。如0x10表示第一个Y寄存器写1，第二个写0.我们只会有一个控制，因此第二个始终赋0即可
		unsigned char ucValue = (*(unsigned char *)(szBinValue));
		if(ucValue == 0)
			*pData = 0x00;
		else
			*pData = 0x10;
	}
	else
	{
		/*
		if(pTag->nLenBits == 1) // 对D5.0进行控制，也是允许的
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??设备(%s)控制数据块(%s)的tag:%s的地址:%s,写请求不安全, 不被允许，写入数据长度(%d)",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, szAddress, nBinValueLen);
			sprintf(szTip, "addr:BX.Y, do not support control ,tag:%s", pTag->szName);
			Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -1;
		}*/
		if(nBytes2Write > nBinValueLen)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??设备(%s)控制数据块(%s)的tag:%s写请求，数据长度(%d)，应为长度:%d",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, nBinValueLen, nBytes2Write);
			sprintf(szTip, "tag:%s, towrite bytes:%d,supplied bytes:%d too small", pTag->szName, nBytes2Write, nBinValueLen);
			//Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -2;
		}
		memcpy(pData, szBinValue, nBytes2Write);
	}

	// 先判断是否要清除标志位
	if(GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);	

	long lSentBytes = Drv_Send(pDevice, (char *)&reqPack, reqPack.nTotalPackLen, 100);
	if(lSentBytes != reqPack.nTotalPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "向设备(%s)发送读写tag(%s)请求失败(发送%d个字节，实际发送%d个)",
			pDevice->szName, szTagName, reqPack.nTotalPackLen, lSentBytes);
		sprintf(szTip, "tag:%s, send failed,should:%d,actual:%d", pTag->szName, reqPack.nTotalPackLen, lSentBytes);
		//Drv_SetCtrlResult(lCmdId, -100, szTip);
		return -1;
	}
	
	//记录一下请求的时间
	time_t tmRequest;
	time(&tmRequest);

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!控制命令：向设备(%s)发送写tag(%s)请求成功，实际发送%d个",
		pDevice->szName, pTag->szName, lSentBytes);

	// 从设备接收应答消息
	long lRet = 0;
	long lCurRecvLen = 0;
	char szResponse[MAX_RESPONSE_BUFLEN] = {0};
	while(true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + lCurRecvLen, sizeof(szResponse) - lCurRecvLen, pDevice->nRecvTimeout);
		if(lRecvBytes <= 0) // 没收到，直接认为错误，返回
			break;

		//Drv_LogHex(szResponse + lCurRecvLen, lRecvBytes);
		lCurRecvLen += lRecvBytes;
		if(lCurRecvLen >= sizeof(PACKET_HEAD)) // 至少要是1个包头的大小，才能得到长度
		{
			PACKET_HEAD packHead;
			memcpy(&packHead, szResponse, sizeof(PACKET_HEAD));
			if(!IsPacketValidResponse(pDevice, pTagGroup, &packHead)) // 包头都不对，那么肯定抛出错误信息
			{

				Drv_LogMessage(PK_LOGLEVEL_ERROR, "从设备(%s)接收到的读数据块(%s)请求，头部格式非法(长度：%d)",
					pDevice->szName, pTagGroup->szAutoGroupName,reqPack.nTotalPackLen);
				sprintf(szTip, "tag:%s, recv %d bytes, header invalid", pTag->szName, lCurRecvLen);
				//Drv_SetCtrlResult(lCmdId, -200, szTip);
				return -1;
			}

			if (lCurRecvLen >= sizeof(PACKET_HEAD) + packHead.nBodyDataLen)	// 大于1个包的长度，则直接返回
				break;
		}
	}
	if(lCurRecvLen <= 0){
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"send to device %d bytes success,but receive no bytes",lSentBytes);
		sprintf(szTip, "tag:%s, recv failed,should:%d,actual:%d", pTag->szName, lSentBytes, lCurRecvLen);
		//Drv_SetCtrlResult(lCmdId, -200, szTip);
	}

	lRet = ParsePackage(pDevice, pTagGroup, szResponse, lCurRecvLen, tmRequest);
	if(lRet != PK_SUCCESS)
	{
		CheckBlockStatus(pDevice, pTagGroup, -1);
		SetClearRecvBufferFlag(pDevice);	// 设置下次发送前清除标志位标记

		sprintf(szTip, "tag:%s, parse package failed,ret:%d", pTag->szName, lRet);
		//Drv_SetCtrlResult(lCmdId, -200, szTip);
	}
	else
	{
		// 设置失败请求计数
		CheckBlockStatus(pDevice, pTagGroup, 0);
		sprintf(szTip, "tag:%s, ok", pTag->szName);
		//Drv_SetCtrlResult(lCmdId, PK_SUCCESS, szTip);

	}
	
	return 0;
}
	
#define BLOCK_TYPE_NUM_BYBIT		sizeof(g_arrTypeNameByBit)/sizeof(g_arrTypeNameByBit[0])
#define BLOCK_TYPE_NUM_BYBWORD	 	sizeof(g_arrTypeNameByWord)/sizeof(g_arrTypeNameByWord[0])

// 支持的地址格式：按位类型包括：X、Y、M、S、L、F、B、V、CS、CC、TS、TC、SS、SC、SB、S、DX、DY。按字类型包括：TN、SN、CN、D、Z、W、SW。具体存放在g_arrTypeNameByBit和g_arrTypeNameByWord中
// 输入：szAddressInDevice：变量地址，按字类型：D2025,D2026.2,TN2,TN2.1。按位类型：X1,Y2,TS2
// 兼容地址格式：D.2025.2,D.2025.2
// 输入：nTagLenBits，根据tag类型计算得到的tag值位数
// 输出：*pnStartBits、*pnEndBits, 相对于相对整个物理块（如AI、DO、D、DB1，而不是重组的某个AI内的某个Group）内的起始位、结束位（含结束位，如16则起始位0，结束位15），以位为单位
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameBufLen, int *pnStartBits, int *pnEndBits)
{
	memset(szBlockName, 0, nBlockNameBufLen);
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// 复制地址，并去掉#和：号。该方法可能会被多线程调用，不能用静态变量
	char szAddress[PK_IOADDR_MAXLEN + 1] = {0};
	char *pDest = szAddress;
	char *pTmp = szAddressInDevice;
	while(*pTmp!='\0')
	{
		if(*pTmp == '#' || *pTmp == ':') // 仅仅复制非: 和非#
			*pDest = '.';
		else
			*pDest = *pTmp;
		pTmp ++;
		pDest ++;
	}

	pTmp = szAddress; // 获取字符串的首地址

	// 找到第一个是阿拉伯数字的字符，之前的就是块名
	pDest = szBlockName;
	while(*pTmp!='\0')
	{
		if(*pTmp >= '0' && *pTmp <= '9' || *pTmp == '.') // 找到第一个是阿拉伯数字的字符，之前的就是块名
			break;
		*pDest = *pTmp; // 复制字符
		pTmp ++;
		pDest ++;
	}
	if(strlen(szBlockName) == 0)//如果第一个字符是阿拉伯数字，那么就认为是D块
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no block type name! assume :D", szAddressInDevice);
		strcpy(szBlockName, BLOCK_TYPE_NAME_D);
	}
	if(*pTmp=='.') // 跳过.号
		pTmp ++;

	// 找到块名后面的地址，直到结束或者有.号
	char szRegisterNo[PK_DATABLOCKTYPE_MAXLEN] = {0};
	pDest = szRegisterNo;
	while(*pTmp!='\0' && *pTmp!='.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp ++;
		pDest ++;
	}
	if(strlen(szRegisterNo) == 0)//如果第一个字符是阿拉伯数字，那么就认为是D块
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, 寄存器地址位配置, 异常", szAddressInDevice);
		return -1;
	}

	// 找到地址后面的位号，有.号
	if(*pTmp =='.') // 跳过.号自己
		pTmp ++;

	char szBitNo[PK_DATABLOCKTYPE_MAXLEN] = {0};
	pDest = szBitNo;
	while(*pTmp!='\0' && *pTmp!='.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp ++;
		pDest ++;
	}

	bool bBlockByBit = false;
	bool bBlockByWord = false;
	for(int i = 0; i < BLOCK_TYPE_NUM_BYBWORD; i ++){
		if(PKStringHelper::StriCmp(g_arrTypeNameByWord[i], szBlockName) == 0)
		{
			bBlockByWord = true;
			break;
		}
	}
	if(!bBlockByWord)
	{
		for(int i = 0; i < BLOCK_TYPE_NUM_BYBIT; i ++){
			if(PKStringHelper::StriCmp(g_arrTypeNameByBit[i], szBlockName) == 0)
			{
				bBlockByBit = true;
				break;
			}
		}
	}
	if(!bBlockByWord && !bBlockByBit)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, 数据块:%s不支持, 异常", szAddressInDevice,szBlockName);
		return -2;
	}
	
	// 计算起始地址.三菱PLC是从地址0开始的，如D0，X0和Y，不用-1
	int	 nStartAddr = ::atoi(szRegisterNo);
	if(bBlockByWord) // bit
		*pnStartBits = nStartAddr * 16;
	else
		*pnStartBits = nStartAddr;

	// 计算起始位，对于字地址有效
	if(strlen(szBitNo) > 0)
	{
		int nBitNo = 0;
		nBitNo = ::atoi(szBitNo);	
		if(nBitNo >= 0)
			*pnStartBits += nBitNo;
	}

	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}