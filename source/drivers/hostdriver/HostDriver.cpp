/*
 *	电脑信息采集.
 */
#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include "pkdbapi/pkdbapi.h"
#include "Peer2PeerData.h"
#include "iostream"
#include <fstream>  
#include <streambuf>
#include<algorithm>
#include <map>
#include <vector>
#include <cstdio>

using namespace std;
/*
标题	配置地址	地址说明

CPU当前值	cpu:total	CPU当前总值

CPU一段时间的平均值	cpu:total,avg:m1	CPU一分钟平均值
cpu:total,avg:m10	CPU十分钟平均值
cpu:total,avg:m30	CPU三十分钟平均值
cpu:total,avg:m60	CPU六十分钟平均值
cpu:total,avg:h3	CPU三小时平均值
cpu:total,avg:h6	CPU六小时平均值
cpu:total,avg:h12	CPU十二小时平均值
cpu:total,avg:h24	CPU二十四小时平均值

内存当前值	memory:total	Memory当前总值
memory:free	Memory当前空闲值
memory:freepercent	Memory当前空闲率
内存一段时间的空闲值和空闲率	memory:free,avg:m1	Memory一分钟平均空闲值
memory:freepercent,avg:m1	Memory一分钟平均空闲率
memory:free,avg:m10	Memory十分钟平均空闲值
memory:freepercent,avg:m10	Memory十分钟平均空闲率
memory:free,avg:m30	Memory三十分钟平均空闲值
memory:freepercent,avg:m30	Memory三十分钟平均空闲率
memory:free,avg:m60	Memory六十分钟平均空闲值
memory:freepercent,avg:m60	Memory六十分钟平均空闲率
memory:free,avg:h3	Memory三小时平均空闲值
memory:freepercent,avg:h3	Memory三小时平均空闲率
memory:free,avg:h6	Memory六小时平均空闲值
memory:freepercent,avg:h6	Memory六小时平均空闲率
memory:free,avg:h12	Memory十二小时平均空闲值
memory:freepercent,avg:h12	Memory十二小时平均空闲率
memory:free,avg:h24	Memory二十四小时平均空闲值
memory:freepercent,avg:h24	Memory二十四小时平均空闲率

磁盘当前值	disk:total,X	Disk当前总值
disk:free,X	Disk当前空闲值
disk:freepercent,X	Disk当前空闲率

命令控制	ctl:filepath	执行文件命令
命令控制2	ctl:kill	进程终止命令

进程	process:name,Y	进程监控

进程	"SELECT processname\
FROM (\
SELECT\
t_extra_sysmon_host.id\
FROM\
t_device_list,\
t_extra_sysmon_host,\
t_device_driver\
WHERE\
t_device_list.name = t_extra_sysmon_host.name\
AND t_device_driver.id = t_device_list.driver_id\
) T,\
t_extra_sysmon_process\
WHERE\
T.id = t_extra_sysmon_process.host_id ;"	进程监控

代理连接	status	代理连接状态地址

*/

#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif //WIN32

using namespace std;

#define N_USER_DATA_PACKAGE_DATA_LENGTH			0	//默认值

//

CPKDbApi g_db;

void getCurrentTime(char *currentTime)
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); //此函数获得的tm结构体的时间，是已经进行过时区转化为本地时间  
	sprintf(currentTime, "%d-%02d-%02d %02d:%02d:%02d", \
		1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

/*---Request Package Start---*/
int RequestPackage(char* szSendData, unsigned int nDataLength, unsigned short nPortocolCode, map<unsigned short, unsigned int> &mapTypeValue, unsigned short nTypeDriveLetter, map<string, unsigned int> &mapDriveLetterValue)
{
	char *tmp = szSendData;
	//DataLength 
	memcpy(tmp, &nDataLength, 4);
	tmp += 4;
	//PortocolCode 
	memcpy(tmp, &nPortocolCode, 2);
	tmp += 2;
	//Count  
	unsigned short nCount = mapTypeValue.size() + mapDriveLetterValue.size();
	memcpy(tmp, &nCount, 2);
	tmp += 2;
	//Type-Value
	for (map<unsigned short, unsigned int>::iterator item = mapTypeValue.begin(); item != mapTypeValue.end(); item++)
	{
		memcpy(tmp, &(item->first), 2);
		tmp += 2;
		memcpy(tmp, &(item->second), 4);
		tmp += 4;
	}
	//DriveLetterValue
	for (map<string, unsigned int>::iterator item = mapDriveLetterValue.begin(); item != mapDriveLetterValue.end(); item++)
	{
		memcpy(tmp, &nTypeDriveLetter, 2);
		tmp += 2;
		memcpy(tmp, &(item->second), 4);
		tmp += 4;
		unsigned int nLength = strlen(item->first.c_str());
		memcpy(tmp, &nLength, 4);
		tmp += 4;
		strncpy(tmp, item->first.c_str(), nLength);
		tmp += nLength;
	}

	return tmp - szSendData;
}

int RequestPackage(char* szSendData, unsigned int nDataLength, unsigned short nPortocolCode, unsigned short nType, unsigned int nParameterLength, const char* szParameter)
{
	char *tmp = szSendData;
	//DataLength 
	memcpy(tmp, &nDataLength, 4);
	tmp += 4;
	//PortocolCode 
	memcpy(tmp, &nPortocolCode, 2);
	tmp += 2;
	//Type   
	memcpy(tmp, &nType, 2);
	tmp += 2;
	//ParameterLength
	memcpy(tmp, &nParameterLength, 4);
	tmp += 4;
	//Parameter
	strncpy(tmp, szParameter, nParameterLength);
	tmp += nParameterLength;

	return tmp - szSendData;
}

int RequestPackage(char* szSendData, unsigned int nDataLength, unsigned short nPortocolCode, vector<string> &vecParameter)
{
	char *tmp = szSendData;
	//DataLength 
	memcpy(tmp, &nDataLength, 4);
	tmp += 4;
	//PortocolCode 
	memcpy(tmp, &nPortocolCode, 2);
	tmp += 2;
	//TypeCount
	unsigned short nTpyeCount = vecParameter.size();
	memcpy(tmp, &nTpyeCount, 2);
	tmp += 2;
	for (vector<string>::iterator item = vecParameter.begin(); item != vecParameter.end(); item++)
	{ 
		unsigned int nParameterLength = item->length();
		//ParameterLength   
		memcpy(tmp, &nParameterLength, 4);
		tmp += 4;
		//Parameter
		strncpy(tmp, item->c_str(), nParameterLength);
		tmp += nParameterLength;
	}

	return tmp - szSendData;
}
/*---Request Package End---*/

/*---Resolve Start---*/
int ParameterResolve(char* szReceive, unsigned int nParameterLength, char* &szParameter)
{
	const char *tmp = szReceive;
	szParameter = new char[nParameterLength + 1]();
	strncpy(szParameter, tmp, nParameterLength);
	tmp += nParameterLength;
	return tmp - szReceive;
}

int ParameterLengthResolve(char* szReceive, unsigned int &nParameterLength)
{
	nParameterLength = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nParameterLength), tmp, 4);
	tmp += 4;
	return tmp - szReceive;
}

int TypeValueResolve(char* szReceive, unsigned int &nTypeValue)
{
	nTypeValue = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nTypeValue), tmp, 4);
	tmp += 4;
	return tmp - szReceive;
}

