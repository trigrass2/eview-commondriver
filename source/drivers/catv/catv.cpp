
#include "math.h"
#include <memory.h>
//#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>

#include "driversdk/pkdrvcmn.h"
#include "common/pkGlobalHelper.h"
///#include "crc.h"
#include <map>
#include "list"
#include "json\reader.h"
#include "json\value.h"

//typedef __time32_t time_t;
#include"KedaPlat\kdm_mcusdk.h"
#include "KedaPlat/kdvtype.h"
#include "KedaPlat/McuSdkType.h"
#include "KedaPlat/hashIdMap.h"
#include "afxtempl.h"
#include"io.h"

#pragma  comment(lib,"jsoncpp")
#pragma comment(lib,"mcusdk")
//#include "DVRFunc.h"
using namespace std;
using namespace Json;
typedef unsigned short      WORD;
std::vector<DEVICEINFO>			m_vcAllDevInfo;//�洢���ĵ�����ͷ

PKDEVICE *_PDevice;

//int SDK_Envent_CallBack(EVENTINFO *pEventInfo, u32 lUserData);
typedef void (*pFuncSetSDKErrInfo)(bool bSDK, long lErrCode, const char* szErrMeg, long lSDKErrCode, const char* szSDKErrMeg);
//DEVICEINFO *GetDevInfoByDeviceId(CString strDeviceId);

enum QueryVideoResult
{
	eStartQuery = 0,
	eQureyFailed = 1,
	eQureySucc = 2,
};
struct tagQueryVideoResult
{
	QueryVideoResult m_eQueryResult;
	u32				 m_errorCode;

	tagQueryVideoResult()
	{
		m_eQueryResult = eStartQuery;
		m_errorCode = 0;
	}
};
typedef struct _CYCLECAMINFO
{
	map<string,string> *pCamMap;	//�洢��ǰ��cam�б�;
	int camIndex;		//cam���;
	time_t cycStartTime;	//��ѯ��ʼʱ��;
	time_t startTime;	//������ʼʱ��;
	time_t curTime;		//��ǰʱ��;
	int cycTime;		//����ʱ��;
	int privilege;		//Ȩ��;
	int TVWallHoldTime;	//��ռʱ��;
	char modeType[32];
	_CYCLECAMINFO(){
		//pCamList=NULL;
		cycTime=10;
		TVWallHoldTime=privilege=0;
	}
}CYCLECAMINFO,*pCYCLECAMINTO;


static CMap<long, long, long, long> CHashIdMap;
static CMap<long, long, long, long> m_mapDownloadNo2DownloadedSize; //���ر�ŵ��Ѿ����سߴ�Ĺ�ϣ��;
static CMap<long, long, long, long> m_mapDownloadNo2TotalSize;		 //���ر�;

std::vector<TTvWallInfoData>	m_tTVWallInfoList;					//����ǽ�б�;
TTvWallPlayData					m_tWallPlaydata;			//
pFuncSetSDKErrInfo m_pSetSDKErrInfoCallback;
map<string,DEVICEINFO>	m_mapDevId2Device;		 //���豸IDӳ�䵽�豸��Ϣ��map
map<int,DEVCHN>			m_mapPlayId2DevChn;		// LensControl��Ҫ����playid�ҵ�DevChn

static map<string,CYCLECAMINFO> m_mapCycleWithPrivilege;	//����һ������Ȩ�޵Ĵ洢��ѯ��map;string��ʾ����ǽ

map<string,CYCLECAMINFO>::iterator itMap=m_mapCycleWithPrivilege.begin();
map<string,string>::iterator itCamMap;

map<string,string> arrCamList1,arrCamList2,arrCamList3,arrCamList4,arrCamList5,arrCamList6,arrCamList7,arrCamList8,arrCamList9,arrCamList10,arrCamList11,arrCamList12,
	arrCamList13,arrCamList14,arrCamList15,arrCamList16,arrCamList17,arrCamList18,arrCamList19,arrCamList20,arrCamList21,arrCamList22,arrCamList23,arrCamList0;
tagQueryVideoResult		m_tagQueryResult;
void * m_pMcuSdkHandle = NULL;
long loginID;
bool bConnected=false;

int IDOfPlayTV=0;		//��ǽ���ӵ�ID
int nTVWALLTOTALNUM = 24;//����ǽ����;
int TVWallHoldTime = 30	;//����ǽ��ռʱ��;
int cycTime=3;


//�滻�ַ���
void Repalce_All(std::string& str, std::string& strOld, std::string& strNew)
{
	std::string::size_type pos(0);
	while(pos != std::string::npos)
	{
		pos = str.find(strOld, pos);
		if(pos != std::string::npos)
		{
			str.replace(pos, strOld.length(), strNew);
			pos += strOld.length();
		}
		else
			break;
	}
}
CString GetNextToken(CString &strSrc, TCHAR nSep)
{
	long nSlash = strSrc.Find(nSep);

	CString strToken;
	if (nSlash < 0)
	{
		strToken = strSrc;
		strSrc.Empty();
	}
	else
	{
		strToken = strSrc.Left(nSlash);
		strSrc = strSrc.Right(strSrc.GetLength() - nSlash - 1);
	}

	return strToken;
}
//����ͷת����list��ʽ;
int str2List(Json::Value root,map<string,string> *camMap)
{
	if(root.isNull())
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"cameras�ֶ�û���յ���Ҫ��ѯ������ͷ:");
		return -1;
	}
	camMap->clear();
	
	//json����ת��,���浽list��;
	for (int i=0;i<root["cameras"].size();i++)
	{
		Json::Value camPairs=root["cameras"][i];
		string id=camPairs["id"].asString();
		string name=camPairs["name"].asString();
		camMap->insert(map<string,string>::value_type(name,id));
	}

	return 0;
}

