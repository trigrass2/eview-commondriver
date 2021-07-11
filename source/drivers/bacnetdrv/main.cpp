
#include <ace/ACE.h>
#include "ace/OS.h"
#include "math.h"
#include "AutoGroup_ObjDev.h"
#include "pkcomm/pkcomm.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include <bacdef.h>
#include "datalink.h"
#include "rpm.h"
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

#include "config.h"
#include "bactext.h"
#include "bacerror.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "whois.h"
#define MAX_PROPERTY_VALUES 64

void GetBacAddress(BACNET_ADDRESS * Target_Address, string strDeviceIp, int nDevicePort, int nNetNo, uint32_t Device_Instance);
static void Init_Service_Handlers(void);

void CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess)
 {
	 if(lSuccess == PK_SUCCESS)
		 pTagGroup->nFailCountRecent = 0;
	 else
	 {
		 if(pTagGroup->nFailCountRecent > 3)	// ���ʧ�ܴ���
		 {
			 UpdateGroupQuality(pDevice, pTagGroup, -1, "failcount>3");
			 pTagGroup->nFailCountRecent = 0; // �������̫����ѭ��
		 }
		 else
			 pTagGroup->nFailCountRecent += 1;
	 }
 }

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// ��ȡ�豸ʵ����
	int nDeviceInstanceNo = 0;
	if(!pDevice->szParam1 || strlen(pDevice->szParam1) <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s ����1����������bacnet�豸ʵ���ţ�����bacnet���߲鿴��", pDevice->szName);
	else
		nDeviceInstanceNo = ::atoi(pDevice->szParam1);

	// ��ȡ�豸�����
	int nNetNo = 0;
	if(!pDevice->szParam2 || strlen(pDevice->szParam2) <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s ����2����������bacnet�豸�����ţ�����bacnet���߲鿴��", pDevice->szName);
	else
		nNetNo = ::atoi(pDevice->szParam2);

	// ��ȡ�豸apdu
	int nMaxApdu = 1024;
	if(!pDevice->szParam3 || strlen(pDevice->szParam3) <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s ����3����������bacnet�豸apduֵ������bacnet���߲鿴��", pDevice->szName);
	else
		nMaxApdu = ::atoi(pDevice->szParam3);
	
	//��ȡ�豸��IP
	char nBacnetAddress = NULL;
	if(!pDevice->szConnParam || strlen(pDevice->szConnParam) <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸 %s �����ַ�������������bacnet�豸IP(XXX.XXX.XXX.XXX:47808��ʽ)������bacnet���߲鿴��", pDevice->szName);
	else
	{
		char szConnParam[PK_DESC_MAXLEN] = {0};
		strncpy(szConnParam, pDevice->szConnParam, sizeof(szConnParam) - 1);
		string strDeviceIp = "127.0.0.1";
		char szDeviceIp[PK_NAME_MAXLEN] = {0};
		int nDevicePort = 47808;
		string strConnParam = szConnParam;
		strConnParam +=";";
		// �Ϸ�����ip=127.xx.xx.xx/129.xx.xx.xx.xx/10.2.xx.xx;ip=128.xx.xx.xx/129.xx;ip=.....
		int nPos = strConnParam.find("ip=");
		if(nPos != string::npos)
		{
			strConnParam = strConnParam.substr(nPos + strlen("ip=")); // 127.0.0.1/129.xx;ip=xxx.port=502;
			string strDevicesIP = strConnParam;
			int nPosEnd = strDevicesIP.find(';'); 
			if(nPosEnd != string::npos)
			{
				strConnParam = strDevicesIP.substr(nPosEnd + 1);
				strDeviceIp = strDevicesIP.substr(0, nPosEnd);
			}
			else
				strConnParam = "";
		}

		nPos = strConnParam.find("port=");
		if(nPos != string::npos)
		{
			strConnParam = strConnParam.substr(nPos + strlen("port=")); // 127.0.0.1/129.xx;ip=xxx.port=502;
			string strDevicePort = strConnParam;
			int nPosEnd = strDevicePort.find(';'); 
			if(nPosEnd != string::npos)
			{
				strConnParam = strDevicePort.substr(nPosEnd + 1);
				strDevicePort = strDevicePort.substr(0, nPosEnd);
				nDevicePort = ::atoi(strDevicePort.c_str());
			}
			else
				strDevicePort = "";
		}

		//��IP������Ҫ����Ϊ BACNET_ADDRESS
		BACNET_ADDRESS *pTargetAddess = new BACNET_ADDRESS();
		GetBacAddress(pTargetAddess, strDeviceIp, nDevicePort, nNetNo, nDeviceInstanceNo);
		pDevice->pUserData[0] = (void *) pTargetAddess;
	}

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "�豸 %s bacnet����,ʵ���ţ�%d, ����ţ�%d��apdu��%d", pDevice->szName, nDeviceInstanceNo, nNetNo, nMaxApdu);
	//����IP
	BACNET_ADDRESS *pMyAddess = new BACNET_ADDRESS();
	//GetBacAddress(pMyAddess, "192.168.10.108", 47808, 0, 0);
	pDevice->pUserData[1] = (void *) pMyAddess;

	// ������������ļ����豸���������������Ա�OnTimer��OnControl�з���ʹ��
	// ����0���������
	pDevice->nUserData[1] = nDeviceInstanceNo;
	pDevice->nUserData[2] = nNetNo;
	pDevice->nUserData[3] = nMaxApdu;

	// ��ȡ�����е�tag��

	// ��������鴦�������е�tag��������BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxItemsOfGroup = nMaxApdu; // -20; // modbusÿ���豸��������ͬ��Ӧ������Ϊ�����������ȽϺ��� ???? -20

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();
	// Group: [tag:{ObjectId,propId,propIndex}]

	int i = 0;

	// ���ݵ�ַ���õ�tag��ĸ�������
	// �ο�bacnet�������ԵĶ��壬�����ַ��ʽ�� ObjType:ObjInstId:PropertyName:ArrayIndex  PropertyName������ΪPV��ArrayIndex������Ϊ-1
	// �磺AI:0[:PV][-1]
	for(int iTag = 0; iTag < pDevice->nTagNum; iTag ++)
	{
		PKTAG *pTag = pDevice->ppTags[iTag];
		char *pTmp = NULL;
		char szAddress[1024] = {0};
		strncpy(szAddress, pTag->szAddress, sizeof(szAddress) - 1);
		const char *ptrAddr = ACE_OS::strtok_r(szAddress, ":.#,", &pTmp); // AI:0[:PV][-1]
		char cStartAddrType = '\0';
		int	 nStartAddr = 0;
		char szObjTypeName[PK_NAME_MAXLEN] = {0};
		int nObjIndex = 0;
		char szPropertyName[PK_NAME_MAXLEN] = {0};
		strcpy(szPropertyName, "PV");
		int nArrayIndex = BACNET_ARRAY_ALL;
		int nIndex = 0;
		while(ptrAddr)// AI:0[:PV][-1]
		{
			if(nIndex  == 0) //  AI
				strncpy(szObjTypeName, ptrAddr, sizeof(szObjTypeName) - 1);
			else if(nIndex  == 1) // 0
				nObjIndex = ::atoi(ptrAddr);
			else if(nIndex  == 2) // PV
				strncpy(szPropertyName, ptrAddr, sizeof(szPropertyName) - 1);
			else if(nIndex  == 3) // -1
				nArrayIndex = ::atoi(ptrAddr);

			nIndex ++;
			ptrAddr = ACE_OS::strtok_r(NULL, ":", &pTmp);
		}

		// ������ݱ�����ַ�е����ͣ��õ�bacnet�ı�������
		//����pDevice�е�4����ʾ����ʵ������
		int nObjType = 0;
		if(PKStringHelper::StriCmp(szObjTypeName, "AI") == 0) // AO/BI/AI/BO/BV/AV
			nObjType = OBJECT_ANALOG_INPUT;
		else if(PKStringHelper::StriCmp(szObjTypeName, "AO") == 0)
			nObjType = OBJECT_ANALOG_OUTPUT;
		else if(PKStringHelper::StriCmp(szObjTypeName, "AV") == 0)
			nObjType = OBJECT_ANALOG_VALUE;
		else if(PKStringHelper::StriCmp(szObjTypeName, "BI") == 0)
			nObjType = OBJECT_BINARY_INPUT;
		else if(PKStringHelper::StriCmp(szObjTypeName, "BO") == 0)
			nObjType = OBJECT_BINARY_OUTPUT;
		else if(PKStringHelper::StriCmp(szObjTypeName, "BV") == 0)
			nObjType = OBJECT_BINARY_VALUE;

		//����pDevice�е�6����ʾ����ʵ�����ԡ���
		int nPropId = 0;
		if(PKStringHelper::StriCmp(szPropertyName,"PV") == 0)
		{//85
			nPropId = PROP_PRESENT_VALUE;
		}
		else if(PKStringHelper::StriCmp(szPropertyName,"PA") == 0)
		{//87
			nPropId = PROP_PRIORITY_ARRAY;
		}
		else if (PKStringHelper::StriCmp(szPropertyName,"ID") == 0)
		{//75
			nPropId = PROP_OBJECT_IDENTIFIER;
		}
		else if(PKStringHelper::StriCmp(szPropertyName,"NAME") == 0)
		{//77
			nPropId = PROP_OBJECT_NAME;
		}
		else if(PKStringHelper::StriCmp(szPropertyName,"TYPE") == 0)
		{//79
			nPropId = PROP_OBJECT_TYPE;
		}
		if (nPropId > MAX_BACNET_PROPERTY_ID) {
			Drv_LogMessage(PK_LOGLEVEL_ERROR,  "property=%u - it must be less than %u",nPropId);
			continue;
		}
	
		pTag->nData1 = nObjType;
		pTag->nData2 = nObjIndex;//��ʾ����ʵ������
		pTag->nData3 = nPropId;
		pTag->nData4 = nArrayIndex;//��ʾ����������������
	}

	// ���ɶ������Դ�ȡ�ṹ
	for(i=0; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGrp = vecTagGroup[i];

		vector<BACNET_READ_ACCESS_DATA *> vectorReadAccessData;
		for(int iTag = 0; iTag < pTagGrp->vecTags.size(); iTag ++)
		{
			PKTAG *pTag = pTagGrp->vecTags[iTag];
			BACNET_READ_ACCESS_DATA *pObjectAccess = new BACNET_READ_ACCESS_DATA();
			BACNET_PROPERTY_REFERENCE *pPropertyRef = (BACNET_PROPERTY_REFERENCE *)calloc(1, sizeof(BACNET_PROPERTY_REFERENCE));
			pPropertyRef->next = NULL;
			pPropertyRef->propertyArrayIndex = pTag->nData4;
			pPropertyRef->propertyIdentifier = (BACNET_PROPERTY_ID )pTag->nData3;
			
			pObjectAccess->listOfProperties = pPropertyRef;
			pObjectAccess->object_type = (BACNET_OBJECT_TYPE) pTag->nData1;
			pObjectAccess->object_instance = pTag->nData2;
			vectorReadAccessData.push_back(pObjectAccess);
		}

		// BACNET_READ_ACCESS_DATA:{listOfProperties:[next]}
		// BACNET_READ_ACCESS_DATA������ָ��next����ֵ
		int iReadAccessData = 0;
		for(; iReadAccessData < vectorReadAccessData.size() - 1; iReadAccessData ++)
		{
			BACNET_READ_ACCESS_DATA *pObjectAccess = vectorReadAccessData[iReadAccessData];
			BACNET_READ_ACCESS_DATA *pObjectAccess2 = vectorReadAccessData[iReadAccessData+1];
			pObjectAccess->next = pObjectAccess2;
		}
		if(vectorReadAccessData.size() > 0)
			vectorReadAccessData[vectorReadAccessData.size() - 1]->next = NULL;

		pTagGrp->pParam1 = (void *)vectorReadAccessData[0];
		vectorReadAccessData.clear();
	}

	/* setup my info */
	Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
	address_init();
	Init_Service_Handlers();
	dlenv_init("");
	atexit(datalink_cleanup);

	// ���ö�ʱ��
	for(i=0; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGrp = vecTagGroup[i];
		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGrp->nPollRate;
		timerInfo.nPhaseMS = 0;
		timerInfo.pUserData[0] = pTagGrp;
		Drv_CreateTimer(pDevice,&timerInfo); // �趨��ʱ��
	}

	return 0;
}

/*
	����ʼ���豸
*/

PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	//int nTimerNum = 0;
	//PKTIMER * pTimers = Drv_GetTimers(pDevice, &nTimerNum);
	//int i = 0;
	//for(i = 0; i < nTimerNum; i ++)
	//{
	//	PKTIMER *pTimerInfo = &pTimers[i];
	//	DRVGROUP *pTagGroup = (DRVGROUP *)pTimerInfo->pUserData[0];
	//	delete pTagGroup;
	//	pTagGroup = NULL;
	//	Drv_DestroyTimer(pDevice, pTimerInfo);
	//}
	return 0;
}

/************************************************************************/
/* ����  BACNET_ADDRESS  max_apdu  */
/************************************************************************/

void GetBacAddress(BACNET_ADDRESS * Target_Address, string strDeviceIp, int nDevicePort, int nNetNo, uint32_t Device_Instance){
	char szDeviceIp[PK_DATABLOCKNAME_MAXLEN] = {0};
	strncpy(szDeviceIp, strDeviceIp.c_str(), sizeof(szDeviceIp));
	
	int nIndex = 0;
	char *pTmp = NULL;
	const char *ptrAddr = ACE_OS::strtok_r(szDeviceIp, ".", &pTmp); // 	DB4:X1.1,C:1
	while(ptrAddr && nIndex <= 4)
	{
		int nMac = ::atoi(ptrAddr);
		Target_Address->mac[nIndex] = nMac;
		nIndex ++;
		ptrAddr = ACE_OS::strtok_r(NULL, ".", &pTmp);
	}

	unsigned short uPort = nDevicePort;
	Target_Address->mac[4] = (uPort & 0xFF00) >> 8;
	Target_Address->mac[5] = uPort & 0xFF;

	Target_Address->mac_len = 6;
	Target_Address->len = 6;
	Target_Address->net = nNetNo;
	Target_Address->adr[0] = Device_Instance;
}

/**
 *  �趨�����ݿ鶨ʱ���ڵ���ʱ�ú���������.
 *  
 *  �����ڸú��������豸���������ȡʵʱ���ݻ����豸���Ϳ���ָ��.
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];
	BACNET_ADDRESS *dest =(BACNET_ADDRESS *) pDevice->pUserData[0];
	int nAPDU = pDevice->nUserData[3];
	 uint8_t szRecvBuf[MAX_MPDU] = { 0 };
	 uint16_t pdu_len = 0;
	
	// ����ս��ջ�������������ǰ������������û�����ü����ջ����ڻ�������Ӱ�챾������

	// ����pTagGroup����Ϣ����֯Ϊuint8_t Send_Read_Property_Multiple_Request
	uint8_t buffer[MAX_PDU] = { 0 };
	BACNET_READ_ACCESS_DATA *pReadAccess = (BACNET_READ_ACCESS_DATA *)pTagGroup->pParam1;
	//char *argv[] ={"0","1","1","85,87[1],87"};
	int  nDeviceObjId = pDevice->nUserData[1];//�豸id
	int uTransID = Send_Read_Property_Multiple_Request(buffer, sizeof(buffer), nDeviceObjId, pReadAccess,pDevice);
	pDevice->nUserData[0] = uTransID;

	/* returns 0 bytes on timeout */
	BACNET_ADDRESS *pBacDeviceAddr = (BACNET_ADDRESS *)pDevice->pUserData[0];
	pdu_len = datalink_receive(pBacDeviceAddr, szRecvBuf, MAX_MPDU, 1000, pDevice);

	/* process */
	if (pdu_len) {
		npdu_handler(pBacDeviceAddr, szRecvBuf, pdu_len, pTagGroup, (uint8_t)uTransID);
	}

	UpdateGroupData(pDevice, pTagGroup);
	// �޸�blvc.c�е�bvlc_send_mpdu

	int res = 0;
	if(0==res) 
	{
		//UpdateBlockData(pDevice, pTagGroup, (char *)dc->resultPointer,dc->AnswLen, 0);
		CheckBlockStatus(pDevice, pTagGroup, 0);
		return 0;
	}
	else 
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "failed to read data block,areatype:%s.", pTagGroup->szAutoGroupName);  
		CheckBlockStatus(pDevice, pTagGroup, -1);
		return -1;
	};
}

