// DRVTAG��nData1����ʼ��ַλ�������AI��0��ַ����nData2�ǽ�����ַλ��nData3���ڸÿ��ڵ���ʼλ��
// �����PLC��intel���ֽ���������һģһ���ģ�������������float��int32��int16��double������ֱ��ת��������Ҫ���ֽ������ˣ�
/*
	X/Y/M/D

	#define  BLOCK_TYPE_NAME_X				"X"		// ����̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_Y				"Y"		// ����̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_M				"M"		// �����̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_S				"S"		// ״̬,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_L				"L"		// ����̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_F				"F"		// ������,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_B				"B"		// ���Ӽ̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_V				"V"		// ���ؼ̵���,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_CS				"CS"	// �������͸��ټ�����,����,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_CC				"CC"	// �������͸��ټ�����,��Ȧ,λ��Ԫ����
	#define  BLOCK_TYPE_NAME_TS				"TS"	// ��ʱ��,���㣬λ��Ԫ����
	#define  BLOCK_TYPE_NAME_TC				"TC"	// ��ʱ��,��Ȧ��λ��Ԫ����
	#define  BLOCK_TYPE_NAME_SS				"SS"	// �ۼƶ�ʱ��,���㣬λ��Ԫ����
	#define  BLOCK_TYPE_NAME_SC				"SC"	// �ۼƶ�ʱ��,��Ȧ��λ��Ԫ����
	#define  BLOCK_TYPE_NAME_SB				"SB"	// ��������̵���,��Ȧ��λ��Ԫ����
	#define  BLOCK_TYPE_NAME_S				"S"		// �����̵���,��Ȧ��λ��Ԫ����
	#define  BLOCK_TYPE_NAME_DX				"DX"	// ֱ������̵���,��Ȧ��λ��Ԫ����
	#define  BLOCK_TYPE_NAME_DY				"DY"	// ֱ������̵���,��Ȧ��λ��Ԫ����

	#define  BLOCK_TYPE_NAME_TN				"TN"	// ��ʱ��,��ǰֵ������Ԫ����
	#define  BLOCK_TYPE_NAME_SN				"SN"	// �ۼƼĴ���,��ǰֵ,����Ԫ����
	#define  BLOCK_TYPE_NAME_CN				"CN"	// �������͸��ټ�����,��ǰֵ,����Ԫ����
	#define  BLOCK_TYPE_NAME_D				"D"		// ���ݼĴ���,����Ԫ����
	#define  BLOCK_TYPE_NAME_Z				"Z"		// ��ַ�Ĵ���,����Ԫ����
	#define  BLOCK_TYPE_NAME_W				"W"		// ���ӼĴ���,����Ԫ����
	#define  BLOCK_TYPE_NAME_SW				"SW"	// ��������̵���,��Ȧ������Ԫ����
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

map<string,int> g_mapTypeName2Code; // �洢���������Ƶ��ڲ�ͨ�Ŵ���

// ��λΪ��λ����Ԫ����������
int g_arrTypeCodeByBit[] = {BLOCK_TYPE_CODE_X,BLOCK_TYPE_CODE_Y,BLOCK_TYPE_CODE_M,BLOCK_TYPE_CODE_S,BLOCK_TYPE_CODE_L,BLOCK_TYPE_CODE_F,BLOCK_TYPE_CODE_B,BLOCK_TYPE_CODE_V,BLOCK_TYPE_CODE_TS,
BLOCK_TYPE_CODE_TC,BLOCK_TYPE_CODE_CS,BLOCK_TYPE_CODE_CC,BLOCK_TYPE_CODE_SS,BLOCK_TYPE_CODE_SC,BLOCK_TYPE_CODE_SB,BLOCK_TYPE_CODE_S,BLOCK_TYPE_CODE_DX,BLOCK_TYPE_CODE_DY};
// ����Ϊ��λ����Ԫ����������
int g_arrTypeCodeByWord[] = {BLOCK_TYPE_CODE_D,BLOCK_TYPE_CODE_TN,BLOCK_TYPE_CODE_SN,BLOCK_TYPE_CODE_Z,BLOCK_TYPE_CODE_W,BLOCK_TYPE_CODE_SW};

// ��λΪ��λ����Ԫ����������
char g_arrTypeNameByBit[][PK_NAME_MAXLEN] = {BLOCK_TYPE_NAME_X,BLOCK_TYPE_NAME_Y,BLOCK_TYPE_NAME_M,BLOCK_TYPE_NAME_S,BLOCK_TYPE_NAME_L,BLOCK_TYPE_NAME_F,BLOCK_TYPE_NAME_B,BLOCK_TYPE_NAME_V,BLOCK_TYPE_NAME_TS,
BLOCK_TYPE_NAME_TC,BLOCK_TYPE_NAME_CS,BLOCK_TYPE_NAME_CC,BLOCK_TYPE_NAME_SS,BLOCK_TYPE_NAME_SC,BLOCK_TYPE_NAME_SB,BLOCK_TYPE_NAME_S,BLOCK_TYPE_NAME_DX,BLOCK_TYPE_NAME_DY};
// ����Ϊ��λ����Ԫ����������
char g_arrTypeNameByWord[][PK_NAME_MAXLEN] = {BLOCK_TYPE_NAME_D,BLOCK_TYPE_NAME_TN,BLOCK_TYPE_NAME_SN,BLOCK_TYPE_NAME_Z,BLOCK_TYPE_NAME_W,BLOCK_TYPE_NAME_SW};

// �Ĵ������ͣ��ǰ�λ���ǰ���
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

 // �Ƿ���Ҫ�����־λ
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
		 if(pTagGroup->nFailCountRecent > 3)	// ���ʧ�ܴ���
		 {
			 UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, "read failcount:%d", pTagGroup->nFailCountRecent);
			 pTagGroup->nFailCountRecent = 0; // �������̫����ѭ��
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

	 // ��ȡ��������
	 unsigned short uExitCode = 0;
	 memcpy(&uExitCode, pDataBufP, sizeof(unsigned short));
	 if(uExitCode != 0) //���������ݴ���
	 {
		 Drv_LogMessage(PK_LOGLEVEL_ERROR, "[%s][%s]������ʱ, ���ְ�����: %d, �����룺%d, ������������ΪBAD!",
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
	// ѭ�������յ������п������ݰ�
	// 2��ʾվ�ţ�1���ֽڣ��͹����루1���ֽڣ���
	// �κ�����Ӧ���ͷ6���ֽڶ���һ�������壬��7�͵�8���ֽ�Ҳ��һ�������壨վ�ź͹����룩
	long nRecvBytes = lRecvBufLen;
	bool bFoundThisResponse = false;
	char *pCurBuf = (char *)pszRecvBuf;
	while(nRecvBytes > sizeof(PACKET_HEAD))
	{
		PACKET_HEAD packHeader;
		memcpy(&packHeader, pszRecvBuf, sizeof(PACKET_HEAD));
		int nBodyDataLen = packHeader.nBodyDataLen;
		int nTotalPacketLen = nBodyDataLen + sizeof(PACKET_HEAD);
		if(nRecvBytes < nTotalPacketLen) // ���Ȳ���һ����
		{
			nRet = -20;
			UpdateGroupQuality(pDevice, pTagGroup, nRet, "parsing,packlen:%d < packlen of head:%d,discard", nRecvBytes, nTotalPacketLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,group:%s,parsing,packlen:%d < packlen of head:%d,discard", 
				pDevice->szName, pTagGroup->szAutoGroupName, nRecvBytes, nTotalPacketLen);
			break;
		}
		if(nTotalPacketLen > sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "������ʱ, ���ְ�����: %d, �������İ�����:%d, �����豸%s���ν��յ���������!",
				nTotalPacketLen, sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN, pDevice->szName);
			nRet = -10;
			UpdateGroupQuality(pDevice, pTagGroup, nRet, "parsing,packlen:%d,maxpacklen:%d,discard", nTotalPacketLen, sizeof(PACKET_HEAD) + MAX_RESPONSE_PACKDATA_LEN);
			break;
		}

		//char *pThisPackData = pszRecvBuf + sizeof(PACKET_HEAD); // ָ��ǰ�����
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

// ������豸��Ϣ�����õİ�����
// ����1����ʾͨ�����ͣ�������ƣ�E71����ASCII�����ڣ�Э���ʽ1����ASCII�����ڣ�Э���ʽ2��
// TagGroup��nUserData[1]--��Ԫ�����룬[2]--�ǰ�λ���ǰ���
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// ��ȡ�����е�tag�㡣��Ҫ��tag��洢����ƫ�ƣ�λ�������ȣ�λ�����������tag������б��Ա���㣩
	// ��������鴦�������е�tag��������BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = 960; // ����pdf�ĵ��жϣ�ÿ�ο��Զ�ȡ480���ֵĴ�С

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();

	unsigned int i = 0;
	for(; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGrp = vecTagGroup[i];
		// ����ʽ��λ���ǰ��ֵ���Ԫ��
		pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE] = GetCompCode(pTagGrp->szHWBlockName);
		pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD] = GetRegisterLenType(pTagGrp->nUserData[1] );
		if(pTagGrp->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD] == BLOCK_LENTYPE_ID_UNDEFINED)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "��֧�ֵ�����PLC��Ԫ�����ͣ�%s",pTagGrp->szHWBlockName);
			UpdateGroupQuality(pDevice, pTagGrp, -1, "unsupported comptype:%s",pTagGrp->szHWBlockName);
			continue;
		}

		// �����Ŀ�ʼ�Ĵ����͸���������λ���������ֻ���λ�Ĵ�����������������ȡ�����Ծ���ֵ16
		int nRegisterLenBits = 16;
		CalcGroupRegisterInfo(pTagGrp, nRegisterLenBits); // �����ֻ���λ�Ĵ�����������������ȡ�����Ծ���ֵ16

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.pUserData[TIMERINFO_PUSERDATA_INDEX_TAGGROUP] = pTagGrp;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // �趨��ʱ��
	}
	return 0;
}

/*
	����ʼ���豸
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
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	char szTip[PK_NAME_MAXLEN] = {0};
	// ��֯��ȡ��Ϣ
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[TIMERINFO_PUSERDATA_INDEX_TAGGROUP];
	unsigned short uTransID = (unsigned short) ++ pDevice->nUserData[DEVICE_NUSERDATA_INDEX_TRANSID];
	REQ_PACKET reqPack;
	unsigned char nCompCode = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE];
	int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
	if(nBitOrWord == BLOCK_LENTYPE_ID_UNDEFINED){
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "��֧�ֵ�����PLC��Ԫ�����ͣ�%s",pTagGroup->szHWBlockName);
		UpdateGroupQuality(pDevice, pTagGroup, -100, "unspported plc comp type:%s", pTagGroup->szHWBlockName);
		return -1;
	}
	reqPack.nCommand = COMMAND_READ_BULK;
	char *pCurData = reqPack.szReqData;
	// ��Ҫ���㣺��λ�����ݳ��ȡ���������
	// ��������������ʼ��Ԫ����2B������Ԫ�����루1B������Ԫ��������2B��
	//unsigned short nStartRegisterNo = 0;
	//unsigned short nWordNum = 0;
	// Ϊ�β���λȡ����Ϊ��λȡ9��λ�����ݣ�10 10 00 00 00���Ƚ���֣��ƺ���ÿ4λ��ʾ1λ��������ȡ��
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
	StartAddr2Packet(&reqPack, pTagGroup->nBeginRegister); // �Ĵ�����0��ʼ������Ҫ+1
	reqPack.ucCompCode =nCompCode; // ��Ԫ��
	reqPack.uCompNum = pTagGroup->nRegisterNum;// ��Ԫ������
	reqPack.header.nBodyDataLen = (char *)reqPack.szReqData - (char *)&reqPack.nCPUTimer; // �������ĳ���
	reqPack.nTotalPackLen = (char *)reqPack.szReqData - (char *)&reqPack.header.nStartFlag; // �����ܳ���

	// ����ս��ջ�������������ǰ������������û�����ü����ջ����ڻ�������Ӱ�챾������
	if(GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);	

	// ���ɵĶ���Ϣ���ȣ�Ӧ���ǹ̶���12���ֽ�
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

	// ���豸����Ӧ����Ϣ
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
		if(lCurRecvLen >= sizeof(PACKET_HEAD)) // ����Ҫ��1����ͷ�Ĵ�С�����ܵõ�����
		{
			PACKET_HEAD packHead;
			memcpy(&packHead, szResponse, sizeof(PACKET_HEAD));
			if(!IsPacketValidResponse(pDevice, pTagGroup, &packHead)) // ��ͷ�����ԣ���ô�϶��׳�������Ϣ
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)���յ��Ķ����ݿ�(%s)����ͷ����ʽ�Ƿ�(���ȣ�%d)������ţ�%d",
					pDevice->szName, pTagGroup->szAutoGroupName,reqPack.nTotalPackLen, uTransID);
				CheckBlockStatus(pDevice, pTagGroup, -100); // ����״̬
				return -1;
			}

			if (lCurRecvLen >= sizeof(PACKET_HEAD) + packHead.nBodyDataLen)	// ����1�����ĳ��ȣ���ֱ�ӷ���
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
		SetClearRecvBufferFlag(pDevice);	// �����´η���ǰ�����־λ���
	}
	else
	{
		// ����ʧ���������
		CheckBlockStatus(pDevice, pTagGroup, 0);
	}
	
	return lRet;
}

/**
 *  ���п�������ʱ�ú���������.
 *  @param  -[in]  szStrValue������ֵ����blob�ⶼ�Ѿ��Ѿ�ת��Ϊ�ַ�����blobת��Ϊbase64����
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

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@�յ�����������豸(%s)��tag(%s)���п��ƣ���ַ:%s",
		pDevice->szName, szTagName, szAddress);

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	REQ_PACKET reqPack;
	unsigned char nCompCode = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_COMPCODE];
	int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
	if(nCompCode == BLOCK_LENTYPE_ID_UNDEFINED){
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "��֧�ֵ�����PLC��Ԫ�����ͣ�%s",pTagGroup->szHWBlockName);
		sprintf(szTip, "unspported plc comp type:%s", pTagGroup->szHWBlockName);
		//Drv_SetCtrlResult(lCmdId, -100, szTip);
		return -1;
	}

	reqPack.nCommand = COMMAND_WRITE_BULK;
	// ��Ҫ���㣺��λ�����ݳ��ȡ���������
	// д����������ʼ��Ԫ����2B������Ԫ�����루1B������Ԫ��������2B��
	int nStartBits = pTag->nStartBit;// ����ڿ��ڵ���ʼ��ַλ����AI/DI)
	int nEndBits = pTag->nEndBit; // ����ڿ��ڵĽ�����ַλ����AI/DI)
	int nLenBits = nEndBits - nStartBits + 1; // ���ȣ�λ��ʾ��
	int  nRegisterNum = 0; // �Ĵ�����Ŀ
	unsigned short nStartRegisterNo = 0; // ��ʼ�Ĵ�����ַ
	int nBytes2Write = 0;
	// D�飨��Ϊ��λ��Ҳ���԰���λ��д���������ﲻ�����ݿ���������жϣ����Ǳ������ݵ���������ж�
	if(nBitOrWord == BLOCK_LENTYPE_ID_BIT) // ��λ�����Ŀ�
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

	// ��Ҫ���Ƶ���ʼ��ַ��Ԫ������ת��Ϊ��
	StartAddr2Packet(&reqPack, nStartRegisterNo);
	reqPack.ucCompCode =nCompCode;
	reqPack.uCompNum = nRegisterNum;
	reqPack.header.nBodyDataLen = (char *)reqPack.szReqData + nBytes2Write - (char *)&reqPack.nCPUTimer;
	reqPack.nTotalPackLen = (char *)reqPack.szReqData + nBytes2Write - (char *)&reqPack.header.nStartFlag; // �����ܳ���

	// д��Ҫ���Ƶĸ�������
	char *pData = reqPack.szReqData;
	if(nLenBits == 1) //һ��ֻ�������1��λ��һ���Ĵ�����
	{
		/*
		if(nBytes2Write > nBinValueLen)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??���豸(%s)�������ݿ�(%s)��tag:%s�������ݳ���(%d)��ӦΪ����:%d",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, nBinValueLen, nBytes2Write);
			sprintf(szTip, "tag:%s, towrite bytes:%d,supplied bytes:%d too small", pTag->szName, nBytes2Write, nBinValueLen);
			//Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -1;
		}
		*/
		// ����PLC��Yд1�����⣬ÿ���ֽڱ�ʾ����λ�Ŀ��ơ���0x10��ʾ��һ��Y�Ĵ���д1���ڶ���д0.����ֻ����һ�����ƣ���˵ڶ���ʼ�ո�0����
		unsigned char ucValue = (*(unsigned char *)(szBinValue));
		if(ucValue == 0)
			*pData = 0x00;
		else
			*pData = 0x10;
	}
	else
	{
		/*
		if(pTag->nLenBits == 1) // ��D5.0���п��ƣ�Ҳ�������
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??�豸(%s)�������ݿ�(%s)��tag:%s�ĵ�ַ:%s,д���󲻰�ȫ, ��������д�����ݳ���(%d)",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, szAddress, nBinValueLen);
			sprintf(szTip, "addr:BX.Y, do not support control ,tag:%s", pTag->szName);
			Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -1;
		}*/
		if(nBytes2Write > nBinValueLen)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "??�豸(%s)�������ݿ�(%s)��tag:%sд�������ݳ���(%d)��ӦΪ����:%d",
				pDevice->szName, pTagGroup->szAutoGroupName, szTagName, nBinValueLen, nBytes2Write);
			sprintf(szTip, "tag:%s, towrite bytes:%d,supplied bytes:%d too small", pTag->szName, nBytes2Write, nBinValueLen);
			//Drv_SetCtrlResult(lCmdId, -100, szTip);
			return -2;
		}
		memcpy(pData, szBinValue, nBytes2Write);
	}

	// ���ж��Ƿ�Ҫ�����־λ
	if(GetClearRecvBufferFlag(pDevice))
		Drv_ClearRecvBuffer(pDevice);	

	long lSentBytes = Drv_Send(pDevice, (char *)&reqPack, reqPack.nTotalPackLen, 100);
	if(lSentBytes != reqPack.nTotalPackLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)���Ͷ�дtag(%s)����ʧ��(����%d���ֽڣ�ʵ�ʷ���%d��)",
			pDevice->szName, szTagName, reqPack.nTotalPackLen, lSentBytes);
		sprintf(szTip, "tag:%s, send failed,should:%d,actual:%d", pTag->szName, reqPack.nTotalPackLen, lSentBytes);
		//Drv_SetCtrlResult(lCmdId, -100, szTip);
		return -1;
	}
	
	//��¼һ�������ʱ��
	time_t tmRequest;
	time(&tmRequest);

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!����������豸(%s)����дtag(%s)����ɹ���ʵ�ʷ���%d��",
		pDevice->szName, pTag->szName, lSentBytes);

	// ���豸����Ӧ����Ϣ
	long lRet = 0;
	long lCurRecvLen = 0;
	char szResponse[MAX_RESPONSE_BUFLEN] = {0};
	while(true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + lCurRecvLen, sizeof(szResponse) - lCurRecvLen, pDevice->nRecvTimeout);
		if(lRecvBytes <= 0) // û�յ���ֱ����Ϊ���󣬷���
			break;

		//Drv_LogHex(szResponse + lCurRecvLen, lRecvBytes);
		lCurRecvLen += lRecvBytes;
		if(lCurRecvLen >= sizeof(PACKET_HEAD)) // ����Ҫ��1����ͷ�Ĵ�С�����ܵõ�����
		{
			PACKET_HEAD packHead;
			memcpy(&packHead, szResponse, sizeof(PACKET_HEAD));
			if(!IsPacketValidResponse(pDevice, pTagGroup, &packHead)) // ��ͷ�����ԣ���ô�϶��׳�������Ϣ
			{

				Drv_LogMessage(PK_LOGLEVEL_ERROR, "���豸(%s)���յ��Ķ����ݿ�(%s)����ͷ����ʽ�Ƿ�(���ȣ�%d)",
					pDevice->szName, pTagGroup->szAutoGroupName,reqPack.nTotalPackLen);
				sprintf(szTip, "tag:%s, recv %d bytes, header invalid", pTag->szName, lCurRecvLen);
				//Drv_SetCtrlResult(lCmdId, -200, szTip);
				return -1;
			}

			if (lCurRecvLen >= sizeof(PACKET_HEAD) + packHead.nBodyDataLen)	// ����1�����ĳ��ȣ���ֱ�ӷ���
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
		SetClearRecvBufferFlag(pDevice);	// �����´η���ǰ�����־λ���

		sprintf(szTip, "tag:%s, parse package failed,ret:%d", pTag->szName, lRet);
		//Drv_SetCtrlResult(lCmdId, -200, szTip);
	}
	else
	{
		// ����ʧ���������
		CheckBlockStatus(pDevice, pTagGroup, 0);
		sprintf(szTip, "tag:%s, ok", pTag->szName);
		//Drv_SetCtrlResult(lCmdId, PK_SUCCESS, szTip);

	}
	
	return 0;
}
	