/*
tvId: tvId,						����ID
modeName: modeName,				ģʽ
modeInterval : modeInterval,	ʱ����
modeDescription: modeDescription,
cameras: {id,name}				����ͷ
operateTimeout:prilivage			��ռʱ��
operatePriority:					Ȩ��
*/
int parseCycleCam(char* szBinValue,pCYCLECAMINTO pcycle)
{
	time_t timeNow;
	time(&timeNow);
	//�������json��ʽ;
	Json::Reader reader;
	Json::Value root;
	
	if (reader.parse(szBinValue, root))
	{
		if(!root.isObject())
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"�յ���ʽ����ȷ:%s",szBinValue);
			return -1;
		}
		//��ռʱ��;
		if(root.isMember("operateTimeout"))
			pcycle->TVWallHoldTime=atoi((root["operateTimeout"].asString().c_str()));
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û���յ�userHoldTime�ֶ�,��ʾû�а�ռʱ��,������Ĭ�ϵģ�%d��",TVWallHoldTime);
			pcycle->TVWallHoldTime=TVWallHoldTime;
		}
		//�û�Ȩ��;
		if(root.isMember("operatePriority"))
			pcycle->privilege=atoi((root["operatePriority"].asString().c_str()));
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û���յ�userPrivilege�ֶ�,��ʾû��Ȩ���ֶ�,������Ĭ�ϵģ�5");
			pcycle->privilege=5;
		}
		//ʱ����������
		if(root.isMember("modeInterval"))
			pcycle->cycTime=atoi((root["modeInterval"].asString().c_str()));
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û���յ�modeInterval�ֶ�,��ʾû��ʱ�������ֶ�,������Ĭ�ϵ�:%d",cycTime);
			pcycle->cycTime=cycTime;
		}
		//ģʽ
		if(root.isMember("modeName"))
			strcpy(pcycle->modeType,root["modeName"].asString().data());
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û���յ�modeName�ֶ�,��ʾû��ģʽ�ֶ�,������Ĭ���ֶ�");
			memcpy(pcycle->modeType,"Ĭ��",sizeof("Ĭ��"));
			pcycle->modeType[strlen("Ĭ��")]='\0';
		}
		//����ͷmap
		if(root.isMember("cameras"))
		{	
			if(str2List(root,pcycle->pCamMap)<0)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR,"�յ�����Ϣ������:%s",szBinValue);
			}
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û��modeLayout�ֶΣ���ʾû��ѡ����Ҫ��ѯ������ͷ:%s",szBinValue);
			return -2;
		}
		//map_cam->pCamList=NULL;
		//������յ�ʱ��;
		pcycle->startTime=pcycle->cycStartTime=timeNow;
		pcycle->curTime=timeNow;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"���ܵ���ѯ�������ݸ�ʽ��json������:%s",szBinValue);
		return -1;
	}
	return 0;
}


//���ƾ�ͷ��������;
long VideoPanControl(char *szname,long nPlayID, long nControlCode,long nSpeed, long lElpaseTime,DEVCHN tDevChn)
{
	if ((nPlayID < 0) || (nControlCode < 0) || (nSpeed < 0) )
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"can't control camera\n");
		return -1;
	}

	PTZCMD tPtzCmd;
	tPtzCmd.m_wPtzRange = nSpeed  ; // 0...5-->0...15
	DWORD dwCommand = 0;
	bool isStop = (nSpeed == 0)?true:false; // �Ƿ���Ҫֹͣ������������ȣ�
	tPtzCmd.m_emPtzCmd = emMoveStop;
	u32 errCode = 0;
	BOOL bCtrlRet = Kdm_SendPtzControl(m_pMcuSdkHandle, tDevChn, tPtzCmd, &errCode);
	char ctrlMsg[128]={0};

	//������ת������icv�Ŀ����붨��ת��Ϊ�󻪵Ŀ����� 
	switch (nControlCode)
	{
	case 16416:		//��
		if(!isStop)
		{
			tPtzCmd.m_emPtzCmd = emMoveLeft;
			sprintf(ctrlMsg,"��ת");
		}
		else
			tPtzCmd.m_emPtzCmd = emMoveStop;
		break;
	case 16400:		//��
		if(!isStop)
		{
			tPtzCmd.m_emPtzCmd = emMoveRight;
			sprintf(ctrlMsg,"��ת");
		}
		else
			tPtzCmd.m_emPtzCmd = emMoveStop;
		break;
	case 16512:		//��
		if(!isStop)
		{
			tPtzCmd.m_emPtzCmd = emMoveUp;
			sprintf(ctrlMsg,"��ת");
		}
		else
			tPtzCmd.m_emPtzCmd = emMoveStop;
		break;
	case 16448:		//��
		if(!isStop)	
		{
			tPtzCmd.m_emPtzCmd = emMoveDown;
			sprintf(ctrlMsg,"��ת");
		}
		else
			tPtzCmd.m_emPtzCmd = emMoveStop;
		break;
	case 16384://ֹͣ
		return 0;
	default:
		break;
	}
	//��playid,�Լ�loginid��chanNum��playid�Ķ�Ӧ��ϵ����loginid��chanNum
	long nLoginID = 0;
	long nChanNum = 0;

	long hashidX = GetHashOfLoginIdFormPlayId(nPlayID);
	CHashIdMap.Lookup(hashidX,nLoginID);

	hashidX = GetHashOfChanNumFormPlayId(nPlayID);
	CHashIdMap.Lookup(hashidX,nChanNum);

	bCtrlRet = Kdm_SendPtzControl(m_pMcuSdkHandle, tDevChn, tPtzCmd, &errCode);

	if (!bCtrlRet)//ǿ������ת�� hm;
	{
		//strTemp.LoadString(IDS_STRING257);
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���Ϳ���ʧ��:%d",nControlCode);
		return -2L;
	}
	//strTemp.LoadString(IDS_STRING259);
	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "�ɹ����Ϳ���:%s:%s\n",szname,ctrlMsg);
	return 0;
}


