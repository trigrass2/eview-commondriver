// Modbus �������豸����1Ϊ0��ʾModbusRTU������Ϊ1��ʾModbusTCP����
// ���ݿ����1��ʾվ�š�վ�ű�����ڵ���1
// ����2����д����Щָ�
// ����3��ÿ���������ֽ���
// ����Ҳ����һ����������ʾվ�ţ����һ���豸�����˶����ͬ���飬ÿ�����վ�ſ����ǲ�ͬ�ģ�
// DRVTAG��nData1����ʼ��ַλ�������AI��0��ַ����nData2�ǽ�����ַλ��nData3���ڸÿ��ڵ���ʼλ��
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

#define DEVPARAM_TRANSID							0	// �����
#define DEVPARAM_STATION_NO							1	// վ��
#define DEVPARAM_RWCMD								2	// ��д��ʲôָ��
#define DEVPARAM_MAXBYTES_ONEBLOCK					3	// ÿ��������ID
#define DEVPARAM_CLEARFLAG							4	// �Ƿ���Ҫ�����������
#define PK_TAGDATA_MAXLEN							4096
/**
*  �Ƿ���Ҫ�ߵ��ֽ�ת��.
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
	//modbus�쳣�����صĹ�����Ϊ����Ĺ�����+0x80(����׼modbusЭ�����)
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
*  @param		-[in,out]  char * pPackageHead : ��ͷָ��
*  @param		-[in,out]  bool bModbusTcp : �Ƿ�modbus tcp
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
		return INT_MAX; // ��ͷ���������򷵻�һ�����ֵ��˵����Ҫ������������

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

// ModbusRTUʱ���豸ID����Ϊ0��������1
// ��ȡtag��ĵ��������������û����ȡ�豸�ĵ����������������û����ȱʡΪ1
// ����ʱ������վ�ţ����Ƕ�дʱδ����վ�ţ�������������������������������������
unsigned char GetStationID(PKDEVICE *pDevice, PKTAG *pTag)
{
	unsigned char nStationNo = pDevice->nUserData[DEVPARAM_STATION_NO];
	if (pTag && strlen(pTag->szParam) > 0 && ::atoi(pTag->szParam) > 0)
		nStationNo = ::atoi(pTag->szParam);

	return nStationNo;
}

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

long ParsePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *pszRecvBuf, long lRecvBufLen,
	unsigned short nRequestTransID, time_t tmRequest)
{
	bool bModbusTCP = IsModbusTCP(pDevice);
	if (bModbusTCP &&  lRecvBufLen < PROTOCOL_TCP_PACKAGE_HEADER_LEN || !bModbusTCP && lRecvBufLen < PROTOCOL_RTU_PACKAGE_HEADER_LEN)
	{
		UpdateGroupQuality(pDevice, pTagGroup, -100, "parsing,packlen:%d < headlen:%d,discard", lRecvBufLen, PROTOCOL_TCP_PACKAGE_HEADER_LEN);
		return -100;
	}

	int nStartBytes = pTagGroup->nStartAddrBit / 8; // ��0��ʼ�ĵ�N���ֽ�
	int nEndBytes = pTagGroup->nEndAddrBit / 8; // ��0��ʼ�ĵ�N���ֽ�
	int nLenBytes = nEndBytes - nStartBytes + 1;
	int nBlockSize = nLenBytes; //(int)ceil((pTagGroup->nEndBit - pTagGroup->nStartAddrBit + 1)/8.0f);
	// ѭ�������յ������п������ݰ�
	// 2��ʾվ�ţ�1���ֽڣ��͹����루1���ֽڣ���
	// �κ�����Ӧ���ͷ6���ֽڶ���һ�������壬��7�͵�8���ֽ�Ҳ��һ�������壨վ�ź͹����룩
	long nRecvBytes = lRecvBufLen;
	bool bFoundThisResponse = false;
	char *pCurBuf = (char *)pszRecvBuf;

	while ((bModbusTCP && nRecvBytes >= PROTOCOL_TCP_PACKAGE_HEADER_LEN) ||
		(!bModbusTCP && nRecvBytes >= PROTOCOL_RTU_PACKAGE_HEADER_LEN))
	{
		char *pThisPackHeader = pCurBuf;
		unsigned short uTransID = 0; // ����ModbusTCP��Ч
		unsigned short uPackBodyLen = 0; // ����ModbusTCP��Ч
		if (bModbusTCP)
		{
			memcpy(&uTransID, pCurBuf, 2);	// �����
			if (uTransID < 0) // ����ID���벻��Ϊ0
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "������ʱ, ��������IDΪ %d<0, ����������Ϊ %d, �����豸%s���ν��յ���������!",
					uTransID, nRecvBytes, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -101, "parsing,packlen:%d < utransId:%d<0,discard", nRecvBytes, uTransID);
				return -101;
			}
			if (uTransID == nRequestTransID)
				bFoundThisResponse = true;

			pCurBuf += 2;	// first 2 bytes

			// ����Э1d��š���Զ��0
			unsigned short uProtocolID = 0;
			memcpy(&uProtocolID, pCurBuf, 2);	// Э���
			if (uProtocolID < 0) // Э��ID���벻��Ϊ0
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "������ʱ, ��������Э���Ϊ %d, ������0! ʣ�໺��������Ϊ %d, �����豸%s���ν��յ���������!",
					uProtocolID, lRecvBufLen, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -102, "parsing,packlen:%d < protocolId:%d<0,discard", nRecvBytes, uProtocolID);
				return -102;
			}
			pCurBuf += 2;  // No.3-4 bytes

			// �������ݰ�����
			uPackBodyLen = (((unsigned char)*pCurBuf) << BITS_PER_BYTE) + (unsigned char)*(pCurBuf + 1);
			// ���ȷǷ���������Ϊ0
			if (uPackBodyLen <= 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "������ʱ, ���ְ�����Ϊ %d, ʣ�໺��������Ϊ %d,  �����豸%s���ν��յ���������!",
					uPackBodyLen, lRecvBufLen, pDevice->szName);
				UpdateGroupQuality(pDevice, pTagGroup, -102, "parsing,packlen:%d < uPackBodyLen:%d<0,discard", nRecvBytes, uPackBodyLen);
				return -1;
			}
			pCurBuf += 2; // No.3 5-6 bytes
		} // if(bModbusTCP)

		// վ��
		unsigned char uStationID = (unsigned char)*pCurBuf; // ��7�ֽ�
		unsigned char nFuncCode = (unsigned char)* (pCurBuf + 1); // No.8 bytes

		//modbus�쳣���Ĺ�����Ϊ����Ĺ�����+0x80��������Ϊ�ǺϷ��Ĺ�����
		if (!IsValidFunctionCode(nFuncCode))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "ModbusӦ���(�����:%d)�Ĺ��ܺ�Ϊ %d������ȷ������Ӧ���ܺţ������豸 %s ���ν��յ��������ݣ�",
				uTransID, nFuncCode, pDevice->szName);
			UpdateGroupQuality(pDevice, pTagGroup, -103, "parsing,packlen:%d < invalid funcCode:%d<0", nRecvBytes, nFuncCode);
			return -103;
		}

		// ������峤��+����ǰ��ͷ6���ֽڣ�����ʣ�໺�������ȣ�˵������һ����������
		if (bModbusTCP && (uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN > lRecvBufLen))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s ModbusӦ���(�����:%d)�Ĺ��ܺ�Ϊ %d�������:%d, �в������İ���",
				pDevice->szName, nFuncCode, uTransID);
			UpdateGroupQuality(pDevice, pTagGroup, -104, "parsing,recvbuf:%d < packlen(should):%d", lRecvBufLen, uPackBodyLen + PROTOCOL_TCP_PACKAGE_HEADER_LEN);
			return -104;
		}

		//ͨ��λ�������жϸù�����Ϊ�쳣��������,������쳣��ֱ�������ð�
		if ((nFuncCode & MODBUS_BASE_EXCEPTION_FUNC_CODE) == MODBUS_BASE_EXCEPTION_FUNC_CODE)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Modbus����(�����:%d)�����쳣�����쳣���Ĺ��ܺ�Ϊ %d��", uTransID, nFuncCode);
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

		// 5/6/15/16 ��д�����Ӧ��, ��ȡ�������Ӧ�� 1/2/3/4�Ƕ�����
		bool bReadResponseFromDev = (nFuncCode <= 4);
		if (!bReadResponseFromDev)
		{
			return TAG_QUALITY_GOOD; // ������ܻ��Զ����ÿ��Ƶ�״̬
		}

		// ע�⣺����������ݿ鳤�ȳ���256ʱ��modbusģ�����Ϊ232�ֽڣ���modbusģ�������صİ�ͷΪ�����ֽ���+3����uReturnDataLenΪ232
		// ��ʱ�Ͳ�Ҫ����������ˣ�ֱ������ڴ��˳�
		unsigned short uReturnDataLen = (unsigned char)*(pCurBuf + 2);	// ���ص����ݵĳ���.No.8 byte

		// ����Modbusģ�������������ݳ���232�ֽ�ʱ��uPackBodyΪ���󳤶�+3����uReturnDataLenΪ232
		// ��ˣ��ڽ���ǰ�������ĺϷ�����֤֮��ֻ����������д��DIT��ȥ������
		if (bModbusTCP && (uPackBodyLen & 0x00FF) != uReturnDataLen + 3)//��������˷������ݳ��ȣ��ó���Ϊbody�ĵ��ֽ�-3
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s ������ʱ, ��������������ͷ+������������Ϊ %d, ����������������Ϊ %d, ���߲�ƥ��(ǰ�߳���Ӧ=����+3�������ܻ���������Ϊ %d���������ð�!",
				pDevice->szName, uPackBodyLen, uReturnDataLen, lRecvBufLen);
			UpdateGroupQuality(pDevice, pTagGroup, -106, "recv bodylen(%d)!=retdatalen(%d)+3:%d", uPackBodyLen, uReturnDataLen);
			return -106;
		}

		// һ�ζ�ȡ���Ŀ�С��DIT����Ҳ��Ҫ���µ�
		if (nBlockSize >= uReturnDataLen)
		{
			char *szBlockType = pTagGroup->szHWBlockName; // AI/DI/AO/DO
			// �������ݿ������
			if (NeedSwap(szBlockType))
			{
				// ÿ���Ĵ����ĳ��ȣ�λ��
				int nElemBits = 1;
				if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0 || PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AI) == 0)
					nElemBits = 16;

				// pCurBufָ���6���ֽڣ�Ӧ��������վ�š�������ͳ��ȹ������ֽ�
				SwapByteOrder(pCurBuf + 3, uReturnDataLen, nElemBits / BITS_PER_BYTE);
			}

			// �������ݿ�����ݡ�����OnDatablockTimer�Ķ�ʱ���ж�ȡ�����ݺ󣬵��øýӿڸ��������Ա�������ݿ��ȡ
			UpdateGroupData(pDevice, pTagGroup, pCurBuf + 3, uReturnDataLen, TAG_QUALITY_GOOD);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "dev:%s, grp:%s, updata:%d bytes", pDevice->szName, pTagGroup->szAutoGroupName, uReturnDataLen);
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "���ݿ�(%s)�յ��Ķ�����Ӧ����Ϣ�峤��(%d)���ڴ��г���(%d)��һ��, ����",
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

		Drv_LogMessage(PK_LOGLEVEL_INFO, "�յ�������Ӧ�����豸��:%s,������%s, �����:%d��", pDevice->szName, pTagGroup->szAutoGroupName, uTransID);
		//DRVTIME tvNow;
		//Drv_GetCurrentTime(&tvNow);
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "�յ�������Ӧ�����豸��:%s,������%s, �����:%d����������ʱ��(%u.%u),������ʱ��(%u.%u),ʱ���:%u����",
		//	pDevice->szName, pTagGroup->szAutoGroupName, uTransID, (unsigned long)tmRequest.tv_sec, (unsigned long)tmRequest.tv_usec,
		//	(unsigned long)tvNow.tv_sec, (unsigned long)tvNow.tv_usec, abs((long)((tvNow.tv_sec - tmRequest.tv_sec) * 1000 +  (tvNow.tv_usec - tmRequest.tv_usec)/1000)));
	}//while(m_nRemainBufferLen > PROTOCOL_PACKAGE_HEADER_LEN + 2)

	return PK_SUCCESS;
}

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "�豸param1:վ��,ȱʡΪ1. ����2:0��ʾ����ָ���д,1��ʾ��Ĵ�����д,ȱʡΪ0.  ����3:ÿ��������ֽ���,ȱʡ������.  ����4:δ����");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s)", pDevice->szName);
	pDevice->nUserData[DEVPARAM_TRANSID] = 0;				// �����

	// ����1��վ��
	pDevice->nUserData[DEVPARAM_STATION_NO] = 1;			// վ�ţ�ȱʡΪ1
	if (pDevice->szParam1 != NULL && atoi(pDevice->szParam1) != 0)
		pDevice->nUserData[DEVPARAM_STATION_NO] = atoi(pDevice->szParam1);

	// ����2������ʲôָ��
	pDevice->nUserData[DEVPARAM_RWCMD] = 0;				// ��д��ʲôָ��,0��ʾ������ָ�1��ʾ��Ĵ���ָ��
	if (pDevice->szParam2 != NULL && atoi(pDevice->szParam2) != 0)
		pDevice->nUserData[DEVPARAM_RWCMD] = atoi(pDevice->szParam2);

	// ����3��ÿ��������ֽ���  modbusÿ���豸��������ͬ��Ӧ������Ϊ�����������ȽϺ���
	if (strlen(pDevice->szParam3) > 0)
	{
		int nCfgMaxBytes = ::atoi(pDevice->szParam3);
		if (nCfgMaxBytes > 0)
			pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK] = nCfgMaxBytes;
	}

	// ��ȡ�����е�tag�㡣��Ҫ��tag��洢����ƫ�ƣ�λ�������ȣ�λ�����������tag������б��Ա���㣩

	// ��������鴦�������е�tag��������BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK]; // modbusÿ���豸��������ͬ��Ӧ������Ϊ�����������ȽϺ���
	if (groupOption.nMaxBytesOneGroup <= 0) //���û�����룬��ȡȱʡ230���ֽ�
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
		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // AI,AO��������飬DI��DO����λ���


		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[0] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // �趨��ʱ��
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
/*
int Drv_SetDeviceCommStatus(PKDEVICE *pDevice, int bDevConnected, int nMaxToleranceSeconds)
{
bool bChangedStatus = false;
int nLastConnStatus = pDevice->nUserData[8];
if(!bDevConnected)
{
time_t tmNow;
time(&tmNow);
if(nLastConnStatus == 1)// ��ǰ������,����û����,1--->0
{
if(pDevice->nUserData[9] == 0) // ��ǰû�жϿ�
pDevice->nUserData[9] = tmNow; // ���η��ֶϿ�ʱ��
else // ��ǰ��¼��ʱ�䣬�Ѿ����˶Ͽ���ʶ
{
int nDiscTimeSpan = tmNow - pDevice->nUserData[9]; // ���ϴ�Ϊ�����ϵĲ�ֵ
if(nDiscTimeSpan > nMaxToleranceSeconds)
bChangedStatus = true;
}
}
}
else
{
if(nLastConnStatus == 1)   // ��ǰû���ϣ�����������
{
bChangedStatus = true;
pDevice->nUserData[8] = 1; // �ϴ�����״̬
}
pDevice->nUserData[9] = 0; // ��ʶ֮ǰһֱ��������״̬
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
pTag->nQuality = 0; // ֵ����֮ǰ��ֵ����!!
pTag->nTimeSec = pTag->nTimeMilSec = 0; // ʱ�����Ϊ���µ�
}
else
{
pTag->nQuality = 1;
pTag->nTimeSec = pTag->nTimeMilSec = 0; // ʱ�����Ϊ���µ�
}
}
Drv_UpdateTagsData(pDevice, vecTags);
vecTags.clear();
}
return 0;
}
*/
/**
*  �趨�����ݿ鶨ʱ���ڵ���ʱ�ú���������.
*
*  �����ڸú��������豸���������ȡʵʱ���ݻ����豸���Ϳ���ָ��.
*  1. �Ӳ���HANDLE_BLOCK�п��Ի�ȡ�����ݿ���Ϣ���豸��Ϣ.

*  2. �ú�������Ҫ�����´���
a) �����Ҫ��ʱɨ�裬�����豸���������ȡʵʱ����
b) �����Ҫ��ʱ���ƣ������豸���Ϳ�������
c) �����ͬ����ʽ�����豸���ݣ�����Ҫ�ڸú����еȴ��豸����,ֱ�����ݵ����ʱ���أ�
d) ����첽��ʽ�����豸���ݣ�����Ҫ�ṩһ���������ݵĻص��������ڻص������д����豸�������ݡ�

*  3. �����յ��豸��������ʱ�������豸�������ݣ�
a) �����ʵʱ���ݣ�����Ҫ�������ݿ��ֵ
���Ե�������EXE�ṩ�Ļص�����g_drvCBFuncs.pfn_Drv_UpdateBlock��g_drvCBFuncs.pfn_Drv_UpdateBlockEx
b) ����ǿ�����������������������Ӧ����
*
*  @param  -[in]  HANDLE_BLOCK  hBlk: [���ݿ���]
*
*  @return PK_SUCCESS: ִ�гɹ�
*
*  @version     12/11/2008    Initial Version.
*  @version	10/26/2012  shijunpu  �޸����ڰ������������������һֱ�޷���ȡ������ݺ��ڴ��쳣����.
*  @version	11/2/2012  shijunpu  �޸Ķ���ModbusRTUЭ�飬���ڿ����յ�����վ�ŵİ���ɽ���ʧ�ܶ���������.
*/
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	bool bModbusTCP = IsModbusTCP(pDevice);

	// ��֯��ȡ��Ϣ
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];
	unsigned short uTransID = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];
	char szRequestBuff[DEFAULT_REQUEST_MAXLEN];
	memset(szRequestBuff, 0, sizeof(szRequestBuff));
	char* pTransmit = szRequestBuff;

	// ��ʼ��ַ��ModbusTCP����
	if (bModbusTCP)
	{
		// �����Ϊ2���ֽ�
		memcpy(pTransmit, &uTransID, 2);
		pTransmit += 2;

		// Э���ʶ�����ȸ��ֽڣ�����ֽڣ�����0
		memset(pTransmit, 0, 2);
		pTransmit += 2;

		// �����ֶΣ������ֽڵ������������ֽ�
		*pTransmit = 0;
		pTransmit++;
		//  �����ֶΣ������ֽڵ������������ֽ�
		*pTransmit = 0x06; // ��������ͷ���ĳ��ȣ��̶�Ϊ6���ֽ�
		pTransmit++;
	}

	// ���濪ʼʱModbus���е�
	unsigned char nStationNo = GetStationID(pDevice, NULL);
	// վ��
	memcpy(pTransmit, &nStationNo, 1);
	pTransmit++;

	// ������Modbus����ͷ����5���ֽ�
	// ������
	char *szBlockType = pTagGroup->szHWBlockName;
	int nBlockType = GetBlockTypeId(szBlockType);
	memcpy(pTransmit, &nBlockType, 1);// �������������ֵ��ͬ
	pTransmit++;

	//nStartAddress += 1; // ��ʼ��ַ��0��ʼ.AO:8��nBeginRegisterΪ7
	// ��ʼ��ַ���ֽ�
	*pTransmit = (pTagGroup->nBeginRegister) >> BITS_PER_BYTE; // ��ַ���֣��Ĵ�����������������������������������
	pTransmit++;
	// ��ʼ��ַ���ֽ�
	*pTransmit = (pTagGroup->nBeginRegister) & 0xFF;
	pTransmit++;

	// ��ȡbit/�Ĵ������������ֽ�
	*pTransmit = pTagGroup->nRegisterNum >> BITS_PER_BYTE;
	pTransmit++;
	// ��ȡbit/�Ĵ������������ֽ�
	*pTransmit = pTagGroup->nRegisterNum & 0xFF;
	pTransmit++;

	int nRequestBufLen = pTransmit - szRequestBuff;
	if (!bModbusTCP) // ModbusRTU
	{
		// ����CRC
		unsigned short nCRCValue = CRC16((unsigned char *)szRequestBuff, nRequestBufLen);
		memcpy(pTransmit, &nCRCValue, sizeof(unsigned short));
		pTransmit += 2;
	}
	nRequestBufLen = pTransmit - szRequestBuff;

	// ����ս��ջ�������������ǰ������������û�����ü����ջ����ڻ�������Ӱ�챾������
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	// ���ɵĶ���Ϣ���ȣ�Ӧ���ǹ̶���12���ֽ�
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

	// ���豸����Ӧ����Ϣ
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
		SetClearRecvBufferFlag(pDevice);	// �����´η���ǰ�����־λ���
	}
	else
	{
		// ����ʧ���������
		CheckBlockStatus(pDevice, pTagGroup, 0);
	}

	return nConnStatus;
}

