/**************************************************************
 *  Filename:    Device.cpp
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备信息实体类.
 *
 *  @author:     shijunpu
 *  @version     05/28/2008  shijunpu  Initial Version
 *  @version	2/19/2013  shijunpu  解决内存泄露问题.
**************************************************************/
#include "ace/OS.h"
#include "OPCGroup.h"
#include "OPCServer.h"
#include "CommHelper.h"
//#include "errcode/ErrCode_iCV_DA.hxx"
#include "opcda.h"
#include "COPCCallback.h"

#define _(STRING) (STRING)
int g_dwReadTransID;

const unsigned int COPCGroup::m_nMaxRegTimes = 5;

/**
 *  构造函数.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCGroup::COPCGroup()
{
	m_pOPCServer = NULL;

	memset(m_szName, 0, PK_NAME_MAXLEN);
	memset(m_szDescription, 0, PK_DESC_MAXLEN);
		
	m_pIDataObject = NULL;
	m_pIOPCGroupStateMgt = NULL;
	m_pIOPCSyncIO = NULL;
	m_pIOPCItemMgt = NULL;
	m_pIOPCAsyncIO2 = NULL;
	m_pIGroupUnknown = NULL;
	m_tvLastPollRead = ACE_Time_Value::zero;
	m_tvLastDataReceived = ACE_Time_Value::zero;
	m_bIsAsyncTimeout = true;
	m_pCallBack = NULL;
	m_dwCookie = 0;
	m_pCP = NULL;
	m_pCPC = NULL;

	m_bAsync = true;
	m_bDataFromCache = false;
	m_fDeadband = 0.0f;
	m_hClientGroup = NULL;
	m_nAsyncTimeout = 0;
	m_nGroupState = 0;
	m_nPollRate = 0;
	m_nRefreshRate = 500;
	m_nRegTimes = 0;
}

/**
 *  析构函数.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCGroup::~COPCGroup()
{
	// 注销所有的OPCItems和OPC Group
	RemoveOPCGroupAndItems();

	// 断开与设备的连接
	RELEASE(m_pCP);
	RELEASE(m_pCPC);
	RELEASE(m_pCallBack);
	RELEASE(m_pIGroupUnknown);
	RELEASE(m_pIOPCAsyncIO2);
	RELEASE(m_pIOPCSyncIO);
	RELEASE(m_pIOPCItemMgt);
	RELEASE(m_pIOPCGroupStateMgt);
	RELEASE(m_pIDataObject);

	// 删除所有的数据块
	RemoveAllDataBlock();
}

/**
 *  添加数据块.
 *
 *  @param  -[in]  CDataBlock*  pDataBlock: [comment]
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
long COPCGroup::AddItem(COPCItem* pDataBlock)
{
	CHECK_NULL_PARAMETER_RETURN_ERR(pDataBlock);

	// 检查数据块是否已经存在
	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	for ( ; iter != m_DataBlockMap.end(); iter++)
	{
		COPCItem* pDataBlockInMap = iter->second;
		if (pDataBlockInMap->m_strName.compare(pDataBlock->m_strName) == 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_DEBUG, _("Devicegroup(%s)->device(%s)->datablock(%s) has exist."), m_pOPCServer->m_szName, this->m_szName, pDataBlockInMap->m_strName);
			return EC_ICV_DA_CONFIG_OVERLAP;
		}
	}

	// 数据块不存在，添加数据块
	pDataBlock->m_pOPCGroup = this;
	m_DataBlockMap.insert(std::make_pair(pDataBlock->m_dwHClient,pDataBlock));
	return PK_SUCCESS;
}

/**
 *  移除所有的数据块.
 *
 *
 *  @version     07/09/2008  shijunpu  Initial Version.
 */
long COPCGroup::RemoveAllDataBlock()
{
	COPCItem* pDataBlock = NULL;
	
	// 遍历DataBlockList，删除所有的数据块
	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	while(iter != m_DataBlockMap.end())
	{
		pDataBlock = iter->second;
		m_DataBlockMap.erase(iter++);
		delete pDataBlock;
	}

	return PK_SUCCESS;
}

void COPCGroup::AddOPCGroupAndItems()
{
	// 初始化OPC Group
	InitializeOPCGroup();
	// 添加OPC Item

	COPCItem* pDataBlock = NULL;
	
	// 遍历DataBlockList，添加所有的数据块
	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	while(iter != m_DataBlockMap.end())
	{
		pDataBlock = iter->second;
		pDataBlock->AddOPCItem();
		iter++;
	}

	// 如果为异步读，则要定义CallBack
	if (m_bAsync)
	{
		RegisterCallBack();
	}
	return;
}

