// Modbus 驱动。设备参数1为0表示ModbusRTU驱动，为1表示ModbusTCP驱动
// 数据块参数1表示站号。站号必须大于等于1
// 参数2：读写用哪些指令；
// 参数3：每个块的最大字节数
// 变量也具有一个参数：表示站号（如果一个设备接入了多个不同的组，每个组的站号可能是不同的）
// DRVTAG的nData1是起始地址位（相对于AI的0地址），nData2是结束地址位，nData3是在该块内的起始位数
#include "math.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "OPCServer.h"
#include "OpcServerTask.h"
#include "OPCItem.h"
#include "OPCGroup.h"

#define EC_ICV_INVALID_PARAMETER                    100
#define	INT_MAX										2147483647

#define DEVPARAM_OPCSERVER							0	// opc server

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	HRESULT hr = ::CoInitializeEx(NULL,COINIT_MULTITHREADED); // setup COM lib
	if (FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,("OPC Driver CoInitializeEx failed!"));
		return -1;
	}

	Drv_LogMessage(PK_LOGLEVEL_INFO, ("OPC Driver CoInitializeEx Initialize successfully!"));

	// Allow anyone to be able to call back into us!
	// it's very important to set the security,or it will cause access denied with error code 0x8007005
	CoInitializeSecurity (NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

	return 0;
}


PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	::CoUninitialize();
	return 0;
}


PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	COPCServer *pOPCServer = new COPCServer();
	pDevice->pUserData[DEVPARAM_OPCSERVER] = pOPCServer;
	pOPCServer->m_pDevice = pDevice;
	pOPCServer->StartOPCServerTask(); // start the tsk for this opc server

	return 0;
}

/*
	反初始化设备
*/

PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	int nTimerNum = 0;
	// pDevice->szConnParam, format opcservername@ip
	COPCServer *pOPCServer = (COPCServer *)pDevice->pUserData[DEVPARAM_OPCSERVER];
	if(pOPCServer)
	{
		pOPCServer->StopOPCServerTask();
		delete pOPCServer;
	}
	pDevice->pUserData[DEVPARAM_OPCSERVER] = NULL;
	return 0;
}

/**
 *  
 *  @version     12/11/2008    Initial Version.
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
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

	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "收到控制命令：向设备(%s)的tag(%s)进行控制，地址:%s, 值:%s",
		pDevice->szName, szTagName, szAddress, szStrValue);

	COPCServer *pOPCServer = (COPCServer *)pDevice->pUserData[DEVPARAM_OPCSERVER];
	pOPCServer->SendControlCommand(pDevice, pTag, szStrValue, lCmdId);

	return 0;
}
	
