/*
��������;��ͨ����������
����;��֧�ּ��豸���ƣ�״̬��ȡ
������Ա:xx;
�汾��1.0.0
��ʼ���ڣ�2019-04-26
�������ڣ�2019-04-30
*/

#include "ace/Time_Value.h"
//#include "ace/OS.h"  
#include <string> 
#include "pkcomm/pkcomm.h"
#include "pkdriver/pkdrvcmn.h"
#include "pkdata/pkdata.h"
#include "json/json.h"
#include "pkdbapi/pkdbapi.h"
#include <iostream>
#include <time.h>
#include <ctime>
#include <string>
#include <algorithm>

#define  MAX_LEN_BUUFFER	1024
#define  MAX_LEN_SQLEXECUTE 512
#define  MAX_LEN_SENDBUFFER 64
#define  MAX_LEN_RECVBUFFER 64
#define  ONEDAY_SECONDS    86400

CPKDbApi pkdb;
void UpdateInspectInfo(int id, PKDEVICE *device);	//����Ѳ���¼;
char StringToChar(string str);						//�ַ���ת16����;
time_t StringToTimestamp(string str);				//�ַ���תʱ���;


PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s)", pDevice->szName);
	PKTIMER timerInfo;
	timerInfo.nPeriodMS = 3000;
	void *pTimer = Drv_CreateTimer(pDevice, &timerInfo); // �趨��ʱ������ʱ��ѯ���е�����;
	//��ʼ���豸ʱ��ʼ��ʱ;
	//time_t tNow = time(NULL);
	//ͬʱ��������time_t��������linux�����л���ʾ�δ���;
	time_t ltStart = StringToTimestamp(pDevice->szParam2);
	pDevice->nUserData[0] = ltStart;
	//sqlite���ݿ����ӳ�ʼ��;
	int nRet = pkdb.InitFromConfigFile("db.conf", "database");
	if (nRet != 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, pkdb.InitFromConfigFile(db.conf,database) fail, ret:%d", pDevice->szName, nRet);
		pkdb.UnInitialize();
		return -1;
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice End");
	return 0;
}