int	TypeResolve(char* szReceive, unsigned short &nType)
{
	nType = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nType), tmp, 2);
	tmp += 2;
	return tmp - szReceive;
}

int	TypeCountResolve(char* szReceive, unsigned short &nTypeCount)
{
	nTypeCount = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nTypeCount), tmp, 2);
	tmp += 2;
	return tmp - szReceive;
}

int	ProtocolCodeResolve(char* szReceive, unsigned short &nProcotolCode)
{
	nProcotolCode = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nProcotolCode), tmp, 2);
	tmp += 2;
	return tmp - szReceive;
}

int LengthResolve(char* szReceive, int &nLength)
{
	nLength = 0;
	char *tmp = szReceive;
	memcpy(static_cast<void*>(&nLength), tmp, 4);
	tmp += 4;
	return tmp - szReceive;
}
/*---Resolve End---*/

/*---Receive Start---*/
int ReceiveProcessData(PKDEVICE *pDevice, char* szData, char* szProcessName,int nCount)
{
	char *szTmp = szData;

	unsigned short nStartTime_Year = 0;
	unsigned short nStartTime_Month = 0;
	unsigned short nStartTime_Day = 0;
	unsigned short nStartTime_Hour = 0;
	unsigned short nStartTime_Minute = 0;
	unsigned short nStartTime_Second = 0;
	unsigned int nPid = -1;
	unsigned long long nRunTime = 0;

	memcpy(&nStartTime_Year, szTmp, 2);
	szTmp += 2;
	memcpy(&nStartTime_Month, szTmp, 2);
	szTmp += 2;
	memcpy(&nStartTime_Day, szTmp, 2);
	szTmp += 2;
	memcpy(&nStartTime_Hour, szTmp, 2);
	szTmp += 2;
	memcpy(&nStartTime_Minute, szTmp, 2);
	szTmp += 2;
	memcpy(&nStartTime_Second, szTmp, 2);
	szTmp += 2;

	memcpy(&nPid, szTmp, 4);
	szTmp += 4;

	memcpy(&nRunTime, szTmp, 8);
	szTmp += 8;

	char szGetid[1024] = { 0 };
	sprintf(szGetid, "SELECT t_extra_sysmon_host.id FROM t_extra_sysmon_host WHERE t_extra_sysmon_host.name = '%s';", pDevice->szName);

    vector<vector<string> > vecsTablesID;
	int nRetGetid = g_db.SQLExecute(szGetid, vecsTablesID);	
	if (nRetGetid != 0)
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Execute Sql(%s) Failed, Error(%d).", pDevice->szName,szGetid, nRetGetid);
	nRetGetid = 0;
	if(vecsTablesID.size() > 0 && vecsTablesID[0].size() > 0)
		sscanf(vecsTablesID[0][0].c_str(), "%d", &nRetGetid);
	//string sqlDrop = "DROP TABLE IF EXISTS t_sysmon_process;";
	//string sqlCreate = "";
	//sqlCreate.append("CREATE TABLE t_sysmon_process( ");
	//sqlCreate.append(" id            int(11),");
	//sqlCreate.append(" host_id       int(11),");
	//sqlCreate.append(" progressname  VARCHAR(255),");
	//sqlCreate.append(" displayname   VARCHAR(255),");
	//sqlCreate.append(" starttime     VARCHAR(32),");
	//sqlCreate.append(" runstatus     int(11),");
	//sqlCreate.append(" updatetime    VARCHAR(32) NOT NULL, PRIMARY KEY (updatetime)");
	//sqlCreate.append(");");

	char sz[1024 * 2] = { 0 };
	//sprintf(sz, "SELECT COUNT(*) FROM t_sysmon_process WHERE progressname = '%s';", szProcessName);
	//vector<vector<string>> vecsTables;
	//long nRet = g_db.SQLExecute(sz, vecsTables);
	//if (vecsTables[0][0].compare("0") == 0)
	//{	
	//	sprintf(sz, "INSERT INTO t_sysmon_process(id, progressname, displayname, starttime, runstatus, updatetime) VALUES(%d, '%s', '%s', '%d/%d/%d %d:%d:%d', 0, datetime('now','localtime'));",
	//		nPid, szProcessName, szProcessName, nStartTime_Year, nStartTime_Month, nStartTime_Day, nStartTime_Hour, nStartTime_Minute, nStartTime_Second);
	//}
	//else
	{
		char szCurrentTime[128] = {0};
		getCurrentTime(szCurrentTime);
		char szProcess[256]={0};
		sprintf(szProcess,"process_%d",nCount);
	
		if(nPid <= 0)
		{
			sprintf(sz, "UPDATE t_extra_sysmon_process SET process_id = %d, displayname = '%s', starttime = '%d/%d/%d %d:%d:%d', runstatus = 0, updatetime = '%s' WHERE processname = '%s' AND host_id = %d ;",
				nPid, szProcessName, nStartTime_Year, nStartTime_Month, nStartTime_Day, nStartTime_Hour, nStartTime_Minute, nStartTime_Second, szCurrentTime, szProcessName, nRetGetid);
		
			Drv_UpdateTagsDataByAddress(pDevice,szProcess,"0");
		}
		else
		{
			sprintf(sz, "UPDATE t_extra_sysmon_process SET process_id = %d, displayname = '%s', starttime = '%d/%d/%d %d:%d:%d', runstatus = 1, updatetime = '%s' WHERE processname = '%s' AND host_id = %d ;",
				nPid, szProcessName, nStartTime_Year, nStartTime_Month, nStartTime_Day, nStartTime_Hour, nStartTime_Minute, nStartTime_Second, szCurrentTime, szProcessName, nRetGetid);
			Drv_UpdateTagsDataByAddress(pDevice, szProcess, "1");
		}
	}	
	long nRet = g_db.SQLExecute(sz);
	if (nRet != 0)
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Execute Sql(%s) Failed, Error(%d).", pDevice->szName,sz, nRet);

	return szTmp - szData;
}

int ReceiveDiskData(PKDEVICE *pDevice, char* szData, unsigned int nTypeValue, char* szParameter)
{
	char *szTmp = szData;

	char sz[16] = { 0 };
	if ((nTypeValue & CALC_PRT_ALL) == CALC_PRT_ALL)
	{
		ULLong nTotal = 0;
		memcpy(&nTotal, szTmp, 8);
		szTmp += 8;

		string strDriveLetterHeader = ADDR_DISK_TOTAL + string(",");
		vector<PKTAG *> vecTag_DriveLetter_PRT_ALL;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), vecTag_DriveLetter_PRT_ALL) > 0)
		{
			sprintf(sz, "%.2lf", ((nTotal * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_DriveLetter_PRT_ALL[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_DriveLetter_PRT_ALL);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
	}
	if ((nTypeValue & CALC_PRT_V) == CALC_PRT_V)
	{
		ULLong nFree = 0;
		memcpy(&nFree, szTmp, 8);
		szTmp += 8;

		string strDriveLetterHeader = ADDR_DISK_FREE + string(",");
		vector<PKTAG *> vecTag_DriveLetter_PRT_V;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), vecTag_DriveLetter_PRT_V) > 0)
		{
			sprintf(sz, "%.2lf", ((nFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_DriveLetter_PRT_V[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_DriveLetter_PRT_V);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
	}
	if ((nTypeValue & CALC_PRT_VP) == CALC_PRT_VP)
	{
		double nFreePercent = 0;
		memcpy(&nFreePercent, szTmp, 8);
		szTmp += 8;

		string strDriveLetterHeader = ADDR_DISK_FREEPERCENT + string(",");
		vector<PKTAG *> vecTag_DriveLetter_PRT_VP;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), vecTag_DriveLetter_PRT_VP) > 0)
		{
			sprintf(sz, "%.2lf", nFreePercent);//%
			vecTag_DriveLetter_PRT_VP[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_DriveLetter_PRT_VP);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update DriveLetter Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (char*)(strDriveLetterHeader + string(szParameter)).c_str(), sz);
	}

	return szTmp - szData;
}