/* used to load the app data struct with the proper data
   converted from a command line argument */
bool TagData2BacnetData(int nTagDataType, char *szTagVal, int nTagDataLen, BACNET_APPLICATION_DATA_VALUE * value)
{
	bool status = true;
	//��Ӧ���ʹ���
	switch(nTagDataType)
	{
	case TAG_DT_BOOL:
		value->tag = BACNET_APPLICATION_TAG_BOOLEAN;
		{
			uint8_t nBoolVal = 0;
			memcpy(&nBoolVal, szTagVal, sizeof(uint8_t));
			value->type.Boolean = (nBoolVal != 0?true:false);
		}
		
		break;
	case TAG_DT_UINT32:
		value->tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
		memcpy(&value->type.Unsigned_Int,szTagVal, sizeof(unsigned int));
		break;
	case TAG_DT_INT32:
		value->tag = BACNET_APPLICATION_TAG_SIGNED_INT;
		memcpy(&value->type.Signed_Int,szTagVal, sizeof(int));
		break;
	case TAG_DT_FLOAT:
		value->tag = BACNET_APPLICATION_TAG_REAL;
		memcpy(&value->type.Real,szTagVal, sizeof(float));
		break;
	case TAG_DT_DOUBLE:
		value->tag = BACNET_APPLICATION_TAG_DOUBLE;
		memcpy(&value->type.Double,szTagVal, sizeof(double));
		break;
	default:
		return false;
		break;
	}

    value->next = NULL;

    return status;
}