//��ͷ���ƣ�Զ��������;
long VideoLensControl(char *szname,long nPlayID, long nControlCode, long nSpeed, long lElpaseTime,DEVCHN tdev)
{
	//CString strTemp;
	/*strTemp.LoadString(IDS_STRING243);
	CVLog.LogMessage(LOG_LEVEL_DEBUG, strTemp.GetBuffer(0));*/

	if ((nPlayID == 0) || (nControlCode < 0) || (nSpeed < 0) )
	{
		/*strTemp.LoadString(IDS_STRING107);
		m_pSetSDKErrInfoCallback(false, EC_ICV_CCTV_FUNCPARAMINVALID, strTemp.GetBuffer(0), 0, "");*/
		return -1;
	}

	PTZCMD tPtzCmd;
	tPtzCmd.m_wPtzRange = nSpeed  * 3; // 0...5-->0...15
	unsigned long dwCommand = 0;
	bool isStop = (nSpeed == 0)?true:false; // �Ƿ���Ҫֹͣ������������ȣ�
	tPtzCmd.m_emPtzCmd = emZoomStop;
	char ctrlMsg[128]={0};
	switch (nControlCode)
	{
	case 1:
		if(isStop)
			tPtzCmd.m_emPtzCmd = emZoomStop;
		else
			tPtzCmd.m_emPtzCmd = emZoomIn;
		break;

	case 24576:	//Զ��
		if(isStop)
		{
			tPtzCmd.m_emPtzCmd = emZoomStop;
			sprintf(ctrlMsg,"Զ��");
		}
		else
			tPtzCmd.m_emPtzCmd = emZoomOut;
		break;

	case 5:
		if(isStop)
			tPtzCmd.m_emPtzCmd = emZoomStop;
		else
			tPtzCmd.m_emPtzCmd = emZoomIn;
		break;

	case 6:
		if(isStop)
			tPtzCmd.m_emPtzCmd = emZoomStop;
		else
			tPtzCmd.m_emPtzCmd = emZoomOut;
		break;

		//�޸ĺδ�
	case 3:
		break;

	case 20480:		//����
		if(isStop)
		{
			tPtzCmd.m_emPtzCmd = emZoomStop;
			sprintf(ctrlMsg,"����");
		}
		else
			tPtzCmd.m_emPtzCmd = emZoomIn;
		break;
		break;
	case 16384:
		return 0;
	default:
		break;
	}

	//��playid,�Լ�loginid��chanNum��playid�Ķ�Ӧ��ϵ����loginid��chanNum
	long nLoginID = 0;
	long nChanNum = 0;

	long hashidX = GetHashOfLoginIdFormPlayId(nPlayID);
	CHashIdMap.Lookup(hashidX,nLoginID);

	hashidX = GetHashOfChanNumFormPlayId(nPlayID);
	CHashIdMap.Lookup(hashidX,nChanNum);

	u32 errCode = 0;
	BOOL32 bRtn = Kdm_SendPtzControl(m_pMcuSdkHandle, tdev, tPtzCmd, &errCode);

	if (!bRtn)//ǿ������ת�� hm;
	{
		char* strTemp="����ʧ��";
		Drv_LogMessage(PK_LOGLEVEL_ERROR, strTemp, nPlayID, nLoginID, nChanNum, errCode);
		return -3;
	}
	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "control  zoom success:%s:%s ",szname,ctrlMsg);

	return 0;
}

int SubscriptDeviceStatus()
{
	map<string,DEVICEINFO>::iterator it;
	it=m_mapDevId2Device.begin();
	while(it!=m_mapDevId2Device.end())
	{
		TSUBSDEVS temSubDevs;
		temSubDevs.AddDevice(it->second.domainID);
		Kdm_UnSubscriptDeviceStatus(m_pMcuSdkHandle,temSubDevs,(ESubscriptInfo)13,NULL);
		it++;
	}
	m_vcAllDevInfo.clear();
	DEVICEINFO tDevice;
	memset(&tDevice, 0, sizeof(DEVICEINFO));
	u32 dwDevErrCode = 0;
	//u32 dwDevTaskId = Kdm_GetDeviceByGroup(m_pMcuSdkHandle, m_GroupInfo[i].groupID, &dwCode);
	it=m_mapDevId2Device.begin();
	while(it!=m_mapDevId2Device.end())
	{
		u32 dwsubErrCode = 0;
		TSUBSDEVS temSubDevs;
		temSubDevs.AddDevice(it->second.deviceID);
		DeviceID deviceId;
		sprintf(deviceId.szID,"%s",it->second.deviceID.szID);
		temSubDevs.m_vctDevID[0]=deviceId;
		Kdm_SubscriptDeviceStatus(m_pMcuSdkHandle,temSubDevs,(ESubscriptInfo)13,&dwsubErrCode);
		it++;
	}
	Drv_LogMessage(PK_LOGLEVEL_INFO,"������%d���豸",m_mapDevId2Device.size());
	return 0;
}
//SDK 
int SDK_Envent_CallBack(EVENTINFO *pEventInfo, u32 lUserData)
{
	if (pEventInfo->m_emWork == emStartPlayRec)
	{
		time_t tNow;
		time(&tNow);
		tm *p=gmtime(&tNow);
		char szShowTime[100] = {0};

		sprintf(szShowTime, "current time:%d-%d-%d %d:%d:%d ",
			p->tm_yday,p->tm_mon,p->tm_yday,p->tm_hour,p->tm_min,p->tm_sec);

		Drv_LogMessage(PK_LOGLEVEL_ERROR,szShowTime);
	}
	else if(pEventInfo->m_emWork==emHeartbeatError)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"�����������ӳ���,��������");
		bConnected=false;
		return 2;
	}
	Drv_LogMessage(PK_LOGLEVEL_ERROR,"SDK Err callback");
	return 1;
}
//�豸״̬�ص����ϴ��豸״̬;
int device_states_callback(DeviceID devID, DeviceStatus *pDevStatus, u32 UDevice)//�û�����
{
	//stringstream ss;
	Drv_LogMessage(PK_LOGLEVEL_INFO,"���ڻ�ȡ�������״̬");
	char addr[128]={0};
	memcpy(addr,devID.szID,strlen(devID.szID)-8);
	strcat(addr,"_status");
	vector<PKTAG *> vecCarNo;
	char strOnLine[32]={0};
	//Drv_GetTagsByAddr(_PDevice, addr, vecCarNo);
	for (int i=0;i<_PDevice->vecTags.size();i++)
	{
		PKTAG *ptag=_PDevice->vecTags[i];
		if(!strcmp(ptag->szAddress,addr))
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO,"����������һ���㣬name=%s",ptag->szName);
			vecCarNo.push_back(ptag);
			break;
		}
	}
	if(vecCarNo.empty())
		return 0;
	if(pDevStatus->m_emStatusType == emDevOnline)
	{
		//�����״̬;
		sprintf(strOnLine,"%ld",pDevStatus->m_bOnline);
		Drv_SetTagData_Text(vecCarNo[0], strOnLine, 0, 0, 0);
		Drv_UpdateTagsData(_PDevice, vecCarNo.data(), vecCarNo.size());
		vecCarNo.clear();
		Drv_LogMessage(PK_LOGLEVEL_INFO,"��ȡ�������״̬=%d,�ɹ�",atoi(strOnLine));
	}
	return 1;
}

