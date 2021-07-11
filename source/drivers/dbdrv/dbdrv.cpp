/*
��������;��
���ݿ���������:
����;��
��������ݿ��ж�ȡ��ֵ����ʾ������;
������Ա:xx;
�汾��1.0.0
��ʼ���ڣ�2016-10-27
�������ڣ�2016-11-04
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
	// ��������
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver, ��connparam:���ݿ�t_db_connection��name�е�ֵ�� ��A:Qa;B:Qb;C:Qc;D:Qd��[A=��ѯ��¼����Ϊ0ʱ��ֵ,ȱʡΪ:?��Qa=��ʱ������ȱʡΪ-50001][B=��ѯʧ��ʱ��ֵ,ȱʡΪ:??��Qb=��ʱ������ȱʡΪ-50002][C=���������ݿ�ʱ��TAG��ֵ,ȱʡΪ:???��Qc=��ʱ������ȱʡΪ-50003][D=���ݿ����ô���ʱ��TAG��ֵ,ȱʡΪ:????��Qd=��ʱ������ȱʡΪ-50004]");
	return 0;
}

/*
*  ��ʼ������;
*  @version  01/17/2017 xingxing  Initial Version;
*/
PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	// ��������
	Drv_LogMessage(PK_LOGLEVEL_INFO, "InitDevice(%s), connparam:%s, param1:%s", pDevice->szName, pDevice->szConnParam, pDevice->szParam1);
	CDbDevice *pDbDevice = new CDbDevice(pDevice);
	pDevice->pUserData[0] = pDbDevice;

	pDbDevice->ParseExceptTagValue();

	CPKDbApi pkdb;		// ���ݿ����ӽӿ�
	// Init database
	int nRet = pkdb.InitFromConfigFile("db.conf","database");
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, pkdb.InitFromConfigFile(db.conf,database) fail, ret:%d", pDevice->szName, nRet);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // ���е�����Ϊ���Ӳ���
		pkdb.UnInitialize();
		return nRet;
	}

	string strDbReferName = pDevice->szConnParam; // ָ�����ݿ��t_db_connection��name��
	char szSQL[512];
	sprintf(szSQL, "select * from t_db_connection where name = '%s'", strDbReferName.c_str());
	vector<vector<string> > vecRows;		// ȫ����ѯ��¼
	string strSqlErr;		// ��ѯ������
	nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
	// ������ֵ
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, ִ��SQL��ѯeview���ݿ������쳣:%s, SQL:%s", pDevice->szName, strSqlErr.c_str(), szSQL);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // ���е�����Ϊ���Ӳ���
		return nRet;
	}

	if (vecRows.size() <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, t_db_connection��δ�ҵ�����Ϊ:%s�����ݿ����Ӽ�¼, ����, SQL:%s", pDevice->szName, pDevice->szName, szSQL);
		pDbDevice->UpdateTagsToBadWhenConfigError(pDevice); // ���е�����Ϊ���Ӳ���
		return -1;
	}

	if (vecRows.size() > 1)
	{
		Drv_LogMessage(PK_LOGLEVEL_WARN, "device:%s, t_db_connection���ҵ�:%d��(>1)����Ϊ:%s�����ݿ����Ӽ�¼, ���ǽ�ʹ�õ�һ�����ݿ�, SQL:%s", pDevice->szName, vecRows.size(), strDbReferName.c_str(), szSQL);
	}

	CONN_PARAM param;			// ���Ӳ���
	vector<string> vecRow = vecRows[0];
	PKStringHelper::Safe_StrNCpy(param.szId, vecRow[0].c_str(), sizeof(param.szId));
	PKStringHelper::Safe_StrNCpy(param.szDbName, vecRow[1].c_str(), sizeof(param.szDbName));
	PKStringHelper::Safe_StrNCpy(param.szDbType, vecRow[2].c_str(), sizeof(param.szDbType));
	PKStringHelper::Safe_StrNCpy(param.szConnString, vecRow[3].c_str(), sizeof(param.szConnString));
	PKStringHelper::Safe_StrNCpy(param.szUserName, vecRow[4].c_str(), sizeof(param.szUserName));
	PKStringHelper::Safe_StrNCpy(param.szPassWord, vecRow[5].c_str(), sizeof(param.szPassWord));
	PKStringHelper::Safe_StrNCpy(param.szDescription, vecRow[6].c_str(), sizeof(param.szDescription));
	PKStringHelper::Safe_StrNCpy(param.szCodingSet, vecRow[7].c_str(), sizeof(param.szCodingSet));
	if(vecRow.size() >= 9) // �������ӵ��ֶ�
		PKStringHelper::Safe_StrNCpy(param.szTestTableName, vecRow[8].c_str(), sizeof(param.szTestTableName));
	if (strlen(param.szTestTableName) <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, t_db_connection���һ������Ϊ:%s�����ݿ����Ӽ�¼, ��������testtablenamne�ֶ�Ϊ��ǰ�û����Է��ʵ��ı�������ͼ���������޷�����������", pDevice->szName, param.szDbName);
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, t_db_connection���һ������Ϊ:%s�����ݿ����Ӽ�¼, ������ӵı�Ϊ(testtablenamne):%s, ��ǰ�û�����ʵ��ı�������ͼ���������޷�����������", pDevice->szName, param.szDbName, param.szTestTableName);
	}
	pDbDevice->SetDBParam(param);

	PKTIMER pkTimer;
	pkTimer.nUserData[0] = 1;
	pkTimer.nPeriodMS = 3000;
	Drv_CreateTimer(pDevice, &pkTimer); // �趨��ʱ����ϵͳ�趨;
	Drv_LogMessage(PK_LOGLEVEL_INFO, "Device:%s, InitDevice OK, tagcount:%d;", pDevice->szName, pDevice->nTagNum);
	Drv_SetConnectOKTimeout(pDevice, 10);
	return 0;
}

PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CDbDevice *pDbDevice = (CDbDevice *)pDevice->pUserData[0];

	pDbDevice->CheckDBConnected(); // ִ�м�����ݿ����ӵĲ���
	bool bDBConnected = pDbDevice->IsDBAvailable();
	if (!bDBConnected) // ������Ӳ������ݿ⣬��д�κ�ֵ
	{
		pDbDevice->UpdateTagsToBadWhenDisconnected();	// ����ΪBAD
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
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, ִ��SQL��ѯ�쳣:%s, SQL:%s", pDevice->szName, pTag->szName, strSqlErr.c_str(), szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryFail.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}

		Drv_SetConnectOK(pDevice);
		pDbDevice->m_tmLastDBSuccess = tmNow; // ִ�гɹ��ˣ�˵�����ݿ���������

		if (vecRows.size() <= 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, ����SQL�鵽�ļ�¼����Ϊ0, SQL:%s", pDevice->szName, pTag->szName, szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryNoRecord.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}
		vector<string> vecRow = vecRows[0];
		if (vecRow.size() <= 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, tag:%s, ����SQL�鵽�ļ�¼:%d��, ����:%d <= 0, SQL:%s", pDevice->szName, pTag->szName, vecRows.size(), vecRow.size(), szSQL);
			Drv_SetTagData_Text(pTag, pDbDevice->m_strTagValueWhenQueryNoRecord.c_str(), 0, 0, TAGQUALITY_QUERYFAIL);
			continue;
		}

		if (vecRows.size() > 1)
		{
			Drv_LogMessage(PK_LOGLEVEL_WARN, "device:%s, tag:%s, ����SQL�鵽�ļ�¼����Ϊ:%d>1,��ȡ��һ����¼, SQL:%s", pDevice->szName, pTag->szName, vecRows.size(), szSQL);
		}
		string strTagValue = vecRow[0];
		nRet = Drv_SetTagData_Text(pTag, strTagValue.c_str());
		Drv_LogMessage(PK_LOGLEVEL_INFO, "SetTagData, device:%s, tag:%s, value:%s, ret:%d", pDevice->szName, pTag->szName, strTagValue.c_str(), nRet);

		vecRow.clear();
		vecRows.clear();
	}
	int nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
	Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s, ������:%d��tag��, ret:%d", pDevice->szName, pDevice->nTagNum, nRet);

	return PK_SUCCESS; 
}

//����;
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, char *szBinValue, int nBinValueLen, long lCmdId)
{ 
	return 0;
}

//����ʼ���豸;
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	CDbDevice *pDbDevice = (CDbDevice *)pDevice->pUserData[0];
	pDbDevice->m_db.UnInitialize(); 
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device:%s, db:%s Disconnected", pDevice->szName, pDbDevice->m_connParams.szDbName);
	 
	return 0;
}