/**
 *  ���п�������ʱ�ú���������.
 *  �ڸú����и��ݴ��ݵĲ��������豸�·������������豸�п��Ʒ������������ͬ�����첽��ʽ���տ��Ʒ�����
 */
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{
	uint8_t buffer[MAX_PDU] = { 0 };
	uint8_t szRecvBuf[MAX_MPDU] = { 0 };
	uint16_t pdu_len = 0;
	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;
	bool status = true;

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;;
    int  nDeviceObjId = pDevice->nUserData[1];//�豸id
  
  /* 0 if not set, 1..16 if set */
    uint8_t Property_Priority = 9;//���ȼ�
    BACNET_OBJECT_TYPE Target_Object_Type =(BACNET_OBJECT_TYPE) pTag->nData1;//��������
    uint32_t Target_Object_Instance = pTag->nData2;//����ʵ����
    BACNET_PROPERTY_ID Target_Object_Property =(BACNET_PROPERTY_ID) pTag->nData3;//�������Ժ�
	/* array index value or BACNET_ARRAY_ALL */
	int32_t Target_Object_Property_Index  = pTag->nData4;//������������
   if (Target_Object_Property_Index == -1)
        Target_Object_Property_Index = BACNET_ARRAY_ALL;
    if (nDeviceObjId > BACNET_MAX_INSTANCE) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device-instance=%u - it must be less than %u\r\n",nDeviceObjId, BACNET_MAX_INSTANCE + 1);
        return -1;
    }
    if (Target_Object_Type > MAX_BACNET_OBJECT_TYPE) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "object-type=%u - it must be less than %u\r\n",Target_Object_Type, MAX_BACNET_OBJECT_TYPE + 1);
        return -1;
	}
	if (Target_Object_Instance > BACNET_MAX_INSTANCE) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "object-instance=%u - it must be less than %u\r\n",Target_Object_Instance, BACNET_MAX_INSTANCE + 1);
        return -1;
    }
    if (Target_Object_Property > MAX_BACNET_PROPERTY_ID) {
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "property=%u - it must be less than %u\r\n",Target_Object_Property, MAX_BACNET_PROPERTY_ID + 1);
		return -1;
    }
	/*д������Ҫ��tag-value*/
	BACNET_APPLICATION_DATA_VALUE Target_Object_Property_Value[1];
	Target_Object_Property_Value[0].next = NULL;
	bool bStatus = TagData2BacnetData(pTag->nDataType, szBinValue, nBinValueLen, &Target_Object_Property_Value[0]);
    if(bStatus ==  false)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "TagData2BacnetData failed");
		return -1;
	}  
	/*����д��������*/
	int  uTransID = Send_Write_Property_Request(nDeviceObjId,Target_Object_Type, Target_Object_Instance,Target_Object_Property, &Target_Object_Property_Value[0],Property_Priority,Target_Object_Property_Index,pDevice,buffer);
	/* returns 0 bytes on timeout */
	BACNET_ADDRESS *pBacDeviceAddr = (BACNET_ADDRESS *)pDevice->pUserData[0];//�豸��ַ
	pdu_len = datalink_receive(pBacDeviceAddr, szRecvBuf, MAX_MPDU,1000,pDevice);

	/* process */
	if (pdu_len) {
		npdu_handler(pBacDeviceAddr, szRecvBuf, pdu_len, pTagGroup, (uint8_t)uTransID);
	}


	char szRequestBuff[1024];
	memset(szRequestBuff,0, sizeof(szRequestBuff));
	long nStatus = PK_SUCCESS;
	int  i = 0;

	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@�յ�����������豸(%s)��tag(%s)���п��ƣ���ַ:%s",
		pDevice->szName, szTagName, szAddress);

	int nRequestBufLen = nBinValueLen;

	return 0;
}

