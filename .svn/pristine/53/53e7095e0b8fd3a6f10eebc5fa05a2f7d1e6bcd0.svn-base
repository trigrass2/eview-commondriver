#include "ace/Time_Value.h"
#include "ace/OS.h"  
#include <string> 
#include "pkcomm/pkcomm.h"
#include "pkdata/pkdata.h"
#include "eviewdatadrv.h"
#include "json/json.h"

#define EC_ICV_INVALID_PARAMETER                    100
#define EC_ICV_DRIVER_DATABLOCK_TYPECANNOTWRITE		101
#define EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE			103
#define EC_ICV_DRIVER_DATABLOCK_INVALIDCMDLENGTH	104
#define EC_ICV_BUFFER_TOO_SMALL                     105
#define	INT_MAX										2147483647

#define DEVPARAM_PUSERDATA_REMOTE_EVIEW			0	// 访问远程eview服务的句柄

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备4个参数均未使用, 连接参数输入远程eview的IP!");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s), 远程eveiw的ip:%s", pDevice->szName, pDevice->szConnParam);
	CRemoteEview *pRemoteEview = new CRemoteEview(pDevice, pDevice->szConnParam);
	pDevice->pUserData[DEVPARAM_PUSERDATA_REMOTE_EVIEW] = pRemoteEview;// 访问远程eview服务的句柄
	pRemoteEview->InitRemoteEveiw();
	PKTIMER timerInfo;
	timerInfo.nPeriodMS =2000;
	void *pTimer = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器，定时轮询所有的数据
	Drv_SetConnectOKTimeout(pDevice, 10);
	return 0;
}

/*
反初始化设备;
*/
//#include "windows.h"
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	PKHANDLE hRemoteEview = pDevice->pUserData[DEVPARAM_PUSERDATA_REMOTE_EVIEW];
	if (hRemoteEview)
	{
		pkExit(hRemoteEview);
		pDevice->nUserData[DEVPARAM_PUSERDATA_REMOTE_EVIEW] = NULL;
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	return 0;
}

PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDriver(driver:%s)", pDriver->szName);
	return 0;
}

/**
*  设定的数据块定时周期到达时该函数被调用;
*
*  @return PK_SUCCESS: 执行成功;
*
*  @version     12/11/2008    Initial Version..
*/
// TAG地址：N10:31, N10:32......
// 事务号大约2分钟内不会重复
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CRemoteEview *pRemoteEviewData = (CRemoteEview *)pDevice->pUserData[DEVPARAM_PUSERDATA_REMOTE_EVIEW];// 访问远程eview服务的句柄
	PKDATA *pDataArr = pRemoteEviewData->m_pDataArr;
	int nRet = pkMGet(pRemoteEviewData->m_hRemoteEview, pDataArr, pDevice->nTagNum);
	if (nRet == 0)
	{
		if (pDevice->nTagNum > 0)
		{
			PKDATA *pkData = &pDataArr[0];
			PKTAG *pTag = pDevice->ppTags[0];
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, serverip:%s, success to read remote tagcount:%d, first tag:%s, data:%s", pDevice->szName, pRemoteEviewData->m_strServerIp.c_str(), pDevice->nTagNum, pTag->szName, pkData->szData);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, serverip:%s, success to read remote tagcount:%d", pDevice->szName, pRemoteEviewData->m_strServerIp.c_str(), pDevice->nTagNum);
		Drv_SetConnectOK(pDevice);

		// 对于每个点进行读取
		for (int iTag = 0; iTag < pDevice->nTagNum; iTag++)
		{
			PKDATA *pkData = &pDataArr[iTag];
			PKTAG *pTag = pDevice->ppTags[iTag];

			string strData = pkData->szData;
			Json::Reader reader;
			Json::Value jsonTagVal;
			if (!reader.parse(strData, jsonTagVal, false)) // 失败时取整个字符串
			{
				Drv_SetTagData_Text(pTag, "*", 0, 0, -1000);
				continue;
			}

			string strValue;
			unsigned int nSec = 0;
			unsigned int nMilSec = 0;
			int nQuality = -1000;
			Json::Value  jsonQuality = jsonTagVal["q"];
			if (!jsonQuality.isNull())
			{
				string strQuality = jsonQuality.asString();
				nQuality = ::atoi(strQuality.c_str());
			}
			Json::Value  jsonValue = jsonTagVal["v"];
			if (!jsonValue.isNull())
			{
				strValue = jsonValue.asString();
			}
			Json::Value  jsonTime = jsonTagVal["t"];
			if (!jsonTime.isNull())
			{
				string strTimeStamp = jsonTime.asString();
				nSec = PKTimeHelper::String2HighResTime(strTimeStamp.c_str(), &nMilSec);
			}

			Drv_SetTagData_Text(pTag, strValue.c_str(), nSec, nMilSec, nQuality);
		} // for (int iTag = 0; iTag < pDevice->nTagNum; iTag++)

		nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, serverip:%s, fail to read remote tagcount:%d, return:%d", pDevice->szName, pRemoteEviewData->m_strServerIp.c_str(), pDevice->nTagNum, nRet);
	}



	return 0;
}

/**
*  当有控制命令时该函数被调用.
*  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
*
*  @version     12/11/2008    Initial Version.
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	CRemoteEview *pRemoteEview = (CRemoteEview *)pDevice->nUserData[DEVPARAM_PUSERDATA_REMOTE_EVIEW];// 访问远程eview服务的句柄
	char *szRemoteTagName = pTag->szAddress;
	int nRet = pkControl(pRemoteEview->m_hRemoteEview, szRemoteTagName, "", szStrValue);
	if (nRet == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "[CONTROL]device:%s, localtag:%s, remote eveiw serverip:%s, remote eview tag:%s=%s", pDevice->szName, pTag->szName, pRemoteEview->m_strServerIp.c_str(), szRemoteTagName, szStrValue, nRet);
	}
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "[CONTROL]device:%s, localtag:%s, remote eveiw serverip:%s, remote eview tag:%s=%s, return:%d", pDevice->szName, pTag->szName, pRemoteEview->m_strServerIp.c_str(), szRemoteTagName, szStrValue, nRet);

	return 0;
}
