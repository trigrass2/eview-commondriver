/*
驱动名称;：
数据库连接驱动:
功能;：
定向从数据库中读取数值，显示到界面;
开发人员:xx;
版本：1.0.0
开始日期：2016-10-27
结束日期：2016-11-04
*/
#include <sstream>
#include <algorithm>
#include <string.h>
#include <map>
#include "pkcomm/pkcomm.h"
#include "pkdriver/pkdrvcmn.h"
#include "pkdbapi/pkdbapi.h"
#include "stdio.h"
#include "dbdrv.h"

using  namespace std;

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	// 建立连接
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver, 【connparam:数据库t_db_connection的name列的值】 【A:Qa;B:Qb;C:Qc;D:Qd】[A=查询记录个数为0时的值,缺省为:?。Qa=此时质量，缺省为-50001][B=查询失败时的值,缺省为:??。Qb=此时质量，缺省为-50002][C=连不上数据库时的TAG的值,缺省为:???。Qc=此时质量，缺省为-50003][D=数据库配置错误时的TAG的值,缺省为:????。Qd=此时质量，缺省为-50004]");
	return 0;
}

/*
*  初始化函数;
*  @version  01/17/2017 xingxing  Initial Version;
*/
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// 建立连接
	Drv_LogMessage(PK_LOGLEVEL_INFO, "InitDevice(%s), connparam:%s, param1:%s", pDevice->szName, pDevice->szConnParam, pDevice->szParam1);
	CDbDevice *pDbDevice = new CDbDevice(pDevice);
	pDevice->pUserData[0] = pDbDevice;

	pDbDevice->ParseExceptTagValue();

	CPKDbApi pkdb;		// 数据库连接接口
	// Init database
	int nRet = pkdb.InitFromConfigFile("db.conf","database");
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, pkdb.InitFromConfigFile(db.conf,database) fail, ret:%d", pDevice->szName, nRet);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // 所有点设置为连接不上
		pkdb.UnInitialize();
		return nRet;
	}

	string strDbReferName = pDevice->szConnParam; // 指向数据库表t_db_connection的name列
	char szSQL[512];
	sprintf(szSQL, "select * from t_db_connection where name = '%s'", strDbReferName.c_str());
	vector<vector<string> > vecRows;		// 全部查询记录
	string strSqlErr;		// 查询错误码
	nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
	// 处理返回值
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, 执行SQL查询eview数据库的语句异常:%s, SQL:%s", pDevice->szName, strSqlErr.c_str(), szSQL);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // 所有点设置为连接不上
		return nRet;
	}

	if (vecRows.size() <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, t_db_connection表未找到名称为:%s的数据库连接记录, 请检查, SQL:%s", pDevice->szName, pDevice->szName, szSQL);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // 所有点设置为连接不上
		return -1;
	}

	if (vecRows.size() > 1)
	{
		Drv_LogMessage(PK_LOGLEVEL_WARN, "device:%s, t_db_connection表找到:%d个(>1)名称为:%s的数据库连接记录, 我们将使用第一条数据库, SQL:%s", pDevice->szName, vecRows.size(), strDbReferName.c_str(), szSQL);
	}

	CONN_PARAM param;			// 连接参数
	vector<string> vecRow = vecRows[0];
	PKStringHelper::Safe_StrNCpy(param.szId, vecRow[0].c_str(), sizeof(param.szId));
	PKStringHelper::Safe_StrNCpy(param.szDbName, vecRow[1].c_str(), sizeof(param.szDbName));
	PKStringHelper::Safe_StrNCpy(param.szDbType, vecRow[2].c_str(), sizeof(param.szDbType));
	PKStringHelper::Safe_StrNCpy(param.szConnString, vecRow[3].c_str(), sizeof(param.szConnString));
	PKStringHelper::Safe_StrNCpy(param.szUserName, vecRow[4].c_str(), sizeof(param.szUserName));
	PKStringHelper::Safe_StrNCpy(param.szPassWord, vecRow[5].c_str(), sizeof(param.szPassWord));
	PKStringHelper::Safe_StrNCpy(param.szDescription, vecRow[6].c_str(), sizeof(param.szDescription));
	PKStringHelper::Safe_StrNCpy(param.szCodingSet, vecRow[7].c_str(), sizeof(param.szCodingSet));
	if(vecRow.size() >= 9) // 后面增加的字段
		PKStringHelper::Safe_StrNCpy(param.szTestTableName, vecRow[8].c_str(), sizeof(param.szTestTableName));
	if (strlen(param.szTestTableName) <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, t_db_connection表第一个名称为:%s的数据库连接记录, 必须配置testtablenamne字段为当前用户可以访问到的表名或视图名，否则无法重连！！！", pDevice->szName, param.szDbName);
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, t_db_connection表第一个名称为:%s的数据库连接记录, 检查连接的表为(testtablenamne):%s, 当前用户须访问到的表名或视图名，否则无法重连！！！", pDevice->szName, param.szDbName, param.szTestTableName);
	}
	pDbDevice->SetDBParam(param);

	PKTIMER pkTimer;
	pkTimer.nUserData[0] = 1;
	pkTimer.nPeriodMS = 3000;
	Drv_CreateTimer(pDevice, &pkTimer); // 设定定时器，系统设定;
	Drv_LogMessage(PK_LOGLEVEL_INFO, "Device:%s, InitDevice OK, tagcount:%d;", pDevice->szName, pDevice->nTagNum);
	Drv_SetConnectOKTimeout(pDevice, 10);
	return 0;
}

PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CDbDevice *pDbDevice = (CDbDevice *)pDevice->pUserData[0];

	pDbDevice->CheckDBConnected(); // 执行检查数据库连接的操作
	bool bDBConnected = pDbDevice->IsDBAvailable();
	if (!bDBConnected) // 如果连接不上数据库，则不写任何值
	{
		pDbDevice->UpdateTagsToBadWhenDisconnected();	// 更新为BAD
		return -100;
	}

	time_t tmNow = time(NULL);
	for (int i = 0; i < pDevice->nTagNum; i++)
	{
		PKTAG *pTag = pDevice->ppTags[i];
		char *szSQL = pTag->szAddress;
		string strSqlErr;
		vector<vector<string> > vecRows;
		int nRet = pDbDevice->m_db.SQLExecute(szSQL, vecRows, &strSqlErr);
		if (nRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, 执行SQL查询异常:%s, SQL:%s", pDevice->szName, pTag->szName, strSqlErr.c_str(), szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryFail.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}

		Drv_SetConnectOK(pDevice);
		pDbDevice->m_tmLastDBSuccess = tmNow; // 执行成功了，说明数据库连接正常

		if (vecRows.size() <= 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, 根据SQL查到的记录条数为0, SQL:%s", pDevice->szName, pTag->szName, szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryNoRecord.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}
		vector<string> vecRow = vecRows[0];
		if (vecRow.size() <= 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, 根据SQL查到的记录:%d个, 列数:%d <= 0, SQL:%s", pDevice->szName, pTag->szName, vecRows.size(), vecRow.size(), szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryNoRecord.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}

		if (vecRows.size() > 1)
		{
			Drv_LogMessage(PK_LOGLEVEL_WARN, "device:%s, tag:%s, 根据SQL查到的记录条数为:%d>1,将取第一条记录, SQL:%s", pDevice->szName, pTag->szName, vecRows.size(), szSQL);
		}
		string strTagValue = vecRow[0];
		nRet = Drv_SetTagData_Text(pTag, strTagValue.c_str());
		Drv_LogMessage(PK_LOGLEVEL_INFO, "SetTagData, device:%s, tag:%s, value:%s, ret:%d", pDevice->szName, pTag->szName, strTagValue.c_str(), nRet);

		vecRow.clear();
		vecRows.clear();
	}
	int nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, 更新了:%d个tag点, ret:%d", pDevice->szName, pDevice->nTagNum, nRet);

	return PK_SUCCESS; 
}

//控制;
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{ 
	return 0;
}

//反初始化设备;
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	CDbDevice *pDbDevice = (CDbDevice *)pDevice->pUserData[0];
	pDbDevice->m_db.UnInitialize(); 
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device:%s, db:%s Disconnected", pDevice->szName, pDbDevice->m_connParams.szDbName);
	 
	return 0;
}