void COPCGroup::RemoveOPCGroupAndItems()
{
	COPCItem* pDataBlock = NULL;

	// 遍历DataBlockList，删除所有的数据块
	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	while(iter != m_DataBlockMap.end())
	{
		pDataBlock = iter->second;
		pDataBlock->RemoveOPCItem();
		iter++;
	}

	// 注销OPC组本身
	if ((NULL != m_pOPCServer) && (NULL != m_pOPCServer->m_pIOPCServer) && (NULL != m_hClientGroup))
	{
		m_pOPCServer->m_pIOPCServer->RemoveGroup(m_hClientGroup, FALSE);
	}
}

void COPCGroup::InitializeOPCGroup()
{
	if(!m_pOPCServer->m_pIOPCServer)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("OPC(NAME:%s) the m_pIOPCServer of device group is NULL."), m_pOPCServer->m_szName);
	}

	float fTemp = 0.0f;
	long lTimeBias = 0;
	DWORD dwRevisedUpdateRate = 0;
	HRESULT hr;
	// create an in-active group
	// NOTE: 1st param must not be a NULL or the proxy will puke
	hr = m_pOPCServer->m_pIOPCServer->AddGroup(L"",					// [in] Server name, if NULL OPC Server will generate a unique name
		TRUE		,			// [in] State of group to add
		m_nRefreshRate,		// [in] Requested update rate for group (ms)
		1234,					// [in] Client handle to OPC Group //TODO:
		&lTimeBias,			// [in] Time 
		&m_fDeadband,				// [in] Percent Deadband
		0,						// [in] Localization ID
		&m_hClientGroup,		// [out] Server Handle to group
		&dwRevisedUpdateRate,	// [out] Revised update rate
		IID_IUnknown,			// [in] Type of interface desired
		&m_pIGroupUnknown);	// [out] where to store the interface pointer
	
	if(FAILED(hr))
	{
		//ShowError(hr,"AddGroup()");
		//g_pIOPCServer->Release();
		//return 1;
	}

	if(!m_pIGroupUnknown)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("OPC(CLSID:%s) the m_pIGroupUnknown of device group is NULL."), m_pOPCServer->m_clsidString);
		return;
	}
	
	Drv_LogMessage(PK_LOGLEVEL_INFO, _("Group added, update rate = %ld.\n"), dwRevisedUpdateRate);
	
	RELEASE(m_pIDataObject);
	// Get pointer to OPC Server interfaces required for this program.
	hr = m_pIGroupUnknown->QueryInterface(IID_IDataObject, (void**)&m_pIDataObject);
	if(FAILED(hr))
	{
		;//ShowError(hr,"QueryInterface(IID_IDataObject)");
	}
	RELEASE(m_pIOPCGroupStateMgt);
	hr = m_pIGroupUnknown->QueryInterface(IID_IOPCGroupStateMgt, (void**)&m_pIOPCGroupStateMgt);
	if(FAILED(hr))
	{
		;//ShowError(hr,"QueryInterface(IID_IOPCGroupStateMgt)");
	}

	RELEASE(m_pIOPCItemMgt);
	hr = m_pIGroupUnknown->QueryInterface(IID_IOPCItemMgt, (void**)&m_pIOPCItemMgt);
	if(FAILED(hr))
	{
		;//ShowError(hr,"QueryInterface(IID_IOPCItemMgt)");
	}

	RELEASE(m_pIOPCSyncIO);
	hr = m_pIGroupUnknown->QueryInterface(IID_IOPCSyncIO, (void**)&m_pIOPCSyncIO);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("[%s@%s] QueryInterface(IID_IOPCSyncIO) Failed [ErrCode = %s]"), m_pOPCServer->m_szName, m_pOPCServer->m_strCurrentServer.c_str(), hr);
		;//ShowError(hr,"QueryInterface(IID_IOPCSyncIO)");
	}

	RELEASE(m_pIOPCAsyncIO2);
	hr = m_pIGroupUnknown->QueryInterface(IID_IOPCAsyncIO2, (void**)&m_pIOPCAsyncIO2);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("[%s@%s] QueryInterface(IID_IOPCAsyncIO2) Failed [ErrCode = %s]"), m_pOPCServer->m_szName, m_pOPCServer->m_strCurrentServer.c_str(), hr);
		;//ShowError(hr,"QueryInterface(IID_IOPCAsyncIO2)");
		m_pIOPCAsyncIO2 = NULL;
	}
	
	if(static_cast<int>(dwRevisedUpdateRate) != m_nRefreshRate)
	{
		m_nRefreshRate = dwRevisedUpdateRate;
	}
	
	Drv_LogMessage(PK_LOGLEVEL_INFO, _("Active Group interface added.\n"));
}