long QueryAllDevicesByGroup(GroupID groupID)
{
	u32 dwCode = 0;
	u32 dwDevTaskId = Kdm_GetDeviceByGroup(m_pMcuSdkHandle, groupID, &dwCode);
	if (0 != dwCode)
	{
		return 0;
	}

	DEVICEINFO tDevice;
	memset(&tDevice, 0, sizeof(DEVICEINFO));
	u32 dwDevErrCode = 0;
	int nDevCount = 0;

	while (Kdm_GetDeviceNext(m_pMcuSdkHandle, dwDevTaskId, &tDevice, &dwDevErrCode))
	{
		if (tDevice.nDevType == DEVICEINFO::emTypeTVWall) // ֻ���ı����豸������ƽ̨��������Ϊ�����豸;
		{
			continue;
		}
		nDevCount ++;
		strcpy(tDevice.szParentGroupID.szID,groupID.szID);
		//char* strDevId = tDevice.deviceID.szID; // ��Ҫȥ��@kedacom;
		//for (char* p=strDevId;p;p++)
		//{
		//	if ((*p)=='@')
		//	{
		//		*p='\0';
		//		break;
		//	}
		//}
		//m_mapDevId2Device[strDevId] = tDevice;
		CString strDevId = tDevice.deviceID.szID; // ��Ҫȥ��@kedacom
		if(strDevId.Find("@") >= 0)
			strDevId = strDevId.Left(strDevId.Find("@"));
		m_mapDevId2Device[strDevId.GetBuffer(0)] = tDevice;
		char strManFac[128]={0};
		sprintf(strManFac,"*************** Manufacturer = %s nDevCount = %d \n" ,tDevice.szManufacturer,nDevCount);
		Drv_LogMessage(PK_LOGLEVEL_INFO, strManFac);
		memset(&tDevice, 0, sizeof(DEVICEINFO));
	}
	return 0;
}

// ��ѯ���е��豸;
long QueryAllSubGroups(GroupID groupID)
{
	u32 errorInfo = 0;
	u32 dwRootTaskID = Kdm_GetGroupByGroup(m_pMcuSdkHandle, groupID, &errorInfo);
	if (0 == errorInfo)
	{
		GROUPINFO tGROUPINFO;
		memset(&tGROUPINFO, 0, sizeof(GROUPINFO));
		unsigned long dwAddr = 0;
		BOOL32 _bRtn = Kdm_GetGroupNext(m_pMcuSdkHandle, dwRootTaskID, &tGROUPINFO, &errorInfo);
		while(_bRtn)
		{
			// m_vecGroups.push_back(tGROUPINFO);
			QueryAllDevicesByGroup(tGROUPINFO.groupID);
			QueryAllSubGroups(tGROUPINFO.groupID);
			memset(&tGROUPINFO, 0, sizeof(GROUPINFO));
			_bRtn = Kdm_GetGroupNext(m_pMcuSdkHandle, dwRootTaskID, &tGROUPINFO, &errorInfo);
		}
	}

	return 0;
}
void safe_clear_vc()
{
	for (int tvWallNum = 0;tvWallNum < m_tTVWallInfoList.size();tvWallNum ++)
	{
		if (m_tTVWallInfoList[tvWallNum].m_tvDecoderBindArray)
		{
			//���ڴ���գ�Ȼ���ÿ�;
			delete m_tTVWallInfoList[tvWallNum].m_tvDecoderBindArray;
			m_tTVWallInfoList[tvWallNum].m_tvDecoderBindArray = NULL;
		}
	}
	m_tTVWallInfoList.clear();
}
//��ѯ���е���ǽ;
int query_all_tvWall(std::vector<TTvWallInfoData> & tTVWallInfoList)
{	
	if (bConnected==false)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"(��ѯ����ǽ)��δ��¼���������µ�¼");
		return -1;
	}
	u32 errCode = 0;
	int nTemTvwallNum = 0;
	int i=0;
	for(i=0;i<3;i++)
	{
		BOOL32 bRtn = Kdm_GetTVWallTotal(m_pMcuSdkHandle,nTemTvwallNum,&errCode);
		if(nTemTvwallNum<=0)
		{
			BOOL32 bRtn = Kdm_GetTVWallTotal(m_pMcuSdkHandle,nTemTvwallNum,&errCode);
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"û���ҵ�����ǽ,����Ѱ��%d",i+1);
			Sleep(1000);
		}
	}

	if ( nTemTvwallNum > 0)
	{
		tTVWallInfoList.clear();
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"�ҵ�%d������ǽ",nTemTvwallNum);
		for (int i = 0;i < nTemTvwallNum; i++)
		{	
			TTvWallInfoData tTVWallInfoData;

 			if (Kdm_GetTVWall(m_pMcuSdkHandle,i,tTVWallInfoData,&errCode))
			{
#ifndef _APP_ALLOC_MEM_
				if (tTVWallInfoData.m_tvDecoderBindArrayRealSize > 0)
				{
					TTVDecoderBind* _pDecoderBind = new TTVDecoderBind[tTVWallInfoData.m_tvDecoderBindArrayRealSize];//����
					memcpy(_pDecoderBind,tTVWallInfoData.m_tvDecoderBindArray, 
						sizeof(TTVDecoderBind)*tTVWallInfoData.m_tvDecoderBindArrayRealSize);
					tTVWallInfoData.m_tvDecoderBindArray = _pDecoderBind;
				}
#endif
				tTVWallInfoList.push_back(tTVWallInfoData);
			}
		}
	}
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"δ���ֵ���ǽ,�����¼������");
	return 0;
}

