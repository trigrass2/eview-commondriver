/************************************************************************/
/* 云网关驱动
功能：
1、从每个网关中读取配置信息
2、接收每个网关发过来的数据
3、控制信息下发
4、更新配置信息到redis(pknodeserver)
：放到云中;
侦听通道：name-data  name-config;
*/
/************************************************************************/

#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <stdlib.h>　
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

#define TOPIC_REALDATA_PREFIX			"peak/realdata"				// 实时数据发送通道, peak/realdata/{gatewayID}
#define TOPIC_CONFIG_PREFIX				"peak/config"				// 配置通道, peak/config/{gatewayID}
#define TOPIC_CONTROL_PREFIX			"peak/control"				// 控制通道, peak/control/{gatewayID}

CMqttImpl *g_pMqttObj = NULL;

typedef struct _GWINFO
{
	PKDEVICE *pDevice;
	time_t nLastUpdateSecond; // 上次数据的秒,用于判断是否已经离线。超过1分钟则认为离线
	_GWINFO()
	{
		pDevice = NULL;
		time(&nLastUpdateSecond);
	}
}GWINFO;
map<string, GWINFO*>  g_mapGatewayId2Device;
void my_connect_callback(struct mosquitto *mosq, void *obj, int rc);
void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result);

//发布回调 只有发布成功才会进入这里，断开连接之后不会进入这里;
//所以，每发布一个消息队列，都保存到发送缓存区中，如果发送成功，在这里将他删除；
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
		//Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Connect return failed,%d", nRet);
		return nRet;
	}

	return nRet;
}

int SetDeviceConnected(PKDEVICE *pDevice, int nTimeSpan)
{
	//Drv_LogMessage(PK_LOGLEVEL_INFO, "网关:%s 处于连接状态", pDevice->szName);
	Drv_Connect(pDevice, 1000); // 断开连接，即为设置了断开连接标志
	return 0;
}

int SetDeviceDisconnected(PKDEVICE *pDevice, int nTimeSpan)
{
	//Drv_LogMessage(PK_LOGLEVEL_ERROR, "(SetDeviceDisconnected)gateway:%s disconnect", pDevice->szName);
	Drv_Disconnect(pDevice); // 断开连接，即为设置了断开连接标志
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
		if (nTimeSpan > 90) // 超过60秒还没有收到新的数据。网关每60秒必须发过来一次全量数据
		{
			// 设置状态为未连接
			SetDeviceDisconnected(pGWInfo->pDevice, nTimeSpan);
		}
		else
		{
			SetDeviceConnected(pGWInfo->pDevice, nTimeSpan);
		}
	}

	return 0;
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
		sprintf(szTmp, "%d", bVal ? 1 : 0);
		strRet = szTmp;
		return strRet;
	}

	return "";
}

