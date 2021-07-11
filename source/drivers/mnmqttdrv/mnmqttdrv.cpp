/************************************************************************/
/* ����������
���ܣ�
1����ÿ�������ж�ȡ������Ϣ
2������ÿ�����ط�����������
3��������Ϣ�·�
4������������Ϣ��redis(pknodeserver)
���ŵ�����
����ͨ����name-data  name-config
*/
/************************************************************************/

#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <stdlib.h>��
#include <stdio.h>

#ifdef WIN32
#include "windows.h"
#else
#include "unistd.h"
#include "memory"
#endif

#include "mqttImpl.h"
#include "json/json.h"
#include <string>
#include <map>

using namespace std;

#define TOPIC_REALDATA_PREFIX			"iot/realData"				// ʵʱ���ݷ���ͨ��, peak/realdata/{gatewayID}
#define TOPIC_CONTROL_PREFIX			"iot/control"				// ����ͨ��, peak/control/{gatewayID}

CMqttImpl *g_pMqttObj = NULL;

typedef struct _GWINFO{
	PKDEVICE *pDevice;
	time_t nLastUpdateSecond; // �ϴ����ݵ���,�����ж��Ƿ��Ѿ����ߡ�����1��������Ϊ����
	_GWINFO()
	{
		pDevice = NULL;
		time(&nLastUpdateSecond);
	}
}GWINFO;

map<string, GWINFO*>  g_mapGatewayId2Device;
void my_connect_callback(struct mosquitto *mosq, void *obj, int rc);
void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result);

//�����ص� ֻ�з����ɹ��Ż��������Ͽ�����֮�󲻻��������
//���ԣ�ÿ����һ����Ϣ���У������浽���ͻ������У�������ͳɹ��������ｫ��ɾ����
void my_publish_callback(struct mosquitto *mosq, void *obj, int mid);
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

GWINFO *GetGWInfoByName(string &strGatewayId)
{
	map<string, GWINFO*>::iterator itFound = g_mapGatewayId2Device.find(strGatewayId);
	if (itFound != g_mapGatewayId2Device.end())
	{
		GWINFO *pGWInfo = itFound->second;
		return pGWInfo;
	}
	return NULL;
}

int CheckConnectToMqttServer()
{
	if (g_pMqttObj->m_bConnected)
		return 0;

	int nRet = g_pMqttObj->mqttClient_Connect(3000, my_connect_callback, my_disconnect_callback, my_publish_callback, my_message_callback);
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Connect return failed,%d", nRet);
		return nRet;
	}

	return nRet;
}

int SetDeviceConnected(PKDEVICE *pDevice, int nTimeSpan)
{
	Drv_LogMessage(PK_LOGLEVEL_INFO, "��������:%s ��������״̬", pDevice->szName);
	Drv_Connect(pDevice, 1000); // �Ͽ����ӣ���Ϊ�����˶Ͽ����ӱ�־;
	return 0;
}

// ��ţ���أ�û�����ǲ��������ݰ���;
int SetDeviceDisconnected(PKDEVICE *pDevice, int nTimeSpan)
{
	Drv_LogMessage(PK_LOGLEVEL_ERROR, "(SetDeviceDisconnected)gateway:%s disconnect", pDevice->szName);
	Drv_Disconnect(pDevice); // �Ͽ����ӣ���Ϊ�����˶Ͽ����ӱ�־;
	return 0;
}

int CheckGateWayConnect()
{
	time_t tmNow;
	time(&tmNow);

	map<string, GWINFO*>::iterator itGW = g_mapGatewayId2Device.begin();
	for (; itGW != g_mapGatewayId2Device.end(); itGW++)
	{
		string strGateWay = itGW->first;
		GWINFO *pGWInfo = itGW->second;
		int nTimeSpan = labs(tmNow - pGWInfo->nLastUpdateSecond);
		if (nTimeSpan > 90) // ����60�뻹û���յ��µ����ݡ�����ÿ60����뷢����һ��ȫ������
		{
			// ����״̬Ϊδ����;
			// SetDeviceDisconnected(pGWInfo->pDevice, nTimeSpan);
		}
		else
		{
			SetDeviceConnected(pGWInfo->pDevice, nTimeSpan);
		}
	}

	return 0;
}