int ReceiveMemoryData(PKDEVICE *pDevice, char* szData, unsigned int nTypeValue)
{
	char *szTmp = szData;

	char sz[16] = { 0 };
	if ((nTypeValue & CALC_PRT_ALL) == CALC_PRT_ALL)
	{
		ULLong nTotal = 0;
		memcpy(&nTotal, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_PRT_ALL;
		if (Drv_GetTagsByAddr(pDevice, ADDR_MEMORY_TOTAL, vecTag_Memory_PRT_ALL) > 0)
		{
			sprintf(sz, "%.2lf", ((nTotal * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_PRT_ALL[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_PRT_ALL);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, ADDR_MEMORY_TOTAL, sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, ADDR_MEMORY_TOTAL, sz);
	}
	if ((nTypeValue & CALC_PRT_V) == CALC_PRT_V)
	{
		ULLong nFree = 0;
		memcpy(&nFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_PRT_V;
		if (Drv_GetTagsByAddr(pDevice, ADDR_MEMORY_FREE, vecTag_Memory_PRT_V) > 0)
		{
			sprintf(sz, "%.2lf", ((nFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_PRT_V[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_PRT_V);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, ADDR_MEMORY_FREE, sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, ADDR_MEMORY_FREE, sz);
	}
	if ((nTypeValue & CALC_PRT_VP) == CALC_PRT_VP)
	{
		double nFreePercent = 0;
		memcpy(&nFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_PRT_VP;
		if (Drv_GetTagsByAddr(pDevice, ADDR_MEMORY_FREEPERCENT, vecTag_Memory_PRT_VP) > 0)
		{
			sprintf(sz, "%.2lf", nFreePercent);//%
			vecTag_Memory_PRT_VP[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_PRT_VP);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, ADDR_MEMORY_FREEPERCENT, sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, ADDR_MEMORY_FREEPERCENT, sz);
	}

	ULLong nAvgFree = 0;
	double nAvgFreePercent = 0;
	string strMemoryAvgFreeHeader = ADDR_MEMORY_FREE + string(",");
	string strMemoryAvgFreePercentHeader = ADDR_MEMORY_FREEPERCENT + string(",");
	if ((nTypeValue & CALC_AVG_V_M1) == CALC_AVG_V_M1)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_M1;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_M1).c_str(), vecTag_Memory_AVG_V_M1) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_M1[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_M1);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M1).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M1).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M1) == CALC_AVG_VP_M1)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_M1;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_M1).c_str(), vecTag_Memory_AVG_VP_M1) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_M1[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_M1);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M1).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M1).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_M10) == CALC_AVG_V_M10)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_M10;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_M10).c_str(), vecTag_Memory_AVG_V_M10) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_M10[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_M10);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M10).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M10).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M10) == CALC_AVG_VP_M10)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_M10;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_M10).c_str(), vecTag_Memory_AVG_VP_M10) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_M10[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_M10);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M10).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M10).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_M30) == CALC_AVG_V_M30)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_M30;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_M30).c_str(), vecTag_Memory_AVG_V_M30) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_M30[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_M30);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M30).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M30).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M30) == CALC_AVG_VP_M30)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_M30;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_M30).c_str(), vecTag_Memory_AVG_VP_M30) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_M30[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_M30);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M30).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M30).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_M60) == CALC_AVG_V_M60)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_M60;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_M60).c_str(), vecTag_Memory_AVG_V_M60) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_M60[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_M60);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M60).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_M60).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M60) == CALC_AVG_VP_M60)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_M60;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_M60).c_str(), vecTag_Memory_AVG_VP_M60) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_M60[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_M60);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M60).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_M60).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_H3) == CALC_AVG_V_H3)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_H3;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_H3).c_str(), vecTag_Memory_AVG_V_H3) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_H3[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_H3);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H3).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H3).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H3) == CALC_AVG_VP_H3)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_H3;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_H3).c_str(), vecTag_Memory_AVG_VP_H3) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_H3[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_H3);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H3).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H3).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_H6) == CALC_AVG_V_H6)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_H6;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_H6).c_str(), vecTag_Memory_AVG_V_H6) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_H6[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_H6);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H6).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H6).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H6) == CALC_AVG_VP_H6)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_H6;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_H6).c_str(), vecTag_Memory_AVG_VP_H6) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_H6[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_H6);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H6).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H6).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_H12) == CALC_AVG_V_H12)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_H12;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_H12).c_str(), vecTag_Memory_AVG_V_H12) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_H12[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_H12);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H12).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H12).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H12) == CALC_AVG_VP_H12)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_H12;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_H12).c_str(), vecTag_Memory_AVG_VP_H12) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_H12[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_H12);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H12).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H12).c_str(), sz);
	}

	if ((nTypeValue & CALC_AVG_V_H24) == CALC_AVG_V_H24)
	{
		memcpy(&nAvgFree, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_V_H24;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreeHeader + CALC_AVG_A_H24).c_str(), vecTag_Memory_AVG_V_H24) > 0)
		{
			sprintf(sz, "%.2lf", ((nAvgFree * 1.0) / 1024.0 / 1024.0));//GB
			vecTag_Memory_AVG_V_H24[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_V_H24);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H24).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreeHeader + CALC_AVG_A_H24).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H24) == CALC_AVG_VP_H24)
	{
		memcpy(&nAvgFreePercent, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_Memory_AVG_VP_H24;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strMemoryAvgFreePercentHeader + CALC_AVG_A_H24).c_str(), vecTag_Memory_AVG_VP_H24) > 0)
		{
			sprintf(sz, "%.2lf", nAvgFreePercent);//%
			vecTag_Memory_AVG_VP_H24[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_Memory_AVG_VP_H24);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Memory Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H24).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Memory Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strMemoryAvgFreePercentHeader + CALC_AVG_A_H24).c_str(), sz);
	}

	return szTmp - szData;
}