/**
 *  $(Desp) .
 *  $(Detail) .
 *
 *  @return		void.
 *
 *  @version	???????  ????????  Initial Version.
 *  @version	2/19/2013  shijunpu  解决内存泄露问题 .
 */
void COPCGroup::SyncReadOPCGroup()
{
	if(!m_pIOPCSyncIO)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("OPC device(NAME:%s) the m_pIOPCSyncIO of device group is NULL."), this->m_szName);
		return;
	}

	size_t nItems = m_DataBlockMap.size();

	OPCHANDLE* hServer = new OPCHANDLE[nItems];
	VARIANT vCount;
	OPCITEMSTATE *pItemState = NULL;
	HRESULT *pErrors = NULL;
	HRESULT hr;

	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	int nIndex = 0;
	while(iter != m_DataBlockMap.end())
	{
		if (iter->second->m_hItem)
		{
			hServer[nIndex] = iter->second->m_hItem;
			//::VariantInit(&Val[nIndex]);
			nIndex ++;
		}
		iter++;
	}

	nItems = nIndex;

	::VariantInit(&vCount);
	V_VT(&vCount) = VT_I2;
	V_I2(&vCount) = 0;

	hr = m_pIOPCSyncIO->Read(m_bDataFromCache ? OPC_DS_CACHE : OPC_DS_DEVICE,
			nItems, 
			&hServer[0], 
			&pItemState, 
			&pErrors);

	if (FAILED(hr) || pItemState == NULL || pErrors == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, _("SyncIO->Read device(%s) failed"), this->m_szName);
		if (pItemState != NULL)
		{
			for (int i = 0; i < nItems; i++)
				VariantClear (&pItemState [i].vDataValue);

			::CoTaskMemFree(pItemState);
		}
		if (pErrors != NULL)
		{
			::CoTaskMemFree(pErrors);
		}
		if (hServer != NULL)
		{
			delete[] hServer;
		}		
		return;
	}

	STD_DBMAP::iterator iter1 = m_DataBlockMap.begin();
	nIndex = 0;
	while(iter1 != m_DataBlockMap.end())
	{
		if (iter1->second->m_hItem)
		{
			COPCItem *pOPCItem = iter1->second;
			if (S_OK == pErrors[nIndex])
			{
				pOPCItem->SetValue(pItemState[nIndex].vDataValue, pItemState[nIndex].ftTimeStamp, pItemState[nIndex].wQuality, pErrors[nIndex]);
			}
			else
			{
				pOPCItem->SetValue(pItemState[nIndex].vDataValue, pItemState[nIndex].ftTimeStamp, pItemState[nIndex].wQuality, pErrors[nIndex]);
				Drv_LogMessage(PK_LOGLEVEL_DEBUG, _("OPCServer(%s),SyncIO->Read Group(%s)->Item(%s) failed"), 
					this->m_pOPCServer->m_szName, this->m_szName, iter1->second->m_strName.c_str());
			}
			nIndex ++;
		}
		iter1++;
	}

	if (pItemState != NULL)
	{
		//必须循环释放variant内存，否则会造成内存泄露
		for (int i = 0; i < nItems; i++)
			VariantClear (&pItemState [i].vDataValue);
		::CoTaskMemFree(pItemState);
	}
	if (pErrors != NULL)
	{
		::CoTaskMemFree(pErrors);
	}
	if (hServer != NULL)
	{
		delete[] hServer;
	}
}

void COPCGroup::AsyncReadOPCGroup()
{
	HRESULT hr = S_OK;
	// 不需要主动读取
	return;

	if(!m_pIOPCAsyncIO2)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("OPC device(NAME:%s) the m_pIOPCAsyncIO2 of device group is NULL."), this->m_szName);
	}

	size_t nItems = m_DataBlockMap.size();
	OPCHANDLE *hServer = new OPCHANDLE[nItems];
	VARIANT *Val = new VARIANT[nItems];
	VARIANT vCount;

	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	int nIndex = 0;
	while(iter != m_DataBlockMap.end())
	{
		hServer[nIndex] = iter->second->m_hItem;
		::VariantInit(&Val[nIndex]);
		nIndex ++;
		iter++;
	}
	
	::VariantInit(&vCount);
	V_VT(&vCount) = VT_I2;
	V_I2(&vCount) = 0;

	HRESULT *pErrors = NULL;
	DWORD dwCancelID = 1;


	hr = m_pIOPCAsyncIO2->Read(nItems, hServer, ++g_dwReadTransID, &dwCancelID, &pErrors);
	if(FAILED(hr))
	{
		printf(_("Async read failed. \n"));
	}

	::CoTaskMemFree(pErrors);

	
	for(DWORD dw = 0; dw < nItems; dw++)
	{
		::VariantClear(&Val[dw]);
	}
	::VariantClear(&vCount);

	delete[] hServer;
	delete[] Val;
}

