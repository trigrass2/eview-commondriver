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

#define DEVPARAM_TRANSID							0	// �����;
#define DEVPARAM_MAXBYTES_ONEBLOCK					1	// ÿ���������ֽ���;
#define DEVPARAM_CLEARFLAG							2	// �Ƿ���Ҫ�����������;
#define PK_TAGDATA_MAXLEN							4096

// �Ƿ���Ҫ�����־λ
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
		if (pTagGroup->nFailCountRecent > 3)	// ���ʧ�ܴ���
		{
			char szTip[1024] = { 0 };
			sprintf(szTip, "read failcount:%d", pTagGroup->nFailCountRecent);
			UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, szTip);
			pTagGroup->nFailCountRecent = 0; // �������̫����ѭ��
		}
		else
			pTagGroup->nFailCountRecent += 1;
	}
}


//override the program
PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "�豸param1:վ��,ȱʡΪ1. ����2:0��ʾ����ָ���д,1��ʾ��Ĵ�����д,ȱʡΪ0.  ����3:ÿ��������ֽ���,ȱʡ������.  ����4:δ����");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s,param1->clientno)", pDevice->szName, pDevice->szParam1);
	pDevice->nUserData[DEVPARAM_TRANSID] = 0;				// �����;

	// ��ȡ�����е�tag�㡣��Ҫ��tag��洢����ƫ�ƣ�λ�������ȣ�λ�����������tag������б����Ա����);
	// ��������鴦���������е�tag��������BLOCK;
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK]; // modbusÿ���豸��������ͬ��Ӧ������Ϊ�����������ȽϺ���;
	if (groupOption.nMaxBytesOneGroup <= 0) //���û�����룬��ȡȱʡ230���ֽ�;
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

		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // ��ͬ����������飬��˫�����;

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[0] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // �趨��ʱ��;
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
	����ʼ���豸
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
*  �趨�����ݿ鶨ʱ���ڵ���ʱ�ú���������.
*
*  @return PK_SUCCESS: ִ�гɹ�
*
*  @version     12/11/2008    Initial Version..
*/
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	// ��֯��ȡ��Ϣ
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
*  ���п�������ʱ�ú���������;
*  @param  -[in]  szStrValue������ֵ����blob�ⶼ�Ѿ��Ѿ�ת��Ϊ�ַ�����blobת��Ϊbase64����
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

	// ���ж��Ƿ�Ҫ�����־λ
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	long lSentBytes = Drv_Send(pDevice, message.m_pMsgBuffer, message.m_nMsgBufferLen, 500);
	if (lSentBytes != message.m_nMsgBufferLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)���Ͷ�дtag(%s)����ʧ��(����%d���ֽڣ�ʵ�ʷ���%d��)������ţ�%d",
			pDevice->szName, szTagName, message.m_nMsgBufferLen, lSentBytes, nSendTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -201, "control, sentlen(%d)!=reqlen(%d)", lSentBytes, message.m_nMsgBufferLen);
		return -201;
	}

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!����������豸(%s)����дtag(%s)����ɹ���ʵ�ʷ���%d��, ����ţ�%d",
		pDevice->szName, pTag->szName, lSentBytes, nSendTransID);

	time_t tmRequest;
	time(&tmRequest);
	message.ProcessRecvData(pDevice, pTagGroup, nSendTransID, false, tmRequest);
	return 0;
}


// ֧�ֵĵ�ַ��ʽ��1. ��������:��ʼ��ַ(WR:100)  2.��������:��ʼ��ַ.�ڼ�λ(WR:100.0)
// �������Ͱ�����CIO|WR|HR|AR|TPV|CPV|TCF|CCF|DM|DR|EM|EMC|IR    
// ��ʼ��ַ��0��ʼ����������  #����ĳ������ֽ�Ϊ��λ;
// ���룺szAddressInDevice��������ַ���������ͣ�[CIO|WR|HR|AR|TPV|CPV|TCF|CCF|DM|DR|EM|EMC|IR]:[1-65534][.X]
// ���룺nTagLenBits������tag���ͼ���õ���tagֵλ��
// �����*pnStartBits��*pnEndBits, ���������������飨��CIO��WR��AR�������������ĳ�������ڵ�ĳ��Group���ڵ���ʼλ������λ��������λ����16����ʼλ0������λ15������λΪ��λ
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;

	string strAddress;
	// �����ĵ�ַ���������ʼ��ַ֮��û�� �� �� . �����������һ��ð��;
	bool bFirstDigit = true;
	char *pCur = szAddressInDevice;
	//if (*pCur != '\0')
	while (*pCur != '\0')
	{
		if (*pCur >= '0' && *pCur <= '9') // ��һ����ĸ�����֣���;
		{
			if (bFirstDigit) // ��һ����������;
			{
				if (*(pCur - 1) != ':' && *(pCur - 1) != '.')  // ������ǰ�Ŀ����Ʋ���ð�Ż��� ����CIO10�������;
					strAddress = strAddress + ":"; // ��Ҫ��һ��ð��;
				bFirstDigit = false;
			}
		}
		strAddress = strAddress + *pCur;
		//ָ��ָ����һ����ַ;1121
		*pCur++;
	}

	vector<string> vecBlk = PKStringHelper::StriSplit(strAddress, ":."); // ֧����: �� . �ָ�;
	if (vecBlk.size() >= 1)
		PKStringHelper::Safe_StrNCpy(szBlockName, vecBlk[0].c_str(), nBlockNameLen); // ��������;

	int nStartRegisterNo = 0; // ��ʼ�Ĵ���;
	if (vecBlk.size() >= 2)
		nStartRegisterNo = ::atoi(vecBlk[1].c_str()) - 1;

	int nStartBitOfRegister = 0; // �ڼĴ����ڵ�λ;
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

	*pnStartBits = nStartRegisterNo * nRegisterLenBits + nStartBitOfRegister; // ������ʼ�Ĵ���λ;
	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}