//����ǽ����;
int tvWallPlay(DEVICEINFO *pDeviceInfo, DEVCHN *pDevChn,int IDOfCYCPlayTV)
{
	DEVCHN & tDevChn = *pDevChn;
	BOOL32 bRtn = FALSE;
	u32 errCode = 0;

	if (m_tTVWallInfoList.size() > 0)
	{
		int nIndex = 0;
		bool bFind = false;
		for (int i = 0;i < m_tTVWallInfoList.size();i ++)
		{
			if (strstr(m_tTVWallInfoList[i].m_name,"NanningVideoWall") != NULL)
			{
				nIndex = i;
				bFind = true;
				break;
			}
			if (i>=m_tTVWallInfoList.size())
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR,"δ�ҵ�����ǽ");
				return -3;
			}
		}
		if (!m_tTVWallInfoList.size())
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"without TV wall\n");
			return -4;
		}

		strcpy(m_tWallPlaydata.m_TvWallComData.m_TvWallID,m_tTVWallInfoList[nIndex].m_TvWallID);	//����ǽID
		
		if(IDOfCYCPlayTV<0)
			m_tWallPlaydata.m_TvWallComData.m_tvId =IDOfPlayTV;	//m_tvId����ID��
		else
			m_tWallPlaydata.m_TvWallComData.m_tvId=IDOfCYCPlayTV;//��ѵ
		m_tWallPlaydata.m_TvWallComData.m_tvDivId = 0;
		m_tWallPlaydata.m_devURI = tDevChn.deviceID;	//����ͷID
		m_tWallPlaydata.m_ChnID = tDevChn.nChn;			//����ͷͨ����

		bRtn = Kdm_TVWallStartPlay(m_pMcuSdkHandle,m_tWallPlaydata,&errCode);	//��һ������ǽ�ṹ��
		if (!bRtn)
		{
			char szMsg[1024] = {0};
			sprintf(szMsg, "Kdm_TVWallStartPlay ����ʧ��,����ǽ��%s ���ӣ�%d ����ͷ:%s ͨ����:%d err:%u",m_tTVWallInfoList[nIndex].m_name,m_tWallPlaydata.m_TvWallComData.m_tvId, m_tWallPlaydata.m_devURI.szID,m_tWallPlaydata.m_ChnID,errCode);
			Drv_LogMessage(PK_LOGLEVEL_ERROR,szMsg);
			return -2;
		}
		else
		{
			char szMsg[1024] = {0};
			sprintf(szMsg, "���ųɹ�:����ǽ��%s ,����:%d,����ͷ:%s ͨ���ţ�%d" ,m_tTVWallInfoList[nIndex].m_name,m_tWallPlaydata.m_TvWallComData.m_tvId,m_tWallPlaydata.m_devURI.szID,m_tWallPlaydata.m_ChnID);
			Drv_LogMessage(PK_LOGLEVEL_NOTICE,szMsg);
		}
	}//if (m_tTVWallInfoList.size() > 0)
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"����ǽ�б�Ϊ�գ����Ȼ�ȡ����ǽ");
		return -1;
	}
	m_tWallPlaydata.m_TvWallComData.m_tvId=0;
	return 0;
}

//��ѯ,��������;
int selectCamsToTV(char*szTagName,char *szBinValue,map<string,CYCLECAMINFO> m_mapCycleWithPriv)
{
	if (strncmp(szTagName,"cycle_TV_wall",13))
	{
		return -100;
	}
	else
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"�յ���ѵ����:%s",szBinValue);

	CYCLECAMINFO pcycle1;
	int tvWall=atoi(szTagName+13);
	switch(tvWall)
	{
	case 1:pcycle1.pCamMap=&arrCamList1;break;
	case 2:pcycle1.pCamMap=&arrCamList2;break;
	case 3:pcycle1.pCamMap=&arrCamList3;break;
	case 4:pcycle1.pCamMap=&arrCamList4;break;
	case 5:pcycle1.pCamMap=&arrCamList5;break;
	case 6:pcycle1.pCamMap=&arrCamList6;break;
	case 7:pcycle1.pCamMap=&arrCamList7;break;
	case 8:pcycle1.pCamMap=&arrCamList8;break;
	case 9:pcycle1.pCamMap=&arrCamList9;break;
	case 10:pcycle1.pCamMap=&arrCamList10;break;
	case 11:pcycle1.pCamMap=&arrCamList11;break;
	case 12:pcycle1.pCamMap=&arrCamList12;break;
	case 13:pcycle1.pCamMap=&arrCamList13;break;
	case 14:pcycle1.pCamMap=&arrCamList14;break;
	case 15:pcycle1.pCamMap=&arrCamList15;break;
	case 16:pcycle1.pCamMap=&arrCamList16;break;
	case 17:pcycle1.pCamMap=&arrCamList17;break;
	case 18:pcycle1.pCamMap=&arrCamList18;break;
	case 19:pcycle1.pCamMap=&arrCamList19;break;
	case 20:pcycle1.pCamMap=&arrCamList20;break;
	case 21:pcycle1.pCamMap=&arrCamList21;break;
	case 0:pcycle1.pCamMap=&arrCamList0;break;
	case 22:pcycle1.pCamMap=&arrCamList22;break;
	case 23:pcycle1.pCamMap=&arrCamList23;break;
	default:break;
	}
	//����;��szbinValueд��ṹ��
	if(parseCycleCam(szBinValue,&pcycle1)<0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"�����°���ȷ��ʽ�´�����:{\"name\":\"value\"}");
		return -1;
	}
			
	map<string,CYCLECAMINFO>::iterator it;
	it=m_mapCycleWithPrivilege.find(szTagName);
	if((it==m_mapCycleWithPrivilege.end())||(it->second.privilege<=pcycle1.privilege)||(pcycle1.curTime-it->second.cycStartTime>=it->second.TVWallHoldTime))
	{
		m_mapCycleWithPrivilege[szTagName]=pcycle1;	//��Ҫ���ԣ�û�а��գ����ǿ������캯��;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"Ȩ�޲��������Ե�...��ǰȨ�ޣ�%d",pcycle1.privilege);
		return -1;
	}
	//camList.clear();

	return 0;
}
bool login(PKDEVICE* pdevice, long nIPSize, int nDevicePort, char* pszExtent, long nExtentSize,long &nLoginID)
{	
	char strTemp[64];
	if((pdevice->szParam1 == NULL)||(pszExtent == NULL) || m_pMcuSdkHandle == NULL)
	{
		//strTemp.LoadString(IDS_STRING107);
		//m_pSetSDKErrInfoCallback(false, EC_ICV_CCTV_FUNCPARAMINVALID, strTemp.GetBuffer(), 0, "");
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"��¼����������;");
		return false;
	}
	

	nLoginID = 0;
	WORD dwPort = (WORD)nDevicePort;
	sprintf(strTemp,"%s",pszExtent);

	char * strUserName = strtok(strTemp,",");
	if (NULL==strTemp)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,"��¼��ʽ����ȷ");
		return false;
	}
	char * strPassWord =strtok(NULL,",") ;

	//�����豸���ӵȴ�ʱ��300
	//CLIENT_SetConnectTime(DVR_TIME_OUT, NULL);

	u32 nErrCode = 0;
	// ƽ̨��¼��ַ�����Ҫ��д�˿ڣ���IP:Port��ʽ��д������192.168.0.1:1722
	char strConnAddr[128]={0};
	sprintf(strConnAddr,"%s:%d",pdevice->szParam1,dwPort);
	//Kdm_SetSDKEventCallback(m_pMcuSdkHandle,SDK_Envent_CallBack,(u32)this);
	int bLogined = Kdm_Login(m_pMcuSdkHandle, strUserName, strPassWord, strConnAddr, "WindowsOCX", &nErrCode);
	
	//��½������ 0;
	if (bLogined == FALSE)
	{
		//strTemp.LoadString(IDS_STRING109);
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "%s:%d,%d ",pdevice->szParam1, nDevicePort, nErrCode);
		string strTip;

		//strTip(strTemp, pszDeviceIP, nDevicePort, nErrCode, nErrCode);
		//m_pSetSDKErrInfoCallback(false, EC_ICV_CCTV_FAILTOLOGINDVR, strTip.GetBuffer(0), nErrCode, "");
		return false;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"��¼�ɹ�\n");
		bConnected=true;
		//��ѯ�豸״̬;
		SubscriptDeviceStatus();
		Kdm_SetDevStatusCallback(m_pMcuSdkHandle,device_states_callback,(u32)_PDevice);
	}
	//���ò���������ģʽ;
	Kdm_SetStreamPattern(m_pMcuSdkHandle,emStreamPatenUDP,&nErrCode);

	nLoginID = 1; // ��ʾ��¼�ɹ�;

	// ��ѯ�����鼰���������е��豸;
	m_mapDevId2Device.clear();
	GroupID tRootID;
	memset(&tRootID,0,sizeof(GroupID));
	QueryAllSubGroups(tRootID);

	safe_clear_vc();
	query_all_tvWall(m_tTVWallInfoList);
	return true;
}