void COPCGroup::CheckAsyncTimeout()
{
	if (m_bWatchdogEnable && !m_bIsAsyncTimeout && m_bAsync)
	{
		ACE_Time_Value now = ACE_OS::gettimeofday();
		ACE_Time_Value tvInterval = ACE_Time_Value::zero;
		tvInterval.msec(m_nAsyncTimeout);
		if (m_tvLastDataReceived + tvInterval < now)
		{
			STD_DBMAP::iterator iter = m_DataBlockMap.begin();
			while(iter != m_DataBlockMap.end())
			{
				COPCItem *pDataBlock = iter->second;
				pDataBlock->SetAsyncTimeout();
				iter++;
			}
		}
		m_bIsAsyncTimeout = true;
	}
	
}

long COPCGroup::RegisterCallBack()
{
	HRESULT hr;

	if (m_pCP)
	{
		m_pCP->Unadvise(m_dwCookie);
	}

	RELEASE(m_pCP);
	RELEASE(m_pCPC);
	//RELEASE(m_pCallBack);
	
	CComCOPCCallback *pSink = NULL;
	ATLTRY(pSink = new CComCOPCCallback);
	if(pSink == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_CRITICAL, _("new COPCCallback Out of Memory!"));
		return E_OUTOFMEMORY;
	}

	pSink->AddRef();
	pSink->SetGroup(this);

	if(!m_pIGroupUnknown)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RegisterCallBack: OPC device(NAME:%s) the m_pIGroupUnknown of device group is NULL."), this->m_szName);
		return -1;
	}
	// obtain connection points
	hr = m_pIGroupUnknown->QueryInterface(IID_IConnectionPointContainer, (void**)&m_pCPC);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("QueryInterface(IID_IConnectionPointContainer) ErrCode = %d"), hr);
		delete pSink;
		return hr;
	}

	if(!m_pCPC)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RegisterCallBack OPC device(NAME:%s) the m_pCPC of device group is NULL."), this->m_szName);
		return -1;
	}
	hr = m_pCPC->FindConnectionPoint(IID_IOPCDataCallback, &m_pCP);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("FindConnectionPoint(IID_IOPCDataCallback) ErrCode = %d"), hr);
		//ShowError(hr, "FindConnectionPoint(IID_IOPCDataCallback)");
		delete pSink;
		return hr;
	}

	hr = m_pCP->Advise(pSink, &m_dwCookie);

	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Advise() ErrCode = %d"), hr);
		//ShowError(hr, "Advise()");
		delete pSink;
		return hr;
	}

	m_pCallBack = pSink;
	return PK_SUCCESS;
}

void COPCGroup::SetAllBlockStatusBad( QUALITY_STATE &quality )
{
	//RELEASE(m_pIGroupUnknown);
	//RELEASE(m_pCP);
	//RELEASE(m_pCPC);
	//RELEASE(m_pCallBack);
	//RELEASE(m_pIOPCAsyncIO2);
	//RELEASE(m_pIOPCSyncIO);
	//RELEASE(m_pIOPCItemMgt);
	//RELEASE(m_pIOPCGroupStateMgt);
	//RELEASE(m_pIDataObject);

	// 
	STD_DBMAP::iterator iter = m_DataBlockMap.begin();
	while(iter != m_DataBlockMap.end())
	{
		COPCItem *pDataBlock = iter->second;
		pDataBlock->SetQualityBad(quality);
		iter++;
	}
}

long COPCGroup::GetInvalidItem(std::list<COPCItem*>& lstInvalidItem)
{
	lstInvalidItem.clear();
	std::map<DWORD, COPCItem*>::iterator iter = m_DataBlockMap.begin();
	for (; iter != m_DataBlockMap.end(); ++iter)
	{
		if ( !( iter->second->IsValid() ) )
		{
			lstInvalidItem.push_back( iter->second );
		}
	}

	return lstInvalidItem.size();
}

void COPCGroup::TryRegOnce()
{
	if (m_nRegTimes <= m_nMaxRegTimes)
	{
		std::list<COPCItem*> lstInvalidItem;
		long nItemCnt = GetInvalidItem(lstInvalidItem);
		if (nItemCnt > 0)
		{
			std::list<COPCItem*>::iterator iter = lstInvalidItem.begin();
			for (; iter != lstInvalidItem.end(); ++iter)
			{
				(*iter)->AddOPCItem();
			}
			++m_nRegTimes;
		}	
	}
}