int ReceiveCpuData(PKDEVICE *pDevice, char* szData, unsigned int nTypeValue)
{
	char *szTmp = szData;

	char sz[16] = { 0 };
	if ((nTypeValue & CALC_PRT_ALL) == CALC_PRT_ALL)
	{
		double nUtilization = 0;
		memcpy(&nUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_PRT_ALL;
		if (Drv_GetTagsByAddr(pDevice, ADDR_CPU_TOTAL, vecTag_CPU_PRT_ALL) > 0)
		{
			sprintf(sz, "%.2lf", nUtilization);
			vecTag_CPU_PRT_ALL[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_PRT_ALL);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, ADDR_CPU_TOTAL, sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, ADDR_CPU_TOTAL, sz);
	}

	double nAvgUtilization = 0;
	string strCpuAvgHeader = ADDR_CPU_TOTAL + string(",");
	if ((nTypeValue & CALC_AVG_VP_M1) == CALC_AVG_VP_M1)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_M1;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_M1).c_str(), vecTag_CPU_AVG_VP_M1) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_M1[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_M1);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M1).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M1).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M10) == CALC_AVG_VP_M10)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_M10;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_M10).c_str(), vecTag_CPU_AVG_VP_M10) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_M10[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_M10);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M10).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M10).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M30) == CALC_AVG_VP_M30)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_M30;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_M30).c_str(), vecTag_CPU_AVG_VP_M30) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_M30[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_M30);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M30).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M30).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_M60) == CALC_AVG_VP_M60)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_M60;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_M60).c_str(), vecTag_CPU_AVG_VP_M60) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_M60[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_M60);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M60).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_M60).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H3) == CALC_AVG_VP_H3)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_H3;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_H3).c_str(), vecTag_CPU_AVG_VP_H3) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_H3[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_H3);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H3).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H3).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H6) == CALC_AVG_VP_H6)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_H6;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_H6).c_str(), vecTag_CPU_AVG_VP_H6) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_H6[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_H6);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H6).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H6).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H12) == CALC_AVG_VP_H12)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_H12;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_H12).c_str(), vecTag_CPU_AVG_VP_H12) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_H12[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_H12);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H12).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H12).c_str(), sz);
	}
	if ((nTypeValue & CALC_AVG_VP_H24) == CALC_AVG_VP_H24)
	{
		memcpy(&nAvgUtilization, szTmp, 8);
		szTmp += 8;

		vector<PKTAG *> vecTag_CPU_AVG_VP_H24;
		if (Drv_GetTagsByAddr(pDevice, (char*)(strCpuAvgHeader + CALC_AVG_A_H24).c_str(), vecTag_CPU_AVG_VP_H24) > 0)
		{
			sprintf(sz, "%.2lf", nAvgUtilization);
			vecTag_CPU_AVG_VP_H24[0]->SetVTQ_Ascii(sz, 0, 0, 0, NULL);
			Drv_UpdateTagsData(pDevice, vecTag_CPU_AVG_VP_H24);
			Drv_LogMessage(PK_LOGLEVEL_INFO, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Success.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H24).c_str(), sz);
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "Update Cpu Device(%s) TagAddress(%s) Value(%s) Failed.", pDevice->szName, (strCpuAvgHeader + CALC_AVG_A_H24).c_str(), sz);
	}

	return szTmp - szData;
}
/*---Receive End---*/

/*---Request Start---*/
/*
	请求协议
		头:1 长度:4 协议码:2 进程个数:2 进程名长度:2 进程名值:? ...
*/
int RequestProcessData(PKDEVICE *pDevice, vector<string> &vecProcessName)
{
	int totalLength = PACKHEADERLENGTH + 2 + 2;//加 (协议码 + 进程个数)4字节 
	for (vector<string>::iterator iProcess = vecProcessName.begin(); iProcess != vecProcessName.end(); iProcess++)
	{
		totalLength += 4;//加 (进程名长度)4字节
		totalLength += iProcess->length();//加 进程名值长度
	}
	char *szSend = new char[totalLength]();
	//header
	szSend[0] = HEADER;
	RequestPackage(&szSend[1], totalLength - PACKHEADERLENGTH, PROTOCOL_CODE_P, vecProcessName);

	//if(SOCKCLI_GetConnectStat(g_pRemoteDevice) == 0)
	//{
	//	printf("SOCKCLI_DISCONNECTED\n");
	//	return 0;
	//}

	int ret = Drv_Send(pDevice, szSend, totalLength, 1024);
	if (ret <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Sendprocess(%s) Request(%d) Failed", pDevice->szName, totalLength);
	else
		Drv_LogMessage(PK_LOGLEVEL_INFO, "Sendprocess(%s) Request(%d) Success", pDevice->szName, totalLength);
	delete[] szSend;
	return ret;
}

/*
	请求协议
		头:1 长度:4 协议码:2 类型个数:2 类型a:2 类型a值:4 类型b:2 类型b值:4 ... 类型x:2 类型x值:4 类型x参数长度:4 类型x参数:? ...
	说明
		1.根据类型判断是否有类型参数.
		2.类型值是"位值"看做Int类型，请求时按位做 或"|"运算.
*/
int RequestCPUMemoryDiskData(PKDEVICE* pDevice, map<unsigned short, unsigned int> &mapTypeValue, map<string, unsigned int> &mapDriveLetterValue)
{
	int totalLength = PACKHEADERLENGTH + 2 + 2;//加 (协议码 + 类型个数)4字节
	totalLength += mapTypeValue.size() * (2 + 4);//加 类型个数 * (类型+类型值)6字节
	for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
	{
		totalLength += (2 + 4 + 4);//加 (类型+类型值)6字节 + 类型参数长度4字节
		totalLength += strlen(iDriveLetter->first.c_str());//加 参数值长度
	}
	char *szSend = new char[totalLength]();
	//header
	szSend[0] = HEADER;
	RequestPackage(&szSend[1], totalLength - PACKHEADERLENGTH, PROTOCOL_CODE_R, mapTypeValue, TYPE_DRIVE, mapDriveLetterValue);

	//if(SOCKCLI_GetConnectStat(g_pRemoteDevice) == 0)
	//{
	//	printf("SOCKCLI_DISCONNECTED\n");
	//	return 0;
	//}

	int ret = Drv_Send(pDevice, szSend, totalLength, 1024);
	if (ret <= 0)
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "Send(Device:%s) RequestCPUMemoryDiskData(%d) Failed", pDevice->szName, totalLength);
	else
        Drv_LogMessage(PK_LOGLEVEL_INFO, "Send(Device:%s) RequestCPUMemoryDiskData(%d) Success", pDevice->szName, totalLength);
	delete[] szSend;
	return ret;
}

