#include "ace/Time_Value.h"
#include "ace/OS.h"
#include "cipdrv.h"
#include "math.h"
#include <memory.h>
#include <cstring>
#include <string.h> 
#include <stdlib.h>
#include <string>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "AutoGroup_BlkDev.h"
#include "CIPDevice.h"

#define	INT_MAX										2147483647

#define DEVPARAM_PUSERDATA_CIPDEVICEPOINTER					0	// �����

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s),param1:AB PLC Model,i.e.:SLC500/Logix5000. param2:CPU Slot(zero-based)", pDriver->szName);
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s)", pDevice->szName);
	CIPDevice *pCIPDevice = new CIPDevice(pDevice);
	pDevice->pUserData[DEVPARAM_PUSERDATA_CIPDEVICEPOINTER] = pCIPDevice;				// 

	// ��ȡ�����е�tag�㡣��Ҫ��tag��洢����ƫ�ƣ�λ�������ȣ�λ�����������tag������б��Ա���㣩
	// ��������鴦�������е�tag��������BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 0;
	groupOption.nMaxBytesOneGroup = 100;

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
		if (PKStringHelper::StriCmp(pTagGrp->szHWBlockName, "C") == 0 || PKStringHelper::StriCmp(pTagGrp->szHWBlockName, "N") == 0)
			nRegisterLenBits = 16;
		if (PKStringHelper::StriCmp(pTagGrp->szHWBlockName, "B") == 0)
			nRegisterLenBits = 8;

		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // ��ͬ����������飬��˫�����
		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[0] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // �趨��ʱ��
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "slc, device:%s, autogroup:%s,tagcount:%d, cycle:%d ms,registertype:%s,startregno:%d,endregno:%d",
			pDevice->szName, pTagGrp->szAutoGroupName, pTagGrp->vecTags.size(), pTagGrp->nPollRate, pTagGrp->szHWBlockName, pTagGrp->nBeginRegister, pTagGrp->nEndRegister);
		
		pTagGrp->pIOIAddrGroup = new CIOIGroup();

		int nIndex = 0;
		string strTags;
		for (unsigned int iTag = 0; iTag < pTagGrp->vecTags.size(); iTag++)
		{
			PKTAG *pTag = pTagGrp->vecTags[iTag];
			CIOI *pSymbolAddress = new CIOI();
			pTag->pData1 = pSymbolAddress;
			int nRet = pSymbolAddress->ParseAdress(0, pTag->szAddress, 1);
			if (0 != nRet)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,tag:%s,addr:%s, parse symboladdress failed!PLEASE CHECK!",
					pDevice->szName, pTag->szName, pTag->szAddress, nRet);
			}

			pTagGrp->pIOIAddrGroup->AddIOI_Slice(nIndex, pTag->szAddress, 1);  // ��������ӵ�����
			nIndex++;
			strTags = strTags + "," + pTag->szName;
		}

		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "slc, device:%s, groupname:%s, tagnames:%s", pDevice->szName, pTagGrp->szAutoGroupName, strTags.c_str());
	}
	return 0;
}

/*
����ʼ���豸
*/
//#include "windows.h"
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	CIPDevice *pCIPDevice = (CIPDevice *)pDevice->pUserData[DEVPARAM_PUSERDATA_CIPDEVICEPOINTER];
	pCIPDevice->UnRegisterSession(pDevice);
	delete pCIPDevice;
	pDevice->pUserData[DEVPARAM_PUSERDATA_CIPDEVICEPOINTER] = NULL;
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
// TAG��ַ��N10:31, N10:32......
// ����Ŵ�Լ2�����ڲ����ظ�
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CIPDevice *pCIPDevice = (CIPDevice *)pDevice->pUserData[DEVPARAM_PUSERDATA_CIPDEVICEPOINTER];
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];

	if (pCIPDevice->m_nPLCModel == ABPLC_MODEL_LOGIX5000)
		pCIPDevice->OnTimer_Logix5000_ReadTags(pDevice, pTagGroup);
	else if (pCIPDevice->m_nPLCModel == ABPLC_MODEL_SLC500)
		pCIPDevice->OnTimer_SLC500_ReadTags(pDevice, pTagGroup);
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, model(param1):%s, not support PLC model, must be Logix5000/SLC500", pDevice->szName, pDevice->szParam1);
	}
	return 0;
}

/**
*  ���п�������ʱ�ú���������.
*  @param  -[in]  szStrValue������ֵ����blob�ⶼ�Ѿ��Ѿ�ת��Ϊ�ַ�����blobת��Ϊbase64����
*
*  @version     12/11/2008    Initial Version.
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	return 0;
}

int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;
	string strAddress;
	// �����ĵ�ַ���������ʼ��ַ֮��û�� �� �� . �����������һ��ð��
	bool bFirstDigit = true;
	char *pCur = szAddressInDevice;
	while (*pCur != '\0')
	{
		if (*pCur >= '0' && *pCur <= '9') // ��һ����ĸ�����֣���
		{
			if (bFirstDigit) // ��һ����������
			{
				if (*(pCur - 1) != ':' && *(pCur - 1) != '.')  // ������ǰ�Ŀ����Ʋ���ð�Ż��� ����CIO10�������
					strAddress = strAddress + ":"; // ��Ҫ��һ��ð��
				bFirstDigit = false;
			}
		}
		strAddress = strAddress + *pCur;
		pCur++;
	}
	vector<string> vecBlk = PKStringHelper::StriSplit(strAddress, ":");//PKStringHelper::StriSplit(strAddress, ":."); // ֧����: �� . �ָ�

	if (vecBlk.size() >= 1)
		PKStringHelper::Safe_StrNCpy(szBlockName, vecBlk[0].c_str(), nBlockNameLen); // ��������
	int nStartRegisterNo = 0; // ��ʼ�Ĵ���

	if (vecBlk.size() >= 3)
		nStartRegisterNo = ::atoi(vecBlk[2].c_str());

	//nStartRegisterNo = ::atoi(vecBlk[1].c_str()) - 1;// ԭ����-1?

	int nStartBitOfRegister = 0; // �ڼĴ����ڵ�λ
	if (vecBlk.size() >= 4)
		nStartBitOfRegister = ::atoi(vecBlk[3].c_str());

	int nRegisterLenBits = 16;
	if (PKStringHelper::StriCmp(szBlockName, "N") == 0 || PKStringHelper::StriCmp(szBlockName, "C") == 0)
	{
		nRegisterLenBits = 16;
	}
	if (PKStringHelper::StriCmp(szBlockName, "B") == 0)
	{
		nRegisterLenBits = 1;
	}

	*pnStartBits = nStartRegisterNo * nRegisterLenBits + nStartBitOfRegister; // ������ʼ�Ĵ���λ
	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}


bool is_little_endian()
{
	const int i = 1;
	char *ch = (char*)&i;
	return ch[0] && 0x01;
}

void to_normal(void* src, size_t size)
{
	if (!is_little_endian())
	{
		char chTmp = '\0';
		for (size_t i = 0; i < size / 2; ++i)
		{
			chTmp = ((char *)src)[i];
			((char *)src)[i] = ((char *)src)[size - 1 - i];
			((char *)src)[size - 1 - i] = chTmp;
		}
	}
}
void to_little_endian(void* src, size_t size)
{
	if (!is_little_endian())
	{
		char chTmp = '\0';
		for (size_t i = 0; i < size / 2; ++i)
		{
			chTmp = ((char *)src)[i];
			((char *)src)[i] = ((char *)src)[size - 1 - i];
			((char *)src)[size - 1 - i] = chTmp;
		}
	}
}
