/**************************************************************
 *  Filename:    Device.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: �豸��Ϣʵ����.
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

	// ������ݿ�
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
	// �Ƴ����е����ݿ�
	long RemoveAllDataBlock();
	void InitializeOPCGroup();
	
public:
	std::map<DWORD,COPCItem*>			m_DataBlockMap;	// ���ݿ��б�
	typedef std::map<DWORD,COPCItem*>	STD_DBMAP;
	COPCServer							*m_pOPCServer;	// �����豸��ָ��
	int									m_nGroupState;	// �豸״̬

	char  m_szName[PK_NAME_MAXLEN];			// �豸����
	char  m_szDescription[PK_DESC_MAXLEN];	// �豸����

	int	 m_nRefreshRate;	// �����ˢ������
	float  m_fDeadband;		// ����

	bool m_bDataFromCache;	// �Ƿ��Cache�ж�ȡ��0ʱΪDevice��
	bool m_bAsync;			// �Ƿ��첽��ʽ��ȡ

	int m_nPollRate;		// ͬ��ʱ���ͻ���ˢ�����ڣ��첽�������Watchdog����
	int m_nAsyncTimeout;	// �첽��ʱʱ��
	bool m_bWatchdogEnable;	// �Ƿ�����Watchdog

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

	vector<PKTAG *>	m_vecTags;	// ���ڸ��¸������������
private:
	unsigned int m_nRegTimes;
	static const unsigned int m_nMaxRegTimes;
};

#endif  // _DEVICE_H_