/*
	请求协议
		头:1 长度:4 协议码:2 控制类型:2 控制类型参数长度:4 控制类型参数:? 
*/
int RequestControl(PKDEVICE *pDevice, unsigned short nType, const char* szParameter)
{
	int totalLength = PACKHEADERLENGTH + 2;//加 协议码2字节
	totalLength += 2;//加 (控制类型)2字节 
	totalLength += 4;//加 (控制类型参数长度)4字节 
	unsigned int nParameterLength = strlen(szParameter);
	totalLength += nParameterLength;//加 (参数值长度)nParameterLength字节 

	char *szSend = new char[totalLength]();
	//header
	szSend[0] = HEADER;
	RequestPackage(&szSend[1], totalLength - PACKHEADERLENGTH, PROTOCOL_CODE_C, nType, nParameterLength, szParameter);

	//if(SOCKCLI_GetConnectStat(g_pRemoteDevice) == 0)
	//{
	//	printf("SOCKCLI_DISCONNECTED\n");
	//	return 0;
	//}

	int ret = Drv_Send(pDevice, szSend, totalLength, 1024);
	if (ret <= 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Send(%s) control request length(%d) Failed", pDevice->szName, totalLength);
	else
		Drv_LogMessage(PK_LOGLEVEL_INFO, "Send(%s) control request length(%d) Success", pDevice->szName, totalLength);
	delete[] szSend;
	return ret;
}
/*---Request End---*/


int PackageResolve(PKDEVICE *pDevice, char* szPackageData, int nPackageLength)
{
	char *szReceiveBuf = szPackageData;
	//Header
	szReceiveBuf++;

	//Length
	int nLength;
	szReceiveBuf += LengthResolve(szReceiveBuf, nLength);

	//ProtocolCode
	unsigned short nProtocolCode;
	szReceiveBuf += ProtocolCodeResolve(szReceiveBuf, nProtocolCode);

	//TypeCount
	unsigned short nTypeCount;
	szReceiveBuf += TypeCountResolve(szReceiveBuf, nTypeCount);

	if (nProtocolCode == PROTOCOL_CODE_R)
	{
		while (nTypeCount-- > 0)
		{
			unsigned short nType = 0;
			szReceiveBuf += TypeResolve(szReceiveBuf, nType);
			unsigned int nTypeValue = 0;
			szReceiveBuf += TypeValueResolve(szReceiveBuf, nTypeValue);

			switch (nType)
			{
			case TYPE_CPU:
				szReceiveBuf += ReceiveCpuData(pDevice, szReceiveBuf, nTypeValue);
				break;
			case TYPE_MEMORY:
				szReceiveBuf += ReceiveMemoryData(pDevice, szReceiveBuf, nTypeValue);
				break;
			case TYPE_DRIVE:
			{
				unsigned int nParameterLength = 0;
				szReceiveBuf += ParameterLengthResolve(szReceiveBuf, nParameterLength);
				char *szParameter = { 0 };
				szReceiveBuf += ParameterResolve(szReceiveBuf, nParameterLength, szParameter);
				szReceiveBuf += ReceiveDiskData(pDevice, szReceiveBuf, nTypeValue, szParameter);
				delete[] szParameter;
			}
			break;
			}
		}
	}
	else if (nProtocolCode == PROTOCOL_CODE_P)
	{
		int nCount=0;
		char szGetid[1024] = { 0 };
		sprintf(szGetid, "SELECT  * from  t_extra_sysmon_process where t_extra_sysmon_process.host_id  = \
	    (SELECT t_extra_sysmon_host.id FROM t_extra_sysmon_host WHERE t_extra_sysmon_host.name = '%s');", pDevice->szName);
		vector<vector<string> > vecsTablesID;
		int nRetGetid = g_db.SQLExecute(szGetid, vecsTablesID);	
		int nTotal=vecsTablesID.size();


		while (nTypeCount-- > 0)
		{
			unsigned int nParameterLength = 0;
			szReceiveBuf += ParameterLengthResolve(szReceiveBuf, nParameterLength);
			char *szParameter = { 0 };
			szReceiveBuf += ParameterResolve(szReceiveBuf, nParameterLength, szParameter);
			nCount++;
			if(nCount>nTotal)
				nCount=0;
			szReceiveBuf += ReceiveProcessData(pDevice, szReceiveBuf, szParameter,nCount);
			delete[] szParameter;
		}
	}
	return 0;
}

int ReceiveFromRemoteDevice(PKDEVICE *pDevice)
{
	int nPackageDataLength = 0;
	char *szPackageCleaner = NULL;
	char *szPackageIterator = NULL;
	do
	{
		char *szReceiveArray = new char[RECEIVED_BUFFER_LENGTH]();
		int nReceiveLength = RECEIVED_BUFFER_LENGTH;
		nReceiveLength = Drv_Recv(pDevice, szReceiveArray, nReceiveLength, 1024 * 2);
		if (nReceiveLength <= 0)
		{
            Drv_LogMessage(PK_LOGLEVEL_DEBUG, nReceiveLength == 0 ? "Device:%s, Drv_Recv Done(%d)." : "Device:%s, Drv_Recv Time-Out(%d)...", pDevice->szName, nReceiveLength);
			delete[] szReceiveArray;
			break;
		}
        Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device:%s, Drv_Recv (%d) Bytes", pDevice->szName, nReceiveLength);

		char *szReceive = szReceiveArray;

		do
		{
			if (nPackageDataLength == 0)
			{
				if (szReceive[0] != HEADER)
				{
                    Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Package Data Header-Analysis Error, Size(%d).", pDevice->szName, nReceiveLength);
					break;
				}
				if (nReceiveLength < PACKHEADERLENGTH)
				{
                    Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Package Data Length-Analysis Error, Size(%d).", pDevice->szName, nReceiveLength);
					break;
				}

				//offset HEADER length
				char *szPackageDataLength = szReceive + sizeof(char);

				//解析包数据的长度(不包含包头长度)
				LengthResolve(szPackageDataLength, nPackageDataLength);
				//new Packdata Room
				szPackageCleaner = new char[nPackageDataLength + PACKHEADERLENGTH + 1]();
				szPackageIterator = szPackageCleaner;
				//Copy (HEADER + DataLength)
				memcpy(szPackageIterator, szReceive, PACKHEADERLENGTH);
				szPackageIterator += PACKHEADERLENGTH;

				szReceive += PACKHEADERLENGTH;
				nReceiveLength -= PACKHEADERLENGTH;
			}
			//已接收的包数据长度 < 包数据长度 = 一个不完整的包 
			if (nReceiveLength < nPackageDataLength)
			{
				memcpy(szPackageIterator, szReceive, nReceiveLength);

				szPackageIterator += nReceiveLength;
				nPackageDataLength -= nReceiveLength;

				szReceive += nReceiveLength;
				nReceiveLength -= nReceiveLength;
			}
			else//(nReceiveLength >= nPackageDataLength)
			{
				//已接收的包数据长度 == 包数据长度 = 一个完整的包  
				//已接收的包数据长度 > 包数据长度  = 至少有一个完整的包 + 至少一个数据片段 
				memcpy(szPackageIterator, szReceive, nPackageDataLength);
				//szPackageIterator += nPackageDataLength;
				//Package Whole Length
				PackageResolve(pDevice, szPackageCleaner, (szPackageIterator - szPackageCleaner) + nPackageDataLength);

				szReceive += nPackageDataLength;
				nReceiveLength -= nPackageDataLength;

				nPackageDataLength = 0;

				delete[] szPackageCleaner;
				szPackageCleaner = NULL;
			}

		} while (nReceiveLength > 0);

		delete[] szReceiveArray;

#ifdef WIN32
		Sleep(RECEIVE_SLEEP_MICROSECOND);
#else
		usleep(RECEIVE_SLEEP_MICROSECOND);
#endif //WIN32

	} while (true);//Receiving

	if (szPackageCleaner != NULL)
		delete[] szPackageCleaner;

	return 0;
}

int ReceiveFromMultiRemoteDevice(PKDEVICE *pDevice)
{
	do
	{
		//Received Buffer
		char *szReceiveArray = new char[RECEIVED_BUFFER_LENGTH]();
		//Received Length
		int nReceiveLength = RECEIVED_BUFFER_LENGTH;
		nReceiveLength = Drv_Recv(pDevice, szReceiveArray, nReceiveLength, 1024 * 2);
		if (nReceiveLength <= 0)
		{
            Drv_LogMessage(PK_LOGLEVEL_DEBUG, nReceiveLength == 0 ? "Device:%s Drv_Received Done(%d)." : "Device:%s, Drv_Received Time-Out(%d)...", pDevice->szName, nReceiveLength);
			delete[] szReceiveArray;
			break;
		}
        Drv_LogMessage(PK_LOGLEVEL_INFO, "Device:%s, Drv_Received (%d) Bytes.", pDevice->szName,nReceiveLength);
		//Marked Clean Pointer
		char *szReceive = szReceiveArray;
		//
		do
		{
			//Data Package Length
			if (pDevice->nUserData[0] == 0)
			{
				if (szReceive[0] != HEADER)
				{
                    Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Package Data Header-Analysis Error, Size(%d).", pDevice->szName,nReceiveLength);
					break;
				}
				if (nReceiveLength < PACKHEADERLENGTH)
				{
					//Here Maybe a Problem:
					//When Full Package Additial with a Pices of Next Package which Length Less than PACKHEADERLENGTH
					//The Data Will Be Abandon
                    Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s,Package Data Length-Analysis Error, Size(%d).", pDevice->szName,nReceiveLength);
					break;
				}

				//offset HEADER length
				char *szOffset2DataLength = szReceive + sizeof(char);

				//Resolve Data Length
				LengthResolve(szOffset2DataLength, pDevice->nUserData[0]);
				//Save Package Length
				pDevice->nUserData[1] = pDevice->nUserData[0] + PACKHEADERLENGTH;
				//New Package Data Room
				pDevice->pUserData[0] = new char[pDevice->nUserData[1] + 1]();
				//Mark Movable Pointer for Copy Data
				pDevice->pUserData[1] = pDevice->pUserData[0];
				//Copy Data(HEADER + DataLength)
				memcpy(pDevice->pUserData[1], szReceive, PACKHEADERLENGTH);
				//Move Pointer
				pDevice->pUserData[1] = (char*)pDevice->pUserData[1] + PACKHEADERLENGTH;
				//Shrink Received Data
				szReceive += PACKHEADERLENGTH;
				nReceiveLength -= PACKHEADERLENGTH;
			}
			//已接收的包数据长度 < 包数据长度 = 一个不完整的包 
			if (nReceiveLength < pDevice->nUserData[0])
			{
				//Copy Data(Received)
				memcpy(pDevice->pUserData[1], szReceive, nReceiveLength);
				//Move Pointer
				pDevice->pUserData[1] = (char*)pDevice->pUserData[1] + nReceiveLength;
				//Cut down Package Data Length
				pDevice->nUserData[0] -= nReceiveLength;
				//Shrink Received Data
				szReceive += nReceiveLength;
				nReceiveLength -= nReceiveLength;
			}
			else//(nReceiveLength >= pDevice->nUserData[0])
			{
				//已接收的包数据长度 == 包数据长度 = 一个完整的包  
				//已接收的包数据长度 > 包数据长度  = 至少有一个完整的包 + 至少一个数据片段 
				memcpy(pDevice->pUserData[1], szReceive, pDevice->nUserData[0]);
				//pDevice->pUserData[1] += pDevice->nUserData[0];
				//Package Whole Length
				PackageResolve(pDevice, (char*)pDevice->pUserData[0], pDevice->nUserData[1]);
				//Shrink Received Data
				szReceive += pDevice->nUserData[0];
				nReceiveLength -= pDevice->nUserData[0];
				//Clean Device Received Data
				pDevice->nUserData[0] = pDevice->nUserData[1] = N_USER_DATA_PACKAGE_DATA_LENGTH;
                delete[] (char *)pDevice->pUserData[0];
				pDevice->pUserData[0] = pDevice->pUserData[1] = NULL;
			}
		} while (nReceiveLength > 0);
		//Clean Received Data
		delete[] szReceiveArray;
		//Sleep a while
#ifdef WIN32
		Sleep(RECEIVE_SLEEP_MICROSECOND);
#else
		usleep(RECEIVE_SLEEP_MICROSECOND);
#endif //WIN32
	} while (true);//Receiving
	
	return 0;
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
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	Drv_LogMessage(PK_LOGLEVEL_INFO, "Device(%s) Call OnTimer (%d) Start...", pDevice->szName, timerInfo->nPhaseMS);
	int nTagsCount = pDevice->nTagNum;
	if (nTagsCount == 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s) has no tags configed.", pDevice->szName);
		return 0;
	}

	unsigned int nCpuCombineValue = 0x0;
	unsigned int nMemoryCombineValue = 0x0;
	map<string, unsigned int> mapDriveLetterValue;
	vector<string> vecProcesses;

#pragma region Address Analysis
	for (int iTag = 0; iTag < nTagsCount; iTag++)
	{
		const char *szAddress = pDevice->vecTags[iTag]->szAddress;
		string strAddress = szAddress;
		//string strAddress(pDevice->vecTags[iTag]->szAddress);
		string::size_type iComma = strAddress.find_first_of(',');
		if (iComma == string::npos) // 一段式，具体见文件头。如：cpu:total,memory:total		memory:free		,memory:freepercent
		{
#pragma region CPU
			if (strcmp(szAddress, ADDR_CPU_TOTAL) == 0)
				nCpuCombineValue |= CALC_PRT_ALL;
#pragma endregion
#pragma region Memory
			else if (strcmp(szAddress, ADDR_MEMORY_TOTAL) == 0)
				nMemoryCombineValue |= CALC_PRT_ALL;
			else if (strcmp(szAddress, ADDR_MEMORY_FREE) == 0)
				nMemoryCombineValue |= CALC_PRT_V;
			else if (strcmp(szAddress, ADDR_MEMORY_FREEPERCENT) == 0)
				nMemoryCombineValue |= CALC_PRT_VP;
#pragma endregion
            else if(strcmp(szAddress, ADDR_DEVICE_CONN_STATUS) == 0) // one segment: connstatus
            {
            }
            else
            {
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Tag(%s),Header-Address(%s) Invalid.",
				pDevice->szName, pDevice->vecTags[iTag]->szName, pDevice->vecTags[iTag]->szAddress);
            }
		}
		else // 逗号隔开的两段式地址，如：cpu:total,avg:m1
		{
			string strHeaderAddressFromCombine = strAddress.substr(0, iComma);
			string strSubAddressFromCombine = strAddress.substr(iComma + 1);
#pragma region CPU
			if (strHeaderAddressFromCombine.compare(ADDR_CPU_TOTAL) == 0) // cpu:total,avg:h12
			{
				if (strSubAddressFromCombine.compare(CALC_AVG_A_M1) == 0)
					nCpuCombineValue |= CALC_AVG_VP_M1;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M10) == 0)
					nCpuCombineValue |= CALC_AVG_VP_M10;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M30) == 0)
					nCpuCombineValue |= CALC_AVG_VP_M30;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M60) == 0)
					nCpuCombineValue |= CALC_AVG_VP_M60;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H3) == 0)
					nCpuCombineValue |= CALC_AVG_VP_H3;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H6) == 0)
					nCpuCombineValue |= CALC_AVG_VP_H6;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H12) == 0)
					nCpuCombineValue |= CALC_AVG_VP_H12;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H24) == 0)
					nCpuCombineValue |= CALC_AVG_VP_H24;
				else
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Tag(%s),CPU-Address(%s) Invalid.",
					pDevice->szName, pDevice->vecTags[iTag]->szName, pDevice->vecTags[iTag]->szAddress);
			}
