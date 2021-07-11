/**************************************************************
 *  Filename:    DeviceScanTask.cpp
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备数据块扫描类.
 *
 *  @author:     shijunpu
 *  @version     05/28/2008  shijunpu  Initial Version
**************************************************************/
#include "ace/ACE.h"
#include "ace/Time_Value.h"
#include "OpcServerTask.h"
#include "OPCServer.h"
#include "OPCGroup.h"
#include "OPCItem.h"
#include "pkdriver/pkdrvcmn.h"

COPCServerTask::COPCServerTask(COPCServer* pOPCServer )
{
	m_pOPCServer = pOPCServer;
	m_bStop = false;
	m_bStopped = false;
}

COPCServerTask::~COPCServerTask()
{
	
}

int COPCServerTask::Start()
{
	return activate(THR_NEW_LWP | THR_JOINABLE |THR_INHERIT_SCHED);
}

void COPCServerTask::Stop()
{
	m_bStop = true;
	int nWaitResult = wait();
}

/**
 *  虚函数，继承自ACE_Task.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
int COPCServerTask::svc()
{
	// 该任务中，处理控制命令和定时读写OPCGroup的任务
	ACE_High_Res_Timer::global_scale_factor();
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "svc");
	int nRet = 0;
	long lErr = PK_SUCCESS;
	while(!m_bStop)
	{
		if(m_pOPCServer->IsStop())
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "OPCServerTask, OPCServer(%s) stopped detected.", m_pOPCServer->m_szName);
			break;
		}

		// 尝试获取控制命令
		ACE_Time_Value tvTimes = ACE_OS::gettimeofday() + ACE_Time_Value(0, 20 * 1000); // 20ms
		ACE_Message_Block *pMsgBlk = NULL;
		while(getq(pMsgBlk, &tvTimes) >= 0)
		{
			char *pRecvBuf = pMsgBlk->rd_ptr();
			int nLeftBufLen = pMsgBlk->total_length();
			if(nLeftBufLen < sizeof(OUTPUT_DESC_EX) + sizeof(int))
			{
				Drv_LogMessage(PK_LOGLEVEL_INFO, "OPCServerTask recv an control but len invalid(should:%d,actual:%d)",
					sizeof(OUTPUT_DESC_EX), nLeftBufLen);
				pMsgBlk->release();
				continue;
			}
			OUTPUT_DESC_EX output;
			int nCmdNo = 0;
			memcpy(&nCmdNo, pRecvBuf, sizeof(int));
			pRecvBuf += sizeof(int);
			memcpy(&output, pRecvBuf, sizeof(OUTPUT_DESC_EX));

			Drv_LogMessage(PK_LOGLEVEL_INFO, "OPCServerTask recv an control tagname:%s,addr:%s,value:%s",
				output.pTag->szName, output.pTag->szAddress, output.szValue);

			m_pOPCServer->OnControl(&output);
			pMsgBlk->release();

		}//if(getq(pMsgBlk, &tvTimes) >= 0)

		// 判断连接是否正常？
		if (!m_pOPCServer->IsConnected())
		{	// 不正常则进行冗余切换逻辑
			lErr = m_pOPCServer->ConnectToOPCServer();
		}
		if (lErr != PK_SUCCESS)
		{
			QUALITY_STATE quality = {0};
			quality.nQuality = QUALITY_BAD;
			quality.nSubStatus = SS_NOT_CONNECTED;
			m_pOPCServer->SetAllBlockStatusBad(quality);
			continue;
		}

		// 如果正常，进行读取逻辑
		// 扫描设备组下的所有设备
		std::list<COPCGroup *>::iterator itGroup = m_pOPCServer->m_listGroup.begin();
		for ( ; itGroup != m_pOPCServer->m_listGroup.end(); itGroup++)
		{
			COPCGroup* pGroup = *itGroup;
			ReadOPCGroup(pGroup);
			pGroup->CheckAsyncTimeout(); //检查异步超时
		}
	}//while(!m_bStop)

	Drv_LogMessage(PK_LOGLEVEL_NOTICE,"Process Manager Maintask exit normally！");

	m_bStopped = true;
	return PK_SUCCESS;	
}

/**
 *  处理数据块，若到达扫描周期，则生成读命令.
 *
 *  @param  -[in]  CDataBlock*  pDataBlock: [comment]
 *
 *  @version     07/09/2008  shijunpu  Initial Version.
 */
long COPCServerTask::ReadOPCGroup(COPCGroup* pGroup)
{
	// TODO:
	long nStatus = PK_SUCCESS;

	CHECK_NULL_PARAMETER_RETURN_ERR(pGroup);

	ACE_Time_Value now = ACE_OS::gettimeofday();
	ACE_Time_Value tvInterval = ACE_Time_Value::zero;
	tvInterval.msec(pGroup->m_nPollRate);
	if (now < pGroup->m_tvLastPollRead + tvInterval)
		return PK_SUCCESS; 
	pGroup->m_tvLastPollRead = now;

	pGroup->TryRegOnce();

	if (pGroup->m_bAsync)
	{
		pGroup->AsyncReadOPCGroup();
	}
	else
	{
		pGroup->SyncReadOPCGroup();
	}

	// 更新该组的所有数据到内存数据库
	int nRet = Drv_UpdateTagsData(this->m_pOPCServer->m_pDevice, pGroup->m_vecTags.data(), pGroup->m_vecTags.size());
	if(nRet != PK_SUCCESS)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "从OPC服务(%s)读取到一组数据，并更新该组(%s)失败，返回：%d", m_pOPCServer->m_szName, pGroup->m_szName, nRet);
	}
	return PK_SUCCESS;
}

