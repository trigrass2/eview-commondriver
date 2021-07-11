#ifndef _DBDRV_H_
#define _DBDRV_H_

#include "pkdbapi/pkdbapi.h"
#include "pkcomm/pkcomm.h"

#define TAGVALUE_BAD_DISCONNECTDB		1	// 连不上数据库时的值
#define TAGVALUE_BAD_QUERYFAIL			2	// 查询失败时的值
#define TAGVALUE_BAD_NORECORD			3	// 查询记录个数为0时的值

#define TAGQUALITY_DBCONFIGERROR		-50004	// 查询记录个数为0时的值, ????
#define TAGQUALITY_DISCONNECT			-50003	// 查询记录个数为0时的值, ???
#define TAGQUALITY_QUERYFAIL			-50002	// 查询记录个数为0时的值, ??
#define TAGQUALITY_NORECORD				-50001	// 查询记录个数为0时的值, ?

using namespace std;

typedef struct CONN_PARAM
{
	char szId[PK_NAME_MAXLEN];
	char szDbName[PK_NAME_MAXLEN];
	char szDbType[PK_NAME_MAXLEN];
	char szConnString[PK_DESC_MAXLEN];
	char szUserName[PK_NAME_MAXLEN];
	char szPassWord[PK_NAME_MAXLEN];
	char szDescription[PK_DESC_MAXLEN];
	char szCodingSet[PK_NAME_MAXLEN];
	char szTestTableName[PK_NAME_MAXLEN]; // 测试是否连接的测试表
}CONN_PARAM;

class CDbDevice 
{
public:
	CONN_PARAM m_connParams;
	PKDEVICE *m_pDevice;
	time_t		m_tmLastDBSuccess;	// 上次执行数据库成功或者连接的时间。只要有一个成功执行则更新这个时间。这个用来检查是否成功需要重连。避免连接不上时过快连接数据库
	int			m_nDBType;	// 数据库类型
	string		m_strCheckTestTable; // 检查是否连接的测试数据表。这个表要数据记录个数少些，以免影响性能。
	CPKDbApi	m_db;	// 数据库对象
	string		m_strTagValueWhenDisconnect;	// 连不上数据库时的值
	string		m_strTagValueWhenQueryFail;		// 查询失败时的值
	string		m_strTagValueWhenQueryNoRecord;	// 查询记录个数为0时的值
	string		m_strTagValueWhenDBConfigError;	// 数据库配置错误时的值

	int		m_nTagQualityWhenDisconnect;	// 连不上数据库时的质量
	int		m_nTagQualityWhenQueryFail;		// 查询失败时的质量
	int		m_nTagQualityWhenQueryNoRecord;	// 查询记录个数为0时的质量
	int		m_nTagQualityWhenDBConfigError;	// 数据库配置错误时的质量


	CDbDevice(PKDEVICE *pDevice) 
	{
		this->m_pDevice = pDevice;
		m_tmLastDBSuccess = 0;	// 上次执行数据库成功或者连接的时间。只要有一个成功执行则更新这个时间。这个用来检查是否成功需要重连。避免连接不上时过快连接数据库
	}

	int SetDBParam(CONN_PARAM &param)
	{
		memcpy(&m_connParams, &param, sizeof(CONN_PARAM));
		m_nDBType = CPKDbApi::ConvertDBTypeName2Id(m_connParams.szDbType); // 数据库类型
		return 0;
	}
	// 检查数据库是否连接。如果连接上了则认为
	int CheckDBConnected()
	{
		time_t tmNow = time(NULL);
		int nSpan = labs(tmNow - m_tmLastDBSuccess); // 和上次执行之间的时间差
		if (nSpan < 60) // 和上次执行之间时间差是否超过30秒
			return 0;

		// 60秒还没有结果，需要重连了
		bool bConnected = false;
		if (strlen(m_connParams.szTestTableName) > 0)
			bConnected = m_db.TestConnected(m_connParams.szTestTableName);
		//else
		//	bConnected = m_db.IsConnected();
		if (bConnected)
		{
			return 0;
		}

		// 下面将进行重连！
		m_db.UnInitialize();
		int nRet = m_db.SQLConnect(m_connParams.szConnString, m_connParams.szUserName,m_connParams.szPassWord, m_nDBType, 1000, m_connParams.szCodingSet);
		if (nRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, connect db(connstring:%s,codingset:%s) failed, ret:%d", m_pDevice->szName, m_connParams.szConnString, m_connParams.szCodingSet, nRet);
			m_tmLastDBSuccess = tmNow;
			return nRet;
		}
		m_tmLastDBSuccess = tmNow;
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, connect db(connstring:%s,codingset:%s) success", m_pDevice->szName, m_connParams.szConnString, m_connParams.szCodingSet);
		return 0;
	}

	bool IsDBAvailable() // 数据库是否可以使用。如果连接成功则认为可以使用
	{
		//return m_db.IsConnected(); // 这个不是很准确
	}

