#ifndef _OPC_CALLBACK_H_
#define _OPC_CALLBACK_H_

#include "OPCGroup.h"
#include "opcda.h"
#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>



class COPCCallback;
typedef CComObject<COPCCallback> CComCOPCCallback;

class ATL_NO_VTABLE COPCCallback : 
	public CComObjectRoot,
	public IOPCDataCallback
{
public:

BEGIN_COM_MAP(COPCCallback)
	COM_INTERFACE_ENTRY(IOPCDataCallback)
END_COM_MAP()
     
	STDMETHODIMP OnDataChange( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMasterquality,
    /* [in] */ HRESULT hrMastererror,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);

	STDMETHODIMP OnReadComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMasterquality,
    /* [in] */ HRESULT hrMastererror,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *phClientItems,
    /* [size_is][in] */ VARIANT __RPC_FAR *pvValues,
    /* [size_is][in] */ WORD __RPC_FAR *pwQualities,
    /* [size_is][in] */ FILETIME __RPC_FAR *pftTimeStamps,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);

	STDMETHODIMP OnWriteComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup,
    /* [in] */ HRESULT hrMastererr,
    /* [in] */ DWORD dwCount,
    /* [size_is][in] */ OPCHANDLE __RPC_FAR *pClienthandles,
    /* [size_is][in] */ HRESULT __RPC_FAR *pErrors);

	STDMETHODIMP OnCancelComplete( 
    /* [in] */ DWORD dwTransid,
    /* [in] */ OPCHANDLE hGroup);

	void SetGroup(COPCGroup * pGroup);
private:
	COPCGroup *m_pGroup;
	ACE_Time_Value m_tvLastTimeStamp;
};
#endif