/**
*  �趨�����ݿ鶨ʱ���ڵ���ʱ�ú���������;
*
*  @return PK_SUCCESS: ִ�гɹ�;
*
*  @version     12/11/2008    Initial Version..
*
*  ����һ�趨��
*  t_common_driver_config�ֶ�˵����
*  0:ID   1:DeviceName  2:Tagname  3:Tagaddress  4:CmdBuffer  5:status_success_rule
*  6:status_fail_rule  7:cmdName  8:cyclesec   9��getValueRule
*
*  ������趨��
*  t_device_list�ֶ�˵��:
*  param1:��ѵ����,��λ����;
*  param2:��ѵ��ʼʱ��;
*  param5:�����ϴ�һ��ѯ��ʱ���;
*
*/
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OnTimer start");
	time_t tNow;
	tNow = time(NULL);
	int nDeviceStatus;

	for (int i = 0; i < pDevice->nTagNum; i++)
	{
		PKTAG *pTag = pDevice->ppTags[i];
		string strAddr = pTag->szAddress;
		transform(strAddr.begin(), strAddr.end(), strAddr.begin(), ::tolower);
		
		int nLoc = strAddr.find("code");
		if (strAddr.find("code") >=0)
		{
			char szTemp[32] = { 0 };
			int nRet = Drv_Recv(pDevice, szTemp, sizeof(szTemp), 1000);

			if (nRet > 0)
			{
				Drv_UpdateTagsDataByAddress(pDevice, pTag->szAddress, szTemp);
			}	
		}
	}

	//����ʱ���趨Ѳ������;
	if (strcmp(pDevice->szParam1, ""))
	{
		if (tNow - pDevice->nUserData[0] >= (atoi(pDevice->szParam1)))
		{
			//����Ѳ����ʼʱ��;
			pDevice->nUserData[0] = tNow;
			//ִ��Ѳ�����;
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "��ǰ���ڽ���Device: %s �Զ�Ѳ�����,����ִ����������", pDevice->szName);
			//���Ϳ�����Ϣ���ж�ÿ���豸������״̬;
			char szSQL[MAX_LEN_SQLEXECUTE];
			sprintf(szSQL, "select * from t_common_driver_config where device_name = '%s' and  ischeck =1;", pDevice->szName);
			vector<vector<string> > vecRows;
			string strSqlErr;
			int nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
			if (vecRows.size() == 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name = %s, δ�ҵ���ӦѲ���豸���ã�����t_common_drivers_config", pDevice->szName);
				Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
				//����Ѳ��״̬;
				UpdateInspectInfo(-1, pDevice);
				return -1;
			}
			for (int i = 0; i < vecRows.size(); i++)
			{
				//�ж��豸�����뵱ǰ�豸�Ƿ񱣳�һ��;
				char szSendBuffer[MAX_LEN_BUUFFER] = { 0 };
				char szRecvBuffer[MAX_LEN_BUUFFER] = { 0 };
				string strSuccessRule;
				string strFailureRule;
				if (vecRows[i][1] == pDevice->szName)
				{
					strSuccessRule = vecRows[i][5];		//�ɹ�����;
					strFailureRule = vecRows[i][6];		//ʧ�ܹ���;
					sprintf(szSendBuffer, vecRows[i][4].c_str());
					nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 1000);
					if (nRet < 0)
					{
						//�����豸����ʧ�ܵ�״̬-1;
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -1.(���豸��������ʧ��)", pDevice->szName);
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
						//����Ѳ��״̬;
						UpdateInspectInfo(-1, pDevice);
						vecRows.clear();
						return -1;
					}
					//�жϷ���ֵ�Ƿ���ĳ��������;
					nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 1000);
					if (nRet <= 0)
					{
						//�����豸����ʧ�ܵ�״̬ -1;
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -1.(���豸��������ʧ��)", pDevice->szName);
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
						//����Ѳ��״̬;
						UpdateInspectInfo(-1, pDevice);
						vecRows.clear();
						return -1;
					}
					int nSuccess = strSuccessRule.find(szRecvBuffer);
					int nFailure = strFailureRule.find(szRecvBuffer);
					if (nSuccess > 0)
					{
						//�յ��豸���ӳɹ�������0;
						Drv_UpdateTagsDataByAddress(pDevice, "status", "0", sizeof(int), 1000, 0, 0);
						//����Ѳ��״̬;
						UpdateInspectInfo(0, pDevice);
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: 0.(�յ��豸�������ݣ����ϳɹ�����)", pDevice->szName);
					}

					if (nFailure > 0)
					{
						//�յ��豸����ʧ�ܵ�����-2;
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-2", sizeof(int), 1000, 0, 0);
						//����Ѳ��״̬;
						UpdateInspectInfo(-2, pDevice);
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -2.(�յ��豸�������ݣ�����ʧ�ܹ���)", pDevice->szName);
					}
				}
			}
			//��սṹ��;
			vecRows.clear();
		}
	}

	//�����豸״̬;
	//param�ֶΣ� ����ָ��(5);
	for (int i = 0; i < pDevice->nTagNum; i++)
	{
		PKTAG *pTag = pDevice->ppTags[i];
		string strParam(pTag->szParam);
		string strGet;
		char szSendBuffer[MAX_LEN_SENDBUFFER] = { 0 };
		char szRecvBuffer[MAX_LEN_RECVBUFFER] = { 0 };

		int nLoc = strParam.find("(");
		if (nLoc < 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "δ���÷��Ϲ���Ĳ�ѯ��Ϣ����ִ�в�ѯ����");
			return -1;
		}
		strcpy(szSendBuffer, strParam.substr(0, nLoc).c_str());
		string strSend = szSendBuffer;
		strGet = strParam.substr(nLoc);

		//�豸����4��д0���߲������Ϊ�Ƿ���16��������;
		if (0 == strcmp(pDevice->szParam4, "0") || 0 == strcmp(pDevice->szParam4, ""))
		{
			char sSendBuffer[PK_LOGLEVEL_ERROR] = { 0 };
			vector<string > vStr = PKStringHelper::StriSplit(strSend, " ");
			for (int i = 0; i < vStr.size(); ++i)
			{
				sSendBuffer[i] = StringToChar(vStr[i]);
			}
			vStr.clear();

			int nRet = Drv_Send(pDevice, sSendBuffer, sizeof(sSendBuffer), 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,ִ�У�%s ����ʧ�ܣ� nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				return -1;
			}
			nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 3000);
			if (nRet > 0)
			{
				// ����ַ������ȴ���3����Ҫ��ȡ�����ַ���ʮ��������;
				if (strGet.size() > 3)
				{
					int nLoc1 = atoi(strGet.substr(1, 1).c_str());
					int nLoc2 = atoi(strGet.substr(3, 1).c_str());
					int nVal1, nVal2;
					char szVal[4] = { 0 };

					//nVal1
					if (szRecvBuffer[nLoc1] > 0)
					{
						nVal1 = szRecvBuffer[nLoc1];
					}
					else
					{
						nVal1 = szRecvBuffer[nLoc1] + 256;
					}

					//nVal2
					if (szRecvBuffer[nLoc2] > 0)
					{
						nVal2 = szRecvBuffer[nLoc2];
					}
					else
					{
						nVal2 = szRecvBuffer[nLoc2] + 256;
					}
					int nTotal = nVal1 * 256 + nVal2;
					sprintf(szVal, "%d", nTotal);
					Drv_UpdateTagsDataByAddress(pDevice, pTag->szAddress, szVal);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "��ȡ��ֵ�ɹ�,szAddress: %s, value: %s", pTag->szAddress, szVal);
				}
				else //��ȡ�����ַ���ʮ��������;
				{
					int nLoc = atoi(strGet.substr(1, 1).c_str());
					char szVal[4] = { 0 };
					int nVal;
					//nVal
					if (szRecvBuffer[nLoc] > 0)
					{
						nVal = szRecvBuffer[nLoc];
					}
					else
					{
						nVal = szRecvBuffer[nLoc] + 256;
					}
					sprintf(szVal, "%d", nVal);
					Drv_UpdateTagsDataByAddress(pDevice, pTag->szAddress, szVal);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "��ȡ��ֵ�ɹ�,szAddress: %s, value: %s", pTag->szAddress, szVal);
				}
			}
		}
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OnTimer End");
	return 0;
}

