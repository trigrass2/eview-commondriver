/*
驱动名称;：通用驱动程序
功能;：支持简单设备控制，状态读取
开发人员:xx;
版本：1.0.0
开始日期：2019-04-26
结束日期：2019-04-30
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
void UpdateInspectInfo(int id, PKDEVICE *device);	//更新巡检记录;
char StringToChar(string str);						//字符串转16进制;
time_t StringToTimestamp(string str);				//字符串转时间戳;


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
	void *pTimer = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器，定时轮询所有的数据;
	//初始化设备时开始计时;
	//time_t tNow = time(NULL);
	//同时声明两个time_t变量，在linux下运行会提示段错误;
	time_t ltStart = StringToTimestamp(pDevice->szParam2);
	pDevice->nUserData[0] = ltStart;
	//sqlite数据库连接初始化;
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
*  设定的数据块定时周期到达时该函数被调用;
*
*  @return PK_SUCCESS: 执行成功;
*
*  @version     12/11/2008    Initial Version..
*
*  规则一设定：
*  t_common_driver_config字段说明：
*  0:ID   1:DeviceName  2:Tagname  3:Tagaddress  4:CmdBuffer  5:status_success_rule
*  6:status_fail_rule  7:cmdName  8:cyclesec   9：getValueRule
*
*  规则二设定：
*  t_device_list字段说明:
*  param1:轮训周期,单位是秒;
*  param2:轮训开始时间;
*  param5:保存上次一轮询的时间点;
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

	//根据时间设定巡检周期;
	if (strcmp(pDevice->szParam1, ""))
	{
		if (tNow - pDevice->nUserData[0] >= (atoi(pDevice->szParam1)))
		{
			//更新巡检起始时间;
			pDevice->nUserData[0] = tNow;
			//执行巡检操作;
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "当前正在进行Device: %s 自动巡检操作,请勿执行其他操作", pDevice->szName);
			//发送控制信息，判断每个设备的运行状态;
			char szSQL[MAX_LEN_SQLEXECUTE];
			sprintf(szSQL, "select * from t_common_driver_config where device_name = '%s' and  ischeck =1;", pDevice->szName);
			vector<vector<string> > vecRows;
			string strSqlErr;
			int nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
			if (vecRows.size() == 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name = %s, 未找到对应巡检设备配置，请检查t_common_drivers_config", pDevice->szName);
				Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
				//更新巡检状态;
				UpdateInspectInfo(-1, pDevice);
				return -1;
			}
			for (int i = 0; i < vecRows.size(); i++)
			{
				//判断设备名称与当前设备是否保持一致;
				char szSendBuffer[MAX_LEN_BUUFFER] = { 0 };
				char szRecvBuffer[MAX_LEN_BUUFFER] = { 0 };
				string strSuccessRule;
				string strFailureRule;
				if (vecRows[i][1] == pDevice->szName)
				{
					strSuccessRule = vecRows[i][5];		//成功规则;
					strFailureRule = vecRows[i][6];		//失败规则;
					sprintf(szSendBuffer, vecRows[i][4].c_str());
					nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 1000);
					if (nRet < 0)
					{
						//更新设备连接失败的状态-1;
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -1.(向设备发送请求失败)", pDevice->szName);
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
						//更新巡检状态;
						UpdateInspectInfo(-1, pDevice);
						vecRows.clear();
						return -1;
					}
					//判断返回值是否在某个集合中;
					nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 1000);
					if (nRet <= 0)
					{
						//更新设备连接失败的状态 -1;
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -1.(向设备请求数据失败)", pDevice->szName);
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
						//更新巡检状态;
						UpdateInspectInfo(-1, pDevice);
						vecRows.clear();
						return -1;
					}
					int nSuccess = strSuccessRule.find(szRecvBuffer);
					int nFailure = strFailureRule.find(szRecvBuffer);
					if (nSuccess > 0)
					{
						//收到设备连接成功的命令0;
						Drv_UpdateTagsDataByAddress(pDevice, "status", "0", sizeof(int), 1000, 0, 0);
						//更新巡检状态;
						UpdateInspectInfo(0, pDevice);
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: 0.(收到设备返回数据，符合成功规则)", pDevice->szName);
					}

					if (nFailure > 0)
					{
						//收到设备连接失败的命令-2;
						Drv_UpdateTagsDataByAddress(pDevice, "status", "-2", sizeof(int), 1000, 0, 0);
						//更新巡检状态;
						UpdateInspectInfo(-2, pDevice);
						Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -2.(收到设备返回数据，符合失败规则)", pDevice->szName);
					}
				}
			}
			//清空结构体;
			vecRows.clear();
		}
	}

	//查找设备状态;
	//param字段： 发送指令(5);
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
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "未配置符合规则的查询信息，不执行查询操作");
			return -1;
		}
		strcpy(szSendBuffer, strParam.substr(0, nLoc).c_str());
		string strSend = szSendBuffer;
		strGet = strParam.substr(nLoc);

		//设备参数4填写0或者不填均认为是发送16进制数据;
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
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,执行：%s 操作失败， nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				return -1;
			}
			nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 3000);
			if (nRet > 0)
			{
				// 如果字符串长度大于3，需要获取两个字符的十进制数据;
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
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "获取点值成功,szAddress: %s, value: %s", pTag->szAddress, szVal);
				}
				else //获取单个字符的十进制数据;
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
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "获取点值成功,szAddress: %s, value: %s", pTag->szAddress, szVal);
				}
			}
		}
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OnTimer End");
	return 0;
}

/*
*
*  当有控制命令时该函数被调用;
*  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
*
*  @version     12/11/2008    Initial Version.
*
*  param4: 表示数据发送进制，0:十六进制  1:ASCii码
*
*  param3: 表示点检显示的设备名称，需要填写;
*
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	//手动执行巡检操作, 通过将多个点的值均赋值为1实现;
	if (0 == strcmp(pTag->szAddress, "xunjian"))
	{
		//执行巡检操作;
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "当前正在进行Device : %  手动巡检操作,请勿执行其他操作", pDevice->szName);
		//发送控制信息，判断每个设备的运行状态;
		char szSQL[MAX_LEN_SQLEXECUTE];
		sprintf(szSQL, "select * from t_common_driver_config where device_name = '%s' and  ischeck =1;", pDevice->szName);
		vector<vector<string> > vecRows;
		string strSqlErr;
		int nRet = pkdb.SQLExecute(szSQL, vecRows, &strSqlErr);
		if (nRet != 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name = %s, execute sql failure, SQL = %s", pDevice->szName, szSQL);
			Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
			//更新巡检状态;
			UpdateInspectInfo(-1, pDevice);
			return -1;
		}

		for (int i = 0; i < vecRows.size(); i++)
		{
			//判断设备名称与当前设备是否保持一致;
			char szSendBuffer[MAX_LEN_BUUFFER] = { 0 };
			char szRecvBuffer[MAX_LEN_BUUFFER] = { 0 };
			string strSuccessRule;
			string strFailureRule;

			if (vecRows[i][1] == pDevice->szName)
			{
				strSuccessRule = vecRows[i][5];		//成功规则;
				strFailureRule = vecRows[i][6];		//失败规则;

				sprintf(szSendBuffer, vecRows[i][4].c_str());
				nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 1000);

				if (nRet < 0)
				{
					//更新设备连接失败的状态 -1;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -1.(向设备发送请求失败)", pDevice->szName);

					//更新巡检状态;
					UpdateInspectInfo(-1, pDevice);
					vecRows.clear();
					return -1;
				}

				//判断返回值是否在某个集合中;
				nRet = Drv_Recv(pDevice, szRecvBuffer, sizeof(szRecvBuffer), 1000);
				if (nRet <= 0)
				{
					//更新设备连接失败的状态 -1;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-1", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -1.(向设备请求数据失败)", pDevice->szName);
					//更新巡检状态;
					UpdateInspectInfo(-1, pDevice);

					vecRows.clear();
					return -1;
				}
				int nSuccess = strSuccessRule.find(szRecvBuffer);
				int nFailure = strFailureRule.find(szRecvBuffer);

				if (nSuccess > 0)
				{
					//收到设备连接成功的命令0;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "0", sizeof(int), 1000, 0, 0);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: 0.(收到设备返回数据，符合成功规则)", pDevice->szName);
					//更新巡检状态;
					UpdateInspectInfo(0, pDevice);
				}
				else if (nFailure > 0)
				{
					//收到设备连接失败的命令-2;
					Drv_UpdateTagsDataByAddress(pDevice, "status", "-2", sizeof(int), 1000, 0, 0);
					//更新巡检状态;
					UpdateInspectInfo(-2, pDevice);
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态: -2.(收到设备返回数据，符合失败规则)", pDevice->szName);
				}
				else
				{
					Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device name:%s, 巡检完成，设备的当前状态未知.(收到设备返回数据，但不符合配置规则，请检查规则配置)", pDevice->szName);
				}
			}
		}
		//清空结构体;
		vecRows.clear();
		return 0;
	}

	string strControlAddr(pTag->szAddress);

	//通用控制;
	if ((0 == strcmp(pTag->szAddress, "dapingmu_yihaobuju") || (0 == strcmp(pTag->szAddress, "dapingmu_erhaobuju")) || (0 == strcmp(pTag->szAddress, "dapingmu_sanhaobuju")) || (0 == strcmp(pTag->szAddress, "dapingmu_sihaobuju"))																																												 //大屏模式------应用一号布局（1-4）;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_dapingmubuju6"))					 //模式选择-----应用布局(1-6);
		|| (0 == strcmp(pTag->szAddress, "shixudianyuan_dakaidianyuan")) || (0 == strcmp(pTag->szAddress, "shixudianyuan_guanbidianyuan"))																																																																			 //时序电源--打开电源/关闭电源;
		|| (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi1")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi2")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi3")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi4")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi5")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_moshi6"))													 //音频处理器----应用模式模式（1-4）;
		|| (0 == strcmp(pTag->szAddress, "yinpinchuliqi_jingyin")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_quxiaojingyin")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_zengjiayinliang")) || (0 == strcmp(pTag->szAddress, "yinpinchuliqi_jianxiaoyinliang"))																																			 //音频处理器----增加音量、减小音量、静音、取消静音;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_yinliangkongzhi6"))  //模式选择-----音量控制（1-6）;
		|| (0 == strcmp(pTag->szAddress, "wutaideng_moshi1")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi2")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi3")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi4")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi5")) || (0 == strcmp(pTag->szAddress, "wutaideng_moshi6"))																			 //舞台灯---模式选择（1-6）;
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi2") || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi3"))) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_wutaidengmoshi6"))		 //模式选择-----舞台灯模式(1-6);
		|| (0 == strcmp(pTag->szAddress, "wutaideng_dengguanquankai")) || (0 == strcmp(pTag->szAddress, "wutaideng_dengguanquanguan"))																																																																				 //舞台灯----灯光全开、灯光全关;
		|| (0 == strcmp(pTag->szAddress, "yingdieji_guanji")) || (0 == strcmp(pTag->szAddress, "yingdieji_kaiji")) || (0 == strcmp(pTag->szAddress, "yingdieji_kaicang")) || (0 == strcmp(pTag->szAddress, "yingdieji_guancang"))																																													 //影碟机----开机、关机、开仓、关仓;
		|| (0 == strcmp(pTag->szAddress, "yingdieji_shangyige")) || (0 == strcmp(pTag->szAddress, "yingdieji_xiayige")) || (0 == strcmp(pTag->szAddress, "yingdieji_bofang")) || (0 == strcmp(pTag->szAddress, "yingdieji_zanting"))																																												 //影碟机---上一个、下一个、播放、暂停;	
		|| (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi1")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi2") || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi3"))) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi4")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi5")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_moshi6"))														 //照明灯-----模式选择;(1-6);
		|| (0 == strcmp(pTag->szAddress, "zhaomingdeng_dengguanquankai")) || (0 == strcmp(pTag->szAddress, "zhaomingdeng_dengguanquanguan"))																																																																		 //照明灯-----灯光全开、灯光全关;
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_shangsheng")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_shangsheng"))																																							 //翻转架-----上升(1-4)
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_xiajiang")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_xiajiang"))																																									 //翻转架-----下降(1-4)
		|| (0 == strcmp(pTag->szAddress, "fanzhuanjia1_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia2_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia3_tingzhi")) || (0 == strcmp(pTag->szAddress, "fanzhuanjia4_tingzhi"))																																										 //翻转架-----停止(1-4)
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_shang")) || (0 == strcmp(pTag->szAddress, "sheyingji1_xia")) || (0 == strcmp(pTag->szAddress, "sheyingji1_zuo")) || (0 == strcmp(pTag->szAddress, "sheyingji1_you"))																																															 //摄像头1-----（上下左右）
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_fangda")) || (0 == strcmp(pTag->szAddress, "sheyingji1_suoxiao")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei1")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei2"))																																										     //摄像头1-----（放大、缩小、预置位1、预置位2）
		|| (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei3")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei4")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei5")) || (0 == strcmp(pTag->szAddress, "sheyingji1_yuzhiwei6"))																																										 //摄像头1-----（预置位1、预置位2、预置位3、预置位4）
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_shang")) || (0 == strcmp(pTag->szAddress, "sheyingji2_xia")) || (0 == strcmp(pTag->szAddress, "sheyingji2_zuo")) || (0 == strcmp(pTag->szAddress, "sheyingji2_you"))																																															 //摄像头2-----（上下左右）
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_fangda")) || (0 == strcmp(pTag->szAddress, "sheyingji2_suoxiao")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei1")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei2"))																																										     //摄像头2-----（放大、缩小、预置位1、预置位2）
		|| (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei3")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei4")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei5")) || (0 == strcmp(pTag->szAddress, "sheyingji2_yuzhiwei6"))																																										 //摄像头2-----（预置位1、预置位2、预置位3、预置位4）
		|| (0 == strcmp(pTag->szAddress, "hongwai_tv"))
		|| (strControlAddr.find("ctrl") >= 0)
		|| (0 == strcmp(pTag->szAddress, "dapingmu_dakaidianyuan")) || (0 == strcmp(pTag->szAddress, "dapingmu_guanbidianyuan"))))				 																																																																     //大屏模式----打开电源、关闭电源																																																															 //照明灯------灯光全开、灯光全关;
	{
		if (0 == strcmp(pDevice->szParam4, "1"))
		{
			char szSendBuffer[MAX_LEN_SENDBUFFER] = { 0 };
			sprintf(szSendBuffer, "%s", pTag->szParam);

			int nRet = Drv_Send(pDevice, szSendBuffer, sizeof(szSendBuffer), 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,执行：%s 操作失败， nRet = %d", pDevice->szName, pTag->szAddress, nRet);
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
				Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device : %s ,执行：%s 操作失败， nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				vStr.clear();
				return -1;
			}
			vStr.clear();
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "t_device_list 表格中Param4参数填写有误（0：16进制发送   1：ASCII发送）,请检查配置");
			return -1;
		}
	}

	//==============================================视频矩阵应用==========================================================
	//视频矩阵-----信号切换(1-4)
	//0x0C, 0x01, 0x05, 0x08（模式，ID，输入通道，输出通道）
	if ((0 == strcmp(pTag->szAddress, "shipinjuzhen_output1")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output2")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output3")) || (0 == strcmp(pTag->szAddress, "shipinjuzhen_output4"))
		|| (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen1")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen2")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen3")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen4")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen5")) || (0 == strcmp(pTag->szAddress, "moshixuanze_shipinjuzhen6")))					 //视频矩阵-----大屏幕应用模式（1-6）;
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


			//如果容器大小不是2，则表示控制命令格式发送有误;
			if (vStr.size() != 2)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,视频矩阵切换控制命令格式发送有误：%s", pDevice->szName, szSendBuffer);
				return -1;
			}

			for (int i = 0; i < vStr.size(); ++i)
			{
				szSendBuffer[i + 2] = StringToChar(vStr[i]);
			}
			int nRet = Drv_Send(pDevice, szSendBuffer, vStr.size() + 2, 3000);
			if (nRet < 0)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,视频矩阵切换信号失败，控制命令：%s ， nRet = %d", pDevice->szName, szSendBuffer, nRet);
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
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "Device : %s ,执行：%s 操作失败， nRet = %d", pDevice->szName, pTag->szAddress, nRet);
				return -1;
			}
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "t_device_list 表格中Param4参数填写有误（0：16进制发送   1：ASCII发送）,请检查配置");
			return -1;
		}
	}
	return 0;
}

/*
反初始化设备;
*/
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	return 0;
}

/*
反初始化驱动;
*/
PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDriver(driver:%s)", pDriver->szName);
	return 0;
}

//更新巡检表格;
void UpdateInspectInfo(int id, PKDEVICE *device)
{
	char szCurrentTime[32] = { 0 };
	time_t timep;
	struct tm *p;
	time(&timep);						 /*获得time_t结构的时间，UTC时间*/
	p = localtime(&timep);				 /*转换为struct tm结构的UTC时间*/
	sprintf(szCurrentTime, "%2d-%.2d-%.2d %2d:%.2d:%.2d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	//更新数据库中内容;
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

	//执行更新操作;
	pkdb.SQLExecute(szUpdateSql);
}

//string 转 char; 
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

//字符串转时间戳(linux&windows);
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
	time_t t_ = mktime(&tm_); //已经减了8个时区
	return  mktime(&tm_);
}