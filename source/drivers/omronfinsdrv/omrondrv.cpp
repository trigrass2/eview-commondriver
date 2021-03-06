#include "omrondrv.h"
#include "math.h"
#include "AutoGroup_BlkDev.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "Message.h"

#define EC_ICV_INVALID_PARAMETER                    100
#define EC_ICV_DRIVER_DATABLOCK_TYPECANNOTWRITE		101
#define EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE			103
#define EC_ICV_DRIVER_DATABLOCK_INVALIDCMDLENGTH	104
#define EC_ICV_BUFFER_TOO_SMALL                     105
#define	INT_MAX										2147483647

#define DEVPARAM_TRANSID							0	// 事务号;
#define DEVPARAM_MAXBYTES_ONEBLOCK					1	// 每个块的最大字节数;
#define DEVPARAM_CLEARFLAG							2	// 是否需要先清除缓冲区;
#define PK_TAGDATA_MAXLEN							4096

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


//override the program
PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备param1:站号,缺省为1. 参数2:0表示正常指令读写,1表示多寄存器读写,缺省为0.  参数3:每个包最大字节数,缺省不限制.  参数4:未定义");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s,param1->clientno)", pDevice->szName, pDevice->szParam1);
	pDevice->nUserData[DEVPARAM_TRANSID] = 0;				// 事务号;

	// 获取到所有的tag点。需要在tag点存储块内偏移（位）、长度（位），组包含的tag点对象列表（以便计算);
	// 进行自组块处理，将所有的tag点自组块成BLOCK;
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK]; // modbus每种设备都有所不同，应该是作为参数传过来比较合适;
	if (groupOption.nMaxBytesOneGroup <= 0) //如果没有输入，则取缺省230个字节;
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
		
		int nRegisterLenBits = 1;
		if (PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_CIO_WORD) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_WR_WORD) == 0
			|| PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_HR_WORD) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_AR_WORD) == 0
			|| PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_TIMER_PV) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_COUNTER_PV) == 0
			|| PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_TIMER_CF) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_COUNTER_CF) == 0
			|| PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_DM_WORD) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_DR_WORD) == 0
			|| PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_EM_WORD) == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_EM_CURRENT_WORD) == 0
			)
			nRegisterLenBits = 16;
		else if (PKStringHelper::StriCmp(pTagGrp->szHWBlockName, AREA_NAME_IR_DWORD) == 0)
			nRegisterLenBits = 32;

		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // 不同区域按照字组块，或双字组块;

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[0] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器;
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "omoron, device:%s, autogroup:%s,tagcount:%d, cycle:%d ms,registertype:%s,startregno:%d,endregno:%d",
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
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "omoron, device:%s, groupname:%s, tagnames:%s", pDevice->szName, pTagGrp->szAutoGroupName, strTags.c_str());
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

/**
*  设定的数据块定时周期到达时该函数被调用.
*
*  @return PK_SUCCESS: 执行成功
*
*  @version     12/11/2008    Initial Version..
*/
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	// 组织读取消息
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];
	CMessage message(pDevice, pTagGroup);
	message.BuildReadMessage(pTagGroup);
	int nSendTransID = message.GetTransId(); // = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];

	int lSentBytes = Drv_Send(pDevice, message.m_pMsgBuffer, message.m_nMsgBufferLen, 500);
	if (lSentBytes != message.m_nMsgBufferLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device(%s), fail to send request for datablock(%s) (need send:%d, sent:%d), transaction:%d",	pDevice->szName, pTagGroup->szAutoGroupName, message.m_nMsgBufferLen, lSentBytes, nSendTransID);
		CheckBlockStatus(pDevice, pTagGroup, -1);
		return -1;
	}
	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "device(%s), success to send request for datablock(%s) (sent:%d), transaction:%d", pDevice->szName, pTagGroup->szAutoGroupName, lSentBytes, nSendTransID);
	time_t tmRequest;
	time(&tmRequest);
	message.ProcessRecvData(pDevice, pTagGroup, nSendTransID, true, tmRequest);
	return 0;
}