/*
*
*  ���п�������ʱ�ú���������;
*  @param  -[in]  szStrValue������ֵ����blob�ⶼ�Ѿ��Ѿ�ת��Ϊ�ַ�����blobת��Ϊbase64����
*
*  @version     12/11/2008    Initial Version.
*
*  param4: ��ʾ���ݷ��ͽ��ƣ�0:ʮ������  1:ASCii��
*
*  param3: ��ʾ�����ʾ���豸���ƣ���Ҫ��д;
*
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	//�ֶ�ִ��Ѳ�����, ͨ����������ֵ����ֵΪ1ʵ��;
	if (0 == strcmp(pTag->szAddress, "xunjian"))
	{
		//ִ��Ѳ�����;
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "��ǰ���ڽ���Device : %  �ֶ�Ѳ�����,����ִ����������", pDevice->szName);
		//���Ϳ�����Ϣ���ж�ÿ���豸������״̬;
		char szSQL[MAX_LEN_SQLEXECUTE];
		sprintf(szSQL, "select * from t_common_driver_config where device_name = '%s' and  ischeck =1;", pDevice->szName);
		vector<vector<string> > vecRows;
		string strSqlErr;
		int nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
		if (nRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name = %s, execute sql failure, SQL = %s", pDevice->szName, szSQL);
			Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
			//����Ѳ��״̬;
			UpdateInspectInfo(-1, pDevice);
			return -1;
		}

		for (int i = 0; i < vecRows.size(); i++)
		{
			//�ж��豸�����뵱ǰ�豸�Ƿ񱣳�һ��;
			char szSendBuffer[MAX_LEN_BUUFFER] = { 0 };
			char szRecvBuffer[MAX_LEN_BUUFFER] = { 0 };
			string strSuccessRule;
			string strFailureRule;

			if (vecRows[i][1] == pDevice->szName)
			{
				strSuccessRule = vecRows[i][5];		//�ɹ�����;
				strFailureRule = vecRows[i][6];		//ʧ�ܹ���;

				sprintf(szSendBuffer, vecRows[i][4].c_str());
				nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 1000);

				if (nRet < 0)
				{
					//�����豸����ʧ�ܵ�״̬ -1;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -1.(���豸��������ʧ��)", pDevice->szName);

					//����Ѳ��״̬;
					UpdateInspectInfo(-1, pDevice);
					vecRows.clear();
					return -1;
				}

				//�жϷ���ֵ�Ƿ���ĳ��������;
				nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 1000);
				if (nRet <= 0)
				{
					//�����豸����ʧ�ܵ�״̬ -1;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -1.(���豸��������ʧ��)", pDevice->szName);
					//����Ѳ��״̬;
					UpdateInspectInfo(-1, pDevice);

					vecRows.clear();
					return -1;
				}
				int nSuccess = strSuccessRule.find(szRecvBuffer);
				int nFailure = strFailureRule.find(szRecvBuffer);

				if (nSuccess > 0)
				{
					//�յ��豸���ӳɹ�������0;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "0", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: 0.(�յ��豸�������ݣ����ϳɹ�����)", pDevice->szName);
					//����Ѳ��״̬;
					UpdateInspectInfo(0, pDevice);
				}
				else if (nFailure > 0)
				{
					//�յ��豸����ʧ�ܵ�����-2;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-2", sizeof(int), 1000, 0, 0);
					//����Ѳ��״̬;
					UpdateInspectInfo(-2, pDevice);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬: -2.(�յ��豸�������ݣ�����ʧ�ܹ���)", pDevice->szName);
				}
				else
				{
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, Ѳ����ɣ��豸�ĵ�ǰ״̬δ֪.(�յ��豸�������ݣ������������ù��������������)", pDevice->szName);
				}
			}
		}
		//��սṹ��;
		vecRows.clear();
		return 0;
	}

	string strControlAddr(pTag->szAddress);

	//ͨ�ÿ���;
	if ((0 == strcmp(pTag->szAddress, "dapingmu_yihaobuju") || (0 == strcmp(pTag->szAddress, "dapingmu_erhaobuju")) || (0 == strcmp(pTag->szAddress, "dapingmu_sanhaobuju")) || (0 == strcmp(pTag->szAddress, "dapingmu_sihaobuju"))																																												 //����ģʽ------Ӧ��һ�Ų��֣�1-4��;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju6"))					 //ģʽѡ��-----Ӧ�ò���(1-6);
		|| (0 == strcmp(pTag->szAddress, "shixudianyuan_dakaidianyuan")) || (0 == strcmp(pTag->szAddress, "shixudianyuan_guanbidianyuan"))																																																																			 //ʱ���Դ--�򿪵�Դ/�رյ�Դ;
		|| (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi1")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi2")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi3")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi4")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi5")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi6"))													 //��Ƶ������----Ӧ��ģʽģʽ��1-4��;
		|| (0 == strcmp(pTag->szAddress, "yinpinchuliqi_jingyin")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_quxiaojingyin")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_zengjiayinliang")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_jianxiaoyinliang"))																																			 //��Ƶ������----������������С������������ȡ������;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi6"))  //ģʽѡ��-----�������ƣ�1-6��;
		|| (0 == strcmp(pTag->szAddress, "wutaideng_moshi1")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi2")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi3")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi4")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi5")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi6"))																			 //��̨��---ģʽѡ��1-6��;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi2") || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi3"))) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi6"))		 //ģʽѡ��-----��̨��ģʽ(1-6);
		|| (0 == strcmp(pTag->szAddress, "wutaideng_dengguanquankai")) || (0 == strcmp(pTag->szAddress, "wutaideng_dengguanquanguan"))																																																																				 //��̨��----�ƹ�ȫ�����ƹ�ȫ��;
		|| (0 == strcmp(pTag->szAddress, "yingdieji_guanji")) || (0 == strcmp(pTag->szAddress, "yingdieji_kaiji")) || (0 == strcmp(pTag->szAddress, "yingdieji_kaicang")) || (0 == strcmp(pTag->szAddress, "yingdieji_guancang"))																																													 //Ӱ����----�������ػ������֡��ز�;
		|| (0 == strcmp(pTag->szAddress, "yingdieji_shangyige")) || (0 == strcmp(pTag->szAddress, "yingdieji_xiayige")) || (0 == strcmp(pTag->szAddress, "yingdieji_bofang")) || (0 == strcmp(pTag->szAddress, "yingdieji_zanting"))																																												 //Ӱ����---��һ������һ�������š���ͣ;	
		|| (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi1")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi2") || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi3"))) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi4")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi5")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi6"))														 //������-----ģʽѡ��;(1-6);
		|| (0 == strcmp(pTag->szAddress, "zhaomingdeng_dengguanquankai")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_dengguanquanguan"))																																																																		 //������-----�ƹ�ȫ�����ƹ�ȫ��;
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_shangsheng"))																																							 //��ת��-----����(1-4)
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_xiajiang"))																																									 //��ת��-----�½�(1-4)
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_tingzhi"))																																										 //��ת��-----ֹͣ(1-4)
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_shang")) || (0 == strcmp(pTag->szAddress, "sheyingji1_xia")) || (0 == strcmp(pTag->szAddress, "sheyingji1_zuo")) || (0 == strcmp(pTag->szAddress, "sheyingji1_you"))																																															 //����ͷ1-----���������ң�
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_fangda")) || (0 == strcmp(pTag->szAddress, "sheyingji1_suoxiao")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei1")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei2"))																																										     //����ͷ1-----���Ŵ���С��Ԥ��λ1��Ԥ��λ2��
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei3")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei4")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei5")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei6"))																																										 //����ͷ1-----��Ԥ��λ1��Ԥ��λ2��Ԥ��λ3��Ԥ��λ4��
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_shang")) || (0 == strcmp(pTag->szAddress, "sheyingji2_xia")) || (0 == strcmp(pTag->szAddress, "sheyingji2_zuo")) || (0 == strcmp(pTag->szAddress, "sheyingji2_you"))																																															 //����ͷ2-----���������ң�
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_fangda")) || (0 == strcmp(pTag->szAddress, "sheyingji2_suoxiao")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei1")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei2"))																																										     //����ͷ2-----���Ŵ���С��Ԥ��λ1��Ԥ��λ2��
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei3")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei4")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei5")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei6"))																																										 //����ͷ2-----��Ԥ��λ1��Ԥ��λ2��Ԥ��λ3��Ԥ��λ4��
		|| (0 == strcmp(pTag->szAddress, "hongwai_tv"))
		|| (strControlAddr.find("ctrl") >= 0)
		|| (0 == strcmp(pTag->szAddress, "dapingmu_dakaidianyuan")) || (0 == strcmp(pTag->szAddress, "dapingmu_guanbidianyuan"))))				 																																																																     //����ģʽ----�򿪵�Դ���رյ�Դ																																																															 //������------�ƹ�ȫ�����ƹ�ȫ��;
	{
		if (0 == strcmp(pDevice->szParam4, "1"))
		{
			char szSendBuffer[MAX_LEN_SENDBUFFER] = { 0 };
			sprintf(szSendBuffer, "%s", pTag->szParam);

			int nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,ִ�У�%s ����ʧ�ܣ� nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				return -1;
			}
		}
		else if (0 == strcmp(pDevice->szParam4, "0"))
		{
			char szSendBuffer[PK_LOGLEVEL_ERROR] = { 0 };
			vector<string > vStr = PKStringHelper::StriSplit(pTag->szParam, " ");
			for (int i = 0; i < vStr.size(); ++i)
			{
				szSendBuffer[i] = StringToChar(vStr[i]);
			}

			int nRet = Drv_Send(pDevice, szSendBuffer, vStr.size(), 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device : %s ,ִ�У�%s ����ʧ�ܣ� nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				vStr.clear();
				return -1;
			}
			vStr.clear();
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "t_device_list �����Param4������д����0��16���Ʒ���   1��ASCII���ͣ�,��������");
			return -1;
		}
	}

	//==============================================��Ƶ����Ӧ��==========================================================
	//��Ƶ����-----�ź��л�(1-4)
	//0x0C, 0x01, 0x05, 0x08��ģʽ��ID������ͨ�������ͨ����
	if ((0 == strcmp(pTag->szAddress, "shipinjuzhen_output1")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output2")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output3")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output4"))
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen6")))					 //��Ƶ����-----����ĻӦ��ģʽ��1-6��;
	{

		if (0 == strcmp(pDevice->szParam4, "1"))
		{
			char szSendBuffer[MAX_LEN_SENDBUFFER] = { 0 };
			szSendBuffer[0] = 0x0c;
			szSendBuffer[1] = 0x01;
			vector<string > vStr = PKStringHelper::StriSplit(szStrValue, "-");

			if (strlen(vStr[0].c_str()) == 1)
			{
				vStr[0] = "0" + vStr[0];
			}
			if (strlen(vStr[1].c_str()) == 1)
			{
				vStr[1] = "0" + vStr[1];
			}


			//���������С����2�����ʾ���������ʽ��������;
			if (vStr.size() != 2)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,��Ƶ�����л����������ʽ��������%s", pDevice->szName, szSendBuffer);
				return -1;
			}

			for (int i = 0; i < vStr.size(); ++i)
			{
				szSendBuffer[i + 2] = StringToChar(vStr[i]);
			}
			int nRet = Drv_Send(pDevice, szSendBuffer, vStr.size() + 2, 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,��Ƶ�����л��ź�ʧ�ܣ��������%s �� nRet = %d", pDevice->szName, szSendBuffer, nRet);
				return -1;
			}
			char szSql[MAX_LEN_SQLEXECUTE] = { 0 };
			sprintf(szSql, "select name from t_meeting_matrix_input where id = %d", atoi(vStr[0].c_str()));
			vector<vector<string> > vecRows;
			string strSqlErr;
			nRet = pkdb.SQLExecute(szSql, vecRows, &strSqlErr);
			if (vecRows.size() != 0)
			{
				Drv_UpdateTagsDataByAddress(pDevice, pTag->szAddress, vecRows[0][0].c_str());
			}
			vStr.clear();
			vecRows.clear();
		}
		else if(0 == strcmp(pDevice->szParam4, "0"))
		{
			char szSendBuffer[MAX_LEN_SENDBUFFER] = { 0 };
			sprintf(szSendBuffer, "%s", pTag->szParam);
			int nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,ִ�У�%s ����ʧ�ܣ� nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				return -1;
			}
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "t_device_list �����Param4������д����0��16���Ʒ���   1��ASCII���ͣ�,��������");
			return -1;
		}
	}
	return 0;
}

/*
����ʼ���豸;
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	return 0;
}

/*
����ʼ������;
*/
PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDriver(driver:%s)", pDriver->szName);
	return 0;
}