//侦听redis线程只启动一次;
PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	time_t tmNow;
	time(&tmNow);

	string strMqttServerIp = "127.0.0.1";
	unsigned short usMqttServerPort = 1883; // MQTT标准缺省端口;
	if (strlen(pDriver->szParam1) > 0)
		strMqttServerIp = pDriver->szParam1;

	if (strlen(pDriver->szParam2) > 0)
		usMqttServerPort = ::atoi(pDriver->szParam2);
	if (usMqttServerPort <= 0)
		usMqttServerPort = 1883;

	//Drv_LogMessage(PK_LOGLEVEL_NOTICE, "MQTT Server IP:%s, 端口:%d.　如果要改变缺省的ip(127.0.0.1)和端口(1883), 请配置驱动参数1(ip), 和参数2(端口)", strMqttServerIp.c_str(), usMqttServerPort);
	for (int i = 0; i < pDriver->nDeviceNum; i++)
	{
		PKDEVICE *pDevice = pDriver->ppDevices[i];

		// 创建定时器, 检查是否连接. 只要在第一个设备上创建定时器就可以了;
		if (i == 0)
		{
			PKTIMER timer;
			timer.nPeriodMS = 60 * 1000;
			Drv_CreateTimer(pDevice, &timer);
		}

		Drv_SetConnectOKTimeout(pDevice, 120);
		if (strlen(pDevice->szParam1) <= 0)
		{
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "设备:%s 的第一个参数必须配置为网关的ID!", pDevice->szName);
			continue;
		}

		map<string, GWINFO*>::iterator itFound = g_mapGatewayId2Device.find(pDevice->szParam1);
		if (itFound != g_mapGatewayId2Device.end())
		{
			PKDEVICE *pExistDev = itFound->second->pDevice;
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "设备:%s 的第一个参数:%s(网关ID), 前面已经配置到其他设备了(设备:%s)!", pDevice->szName, pDevice->szParam1, pExistDev->szName);
			continue;
		}

		GWINFO *pGWInfo = new GWINFO();
		pGWInfo->pDevice = pDevice;
		time(&pGWInfo->nLastUpdateSecond);
		g_mapGatewayId2Device[pDevice->szParam1] = pGWInfo;
	}

	//2 配置mqtt;
	g_pMqttObj = new CMqttImpl(strMqttServerIp.c_str(), usMqttServerPort, "eview-gwmqttdrv");
	int nRet = g_pMqttObj->mqttClient_Init(pDriver);
	if (nRet != 0)
	{
		//Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Init return failed,%d", nRet);
	}

	// 检查连接;
	CheckConnectToMqttServer();

	// 下面这个线程只能开启这一次吧？？？？不能放在连接里被开多次;
	g_pMqttObj->mqttClient_StartLoopWithNewThread();
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	//Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--网关驱动(%s),设备(%s),当前变量个数:%d--", pDevice->pDriver->szName, pDevice->szName, pDevice->nTagNum);

	return 0;
}

/*
	反初始化设备;
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	return 0;
}


// 功能：定时检查连接是否断开;
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CheckConnectToMqttServer();
	CheckGateWayConnect();
	return 0;
}

/************************************************************************/
/* 功能;
1、收到控制信息，发送到控制通道
2、消息格式：mqtt gw1_control通道： {name:xxx, value:xxx}
*/
/************************************************************************/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	string strTagName = pTag->szName;
	char* szAddress = pTag->szAddress; // tagnameInGateway
	if (strlen(szAddress) <= 0)
	{
		//Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnControl, device:%s, tagName:%s, value:%s, tagAddr:%s invalid, tagAddr must be:tagnameInGateway, cannot be empty", 
		//		pDevice->szName, pTag->szName, szStrValue, pTag->szAddress);
		return -1;
	}

	string strGatewayId = pDevice->szParam1;
	string strTagNameInGateway = szAddress;

	string strTopicControl = TOPIC_CONTROL_PREFIX;
	strTopicControl = strTopicControl + "/" + strGatewayId;

	int nMid = 0;
	Json::Value jsonTag;
	jsonTag["name"] = strTagNameInGateway;
	jsonTag["value"] = szStrValue;
	jsonTag["action"] = "control"; // 表示控制
	Json::FastWriter fastWriter;
	string strJsonNV = fastWriter.write(jsonTag);
	int nRet = g_pMqttObj->mqttClient_Pub((char*)strTopicControl.c_str(), &nMid, strJsonNV.length(), strJsonNV.c_str());
	if (nRet == 0)
	{
		//Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备:%s 收到控制命令,向网关发送控制命令成功。网关ID(%s),变量(%s), 值:%s",
		//	pDevice->szName, strGatewayId.c_str(), strTagNameInGateway.c_str(), szStrValue);
	}
	else
	{
		//Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备:%s 收到控制命令,向网关发送控制命令失败, 网关ID(%s),变量(%s), 值:%s,错误码:%d",
		//	pDevice->szName, strGatewayId.c_str(), strTagNameInGateway.c_str(), szStrValue, nRet);
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
	//Drv_LogMessage(PK_LOGLEVEL_INFO, "MQTT Server connected!");

	string strTopicRealData = TOPIC_REALDATA_PREFIX;// 实时数据发送通道, peak/realdata/{gatewayID}
	strTopicRealData = strTopicRealData + "/+"; // 所有的实时数据通道。
	string strTopicConfig = TOPIC_CONFIG_PREFIX; // 配置通道, peak/config/{gatewayID}
	strTopicConfig = strTopicConfig + "/+"; // 所有的实时数据通道

	int mid;
	int nRet = g_pMqttObj->mqttClient_Sub(strTopicRealData.c_str(), &mid);
	if (nRet == 0)
	{
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "mqttClient_Sub(topic:%s) success", strTopicRealData.c_str());
	}
	else
	{
		//Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Sub(topic:%s) failed, ret: %d", strTopicRealData.c_str(), nRet);
	}

	nRet = g_pMqttObj->mqttClient_Sub(strTopicConfig.c_str(), &mid);
	if (nRet == 0)
	{
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "mqttClient_Sub(topic:%s) success", strTopicConfig.c_str());
	}
	else
	{
		//Drv_LogMessage(PK_LOGLEVEL_ERROR, "mqttClient_Sub(topic:%s) failed, ret: %d", strTopicConfig.c_str(), nRet);
	}

}