#pragma endregion
#pragma region Memory
			else if (strHeaderAddressFromCombine.compare(ADDR_MEMORY_FREE) == 0) // memory:free,avg:m1
			{
				if (strSubAddressFromCombine.compare(CALC_AVG_A_M1) == 0)
					nMemoryCombineValue |= CALC_AVG_V_M1;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M10) == 0)
					nMemoryCombineValue |= CALC_AVG_V_M10;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M30) == 0)
					nMemoryCombineValue |= CALC_AVG_V_M30;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M60) == 0)
					nMemoryCombineValue |= CALC_AVG_V_M60;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H3) == 0)
					nMemoryCombineValue |= CALC_AVG_V_H3;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H6) == 0)
					nMemoryCombineValue |= CALC_AVG_V_H6;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H12) == 0)
					nMemoryCombineValue |= CALC_AVG_V_H12;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H24) == 0)
					nMemoryCombineValue |= CALC_AVG_V_H24;
				else
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Tag(%s), MemoryFree-Address(%s) Invalid.",
					pDevice->szName, pDevice->vecTags[iTag]->szName, pDevice->vecTags[iTag]->szAddress);
			}
			else if (strHeaderAddressFromCombine.compare(ADDR_MEMORY_FREEPERCENT) == 0) // memory:freepercent,avg:m1
			{ 
				if (strSubAddressFromCombine.compare(CALC_AVG_A_M1) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_M1;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M10) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_M10;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M30) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_M30;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_M60) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_M60;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H3) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_H3;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H6) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_H6;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H12) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_H12;
				else if (strSubAddressFromCombine.compare(CALC_AVG_A_H24) == 0)
					nMemoryCombineValue |= CALC_AVG_VP_H24;
				else
					Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Tag(%s), MemoryFreePercent-Address(%s) Invalid.",
					pDevice->szName, pDevice->vecTags[iTag]->szName, pDevice->vecTags[iTag]->szAddress);
			}