//����Ѳ����;
void UpdateInspectInfo(int id, PKDEVICE *device)
{
	char szCurrentTime[32] = { 0 };
	time_t timep;
	struct tm *p;
	time(&timep);						 /*���time_t�ṹ��ʱ�䣬UTCʱ��*/
	p = localtime(&timep);				 /*ת��Ϊstruct tm�ṹ��UTCʱ��*/
	sprintf(szCurrentTime, "%2d-%.2d-%.2d %2d:%.2d:%.2d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	//�������ݿ�������;
	char szUpdateSql[MAX_LEN_SQLEXECUTE] = { 0 };
	if (-2 == id)
	{
		sprintf(szUpdateSql, "update t_inspect_history set device_status = 'bad', device_miaoshu= '%s,Status: -2', checktime='%s' where device_name='%s'", device->szParam3, szCurrentTime, device->szName);
	}
	else if (-1 == id)
	{
		sprintf(szUpdateSql, "update t_inspect_history set device_status = 'bad', device_miaoshu = '%s,Status: -1', checktime='%s' where device_name='%s'", device->szParam3, szCurrentTime, device->szName);
	}
	else if (0 == id)
	{
		sprintf(szUpdateSql, "update t_inspect_history set device_status = 'good', device_miaoshu = '%s,Status: 0', checktime='%s' where device_name='%s'", device->szParam3, szCurrentTime, device->szName);
	}
	else if (10000 == id)
	{
		sprintf(szUpdateSql, "update t_inspect_history set device_status = 'bad', device_miaoshu = '%s,Status:10000', checktime='%s' where device_name='%s'", device->szParam3, szCurrentTime, device->szName);
	}
	else
	{
		NULL;
	}

	//ִ�и��²���;
	pkdb.SQLExecute(szUpdateSql);
}

//string ת char; 
char StringToChar(string str)
{
	char szTemp[4] = { 0 };
	int nVal1;
	int nVal2;
	int nVal3;
	char cChar;
	sprintf(szTemp, "%s", str.c_str());

	if (szTemp[0] >= 'a' && szTemp[0] <= 'f')
	{
		nVal1 = szTemp[0] - 'a' + 10;
	}
	else if (szTemp[0] >= '0' && szTemp[0] <= '9')
	{
		nVal1 = szTemp[0] - '0';
	}

	if (szTemp[1] >= 'a' && szTemp[1] <= 'f')
	{
		nVal2 = szTemp[1] - 'a' + 10;
	}
	else if (szTemp[1] >= '0' && szTemp[1] <= '9')
	{
		nVal2 = szTemp[1] - '0';
	}

	nVal3 = nVal1 * 16 + nVal2;
	cChar = nVal3;
	return cChar;
}

//�ַ���תʱ���(linux&windows);
time_t StringToTimestamp(string str)
{
	tm tm_;
	int year, month, day, hour, minute, second;
	sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;
	time_t t_ = mktime(&tm_); //�Ѿ�����8��ʱ��
	return  mktime(&tm_);
}