static void MyErrorHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    if (invoke_id >0){
        printf("BACnet Error: %s: %s\r\n",
            bactext_error_class_name((int) error_class),
            bactext_error_code_name((int) error_code));
        //Error_Detected = true;
    }
}

void MyAbortHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    (void) server;
    if (invoke_id >0) {
        printf("BACnet Abort: %s\r\n",
            bactext_abort_reason_name((int) abort_reason));
        //Error_Detected = true;
    }
}

void MyRejectHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    /* FIXME: verify src and invoke id */
    if(invoke_id >0) {
        printf("BACnet Reject: %s\r\n",
            bactext_reject_reason_name((int) reject_reason));
        //Error_Detected = true;
    }
}

void assignVals2Group(BACNET_READ_ACCESS_DATA * rpm_data, void *pvTagGroup)
{
	BACNET_PROPERTY_REFERENCE *listOfProperties;
	BACNET_APPLICATION_DATA_VALUE *value;
	bool array_value = false;

	DRVGROUP *pTagGroup = (DRVGROUP *)pvTagGroup;
	vector<PKTAG *> & vecTag = pTagGroup->vecTags;
	int iTag = 0;
	while(rpm_data) {
		PKTAG *pTag = vecTag[iTag];
		iTag ++;
		// �Ƚ�TAG������Ժ�rpm�����Ƿ�����ͬ��
		if(pTag->nData1 != (int)rpm_data->object_type)
			return ;
		if(pTag->nData2 != (int)rpm_data->object_instance)
			return ;

		listOfProperties = rpm_data->listOfProperties;
		if(listOfProperties == NULL)
		{
			rpm_data = rpm_data->next;
			continue;
		}

		if(pTag->nData3 != (int)listOfProperties->propertyIdentifier)
			return ;

		value = listOfProperties->value;
		if(!value){
			rpm_data = rpm_data->next;
			continue;
		}

		switch(value->tag)
		{
		case BACNET_APPLICATION_TAG_BOOLEAN:
			if(pTag->nDataType != TAG_DT_BOOL)
				break;
			{
				uint8_t ret_val = (value->type.Boolean) ?1:0;
				Drv_SetTagData_Text(pTag, ret_val != 0 ? "1" : "0");
			}
			break;
		case BACNET_APPLICATION_TAG_UNSIGNED_INT:
			if(pTag->nDataType != TAG_DT_UINT32)
				break;
			Drv_SetTagData_Binary(pTag, (void *)&value->type.Unsigned_Int, 4);
			break;
		case BACNET_APPLICATION_TAG_SIGNED_INT:
			if(pTag->nDataType != TAG_DT_INT32)
				break;
			Drv_SetTagData_Binary(pTag, (void *)&value->type.Signed_Int, 4);
			break;
		case BACNET_APPLICATION_TAG_REAL:
			if(pTag->nDataType != TAG_DT_FLOAT)
				break;
			Drv_SetTagData_Binary(pTag, (void *)&value->type.Real, 4);
			break;
		case BACNET_APPLICATION_TAG_DOUBLE:
			if(pTag->nDataType != TAG_DT_DOUBLE)
				break;
			Drv_SetTagData_Binary(pTag, (void *)&value->type.Double, 8);
			break;
		}
		rpm_data = rpm_data->next;
	}
}