/**
*  ���п�������ʱ�ú���������.
*  @param  -[in]  szStrValue������ֵ����blob�ⶼ�Ѿ��Ѿ�ת��Ϊ�ַ�����blobת��Ϊbase64����
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

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@�յ�����������豸(%s)��tag(%s)���п��ƣ���ַ:%s",
		pDevice->szName, szTagName, szAddress);

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	char *szBlockType = pTagGroup->szHWBlockName;
	int nBlockType = GetBlockTypeId(szBlockType);
	int nStartBits = pTag->nStartBit;// ����ڿ��ڵ���ʼ��ַλ����AI/DI)
	int nEndBits = pTag->nEndBit; // ����ڿ��ڵĽ�����ַλ����AI/DI)
	//int nBitNo = 0;
	//int nRet = ParseTagAddrInfo(pTag->szAddress,pTag->nDataType, &nBlockType, &nStartBits, &nEndBits, &nBitNo);

	// AI/DI���ͣ�����д��
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DO) != 0 && PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "����ʱ���ݿ�����Ϊ %s,����ȷ, ����AO��DO��", szBlockType);
		return EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE;
	}

	// ��ʼ��ַ��Ҫ���Ƶĵ�ַ
	long lStartAddress = nStartBits;	// ��0��ʼ�ĵ�ַ// Ҫ���Ƶĵ�ַ����,���ԼĴ�������Ϊ��λ��
	int nLenBits = nEndBits - nStartBits + 1; // ���ȣ�λ��ʾ��
	int  nRegisterNum = nLenBits;
	int  nByteCountToWrite = (int)ceil((nLenBits) / 8.0f);	// д�����ݵ��ֽ���,Э����Ҫ��д�������ֽ���Ϊ��λ��

	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_AO) == 0){
		lStartAddress = (int)ceil(nStartBits / 16.0f); // AO��2���ֽ�Ϊ��λ
		nRegisterNum = (int)ceil(nRegisterNum / 16.0f); // AO��2���ֽ�Ϊ��λ
	}
	else // DO
	{
		lStartAddress = nStartBits;	// ��0��ʼ�ĵ�ַ// Ҫ���Ƶĵ�ַ����,���ԼĴ�������Ϊ��λ��
		nRegisterNum = nLenBits;

	}

	int nFunctionCode = 0; // �����롢��ʼ��ַ�Ͷ�ȡ����,��Ϊ��ͷ��Ϣ����Ҫ�õ�
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
		// �������
		unsigned short uTransID = (unsigned short) ++pDevice->nUserData[DEVPARAM_TRANSID];

		// ����Э����֯д��Ϣ
		// ������ţ����ֽ�
		*pTransmit = uTransID >> BITS_PER_BYTE;
		pTransmit++;

		// ������ţ����ֽ�
		*pTransmit = uTransID & 0xFF;
		pTransmit++;

		// Э���ʶ�����ȸ��ֽڣ�����ֽ�
		*pTransmit = 0;
		pTransmit++;
		*pTransmit = 0;
		pTransmit++;

		// �����ֶΣ����ֽ�
		*pTransmit = (7 + nByteCountToWrite) >> BITS_PER_BYTE;
		pTransmit++;

		// �����ֶΣ����ֽ�
		*pTransmit = (nFunctionCode == FUNCCODE_WRITE_SGLAO || nFunctionCode == FUNCCODE_WRITE_SGLDO) ? MB_WRITEPDU_LENGTH : ((7 + nByteCountToWrite) & 0xFF);   // �����ֽڵ�����
		pTransmit++;
	} // bModbusTCP

	// վ��
	*pTransmit = GetStationID(pDevice, pTag);
	pTransmit++;

	// ������
	*pTransmit = nFunctionCode;
	pTransmit++;

	// ��ʼ��ַ���ֽ�
	*pTransmit = lStartAddress >> BITS_PER_BYTE;
	pTransmit++;

	// ��ʼ��ַ���ֽ�
	*pTransmit = lStartAddress & 0xFF;
	pTransmit++;

	// ������Ϊ06ʱ����Ҫ�⼸���ֶΡ�д�����Ȧ���߼Ĵ�������
	if (nFunctionCode != FUNCCODE_WRITE_SGLAO && nFunctionCode != FUNCCODE_WRITE_SGLDO)
	{
		// д����Ȧ��/�Ĵ������������ֽ�
		*pTransmit = nRegisterNum >> BITS_PER_BYTE;
		pTransmit++;

		// д����Ȧ��/�Ĵ������������ֽ�
		*pTransmit = nRegisterNum & 0xFF;
		pTransmit++;

		// д�����ݵ��ֽ���
		*pTransmit = nByteCountToWrite & 0xFF;
		pTransmit++;
	}

	//////////////////////////////////////
	if (PKStringHelper::StriCmp(BLOCK_TYPE_DO, szBlockType) == 0)
	{
		// һ���Ĵ�����ֻ��1�����أ�Ϊ1ʱ������bitȫ����Ϊ1
		if (nRegisterNum == 1 && nFunctionCode != FUNCCODE_WRITE_MULTIDO) // ����ֵ��0��1
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
			// edit by shijunpu,ǿ��д�����Ȧʱ��һ����д8��λ���������������Ҫ���Ƿ�8������������� 
			for (int i = 0; i < nByteCountToWrite; i++)
			{
				*pTransmit = szBinValue[i];
				pTransmit++;
			}
		}
	}
	else // BLOCK_TYPE_AO
	{
		// ��д�����Ĵ���
		// ���ܸô�����Ҫ���ƵĻ�������ͷ����β�����в��������Ĵ������ȣ���4���ֽڣ�λ��Ҫ���жϳ�����д��
		if (nLenBits == 1 || (nLenBits % 8 != 0)) // ����ֵ��0��1
		{
			// ��д������
			//int lValueStatus = 0;
			//unsigned short nData = 0;
			//PKTAGDATA tagData;
			//strncpy(tagData.szName, pTag->szName,sizeof(tagData.szName) - 1);
			//int lRet = Drv_GetTagData(pDevice,&tagData); // ��ȡ���ϴε�����
			//if(lRet != PK_SUCCESS)
			//	return lRet;

			//if(lValueStatus != TAG_QUALITY_GOOD)
			//	return -1;

			//memcpy(&nData, &tagData, 2); // AO or AI
			//// ����ָ��bit����ֵ
			//int nBitNo = nStartBits % 16; // AO:W5.13, һ����ĳ��Word������
			//nData = nData & ~(1 << nBitNo);
			//nData = nData | (szBinValue[0] << nBitNo);
			//
			//// ��д������
			//*pTransmit = nData >> BITS_PER_BYTE;
			//pTransmit++;
			//*pTransmit = (nData & 0xFF);
			//pTransmit++;
		}
		else // TAG_DATATYPE_ANALOG��TAG_DATATYPE_BLOB��TAG_DATATYPE_TEXT
		{
			// ��д������
			if (NeedSwap(szBlockType))
				SwapByteOrder(szBinValue, nBinValueLen, 2);

			// д���ֽ����պõ��ڼĴ����ֽ�ʱ
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
				// д���ֽ��������ڼĴ����ֽ���ʱ
				// ��һ���֣��Ѿ�swap����ֵ���ɸߵ���
				for (i = 0; i < (int)(nRegisterNum - 1) * 2; i++)
				{
					*pTransmit = szBinValue[i];
					pTransmit++;
				}

				// �ڶ����֣����һ���Ĵ����ĸ��ֽ�
				int nToAdd = nByteCountToWrite - nBinValueLen;
				for (int j = 0; j < nToAdd; j++)
				{
					*pTransmit = 0;
					pTransmit++;
				}

				// �������֣����һ���Ĵ����ĵ��ֽڣ��ɸߵ���
				for (; i < nBinValueLen; i++)
				{
					*pTransmit = szBinValue[i];
					pTransmit++;
				}
			}
		}
	}

	// ���ɵ�д��Ϣ����
	int nRequestBufLen = pTransmit - szRequestBuff;
	if (!bModbusTCP)
	{
		unsigned short nCRCValue = CRC16((unsigned char *)szRequestBuff, nRequestBufLen);
		memcpy(pTransmit, &nCRCValue, sizeof(unsigned short));
		pTransmit += 2;
		nRequestBufLen += sizeof(unsigned short);
	}

	// ���ж��Ƿ�Ҫ�����־λ
	if (GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);

	long lSentBytes = Drv_Send(pDevice, szRequestBuff, nRequestBufLen, 100);
	if (lSentBytes != nRequestBufLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)���Ͷ�дtag(%s)����ʧ��(����%d���ֽڣ�ʵ�ʷ���%d��)������ţ�%d",
			pDevice->szName, szTagName, nRequestBufLen, lSentBytes, uTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -201, "control, sentlen(%d)!=reqlen(%d)", lSentBytes, nRequestBufLen);
		return -201;
	}

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!����������豸(%s)����дtag(%s)����ɹ���������:%d��ʵ�ʷ���%d��, ����ţ�%d",
		pDevice->szName, pTag->szName, nFunctionCode, lSentBytes, uTransID);

	// ���豸����Ӧ����Ϣ
	char szResponse[DEFAULT_RESPONSE_MAXLEN];
	long lRecvBytes = Drv_Recv(pDevice, szResponse, sizeof(szResponse), pDevice->nRecvTimeout);
	if (lRecvBytes <= 0)
	{
		//�޸����ڲ�����ƥ����ɳ����쳣���⣬edit by shijunpu @20121026
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)����дtag(%s)����ʧ��(ʵ���յ�%d��)������ţ�%d",
			pDevice->szName, pTag->szName, lRecvBytes, uTransID);
		UpdateGroupQuality(pDevice, pTagGroup, -202, "control, recvlen(%d)<=0", lRecvBytes);
		return -202;
	}
	/*
	long lRet = ParsePackage(pDevice, pTagGroup, szResponse, lRecvBytes, uTransID);
	if(lRet != PK_SUCCESS)
	SetClearRecvBufferFlag(pDevice);	// �����´η���ǰ�����־λ���
	else*/
	{
		// �����豸ΪAO������λ����ΪDO��ȡAO��ĳһ��λ���������λ������ĳ��AO��2���ֽڣ���
		// ��һ����д�����ڣ�������ͬ��λ�������ƣ�����ֺ���Ḳ��ǰ��Ŀ���ֵ
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


// ֧�ֵĵ�ַ��ʽ����λ���Ͱ�����DI��DO�����׵�ַΪ0��1���������Ͱ�����AI��AO�����׵�ַΪ3��4
// ���룺szAddressInDevice��������ַ���������ͣ�[AI|AO|DI|DO|0|1|2|3][.|#|:]20[.5],AO3,3001,3004,AI20.2,3003.3����λ���ͣ�DI1,DO2,1001,0001��֧����:#�ָ��͵�ַ
// ���룺nTagLenBits������tag���ͼ���õ���tagֵλ��
// �����*pnStartBits��*pnEndBits, ����������������飨��AI��DO��D��DB1�������������ĳ��AI�ڵ�ĳ��Group���ڵ���ʼλ������λ��������λ����16����ʼλ0������λ15������λΪ��λ
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// ���Ƶ�ַ����ȥ��#�ͣ��š��÷������ܻᱻ���̵߳��ã������þ�̬����
	char szAddress[PK_IOADDR_MAXLEN + 1] = { 0 };
	char *pDest = szAddress;
	char *pTmp = szAddressInDevice;
	while (*pTmp != '\0')
	{
		if (*pTmp == '#' || *pTmp == ':') // �������Ʒ�: �ͷ�#
			*pDest = '.';
		else
			*pDest = *pTmp;
		pTmp++;
		pDest++;
	}

	pTmp = szAddress; // ��ȡ�ַ������׵�ַ

	// ���ҿ������ҵ���һ���ǰ��������ֵ��ַ���֮ǰ�ľ��ǿ���
	memset(szBlockName, 0, nBlockNameLen);
	pDest = szBlockName;
	while (*pTmp != '\0')
	{
		if ((*pTmp >= '0' && *pTmp <= '9') || *pTmp == '.') // �ҵ���һ���ǰ��������ֵ��ַ���֮ǰ�ľ��ǿ���
			break;
		*pDest = *pTmp; // �����ַ�
		pTmp++;
		pDest++;
	}
	if (strlen(szBlockName) == 0)//�����һ���ַ��ǰ��������֣���ô����3��4��1��2������
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
		pTmp++; // ����0��1��3��4����ַ�
	}
	if (*pTmp == '.') // ����.��
		pTmp++;

	// �ҵ���������ĵ�ַ��ֱ������������.��
	char szRegisterNo[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szRegisterNo;
	while (*pTmp != '\0' && *pTmp != '.') // ֻҪ���������Ҳ�����.����ֵ
	{
		*pDest = *pTmp; // �����ַ�
		pTmp++;
		pDest++;
	}
	if (strlen(szRegisterNo) == 0)//�����һ���ַ��ǰ��������֣���Ϊ���쳣
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, �Ĵ�����ַλ����, �쳣", szAddressInDevice);
		return -2;
	}
	if (*pTmp == '.') // ����.��
		pTmp++;

	// �ҵ���ַ�����λ�ţ���.��
	char szBitNo[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szBitNo;
	while (*pTmp != '\0' && *pTmp != '.') // ֻҪ���������Ҳ�����.����ֵ
	{
		*pDest = *pTmp; // �����ַ�
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
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, ���ݿ�:%s��֧��, �쳣", szAddressInDevice, szBlockName);
		return -2;
	}

	// ������ʼ��ַ.Modbus�ĵ�ַ���Ǵ�1��ʼ�ģ����Ҫ��ȥ1
	int	 nStartAddr = ::atoi(szRegisterNo);
	if (bBlockByBit) // bit
		*pnStartBits = nStartAddr - 1;
	else
		*pnStartBits = (nStartAddr - 1) * 16;

	// ������ʼλ�������ֵ�ַ��Ч
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