void my_disconnect_callback(struct mosquitto *mosq, void *obj, int result)
{
	// 只是某个设备断开连接了 g_pMqttObj->m_bConnected = false;
	//Drv_LogMessage(PK_LOGLEVEL_ERROR, "MQTT Server Disconnected!");
}

//发布回调 只有发布成功才会进入这里，断开连接之后不会进入这里;
//所以，每发布一个消息队列，都保存到发送缓存区中，如果发送成功，在这里将他删除；
void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
}

// Qualifier: 接收来自通道的消息，暂时放在此处，测试修改后 改为抛消息到一个专门线程写redis;
//消息类型;
/*
(config通道，可在当前线程判断处理，最好抛线程;
1、tag版本信息：
2、tag配置
(data通道，抛到writetag线程处理;
3、tag值
*/
void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	//如果是config通道;
	g_pMqttObj->m_bConnected = true;
	string strTopic = msg->topic;
	char *szMsg = (char *)msg->payload;
	//Drv_LogMessage(PK_LOGLEVEL_INFO, "接收到了一个来自凌峰网关的实时数据信息, topic:%s, 内容:%s", strTopic.c_str(), szMsg);

	int nPos = strTopic.find(TOPIC_REALDATA_PREFIX);
	if (nPos != string::npos) // 找到了实时数据的开头, 则认为是实时数据;
	{
		Json::Value jsonGateWay;
		Json::Reader jsonReader;

		if (!jsonReader.parse(szMsg, jsonGateWay, false))
		{
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到了一个来自网关的实时数据信息, topic:%s, 但不是json格式, 内容:%s", strTopic.c_str(), szMsg);
			return;
		}

		Json::Value jsonGWID = jsonGateWay["id"];
		string strGatewayId = jsonGWID.asString();

		string strUpdateTime = "";
		Json::Value jsonUpdateTime = jsonGateWay["t"];
		if (!jsonUpdateTime.isNull())
			strUpdateTime = Json2String(jsonUpdateTime);

		Json::Value jsonTagsData = jsonGateWay["data"];
		if (!jsonTagsData.isArray())
		{
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到网关的实时数据, topic:%s, data不是数组, 非法! 内容:%s", strTopic.c_str(), szMsg);
			return;
		}

		//string strTagValue = jsonField[strTagNameInGateway].asString();
		GWINFO *pGWInfo = GetGWInfoByName(strGatewayId);
		if (!pGWInfo)
		{
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到网关的实时数据, topic:%s, 根据网关id:%s, 未找到配置的设备, 请先配置设备!", strTopic.c_str(), strGatewayId.c_str());
			return;
		}
		PKDEVICE *pDevice = pGWInfo->pDevice;
		time(&pGWInfo->nLastUpdateSecond);
		Drv_SetConnectOK(pDevice);
		vector<PKTAG *> pktagVec;
		// {"data":[{"test1":"{\"ml\":7,\"q\":0,\"t\":\"2018-08-18 18:41:29.603\",\"v\":\"3952\"}\n"},....]
		int nTotalTagNum = jsonTagsData.size();
		int nInvalidTagNum = 0;
		for (int i = 0; i < jsonTagsData.size(); i++)
		{
			Json::Value jsonOneTag = jsonTagsData[i];
			Json::Value::Members member = jsonOneTag.getMemberNames();
			if (member.size() <= 0)
			{
				//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到网关的实时数据, topic:%s, 网关id:%s, 未找到key(tagname)!", strTopic.c_str(), strGatewayId.c_str());
				nInvalidTagNum++;
				continue;
			}

			string strTagNameInGateway = member[0]; // TAGNAME
			Json::Value jsonTagVTQ = jsonOneTag[strTagNameInGateway];
			if (jsonTagVTQ.isNull())
			{
				//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到网关的实时数据, topic:%s, 根据网关id:%s 和 tag名称:%s, 值为空!", strTopic.c_str(), strGatewayId.c_str(), strTagNameInGateway.c_str());
				nInvalidTagNum++;
				continue;
			}

			if (jsonTagVTQ.isString()) // 这里通常不应该进入，因为进入时应是json对象了，而不是字符串格式
			{
				string strTagVTQ = jsonTagVTQ.asString();
				if (!jsonReader.parse(strTagVTQ.c_str(), jsonTagVTQ, false))
				{
					//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到网关的实时数据, topic:%s, 根据网关id:%s 和 tag名称:%s, 值:%s不是json对象!", strTopic.c_str(), strGatewayId.c_str(), strTagNameInGateway.c_str(), strTagVTQ.c_str());
					nInvalidTagNum ++;
					continue;
				}
			}

			Json::Value jsonTagQuality = jsonTagVTQ["q"];
			Json::Value jsonTagValue = jsonTagVTQ["v"];
			Json::Value jsonTagTime = jsonTagVTQ["t"];
			int nTagQuality = 0;
			if (!jsonTagQuality.isNull())
				nTagQuality = ::atoi(Json2String(jsonTagQuality).c_str());
			unsigned int nTagSecond = 0;
			unsigned int nTagMilSecond = 0;
			if (!jsonTagTime.isNull())
			{
				string strTagTime = Json2String(jsonTagTime);
				if (strTagTime.length() >= 10)
				{
					string strSecond = strTagTime.substr(0, 10);
					string strMilSecond = strTagTime.substr(10);
					nTagSecond = ::atoi(strSecond.c_str());
					nTagMilSecond = ::atoi(strMilSecond.c_str());
				}
			}
			string strTagValue = "";
			if (!jsonTagValue.isNull())
				strTagValue = Json2String(jsonTagValue);

			//string strTagValue = jsonField[strTagNameInGateway].asString();
			PKTAG *tagVec2[100];
			int nTagNumTmp = Drv_GetTagsByAddr(pDevice, strTagNameInGateway.c_str(), tagVec2, 100);
			for (int j = 0; j < nTagNumTmp; j++)
			{
				PKTAG *pTag = tagVec2[j];
				Drv_SetTagData_Text(pTag, strTagValue.c_str(), nTagSecond, nTagMilSecond, nTagQuality);
				pktagVec.push_back(pTag);
			}
		}

		Drv_UpdateTagsData(pDevice, pktagVec.data(), pktagVec.size());
	//	if (nInvalidTagNum == 0)
			//Drv_LogMessage(PK_LOGLEVEL_INFO, "接收到MQTT数据, id:%s, 个数:%d, 发送成功, 时间戳:%s", strGatewayId.c_str(), nTotalTagNum, strUpdateTime.c_str());
	//	else
			//Drv_LogMessage(PK_LOGLEVEL_ERROR, "接收到MQTT数据有非法数据, id:%s, 总个数:%d, 无效个数:%d, 已发送有效个数:%d 成功, 时间戳:%s", strGatewayId.c_str(), nTotalTagNum, nInvalidTagNum, nTotalTagNum - nInvalidTagNum, strUpdateTime.c_str());
		pktagVec.clear();
		return;
	}

	//如果是配置通道，则要么是返回的MD5 要么是返回的所有配置信息
	nPos = strTopic.find(TOPIC_CONFIG_PREFIX);
	if (nPos != string::npos) // 找到了配置Topic的开头, 则认为是配置
	{
		//Drv_LogMessage(PK_LOGLEVEL_INFO, "接收到了一个来自配置信息, topic:%s, 暂时忽略", strTopic.c_str());
		return;
	}
}
