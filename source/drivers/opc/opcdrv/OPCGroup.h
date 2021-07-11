/**************************************************************
 *  Filename:    Device.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备信息实体类.
 *
 *  @author:     lijingjing
 *  @version     05/28/2008  lijingjing  Initial Version
**************************************************************/

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "CommHelper.h"
#include "Quality.h"
#include "OPCItem.h"
#include "opcda.h"
#include "opccomn.h"
#include <ocidl.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <ace/Time_Value.h>

using namespace std;
class COPCServer;
class COPCCallback;

class COPCGroup
{
public:
	COPCGroup();
	virtual ~COPCGroup();

	// 添加数据块
	long AddItem(COPCItem* pItem);
	void AddOPCGroupAndItems();
	void RemoveOPCGroupAndItems();

	long RegisterCallBack();
	void SyncReadOPCGroup();
	void AsyncReadOPCGroup();
	void CheckAsyncTimeout();

	void SetAllBlockStatusBad(QUALITY_STATE &quality);

	long GetInvalidItem(std::list<COPCItem*>& lstInvalidItem);

	unsigned int GetRegTimes() { return m_nRegTimes; }

	void TryRegOnce();

private:
	// 移除所有的数据块
	long RemoveAllDataBlock();
	void InitializeOPCGroup();
	
public:
	std::map<DWORD,COPCItem*>			m_DataBlockMap;	// 数据块列表
	typedef std::map<DWORD,COPCItem*>	STD_DBMAP;
	COPCServer							*m_pOPCServer;	// 所属设备组指针
	int									m_nGroupState;	// 设备状态

	char  m_szName[PK_NAME_MAXLEN];			// 设备名称
	char  m_szDescription[PK_DESC_MAXLEN];	// 设备描述

	int	 m_nRefreshRate;	// 服务端刷新周期
	float  m_fDeadband;		// 死区

	bool m_bDataFromCache;	// 是否从Cache中读取（0时为Device）
	bool m_bAsync;			// 是否异步方式读取

	int m_nPollRate;		// 同步时：客户端刷新周期；异步：服务端Watchdog周期
	int m_nAsyncTimeout;	// 异步超时时间
	bool m_bWatchdogEnable;	// 是否允许Watchdog

	// OPC
	IDataObject *m_pIDataObject;
	IOPCGroupStateMgt *m_pIOPCGroupStateMgt;
	IOPCSyncIO *m_pIOPCSyncIO;
	IOPCItemMgt *m_pIOPCItemMgt;
	IOPCAsyncIO2 *m_pIOPCAsyncIO2;
	IUnknown *m_pIGroupUnknown;

	IConnectionPointContainer *m_pCPC;
	IConnectionPoint *m_pCP;

	COPCCallback*	m_pCallBack;
	DWORD m_dwCookie; // advise cookie for m_pCallBack;

	OPCHANDLE m_hClientGroup;
	ACE_Time_Value m_tvLastPollRead;
	ACE_Time_Value m_tvLastDataReceived;
	bool m_bIsAsyncTimeout;

	vector<PKTAG *>	m_vecTags;	// 用于更新该组的所有数据
private:
	unsigned int m_nRegTimes;
	static const unsigned int m_nMaxRegTimes;
};

#endif  // _DEVICE_H_