/** Handler for a ReadPropertyMultiple ACK.
 * @ingroup DSRPM
 * For each read property, print out the ACK'd data,
 * and free the request data items from linked property list.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void My_Read_Property_Multiple_Ack_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
	BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data,
	void *pvTagGroup,
	int32_t uTransId
	)
{
    int len = 0;
    BACNET_READ_ACCESS_DATA *rpm_data;
    BACNET_READ_ACCESS_DATA *old_rpm_data;
    BACNET_PROPERTY_REFERENCE *rpm_property;
    BACNET_PROPERTY_REFERENCE *old_rpm_property;
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_APPLICATION_DATA_VALUE *old_value;

    if (service_data->invoke_id >0) {
        rpm_data = (BACNET_READ_ACCESS_DATA *)calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
        if (rpm_data) {
            len =
                rpm_ack_decode_service_request(service_request, service_len,
                rpm_data);
        }
        if (len > 0) {

			assignVals2Group(rpm_data,pvTagGroup);

            while (rpm_data) {
                rpm_property = rpm_data->listOfProperties;
                while (rpm_property) {
                    value = rpm_property->value;
                    while (value) {
                        old_value = value;
                        value = value->next;
                        free(old_value);
                    }
                    old_rpm_property = rpm_property;
                    rpm_property = rpm_property->next;
                    free(old_rpm_property);
                }
                old_rpm_data = rpm_data;
                rpm_data = rpm_data->next;
                free(old_rpm_data);
            }

        } else {
            fprintf(stderr, "RPM Ack Malformed! Freeing memory...\n");
            while (rpm_data) {
                rpm_property = rpm_data->listOfProperties;
                while (rpm_property) {
                    value = rpm_property->value;
                    while (value) {
                        old_value = value;
                        value = value->next;
                        free(old_value);
                    }
                    old_rpm_property = rpm_property;
                    rpm_property = rpm_property->next;
                    free(old_rpm_property);
                }
                old_rpm_data = rpm_data;
                rpm_data = rpm_data->next;
                free(old_rpm_data);
            }
        }
    }
}

static void Init_Service_Handlers(void)
{

    Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    /* handle the data coming back from confirmed requests */
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        My_Read_Property_Multiple_Ack_Handler);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}


