

#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <string>
#include <cstring> 
#include <cstdio> 
#include <cstdlib>
#ifdef WIN32

#else

#include <fstream>
#include <sys/times.h>
#include <sys/vtimes.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <unistd.h> //sleep
#include <netdb.h>  
#include <arpa/inet.h> 

#endif //WIN32


//#include <ace/Singleton.h>
//#include <ace/Null_Mutex.h>
//#include <ace/Task.h> 

 
using namespace std; 

#define TIMER_TYPE_ELAPSE_10S			10000

#define TAG_ADDRESS_UPDATE_PING			"pkping"
#define TAG_ADDRESS_RESULT_PING_Y		"1"
#define TAG_ADDRESS_RESULT_PING_N		"0"
#define READ_CHARS_LENGTH	1024 

#ifdef WIN32

#else

void LinesFrom(FILE *pFile, vector<string> &vResult)
{
	vResult.clear();
	char szLine[READ_CHARS_LENGTH] = { 0 };
	while(fgets(szLine, READ_CHARS_LENGTH, pFile) != NULL)
	{
		int nLength = strlen(szLine);
		if(szLine[nLength - 1] == '\n')
			szLine[nLength - 1] = '\0';
		vResult.push_back(szLine);
	}
}

int ExecuteCommand(const char *szCommand, vector<string> &vResult)
{	
	FILE *pCmd = popen(szCommand, "r");
	if(pCmd == NULL) 
		return -1; 
	LinesFrom(pCmd, vResult);
	return pclose(pCmd);
}

#endif  //WIN32

bool Ping(const char *szIPAddress)
{
#ifdef WIN32	
	string cmd("cmd /C ping -n 3 -w 1024 ");
	cmd.append(szIPAddress); 
	//WinExec(cmd.c_str(), SW_HIDE); 
	return system(cmd.c_str()) == 0;
#else
	string cmd("ping -c 3 -W 1 ");
	cmd.append(szIPAddress);
	cmd += " | grep received";
	cmd += " | awk -F , '{ print $2 }' | awk '{ print $1 }'";
	vector<string> vResult;
	ExecuteCommand(cmd.c_str(), vResult);
	if(vResult.size() == 0)
		return false;
	if(vResult[0] == "0")
		return false;
	return true; 
#endif 
}


/**
 *  设定的数据块定时周期到达时该函数被调用.
 *  
 *  可以在该函数中向设备发送请求读取实时数据或向设备发送控制指令.
 *  1. 从参数HANDLE_BLOCK中可以获取到数据块信息和设备信息.
  
 *  2. 该函数中需要做如下处理：  
		a) 如果需要定时扫描，则向设备发送请求读取实时数据
		b) 如果需要定时控制，则向设备发送控制命令
		c) 如果是同步方式接收设备数据，则需要在该函数中等待设备数据,直到数据到达或超时返回；
		d) 如果异步方式接收设备数据，则需要提供一个接收数据的回调函数，在回调函数中处理设备返回数据。

 *  3. 当接收到设备返回数据时，解析设备返回数据：
		a) 如果是实时数据，则需要更新数据块的值
		   可以调用驱动EXE提供的回调函数g_drvCBFuncs.pfn_Drv_UpdateBlock或g_drvCBFuncs.pfn_Drv_UpdateBlockEx
		b) 如果是控制命令反馈，则根据需求做相应处理
 *  
 *  @param  -[in]  HANDLE_BLOCK  hBlk: [数据块句柄]
 *
 *  @return PK_SUCCESS: 执行成功
 *
 *  @version     12/11/2008    Initial Version.
 *  @version	10/26/2012  shijunpu  修改由于包计数错误造成启动后一直无法获取块的数据和内存异常问题.
 *  @version	11/2/2012  shijunpu  修改对于ModbusRTU协议，由于可能收到另外站号的包造成解析失败丢弃的问题.
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)//
{
	Drv_LogMessage(PK_LOGLEVEL_DEBUG, "Device(%s) Ontimer Call.", pDevice->szName);

	//
	if (timerInfo->nUserData[0] == TIMER_TYPE_ELAPSE_10S)
	{ 
		//
		char szIp[18] = { 0 };
		sscanf(pDevice->szConnParam, "ip=%[^;];%*s", szIp);
		Drv_LogMessage(PK_LOGLEVEL_INFO, "Doing Ping ip(%s) ... ", szIp);

		if(Ping(szIp))
		{
			Drv_UpdateTagsDataByAddress(pDevice, TAG_ADDRESS_UPDATE_PING, TAG_ADDRESS_RESULT_PING_Y);
		}
		else
		{
			Drv_UpdateTagsDataByAddress(pDevice, TAG_ADDRESS_UPDATE_PING, TAG_ADDRESS_RESULT_PING_N);
		}
	} 	 
	
	return 0;
}
/**
 *  当有控制命令时该函数被调用.
 *  在该函数中根据传递的参数，向设备下发控制命令，如果设备有控制反馈，则可以用同步或异步方式接收控制反馈。
 
 *  1. 从参数HANDLE_BLOCK中可以获取到数据块信息和设备信息.

 *  @param  -[in]  szBinValue，已经转换为二进制了
 *  @param  -[in]  WriteCmdRec  wrec: [控制命令信息]
 *
 *  @version     12/11/2008    Initial Version.
 *  @version	10/26/2012  shijunpu  修改内存异常问题.
 *  @version	11/2/2012  shijunpu  修改对非标modbus，当扩展参数2不为0的时候才用一次写多寄存器指令
 *                                      同时修改控制地址错位问题.
 *  @version	11/5/2012  shijunpu  强制写多个线圈时不一定是8位的整数倍，因此需要额外处理.
 */
// szAddress, 获取到块类型、位、块内地址, AI:01.1
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{
	// 收到一个控制命令szBinValue，把控制值写到对应的变量，发送到设备，然后读取设备的应答（控制的应答，表示控制成功还是失败，以及失败的原因）
	char *szTagName = pTag->szName;
	char *szTagAddress = pTag->szAddress;
 
	return 0;
}
/*
	初始化设备
*/
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
		//定时器5s ,
	PKTIMER timerElapse10S; 
	timerElapse10S.nUserData[0] = TIMER_TYPE_ELAPSE_10S;
	timerElapse10S.nPeriodMS = TIMER_TYPE_ELAPSE_10S;
	Drv_CreateTimer(pDevice, &timerElapse10S);
	Drv_LogMessage(PK_LOGLEVEL_INFO, "InitDevice With Timer-Elapse(%d).", TIMER_TYPE_ELAPSE_10S);
	
	return 0;
}
/*
	反初始化设备
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{	
	Drv_LogMessage(PK_LOGLEVEL_INFO, "Called UnInitDevice...");

	return 0;
}