#pragma endregion
#pragma region Disk
			else if (strHeaderAddressFromCombine.compare(ADDR_DISK_TOTAL) == 0) // disk:total,X
			{
				bool isExistDriveLetter = false;
				for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
				{
					if (iDriveLetter->first.compare(strSubAddressFromCombine) == 0)
					{
						isExistDriveLetter = true;
						iDriveLetter->second |= CALC_PRT_ALL;
						break;
					}
				}
				if (!isExistDriveLetter)
					mapDriveLetterValue.insert(pair<string, unsigned int>(strSubAddressFromCombine, CALC_PRT_ALL));
			}
			else if (strHeaderAddressFromCombine.compare(ADDR_DISK_FREE) == 0) // disk:free,X
			{
				bool isExistDriveLetter = false;
				for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
				{
					if (iDriveLetter->first.compare(strSubAddressFromCombine) == 0)
					{
						isExistDriveLetter = true;
						iDriveLetter->second |= CALC_PRT_V;
						break;
					}
				}
				if (!isExistDriveLetter)
					mapDriveLetterValue.insert(pair<string, unsigned int>(strSubAddressFromCombine, CALC_PRT_V));
			}
			else if (strHeaderAddressFromCombine.compare(ADDR_DISK_FREEPERCENT) == 0) // disk:freepercent,X
			{
				bool isExistDriveLetter = false;
				for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
				{
					if (iDriveLetter->first.compare(strSubAddressFromCombine) == 0)
					{
						isExistDriveLetter = true;
						iDriveLetter->second |= CALC_PRT_VP;
						break;
					}
				}
				if (!isExistDriveLetter)
					mapDriveLetterValue.insert(pair<string, unsigned int>(strSubAddressFromCombine, CALC_PRT_VP));
			}
#pragma endregion
#pragma region Processes By Address
			//else if (strHeaderAddressFromCombine.compare(ADDR_PROCESS_NAME) == 0)
			//{
			//	if (!strSubAddressFromCombine.empty())
			//		vecProcesses.push_back(strSubAddressFromCombine);
			//}
#pragma endregion

			else
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Tag(%s), Sub-Address(%s) Invalid.",
				pDevice->szName, pDevice->vecTags[iTag]->szName, pDevice->vecTags[iTag]->szAddress);
		}
	}
#pragma endregion

#pragma region Address Checked, Must Be!!!

	//校验memory total
	if ((nMemoryCombineValue & CALC_PRT_ALL) != CALC_PRT_ALL) // 当前总值
	{
		nMemoryCombineValue = 0;
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Important MemoryTotal-Address(%s) Missing.",
			pDevice->szName, ADDR_MEMORY_TOTAL);
	}//校验memory freepercent
	else
	{
		if (((nMemoryCombineValue & CALC_AVG_VP_M1) == CALC_AVG_VP_M1) && ((nMemoryCombineValue & CALC_AVG_V_M1) != CALC_AVG_V_M1))
			nMemoryCombineValue ^= CALC_AVG_VP_M1;

		if (((nMemoryCombineValue & CALC_AVG_VP_M10) == CALC_AVG_VP_M10) && ((nMemoryCombineValue & CALC_AVG_V_M10) != CALC_AVG_V_M10))
			nMemoryCombineValue ^= CALC_AVG_VP_M10;

		if (((nMemoryCombineValue & CALC_AVG_VP_M30) == CALC_AVG_VP_M30) && ((nMemoryCombineValue & CALC_AVG_V_M30) != CALC_AVG_V_M30))
			nMemoryCombineValue ^= CALC_AVG_VP_M30;

		if (((nMemoryCombineValue & CALC_AVG_VP_M60) == CALC_AVG_VP_M60) && ((nMemoryCombineValue & CALC_AVG_V_M60) != CALC_AVG_V_M60))
			nMemoryCombineValue ^= CALC_AVG_VP_M60;

		if (((nMemoryCombineValue & CALC_AVG_VP_H3) == CALC_AVG_VP_H3) && ((nMemoryCombineValue & CALC_AVG_V_H3) != CALC_AVG_V_H3))
			nMemoryCombineValue ^= CALC_AVG_VP_H3;

		if (((nMemoryCombineValue & CALC_AVG_VP_H6) == CALC_AVG_VP_H6) && ((nMemoryCombineValue & CALC_AVG_V_H6) != CALC_AVG_V_H6))
			nMemoryCombineValue ^= CALC_AVG_VP_H6;

		if (((nMemoryCombineValue & CALC_AVG_VP_H12) == CALC_AVG_VP_H12) && ((nMemoryCombineValue & CALC_AVG_V_H12) != CALC_AVG_V_H12))
			nMemoryCombineValue ^= CALC_AVG_VP_H12;

		if (((nMemoryCombineValue & CALC_AVG_VP_H24) == CALC_AVG_VP_H24) && ((nMemoryCombineValue & CALC_AVG_V_H24) != CALC_AVG_V_H24))
			nMemoryCombineValue ^= CALC_AVG_VP_H24;
	}

	//校验disk
	for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
	{
		unsigned int nDriveLetterCombineValue = iDriveLetter->second;
		//total
		if ((nDriveLetterCombineValue & CALC_PRT_ALL) != CALC_PRT_ALL)
		{
			iDriveLetter->second = 0x0;
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnTimer Device(%s),Important DriveLetter(%s)-Address(%s) Missing.",
				pDevice->szName, iDriveLetter->first.c_str(), ADDR_DISK_TOTAL);
		}
		else//freepercent
		{
			unsigned int nDriveLetterCombineValue = iDriveLetter->second;
			if (((nDriveLetterCombineValue & CALC_PRT_VP) == CALC_PRT_VP) && ((nMemoryCombineValue & CALC_PRT_V) != CALC_PRT_V))
				nMemoryCombineValue ^= CALC_PRT_VP;
		}
	}