//����redis�߳�ֻ����һ��;
PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	time_t tmNow;
	time(&tmNow);

	string strMqttServerIp = "127.0.0.1";
	unsigned short usMqttServerPort = 9006; // MQTT��׼ȱʡ�˿�
	if (strlen(pDriver->szParam1) > 0)
		strMqttServerIp = pDriver->szParam1;

	if (strlen(pDriver->szParam2) > 0)
		usMqttServerPort = ::atoi(pDriver->szParam2);
	if (usMqttServerPort <= 0)
		usMqttServerPort = 9006; // default is :1883, mngate is :9006

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "MQTT Server IP:%s, �˿�:%d.�����Ҫ�ı�ȱʡ��ip(127.0.0.1)�Ͷ˿�(%d), ��������������1(ip), �Ͳ���2(�˿�)", 
		strMqttServerIp.c_str(), usMqttServerPort, usMqttServerPort);

	for (int i = 0; i < pDriver->nDeviceNum; i++)
	{
		PKDEVICE *pDevice = pDriver->ppDevices[i];
		Drv_SetConnectOKTimeout(pDevice, 120);
		// ������ʱ��, ����Ƿ�����. ֻҪ�ڵ�һ���豸�ϴ�����ʱ���Ϳ�����
		if (i == 0)
		{
			PKTIMER timer;
			timer.nPeriodMS = 60 * 1000;
			Drv_CreateTimer(pDevice, &timer);
		}

		if (strlen(pDevice->szParam1) <= 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸:%s �ĵ�һ��������������Ϊ���ص�ID!", pDevice->szName);
			continue;
		}

		map<string, GWINFO*>::iterator itFound = g_mapGatewayId2Device.find(pDevice->szParam1);
		if (itFound != g_mapGatewayId2Device.end())
		{
			PKDEVICE *pExistDev = itFound->second->pDevice;
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸:%s �ĵ�һ������:%s(����ID), ǰ���Ѿ����õ������豸��(�豸:%s)!", pDevice->szName, pDevice->szParam1, pExistDev->szName);
			continue;
		}

		GWINFO *pGWInfo = new GWINFO();
		pGWInfo->pDevice = pDevice;
		time(&pGWInfo->nLastUpdateSecond);
		g_mapGatewayId2Device[pDevice->szParam1] = pGWInfo;
	}

	//2 ����mqtt;
	g_pMqttObj = new CMqttImpl(strMqttServerIp.c_str(), usMqttServerPort, "mngw-mqttdrv");
	int nRet = g_pMqttObj->mqttClient_Init(pDriver);
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Init return failed,%d", nRet);
	}

	// �������;
	CheckConnectToMqttServer();

	// ��������߳�ֻ�ܿ�����һ�ΰɣ����������ܷ��������ﱻ����Σ�;
	g_pMqttObj->mqttClient_StartLoopWithNewThread();
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--��������(%s),�豸(%s),��ǰ��������:%d--", pDevice->pDriver->szName, pDevice->szName, pDevice->nTagNum);

	return 0;
}

/*
	����ʼ���豸
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	return 0;
}


// ���ܣ���ʱ��������Ƿ�Ͽ�;
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CheckConnectToMqttServer();
	CheckGateWayConnect();
	return 0;
}

/************************************************************************/
/* ���ܣ�
1���յ�������Ϣ�����͵�����ͨ��
2����Ϣ��ʽ��mqtt gw1_controlͨ���� {name:xxx, value:xxx}
*/
/************************************************************************/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	string strTagName = pTag->szName;
	char* szAddress = pTag->szAddress; // tagnameInGateway
	if (strlen(szAddress) <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, device:%s, tagName:%s, value:%s, tagAddr:%s invalid, tagAddr must be:tagnameInGateway, cannot be empty", 
			pDevice->szName, pTag->szName, szStrValue, pTag->szAddress);
		return -1;
	}

	string strGatewayId = pDevice->szParam1;
	string strTagNameInGateway = szAddress;

	string strTopicControl = TOPIC_CONTROL_PREFIX;
	strTopicControl = strTopicControl + "/" + strGatewayId;

	int nMid = 0;
	Json::Value jsonTags;
	jsonTags["id"] = strGatewayId;
	Json::Value jsonOneTag;
	jsonOneTag[strTagNameInGateway] = szStrValue;
	jsonTags["mt"] = jsonOneTag;

	Json::FastWriter fastWriter;
	string strJsonNV = fastWriter.write(jsonTags);
	int nRet = g_pMqttObj->mqttClient_Pub((char*)strTopicControl.c_str(), &nMid, strJsonNV.length(), strJsonNV.c_str());
	if (nRet == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "�豸:%s �յ���������,�����ط��Ϳ�������ɹ�������ID(%s),����(%s), ֵ:%s",
			pDevice->szName, strGatewayId.c_str(), strTagNameInGateway.c_str(), szStrValue);
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "�豸:%s �յ���������,�����ط��Ϳ�������ʧ��, ����ID(%s),����(%s), ֵ:%s,������:%d",
			pDevice->szName, strGatewayId.c_str(), strTagNameInGateway.c_str(), szStrValue, nRet);
	}
	return nRet;
}


PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	g_pMqttObj->mqttCLient_UnInit();
	return 0;
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	g_pMqttObj->m_bInit = true;
	g_pMqttObj->m_bConnected = true;
	Drv_LogMessage(PK_LOGLEVEL_INFO, "MQTT Server connected!");

	string strTopicRealData = TOPIC_REALDATA_PREFIX;// ʵʱ���ݷ���ͨ��, peak/realdata/{gatewayID}
	strTopicRealData = strTopicRealData + "/+"; // ���е�ʵʱ����ͨ��;

	int mid;
	int nRet = g_pMqttObj->mqttClient_Sub(strTopicRealData.c_str(), &mid);
	if (nRet == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "mqttClient_Sub(topic:%s) success", strTopicRealData.c_str());
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Sub(topic:%s) failed, ret: %d", strTopicRealData.c_str(), nRet);
	}
}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
	// g_pMqttObj->m_bConnected = false;
	Drv_LogMessage(PK_LOGLEVEL_ERROR, "MQTT Server Disconnected!");
}

//�����ص� ֻ�з����ɹ��Ż��������Ͽ�����֮�󲻻��������;
//���ԣ�ÿ����һ����Ϣ���У������浽���ͻ������У�������ͳɹ��������ｫ��ɾ��;
void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
}