long GetDeviceAndChannel(DEVICEINFO *pDeviceInfo, DEVCHN *pDevChn, char *szCamChannel)
{
	CString strChannel = szCamChannel;
	CString strDeviceId = GetNextToken(strChannel,','); // 0b7388a6d99349ed82513066e9729a9f
	CString strChannelNo = GetNextToken(strChannel,','); // 0b7388a6d99349ed82513066e9729a9f
	if(strChannelNo.IsEmpty())
		strChannelNo = "1"; // ���δ��������Ϊ��1ͨ��

	map<string,DEVICEINFO>::iterator itMap = m_mapDevId2Device.find(strDeviceId.GetBuffer(0));
	if(itMap == m_mapDevId2Device.end())
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "��������ͷID�����豸ʧ��, id=%s, devCount=%d", strDeviceId.GetBuffer(0), m_mapDevId2Device.size());
		return -1;
	}
	DEVICEINFO &devInfo = itMap->second;
	pDeviceInfo = &itMap->second;
	DEVCHN &tDevChn = *pDevChn;
	int	nDevIndex = -1;
	memset(&tDevChn, 0, sizeof(DEVCHN));
	strcpy(tDevChn.deviceID.szID, devInfo.deviceID.szID); // "0b7388a6d99349ed82513066e9729a9f@kedacom"
	strcpy(tDevChn.domainID.szID,devInfo.domainID.szID); // 014abdeba3234c0b8f49804e1d711684
	int nChannelNo = ::atoi(strChannelNo) - 1;
	tDevChn.nChn = nChannelNo;
	tDevChn.nSrc = nChannelNo;
	return 0;
}

//��ʼ���豸,�����������;
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	string strTemp;
	PKTIMER timerInfo1;
	timerInfo1.nPeriodMS = 2 * 1000;

	long lRet = Drv_CreateTimer(pDevice,&timerInfo1); // �趨��ʱ��;
	if (!m_pMcuSdkHandle)
	{
		m_pMcuSdkHandle = Kdm_CreateMcuSdk();
		if (!m_pMcuSdkHandle)
		{
			char* strTip;
			strTip="Create SDK failed!";
			Drv_LogMessage(PK_LOGLEVEL_ERROR, strTip);;
			//m_pSetSDKErrInfoCallback(false, EC_ICV_CCTV_FAILTOINITDVR, strTip.GetBuffer(0), -1, "");
			m_pMcuSdkHandle = NULL;
			return -1;
		}

		EBussinessMod eBussMod = emPlat2BS;
		Kdm_ModualSelect(m_pMcuSdkHandle, eBussMod, emG900, emBaseDec);

		//��ʼ������sdk,���е��õĿ�ʼ	;
		int bInited = Kdm_Init(m_pMcuSdkHandle);
		if (!bInited)
		{
			Kdm_DestroyMcuSdk(m_pMcuSdkHandle);
			m_pMcuSdkHandle = NULL;
			char* strTip;
			strTip="��ʼ��SDK ʧ��!";
			Drv_LogMessage(PK_LOGLEVEL_ERROR, strTip);
			//m_pSetSDKErrInfoCallback(false, EC_ICV_CCTV_FAILTOINITDVR, strTip.GetBuffer(0), -1, "");
			return -2;
		}
		Kdm_SetSDKEventCallback(m_pMcuSdkHandle,SDK_Envent_CallBack,0);
		Kdm_SetSaveLogFile(m_pMcuSdkHandle, 1, "kedaplat");
	}
	bool nretLogin=false;
	for(int i=0;i<3;i++)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"��ʼ��¼%d",i);
		//��¼kedaƽ̨;
		nretLogin=login(pDevice,sizeof(pDevice->szParam1),atoi(pDevice->szParam2),pDevice->szParam3,sizeof(pDevice->szParam3),loginID);
		//Sleep(2000);
		if(bConnected==true)
			break;
	}

	_PDevice=pDevice;	
	return 0;
}