/**
*  当有控制命令时该函数被调用;
*  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
*
*  @version     12/11/2008    Initial Version.
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	char szBinValue[PK_TAGDATA_MAXLEN] = { 0 };
	int nBinValueLen = 0;
	Drv_TagValStr2Bin(pTag, szStrValue, szBinValue, sizeof(szBinValue), &nBinValueLen);

	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;
	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	CMessage message(pDevice, pTagGroup);
	message.BuildWriteMessage(pDevice, pTagGroup, pTag, szBinValue, nBinValueLen);

	int nSendTransID = message.GetTransId(); //  = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];

	// 先判断是否要清除标志位
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	long lSentBytes = Drv_Send(pDevice, message.m_pMsgBuffer, message.m_nMsgBufferLen, 500);
	if (lSentBytes != message.m_nMsgBufferLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "向设备(%s)发送读写tag(%s)请求失败(发送%d个字节，实际发送%d个)，事务号：%d",
			pDevice->szName, szTagName, message.m_nMsgBufferLen, lSentBytes, nSendTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -201, "control, sentlen(%d)!=reqlen(%d)", lSentBytes, message.m_nMsgBufferLen);
		return -201;
	}

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!控制命令：向设备(%s)发送写tag(%s)请求成功，实际发送%d个, 事务号：%d",
		pDevice->szName, pTag->szName, lSentBytes, nSendTransID);

	time_t tmRequest;
	time(&tmRequest);
	message.ProcessRecvData(pDevice, pTagGroup, nSendTransID, false, tmRequest);
	return 0;
}


// 支持的地址格式：1. 区块类型:起始地址(WR:100)  2.区块类型:起始地址.第几位(WR:100.0)
// 区块类型包括：CIO|WR|HR|AR|TPV|CPV|TCF|CCF|DM|DR|EM|EMC|IR    
// 起始地址：0开始。。。。。  #后面的长度以字节为单位;
// 输入：szAddressInDevice：变量地址，按字类型：[CIO|WR|HR|AR|TPV|CPV|TCF|CCF|DM|DR|EM|EMC|IR]:[1-65534][.X]
// 输入：nTagLenBits，根据tag类型计算得到的tag值位数
// 输出：*pnStartBits、*pnEndBits, 相对于相对整个区块（如CIO、WR、AR，而不是重组的某个区块内的某个Group）内的起始位、结束位（含结束位，如16则起始位0，结束位15），以位为单位
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;

	string strAddress;
	// 拷贝的地址，将块和起始地址之间没有 ： 或 . 的情况，增加一个冒号;
	bool bFirstDigit = true;
	char *pCur = szAddressInDevice;
	//if (*pCur != '\0')
	while (*pCur != '\0')
	{
		if (*pCur >= '0' && *pCur <= '9') // 第一个字母是数字，且;
		{
			if (bFirstDigit) // 第一次遇到数字;
			{
				if (*(pCur - 1) != ':' && *(pCur - 1) != '.')  // 且数字前的块名称不是冒号或点号 ，即CIO10这种情况;
					strAddress = strAddress + ":"; // 需要补一个冒号;
				bFirstDigit = false;
			}
		}
		strAddress = strAddress + *pCur;
		//指针指向下一个地址;1121
		*pCur++;
	}

	vector<string> vecBlk = PKStringHelper::StriSplit(strAddress, ":."); // 支持以: 或 . 分割;
	if (vecBlk.size() >= 1)
		PKStringHelper::Safe_StrNCpy(szBlockName, vecBlk[0].c_str(), nBlockNameLen); // 拷贝块名;

	int nStartRegisterNo = 0; // 起始寄存器;
	if (vecBlk.size() >= 2)
		nStartRegisterNo = ::atoi(vecBlk[1].c_str()) - 1;

	int nStartBitOfRegister = 0; // 在寄存器内的位;
	if (vecBlk.size() >= 3)
		nStartBitOfRegister = ::atoi(vecBlk[2].c_str());

	int nRegisterLenBits = 16;
	if (PKStringHelper::StriCmp(szBlockName, AREA_NAME_CIO_WORD) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_WR_WORD) == 0
		|| PKStringHelper::StriCmp(szBlockName, AREA_NAME_HR_WORD) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_AR_WORD) == 0
		|| PKStringHelper::StriCmp(szBlockName, AREA_NAME_TIMER_PV) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_COUNTER_PV) == 0
		|| PKStringHelper::StriCmp(szBlockName, AREA_NAME_TIMER_CF) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_COUNTER_CF) == 0
		|| PKStringHelper::StriCmp(szBlockName, AREA_NAME_DM_WORD) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_DR_WORD) == 0
		|| PKStringHelper::StriCmp(szBlockName, AREA_NAME_EM_WORD) == 0 || PKStringHelper::StriCmp(szBlockName, AREA_NAME_EM_CURRENT_WORD) == 0
		|| PKStringHelper::StriCmp(szBlockName, "W") == 0 // WORD
		)
		nRegisterLenBits = 16;
	else if (PKStringHelper::StriCmp(szBlockName, AREA_NAME_IR_DWORD) == 0 || PKStringHelper::StriCmp(szBlockName, "D") == 0
		|| PKStringHelper::StriCmp(szBlockName, "DW") == 0) // DWORD
		nRegisterLenBits = 32;
	else if (strcmp(szBlockName, "B") == 0) // Byte
		nRegisterLenBits = 8;
	else if (strcmp(szBlockName, "b") == 0) // bit
		nRegisterLenBits = 1;
	else
		nRegisterLenBits = 16;

	*pnStartBits = nStartRegisterNo * nRegisterLenBits + nStartBitOfRegister; // 计算起始寄存器位;
	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}