#pragma endregion

	map<unsigned short, unsigned int> mapTypeValue;
	if (nCpuCombineValue != 0x0)
		mapTypeValue.insert(pair<unsigned short, unsigned int>(TYPE_CPU, nCpuCombineValue));
	if (nMemoryCombineValue != 0x0)
		mapTypeValue.insert(pair<unsigned short, unsigned int>(TYPE_MEMORY, nMemoryCombineValue));
	for (map<string, unsigned int>::iterator iDriveLetter = mapDriveLetterValue.begin(); iDriveLetter != mapDriveLetterValue.end(); iDriveLetter++)
	{
		if (iDriveLetter->second == 0x0)
        {
            map<string, unsigned int>::iterator itToDel = iDriveLetter;
            iDriveLetter ++;
            mapDriveLetterValue.erase(itToDel);
        }
	}

#pragma region Processes By Sqlite

	char szHostid[128] = { 0 };
	sprintf(szHostid, "SELECT id FROM t_extra_sysmon_host WHERE name = '%s';", pDevice->szName);
	vector<vector<string> > vecshTables;

	string strHostid; 
	long nRes = g_db.SQLExecute(szHostid, vecshTables);	
	if (nRes != 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Execute Sql(%s) Failed, Error(%d).", pDevice->szName,szHostid, nRes);
	
	//memset(pDevice->szUserData[0], 0, sizeof(pDevice->szUserData[0]));
	if(vecshTables.size() > 0 && vecshTables[0].size() > 0)
		strHostid=vecshTables[0][0];


	char sz[1024] = { 0 };
	sprintf(sz, "SELECT processname\
		FROM (\
		SELECT\
		t_extra_sysmon_host.id\
		FROM\
		t_device_list,\
		t_extra_sysmon_host,\
		t_device_driver\
		WHERE\
		t_device_list.name = t_extra_sysmon_host.name\
		AND t_device_driver.id = t_device_list.driver_id\
		) T,\
		t_extra_sysmon_process\
		WHERE\
		T.id = t_extra_sysmon_process.host_id and t_extra_sysmon_process.host_id =%s ORDER BY t_extra_sysmon_process.id;",strHostid.c_str());
	
	vecProcesses.clear();
    vector<vector<string> > vecsTables;
	long nRet = g_db.SQLExecute(sz, vecsTables);
	if (nRet != 0)
        Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s,Execute Sql(%s) Failed, Error(%d).", pDevice->szName,sz, nRet);
	else
	{
		if(vecsTables[0].size() > 0 && vecsTables[0][0].size() > 0)
		{
            for(vector<vector<string> >::iterator item = vecsTables.begin(); item != vecsTables.end(); item++)
				vecProcesses.push_back(item->front());
		}
	}
#pragma endregion

	nRet = RequestCPUMemoryDiskData(pDevice, mapTypeValue, mapDriveLetterValue);
	static int nProcessRequestTimes = 1;
	if(nProcessRequestTimes++  == 5)
	{
		nProcessRequestTimes = 1;
		nRet = RequestProcessData(pDevice, vecProcesses);
	}

#pragma region Depend on Device Conn-Status
	if (nRet > 0)
	{
		//ReceiveFromRemoteDevice(pDevice);
		ReceiveFromMultiRemoteDevice(pDevice);
	}
	Drv_UpdateTagsDataByAddress(pDevice, ADDR_DEVICE_CONN_STATUS, nRet <= 0 ? "0" : "1");

#pragma endregion

	Drv_LogMessage(PK_LOGLEVEL_INFO, "Device(%s) Call OnTimer (%d) End.",pDevice->szName, timerInfo->nPhaseMS);
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
    Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OnControl To Driver(%s), Device(%s), Parameter[Name:%s, Address:%s, Value(%s)].",
		pDevice->pDriver->szName, pDevice->szName, pTag->szName, pTag->szAddress, szStrValue);

	if (strcmp(pTag->szAddress, CTRL_CMD_FILE_PATH) == 0)
		RequestControl(pDevice, TYPE_CONTROL_RUN, szStrValue);
	else if(strcmp(pTag->szAddress, CTRL_CMD_PROCESS_KILL) == 0)
		RequestControl(pDevice, TYPE_CONTROL_KILL, szStrValue);

	return 0;
}

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	g_db.InitFromConfigFile("db.conf", "database"); // 从eview的数据库初始化
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Init HostDrive(%s), Devices(%s), Parameters(%s).", pDevice->pDriver->szName, pDevice->szName, pDevice->szConnParam);
	//多设备的设备通信参数配置,如下：(注释代码勿删！！！！！！)
	//设备需要再接收的包数据的动态长度(不包含头长度)
	pDevice->nUserData[0] = N_USER_DATA_PACKAGE_DATA_LENGTH;
	//设备该次完整数据包的长度(包含头长度)
	//pDevice->nUserData[1] = PACKAGE_DATA_LENGTH;
	//设备接收的完整的包数据
	//pDevice->pUserData[0] = new char[pDevice->nUserData[0] + PACKHEADERLENGTH + 1]();
	//设备接受的完整的包的数据块的当前动态位置指针
	//pDevice->pUserData[1] = pDevice->pUserData[0];

	//进程数据操作 
	//char szIp[16] = { 0 };
	//sscanf(pDevice->szConnParam, "ip=%[^;];%*s", szIp);
	//g_db.Init(); 
	//g_db.InitLocalSQLite();
	char sz[128] = { 0 };
	sprintf(sz, "SELECT id FROM t_extra_sysmon_host WHERE name = '%s';", pDevice->szName);
	vector<vector<string> > vecsTables;

	long nRet = g_db.SQLExecute(sz, vecsTables);	
	if (nRet != 0)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device:%s, Execute Sql(%s) Failed, Error(%d).", pDevice->szName,sz, nRet);
	pDevice->szUserData[0][0] = 0;
	//memset(pDevice->szUserData[0], 0, sizeof(pDevice->szUserData[0]));
	if(vecsTables.size() > 0 && vecsTables[0].size() > 0)
		strncpy(pDevice->szUserData[0], vecsTables[0][0].c_str(), vecsTables[0][0].length());

	PKTIMER trRequest;
	trRequest.nPeriodMS = 4000;
	void *pTimerHandle = Drv_CreateTimer(pDevice, &trRequest); // 设定定时器，系统设定;
	pDevice->nRecvTimeout = 4000;		//超时请求3000ms
	return 0;
}

PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Uninit HostDrive(%s), Devices(%s), Parameters(%s).", pDevice->pDriver->szName, pDevice->szName, pDevice->szConnParam);

	return 0;
}
