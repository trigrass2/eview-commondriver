

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
 *  ���п�������ʱ�ú���������.
 *  �ڸú����и��ݴ��ݵĲ��������豸�·������������豸�п��Ʒ������������ͬ�����첽��ʽ���տ��Ʒ�����
 
 *  1. �Ӳ���HANDLE_BLOCK�п��Ի�ȡ�����ݿ���Ϣ���豸��Ϣ.

 *  @param  -[in]  szBinValue���Ѿ�ת��Ϊ��������
 *  @param  -[in]  WriteCmdRec  wrec: [����������Ϣ]
 *
 *  @version     12/11/2008    Initial Version.
 *  @version	10/26/2012  shijunpu  �޸��ڴ��쳣����.
 *  @version	11/2/2012  shijunpu  �޸ĶԷǱ�modbus������չ����2��Ϊ0��ʱ�����һ��д��Ĵ���ָ��
 *                                      ͬʱ�޸Ŀ��Ƶ�ַ��λ����.
 *  @version	11/5/2012  shijunpu  ǿ��д�����Ȧʱ��һ����8λ���������������Ҫ���⴦��.
 */
// szAddress, ��ȡ�������͡�λ�����ڵ�ַ, AI:01.1
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{
	// �յ�һ����������szBinValue���ѿ���ֵд����Ӧ�ı��������͵��豸��Ȼ���ȡ�豸��Ӧ�𣨿��Ƶ�Ӧ�𣬱�ʾ���Ƴɹ�����ʧ�ܣ��Լ�ʧ�ܵ�ԭ��
	char *szTagName = pTag->szName;
	char *szTagAddress = pTag->szAddress;
 
	return 0;
}
/*
	��ʼ���豸
*/
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
		//��ʱ��5s ,
	PKTIMER timerElapse10S; 
	timerElapse10S.nUserData[0] = TIMER_TYPE_ELAPSE_10S;
	timerElapse10S.nPeriodMS = TIMER_TYPE_ELAPSE_10S;
	Drv_CreateTimer(pDevice, &timerElapse10S);
	Drv_LogMessage(PK_LOGLEVEL_INFO, "InitDevice With Timer-Elapse(%d).", TIMER_TYPE_ELAPSE_10S);
	
	return 0;
}
/*
	����ʼ���豸
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{	
	Drv_LogMessage(PK_LOGLEVEL_INFO, "Called UnInitDevice...");

	return 0;
}
