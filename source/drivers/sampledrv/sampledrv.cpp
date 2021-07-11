// 示例驱动程序。实现功能：配置的数据自动变化，当发生控制时数据则不再变化

#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <stdlib.h>
#include <stdio.h>

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--示例驱动(%s),设备(%s),将模拟%d个变量--", pDevice->pDriver->szName, pDevice->szName, pDevice->nTagNum);
	int nInitVal = 0;
	if (strlen(pDevice->szParam1) == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--示例驱动(%s),设备(%s),每个变量将从0开始自增", pDevice->pDriver->szName, pDevice->szName);
		nInitVal = 0;
	}
	else
	{
		nInitVal = ::atoi(pDevice->szParam1);
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--示例驱动(%s),设备(%s),变量的初值为:%d,不自增", pDevice->pDriver->szName, pDevice->szName, nInitVal);
	}

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
	{
		PKTAG *pTag = pDevice->ppTags[i];
		char szValue[32];
		sprintf(szValue, "%d", nInitVal);
		Drv_SetTagData_Text(pTag, szValue,  0, 0, 0);
		vecTags.push_back(pTag);
	}
	Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());
	vecTags.clear();

	PKTIMER timer;
	timer.nPeriodMS = 1000;
	Drv_CreateTimer(pDevice, &timer);
	return 0;
}

/*
	反初始化设备
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "反初始化: 驱动:%s, 设备:%s, 连接参数: %s", pDevice->pDriver->szName, pDevice->szName, pDevice->szConnParam);

	return 0;
}

/**
 *  设定的数据块定时周期到达时该函数被调用.
 *  
 *  可以在该函数中向设备发送请求读取实时数据或向设备发送控制指令.
 *  2. 该函数中需要做如下处理：  
		a) 如果需要定时扫描，则向设备发送请求读取实时数据
		b) 如果需要定时控制，则向设备发送控制命令
		c) 如果是同步方式接收设备数据，则需要在该函数中等待设备数据,直到数据到达或超时返回；
		d) 如果异步方式接收设备数据，则需要提供一个接收数据的回调函数，在回调函数中处理设备返回数据。

 *  3. 当接收到设备返回数据时，解析设备返回数据：
		a) 如果是实时数据，则需要更新数据块的值
		   可以调用函数 Drv_UpdateTagsData
 *  
 *  @param  -[in]  PKDEVICE  pDevice: [设备信息]
 *  @param  -[in]  PKTIMER  timerInfo: [定时器信息，定时器必须自行开启] 
 *  @return PK_SUCCESS: 执行成功
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	bool bAutoInc = false;
	if (strlen(pDevice->szParam1) == 0)
		bAutoInc = true;
	else // 自增的值不需要去管
		bAutoInc = false;

	vector<PKTAG *> vecTags;
	char szTagValue[2048] = { 0 };
	for (int i = 0; i < pDevice->nTagNum; i++)
	{
		PKTAG *pTag = pDevice->ppTags[i];
		if (bAutoInc && pTag->nData2 == 0) // 数值自增，且未被控制过，那么变量的需要改变,否则就不需要改变
		{
			pTag->nData1++;

			if (pTag->nDataType == TAG_DT_CHAR || pTag->nDataType == TAG_DT_UCHAR || pTag->nDataType == TAG_DT_INT16 || pTag->nDataType == TAG_DT_UINT16 || pTag->nDataType == TAG_DT_INT32 || pTag->nDataType == TAG_DT_UINT32)
				sprintf(szTagValue, "%d", pTag->nData1);
			else if (pTag->nDataType == TAG_DT_BOOL)
			{
				sprintf(szTagValue, "%d", pTag->nData1 % 2);
			}
			else if (pTag->nDataType == TAG_DT_FLOAT || pTag->nDataType == TAG_DT_DOUBLE)
			{
				sprintf(szTagValue, "%d", pTag->nData1);
			}
			else if (pTag->nDataType == TAG_DT_TEXT)
			{
				sprintf(szTagValue, "Text%d", pTag->nData1);
			}
			Drv_SetTagData_Text(pTag, (char *)szTagValue, 0, 0, 0);
		}
		else
		{
			pTag->nQuality = 0;
			pTag->nTimeSec = 0;
			pTag->nTimeMilSec = 0;
		}
		vecTags.push_back(pTag);
	}
	Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!模拟驱动simdriver：向驱动(%s),设备(%s),更新了%d个变量值!!!", pDevice->pDriver->szName, pDevice->szName, vecTags.size());

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
	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "--示例驱动收到控制命令：向驱动(%s),设备(%s),变量(名称:%s, 地址:%s)控制,值:%s--",
		pDevice->pDriver->szName, pDevice->szName, szTagName, szAddress, szStrValue);
	pTag->nData2 = 1; // 一旦被控制后，模拟变量的值就不再改变了

	vector<PKTAG *> vecTags;
	vecTags.push_back(pTag);
	Drv_SetTagData_Text(pTag, (char *)szStrValue, 0, 0, 0); // 设置值，质量为GOOD，时间取当前时间
	Drv_UpdateTagsData(pDevice, vecTags.data(), vecTags.size());

	return 0;
}