string Json2String(Json::Value &jsonVal)
{
	if (jsonVal.isNull())
		return "";

	string strRet = "";
	if (jsonVal.isString())
	{
		strRet = jsonVal.asString();
		return strRet;
	}

	if (jsonVal.isInt())
	{
		int nVal = jsonVal.asInt();
		char szTmp[32];
		sprintf(szTmp, "%d", nVal);
		strRet = szTmp;
		return strRet;
	}

	if (jsonVal.isDouble())
	{
		double dbVal = jsonVal.asDouble();
		char szTmp[32];
		sprintf(szTmp, "%f", dbVal);
		strRet = szTmp;
		return strRet;
	}

	if (jsonVal.isBool())
	{
		bool bVal = jsonVal.asBool();
		char szTmp[32];
		sprintf(szTmp, "%d", bVal?1:0);
		strRet = szTmp;
		return strRet;
	}

	return "";
}
// Qualifier: ��������ͨ������Ϣ����ʱ���ڴ˴��������޸ĺ� ��Ϊ����Ϣ��һ��ר���߳�дredis;
//��Ϣ����:
/*
(configͨ�������ڵ�ǰ�߳��жϴ�����������߳�);
1��tag�汾��Ϣ��
2��tag����
(dataͨ�����׵�writetag�̴߳���);
3��tagֵ
*/
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	//�����configͨ��;
	g_pMqttObj->m_bConnected = true;
	string strTopic = msg->topic;
	char *szMsg = (char *)msg->payload;

	Drv_LogMessage(PK_LOGLEVEL_INFO, "���յ���һ��������ţ���ص�ʵʱ������Ϣ, topic:%s, ����:%s", strTopic.c_str(), szMsg);

	int nPos = strTopic.find(TOPIC_REALDATA_PREFIX);
	if (nPos == string::npos) // �ҵ���ʵʱ���ݵĿ�ͷ, ����Ϊ��ʵʱ����
		return;

	Json::Value jsonGateWay;
	Json::Reader jsonReader;

	if (!jsonReader.parse(szMsg, jsonGateWay, false))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���յ���һ���������ص�ʵʱ������Ϣ, topic:%s, ������json��ʽ, ����:%s", strTopic.c_str(), szMsg);
		return;
	}

	Json::Value jsonGWID = jsonGateWay["id"];
	string strGatewayId = jsonGWID.asString();
	//string strTagValue = jsonField[strTagNameInGateway].asString();
	GWINFO *pGWInfo = GetGWInfoByName(strGatewayId);
	if (!pGWInfo)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���յ����ص�ʵʱ����, topic:%s, ��������id:%s, δ�ҵ����õ��豸, ���������豸!", strTopic.c_str(), strGatewayId.c_str());
		return;
	}

	string strUpdateTime = "";
	Json::Value jsonUpdateTime = jsonGateWay["ts"];
	if (!jsonUpdateTime.isNull())
		strUpdateTime = Json2String(jsonUpdateTime);
	unsigned int nUpdateTime = ::atol(strUpdateTime.c_str());

	Json::Value jsonState = jsonGateWay["state"];
	if (!jsonState.isNull())
	{
		int nState = jsonState.asInt();
		if (nState == 1) // 1��ʾ�豸�Ͽ�
		{
			SetDeviceDisconnected(pGWInfo->pDevice, 100);
			//Drv_Disconnect(pGWInfo->pDevice);
			return;
		}
		else // 0
		{
			// g_pMqttObj->m_bConnected = true;
			SetDeviceConnected(pGWInfo->pDevice, 100);
			// Drv_Connect(pDevice, 100);
			return;
		}
	}

	Json::Value jsonTagsData = jsonGateWay["mt"];
	if (jsonTagsData.isNull())
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���յ����ص�ʵʱ����, topic:%s, ����id:%s, ������mt�ڵ�, ֵ:%sΪ��!", strTopic.c_str(), strGatewayId.c_str(), szMsg);
		return;
	}
	PKDEVICE *pDevice = pGWInfo->pDevice;
	time(&pGWInfo->nLastUpdateSecond);
	Drv_SetConnectOK(pDevice); // �յ����ݣ���־�豸����״̬OK
	vector<PKTAG *> pktagVec;
	map<PKDEVICE *, vector<PKTAG *> *> mapDev2TagVec;
	// { "id" :"dv_107",  "mt":{"r30" :48.85,  "r14" :120},"ts" :1524449970589}
	int nTotalTagNum = jsonTagsData.size();
	int nInvalidTagNum = 0;

	Json::Value::Members members = jsonTagsData.getMemberNames();
	for (int i = 0; i < members.size(); i++)
	{
		string strTagNameInGateway = members[i]; // TAGNAME
		Json::Value jsonTagValue = jsonTagsData[strTagNameInGateway];
		if (jsonTagValue.isNull())
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "���յ����ص�ʵʱ����, topic:%s, ��������id:%s �� tag����:%s, ֵΪ��!", strTopic.c_str(), strGatewayId.c_str(), strTagNameInGateway.c_str());
			nInvalidTagNum++;
			continue;
		}
		string strTagValue = Json2String(jsonTagValue);// .asString();

		//string strTagValue = jsonField[strTagNameInGateway].asString();
		PKTAG * tagVec2[100];
		int nTagNum = Drv_GetTagsByAddr(pDevice, strTagNameInGateway.c_str(), tagVec2, 100);
		for (int j = 0; j < nTagNum; j++)
		{
			PKTAG *pTag = tagVec2[j];
			Drv_SetTagData_Text(pTag, strTagValue.c_str(), nUpdateTime, 0, 0);
			pktagVec.push_back(pTag);
		}
	}

	Drv_UpdateTagsData(pDevice, pktagVec.data(), pktagVec.size());
	if (nInvalidTagNum == 0)
		Drv_LogMessage(PK_LOGLEVEL_INFO, "���յ�MQTT����, id:%s, ����:%d, ���ͳɹ�, ʱ���:%s", strGatewayId.c_str(), nTotalTagNum, strUpdateTime.c_str());
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "���յ�MQTT�����зǷ�����, id:%s, �ܸ���:%d, ��Ч����:%d, �ѷ�����Ч����:%d �ɹ�, ʱ���:%s", strGatewayId.c_str(), nTotalTagNum, nInvalidTagNum, nTotalTagNum - nInvalidTagNum, strUpdateTime.c_str());
}