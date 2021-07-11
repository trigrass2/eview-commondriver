#include "COPCCallback.h"
#include "pkcomm/pkcomm.h"
#include "ace/High_Res_Timer.h"
#include "ace/OS_NS_sys_time.h"
#define DT_SIZE_TIMESTR						24		// strlen("2008-01-01 12:12:12.000") + 1

#define _(STRING) (STRING)
CComModule _Module;

/**
 *  将每个callback对象与相应的opc group绑定
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
void COPCCallback::SetGroup( COPCGroup * pDevice )
{
	m_pGroup = pDevice;
	m_tvLastTimeStamp = ACE_Time_Value(0);
}

/**
 *  OPC Server上数据发生变化后，通过回调函数来更新dit中的数据
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
STDMETHODIMP COPCCallback::OnDataChange( 
/* [in] */ DWORD dwTransid,
/* [in] */ OPCHANDLE hGroup,
/* [in] */ HRESULT hrMasterquality,
/* [in] */ HRESULT hrMastererror,
/* [in] */ DWORD dwCount,
/* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
/* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
/* [size_is][in] */ WORD __RPC_FAR *pwQualities,
/* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
/* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
{
	//更新数据，与OnReadCompleted功能相同
	//return S_OK;
	//找到相应的datablock，并更新其值
	return OnReadComplete(dwTransid, hGroup, hrMasterquality, hrMastererror, dwCount, 
					phClientItems, pvValues, pwQualities, pftTimeStamps, pErrors);
}

/**
 *  OPC Server上异步读取完成时，通过回调函数来更新dit中的数据
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
STDMETHODIMP COPCCallback::OnReadComplete( 
/* [in] */ DWORD dwTransid,
/* [in] */ OPCHANDLE hGroup,
/* [in] */ HRESULT hrMasterquality,
/* [in] */ HRESULT hrMastererror,
/* [in] */ DWORD dwCount,
/* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
/* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
/* [size_is][in] */ WORD __RPC_FAR *pwQualities,
/* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
/* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
{
	if(FAILED(hrMastererror))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Asynchronous Read or Update data failed, ErrCode=%d."), hrMastererror);//异步读取或数据更新失败, ErrCode=%d
	}

	ACE_Time_Value timeStamp = ACE_Time_Value(pftTimeStamps[0]);
	char szTimeBuf[DT_SIZE_TIMESTR];

	PKTimeHelper::HighResTime2String(szTimeBuf, DT_SIZE_TIMESTR, timeStamp.sec(), timeStamp.usec()/1000);
	
	Drv_LogMessage(PK_LOGLEVEL_INFO, _("COPCCallback::OnReadComplete Start [time=%s]"), szTimeBuf);
	
	//找到相应的datablock，并更新其值
	DWORD dw=0;
	COPCGroup::STD_DBMAP::iterator iter;
	for(; dw < dwCount; dw++)
	{
		iter = m_pGroup->m_DataBlockMap.find(phClientItems[dw]);
		if (iter != m_pGroup->m_DataBlockMap.end())
		{
			iter->second->SetValue(pvValues[dw], pftTimeStamps[dw], pwQualities[dw], pErrors[dw]);
		}
	} // end for
	Drv_LogMessage(PK_LOGLEVEL_INFO, _("COPCCallback::OnReadComplete End [total=%d time=%s]"), dw, szTimeBuf);

	//更新设备的读取状态
	//m_tvLastTimeStamp = timeStamp;
	m_pGroup->m_tvLastDataReceived = ACE_OS::gettimeofday();
	m_pGroup->m_bIsAsyncTimeout = false;
	return S_OK;
}

/**
 *  OPC Server上异步写数据完成后，通过回调函数来写控制反馈
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
STDMETHODIMP COPCCallback::OnWriteComplete( 
/* [in] */ DWORD dwTransid,
/* [in] */ OPCHANDLE hGroup,
/* [in] */ HRESULT hrMastererr,
/* [in] */ DWORD dwCount,
/* [size_is][in] */ OPCHANDLE __RPC_FAR *pClienthandles,
/* [size_is][in] */ HRESULT __RPC_FAR *pErrors)
{
	if(FAILED(hrMastererr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("General Async2 Write, ErrCode=%d"), hrMastererr);
	}

	//CCtrlProcessTask::ReturnWriteAck(dwTransid, pErrors[0]);

	return S_OK;
}

STDMETHODIMP COPCCallback::OnCancelComplete( 
/* [in] */ DWORD dwTransid,
/* [in] */ OPCHANDLE hGroup)
{
	return S_OK;
}