	int UpdateTagsToBadWhenDisconnected()
	{
		for (int i = 0; i < m_pDevice->nTagNum; i++)
		{
			PKTAG *pTag = m_pDevice->ppTags[i];
			Drv_SetTagData_Text(pTag, m_strTagValueWhenDisconnect.c_str(), NULL, NULL, TAGQUALITY_DISCONNECT);
		}
		int nRet = Drv_UpdateTagsData(m_pDevice, m_pDevice->ppTags, m_pDevice->nTagNum);
		return nRet;
	}

	int UpdateTagsToBadWhenConfigError(PKDEVICE *pDevice)
	{
		for (int i = 0; i < m_pDevice->nTagNum; i++)
		{
			PKTAG *pTag = m_pDevice->ppTags[i];
			Drv_SetTagData_Text(pTag, m_strTagValueWhenDBConfigError.c_str(), 0, 0, TAGQUALITY_DBCONFIGERROR);
		}
		int nRet = Drv_UpdateTagsData(m_pDevice, m_pDevice->ppTags, m_pDevice->nTagNum);
		return nRet;
	}

	// 解析连接不上时的参数的值
	int ParseExceptTagValue()
	{
		m_strTagValueWhenQueryNoRecord = "?";	// 查询记录个数为0时的值
		m_strTagValueWhenQueryFail = "??";		// 查询失败时的值
		m_strTagValueWhenDisconnect = "???";	// 连不上数据库时的值
		m_strTagValueWhenDBConfigError = "????";	// 数据库配置错误时的值

		m_nTagQualityWhenQueryNoRecord = TAGQUALITY_NORECORD;	// 查询记录个数为0时的质量
		m_nTagQualityWhenQueryFail = TAGQUALITY_QUERYFAIL;		// 查询失败时的质量
		m_nTagQualityWhenDisconnect = TAGQUALITY_DISCONNECT;	// 连不上数据库时的质量
		m_nTagQualityWhenDBConfigError = TAGQUALITY_DBCONFIGERROR;	// 数据库配置错误时的质量

		string strExcepTagValue = m_pDevice->szParam1;
		vector<string> vecParam = PKStringHelper::StriSplit(strExcepTagValue, ";");
		if (vecParam.size() >= 1 && vecParam[0].length() > 0)
		{
			vector<string> vecValQuality = PKStringHelper::StriSplit(vecParam[0], ":");
			if (vecValQuality.size() >= 1 && vecValQuality[0].length() > 0)
			{
				m_strTagValueWhenQueryNoRecord = vecValQuality[0];
			}
			if (vecValQuality.size() >= 2 && vecValQuality[1].length() > 0)
			{
				m_nTagQualityWhenQueryNoRecord = ::atoi(vecValQuality[1].c_str());
			}			
		}
		if (vecParam.size() >= 2 && vecParam[1].length() > 0)
		{
			vector<string> vecValQuality = PKStringHelper::StriSplit(vecParam[1], ":");
			if (vecValQuality.size() >= 1 && vecValQuality[0].length() > 0)
			{
				m_strTagValueWhenQueryFail = vecValQuality[0];
			}
			if (vecValQuality.size() >= 2 && vecValQuality[1].length() > 0)
			{
				m_nTagQualityWhenQueryFail = ::atoi(vecValQuality[1].c_str());
			}
		}
		if (vecParam.size() >= 3 && vecParam[2].length() > 0)
		{
			vector<string> vecValQuality = PKStringHelper::StriSplit(vecParam[2], ":");
			if (vecValQuality.size() >= 1 && vecValQuality[0].length() > 0)
			{
				m_strTagValueWhenDisconnect = vecValQuality[0];
			}
			if (vecValQuality.size() >= 2 && vecValQuality[1].length() > 0)
			{
				m_nTagQualityWhenDisconnect = ::atoi(vecValQuality[1].c_str());
			}
		}

		if (vecParam.size() >= 4 && vecParam[3].length() > 0)
		{
			vector<string> vecValQuality = PKStringHelper::StriSplit(vecParam[3], ":");
			if (vecValQuality.size() >= 1 && vecValQuality[0].length() > 0)
			{
				m_strTagValueWhenDBConfigError = vecValQuality[0];
			}
			if (vecValQuality.size() >= 2 && vecValQuality[1].length() > 0)
			{
				m_nTagQualityWhenDBConfigError = ::atoi(vecValQuality[1].c_str());
			}
		}

		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, param2:%s, parsed bad tag value. NoRecord:%s:%d, QueryFail:%s:%d, Disconnect:%s:%d, ConfigError:%s:%d",
			m_pDevice->szName, m_pDevice->szParam2, m_strTagValueWhenQueryNoRecord.c_str(), m_nTagQualityWhenQueryNoRecord, m_strTagValueWhenQueryFail.c_str(), m_nTagQualityWhenQueryFail, 
			m_strTagValueWhenDisconnect.c_str(), m_nTagQualityWhenDisconnect, m_strTagValueWhenDBConfigError.c_str(),  m_nTagQualityWhenDBConfigError);
		return 0;
	}
};

#endif