//map:tvwall,cam,Ȩ��,��ǰʱ�䣬��ʼʱ�䣬����;
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	if (bConnected==false)
	{
		for(int i=0;i<3;i++)
		{
			Drv_LogMessage(PK_LOGLEVEL_NOTICE,"��ʼ��¼");
			login(pDevice,sizeof(pDevice->szParam1),atoi(pDevice->szParam2),pDevice->szParam3,sizeof(pDevice->szParam3),loginID);
			if(bConnected==true)
				break;
		}
	}
	//�жϵ���ǽ�Ƿ��ǿգ��������ٴ���ղ�ѯ;
	if(m_tTVWallInfoList.empty())
	{
		safe_clear_vc();
		query_all_tvWall(m_tTVWallInfoList);
	}
	//��ȡ��ǰʱ��
	time_t tmNow;
	time(&tmNow);
	//��ѯmap�����е�����ͷ;
	itMap=m_mapCycleWithPrivilege.begin();

	//ѭ������24����ǽ;
	for (;itMap!=m_mapCycleWithPrivilege.end();itMap++)
	{
		//��Ҫ�л���ʱ�䵽��;�л�һ������ǽ������ͷ;
		//�˴���ÿ������ͷ��ͣ��ʱ��;
		if(tmNow-itMap->second.startTime>=itMap->second.cycTime)
		{
			itMap->second.startTime=tmNow;
			itMap->second.curTime=tmNow;
			itCamMap=(itMap->second).pCamMap->begin();

			
			//Ѱ����Ҫ��ʾ��cam;����
			for(int i=0;i<(itMap->second.camIndex);i++)
			{
				itCamMap++;
				if (itCamMap==(itMap->second.pCamMap)->end())//����������һ������ֱ����ʾ��һ��
				{
					itCamMap=(itMap->second.pCamMap)->begin();
					itMap->second.camIndex=0;
					
					Drv_LogMessage(PK_LOGLEVEL_DEBUG,"����ͷmap pCamMap������:%d",itMap->second.pCamMap->size());
					break;
				}
			}
			//�жϵ���ǽID;
			int idOfPlayTV=atoi(itMap->first.c_str()+13);
			if((idOfPlayTV<0)||(idOfPlayTV>23))
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR,"����ǽID��ȡ����,�����ǵ���ǽ�������ó���%s",itMap->first.c_str());
				return 0;
			}
			if(itMap->second.pCamMap->size()>1)
			{
				char *szChannel = (char*)(*itCamMap).second.data();
				DEVICEINFO devInfo;
				DEVCHN tDevChn;
				long lRet = GetDeviceAndChannel(&devInfo, &tDevChn, szChannel);
				if(lRet != 0)
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "�����豸ID�����豸ʧ��, cam channel=%s", szChannel);
					return -1;
				}

				tvWallPlay(&devInfo, &tDevChn,idOfPlayTV);
			}
			itMap->second.camIndex++;
			string szTVmode=itMap->first;
			
			string szTVcam=itMap->first;
			
			Repalce_All(szTVmode,string("cycle"), string("mode") );
			Repalce_All(szTVcam,string("cycle"), string("cam") );
			//sprintf(szTVmode,"%s_mode",itMap->first);
			//sprintf(szTVcam,"%s_cam",itMap->first);

			//����tag��mode��cam;
			vector<PKTAG *> tagVector;
			int num=0;
			for (int i=0;i<pDevice->nTagNum;i++)
			{
				PKTAG* ptag=pDevice->ppTags[i];
				//ѡ�����ǽģʽ;
				if (!strcmp(ptag->szAddress,(char*)(szTVmode.c_str())))
				{
					Drv_SetTagData_Text(ptag, itMap->second.modeType, 0, 0, 0);

					//strcpy(ptag->szData,itMap->second.modeType);
					tagVector.push_back(ptag);
					Drv_UpdateTagsData(pDevice,tagVector.data(), tagVector.size());
					tagVector.clear();
					num++;
				}
				//ѡ������ͷ
				else if(!strcmp(ptag->szAddress,(char*)(szTVcam.c_str())))
				{
					Drv_SetTagData_Text(ptag, (char*)itCamMap->first.c_str(), 0, 0, 0);
					//strcpy(ptag->szData,(char*)itCamMap->first.data());
					tagVector.push_back(ptag);
					Drv_UpdateTagsData(pDevice,tagVector.data(), tagVector.size());
					tagVector.clear();
					//Drv_UpdateTagsBinValue(pDevice,tagVector,(char*)itCamMap->first.data());
					num++;
				}
				if(num==2)	//����ȫ���ҵ����˳�;
					break;
			}
			//Drv_UpdateTagsData(pDevice,tagVector);
			
			continue;
		}
		else
		{
			itMap->second.curTime=tmNow;
			continue;
		}
	}
	
	return 0;
}


PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{
	DEVCHN pDevchn;
	DEVICEINFO pDeviceInfo;
	int packlen=0;
	long lCurRecvLen = 0;
	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;
	//char *szAddress = "tv_wall18";
	Drv_LogMessage(PK_LOGLEVEL_NOTICE,"recv a control:%s:%s",pTag->szName ,szBinValue);
	int val;
	//������ǽ����ĸ�ʽ������;
	if(((strcmp(szTagName,"picchose")!=0)&&(strcmp(szTagName,"camchose")!=0))&&(strncmp(szTagName,"cycle_TV_wall",13))&&(strncmp(szTagName,"tvwallname",10)))
	{
		GetDeviceAndChannel(&pDeviceInfo,&pDevchn,szAddress);
		if(((strcmp(szTagName,"picchose")!=0)&&(strcmp(szTagName,"camchose")!=0))&&(strncmp(szTagName,"cycle_TV_wall",13)))
		{
			switch (atoi(szBinValue))
			{
			//case 24576://Զ��;
			//
			//case 20480://����;
			//	VideoLensControl(szTagName,atoi(szAddress),atoi(szBinValue),9,1000,pDevchn);
			//	memcpy(pTag->szData,szBinValue,sizeof(szBinValue));
			//	break;
			//case 8://��Ȧ��;
			//	break;
			//case 16://��ȦС;
			//	break;
			//case 16384://��ֹ̨ͣ;

			//case 16512://��;

			//case 16448://��;

			//case  16414://��;

			//case  16400://��;

			//	VideoPanControl(szTagName,atoi(szAddress),atoi(szBinValue),3,1000,pDevchn);
			//	memcpy(pTag->szData,szBinValue,sizeof(pTag->szData)-1);
			//	/*pTag->szTagData[0]=1;*/
			//	break;
			//default:
			//	break;


			case 6://Զ��;
				val=24576;
				VideoLensControl(szTagName,atoi(szAddress),val,9,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				break;
				//
			case 5://����;
				val=20480;
				VideoLensControl(szTagName,atoi(szAddress),val,9,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				break;

			case 13://��ֹ̨ͣ;
				val = 16384;
				VideoPanControl(szTagName,atoi(szAddress),val,3,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				break;

			case 1://��;
				val = 16512;
				VideoPanControl(szTagName,atoi(szAddress),val,3,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				/*pTag->szTagData[0]=1;*/
				break;
		
			case 2://��;
				val =16448;
				VideoPanControl(szTagName,atoi(szAddress),val,3,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				/*pTag->szTagData[0]=1;*/
				break;
		
			case  3://��;
				val = 16416;
				VideoPanControl(szTagName,atoi(szAddress),val,3,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				/*pTag->szTagData[0]=1;*/
				break;
		
			case  4://��;
				val = 16400;
				VideoPanControl(szTagName,atoi(szAddress),val,3,1000,pDevchn);
				Drv_SetTagData_Binary(pTag, szBinValue, nBinValueLen);
				/*pTag->szTagData[0]=1;*/
				break;
			default:
				break;
			};
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, pTag->szData);
		}
	}
	else if(!strcmp(szTagName,"picchose")) //ѡ����
	{
		IDOfPlayTV=atoi(szBinValue);//����Ҫ�ĵ���ǽID���Ƶ�ȫ�ֱ����С�Ĭ����0��
	}
	else if(!strcmp(szTagName,"camchose"))	//ѡ������ͷ
	{
		char CamNo[8]="cam";
		strcat(CamNo,szBinValue);
		vector<PKTAG *>	tagVector;		// �������Ӧ�ĵ�����;
		// ���������Ѿ����õ�tag��,�ҵ��Ǳ������Ӧ��ַ�ģ��Ž�ȥ��Ŀ����Ϊ�˼��ٸ������ݵ��ٶ�;
		int iTag;
		for( iTag = 0; iTag < pDevice->nTagNum; iTag ++)
		{
			PKTAG *pTag = pDevice->vecTags[iTag];
			if (!strcmp(pTag->szName,CamNo))
			{
				char szChannel[64]={0};
				sprintf(szChannel,"%s",pTag->szAddress);
				GetDeviceAndChannel(&pDeviceInfo,&pDevchn,szChannel);
				if(tvWallPlay(&pDeviceInfo,&pDevchn,-1)<0)
				{
					Drv_LogMessage(PK_LOGLEVEL_ERROR,"����ʧ��");	;
				}
				break;
			}
			
		}
		if (iTag>=pDevice->nTagNum)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR,"δ�ҵ�����ͷ��%s",CamNo);
			Drv_LogMessage(PK_LOGLEVEL_NOTICE,"��ֹͣ���ͣ��������´�����...");
			return 0;
		}

		return 0;
	} //end if(!strcmp(szTagName,"camchose"))	//ѡ������ͷ
	else if(selectCamsToTV(szTagName,szBinValue,m_mapCycleWithPrivilege)>=0)
	{
		//��ʼ��ѯ
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"�ɹ���ѯ�յ�����,��ʼ׼�����Ͳ�������...");
		map<string,string>::iterator it=m_mapCycleWithPrivilege[szTagName].pCamMap->begin();
		m_mapCycleWithPrivilege[szTagName].camIndex=1;
		char *szChannel = (char*)(*it).second.data();
		int iDOfPlayTV=atoi(szTagName+13);
		DEVICEINFO devInfo;
		DEVCHN tDevChn;
		long lRet = GetDeviceAndChannel(&devInfo, &tDevChn, szChannel);
		if(lRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�����豸ID�����豸ʧ��, cam channel=%s", szChannel);
			return -1;
		}

		tvWallPlay(&devInfo, &tDevChn,iDOfPlayTV);

	}
	else if(!strncmp(szTagName,"tvwallname",10))
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"�յ�����������Ϣ");
		//ɾ��֮ǰ����ѵ�б�
		int tvNo=(szTagName[strlen(szTagName)-2]-'0')*10+(szTagName[strlen(szTagName)-1]-'0');
		char KeyToErase[20];
		sprintf(KeyToErase,"cycle_TV_wall%d",tvNo);
		map<string,CYCLECAMINFO>::iterator it=m_mapCycleWithPrivilege.find(KeyToErase);
		if(it!=m_mapCycleWithPrivilege.end())
		m_mapCycleWithPrivilege.erase(it);
		DEVICEINFO devInfo;
		DEVCHN tDevChn;

		long lRet = GetDeviceAndChannel(&devInfo, &tDevChn,szBinValue);
		if(lRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�����豸ID�����豸ʧ��, cam channel=%s",szBinValue);
			return -1;
		}
		
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"ʹ�õ���ǽID��%d",tvNo);
		if(0==tvWallPlay(&devInfo, &tDevChn,tvNo))
		{
			Drv_LogMessage(PK_LOGLEVEL_NOTICE,"�������ò��ųɹ�");
		}
	/*		
		if(pLiandongIndex-arrTVWall>=23)
		{
			pLiandongIndex=arrTVWall;
		}
		else 
			pLiandongIndex++;
			*/
	}
	return 0;
}