#define BLOCK_TYPE_NUM_BYBIT		sizeof(g_arrTypeNameByBit)/sizeof(g_arrTypeNameByBit[0])
#define BLOCK_TYPE_NUM_BYBWORD	 	sizeof(g_arrTypeNameByWord)/sizeof(g_arrTypeNameByWord[0])

// ֧�ֵĵ�ַ��ʽ����λ���Ͱ�����X��Y��M��S��L��F��B��V��CS��CC��TS��TC��SS��SC��SB��S��DX��DY���������Ͱ�����TN��SN��CN��D��Z��W��SW����������g_arrTypeNameByBit��g_arrTypeNameByWord��
// ���룺szAddressInDevice��������ַ���������ͣ�D2025,D2026.2,TN2,TN2.1����λ���ͣ�X1,Y2,TS2
// ���ݵ�ַ��ʽ��D.2025.2,D.2025.2
// ���룺nTagLenBits������tag���ͼ���õ���tagֵλ��
// �����*pnStartBits��*pnEndBits, ����������������飨��AI��DO��D��DB1�������������ĳ��AI�ڵ�ĳ��Group���ڵ���ʼλ������λ��������λ����16����ʼλ0������λ15������λΪ��λ
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameBufLen, int *pnStartBits, int *pnEndBits)
{
	memset(szBlockName, 0, nBlockNameBufLen);
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// ���Ƶ�ַ����ȥ��#�ͣ��š��÷������ܻᱻ���̵߳��ã������þ�̬����
	char szAddress[PK_IOADDR_MAXLEN + 1] = {0};
	char *pDest = szAddress;
	char *pTmp = szAddressInDevice;
	while(*pTmp!='\0')
	{
		if(*pTmp == '#' || *pTmp == ':') // �������Ʒ�: �ͷ�#
			*pDest = '.';
		else
			*pDest = *pTmp;
		pTmp ++;
		pDest ++;
	}

	pTmp = szAddress; // ��ȡ�ַ������׵�ַ

	// �ҵ���һ���ǰ��������ֵ��ַ���֮ǰ�ľ��ǿ���
	pDest = szBlockName;
	while(*pTmp!='\0')
	{
		if(*pTmp >= '0' && *pTmp <= '9' || *pTmp == '.') // �ҵ���һ���ǰ��������ֵ��ַ���֮ǰ�ľ��ǿ���
			break;
		*pDest = *pTmp; // �����ַ�
		pTmp ++;
		pDest ++;
	}
	if(strlen(szBlockName) == 0)//�����һ���ַ��ǰ��������֣���ô����Ϊ��D��
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no block type name! assume :D", szAddressInDevice);
		strcpy(szBlockName, BLOCK_TYPE_NAME_D);
	}
	if(*pTmp=='.') // ����.��
		pTmp ++;

	// �ҵ���������ĵ�ַ��ֱ������������.��
	char szRegisterNo[PK_DATABLOCKTYPE_MAXLEN] = {0};
	pDest = szRegisterNo;
	while(*pTmp!='\0' && *pTmp!='.') // ֻҪ���������Ҳ�����.����ֵ
	{
		*pDest = *pTmp; // �����ַ�
		pTmp ++;
		pDest ++;
	}
	if(strlen(szRegisterNo) == 0)//�����һ���ַ��ǰ��������֣���ô����Ϊ��D��
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, �Ĵ�����ַλ����, �쳣", szAddressInDevice);
		return -1;
	}

	// �ҵ���ַ�����λ�ţ���.��
	if(*pTmp =='.') // ����.���Լ�
		pTmp ++;

	char szBitNo[PK_DATABLOCKTYPE_MAXLEN] = {0};
	pDest = szBitNo;
	while(*pTmp!='\0' && *pTmp!='.') // ֻҪ���������Ҳ�����.����ֵ
	{
		*pDest = *pTmp; // �����ַ�
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
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, ���ݿ�:%s��֧��, �쳣", szAddressInDevice,szBlockName);
		return -2;
	}
	
	// ������ʼ��ַ.����PLC�Ǵӵ�ַ0��ʼ�ģ���D0��X0��Y������-1
	int	 nStartAddr = ::atoi(szRegisterNo);
	if(bBlockByWord) // bit
		*pnStartBits = nStartAddr * 16;
	else
		*pnStartBits = nStartAddr;

	// ������ʼλ�������ֵ�ַ��Ч
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