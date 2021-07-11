/**************************************************************
 *  Filename:    DataBlock.cpp
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: ���ݿ���Ϣʵ����.
 *
 *  @author:     shijunpu
 *  @version     05/28/2008  shijunpu  Initial Version
 *  @version	2/19/2013  shijunpu  ����ڴ�й¶����.
**************************************************************/
#include "ace/OS.h"
#include "ace/Auto_Ptr.h"
#include "OPCGroup.h"
#include "OPCServer.h"
#include "opcdrv.h"
#include <windows.h>  
#include <atlbase.h>   
#include <atlconv.h>
#include <comdef.h>
#include "pkcomm/pkcomm.h"
#include "opcdrv.h"
#define _(STRING) (STRING)

extern int g_dwReadTransID; 

static DWORD g_dwClientHandle = 1000;

//֧�ֵĸ�������
char* COPCItem::m_szOPCDataTypeArray[] = {
	"Default",
	"Boolean",
	"Byte",
	"Char",
	"Float",
	"Long",
	"Double",
	"Short",
	"String",
	"Word",
	"DWord"
};

//��ÿ�����Ͷ�Ӧ��variant������
VARTYPE COPCItem::m_nOPCDataType[] = {
		VT_EMPTY,
		VT_BOOL,
		VT_UI1,
		VT_I1,
		VT_R4,
		VT_I4,
		VT_R8,
		VT_I2,
		VT_BSTR,
		VT_UI2,
		VT_UI4,
};

//ÿ���������ݿ����Ҫ�ĳ���(������ʱ���)
const size_t COPCItem::m_nOPCBlockSizeByType[] = {
	MAX_TAGDATA_LEN,
		1,
		1,
		1,
		4,
		4,
		8,
		2,
		MAX_TAGDATA_LEN,
		2,
		4,
};

// ����OPC���������Ͷ���
static VARTYPE m_nVarTypeByChType[] = 
{
	VT_BSTR,	// #define DT_ASCII			0		// ASCII string, maximum: 127
	VT_I2,		// #define DT_SINT16			1		// 16 bit Signed Integer value
	VT_UI2,		// #define DT_UINT16			2		// 16 bit Unsigned Integer value
	VT_R4,		// #define DT_FLT				3		// 32 bit IEEE float
	VT_BOOL,	// #define DT_BIT				4		// 1 bit value
	VT_EMPTY,	// #define DT_TIM				5		// 4 byte TIME (H:M:S:T)
	VT_UI4,		// #define DT_ULONG			6		// 32 bit integer value
	VT_I4,		// #define DT_SLONG			7		// 32 bit signed integer value
	VT_R8,		// #define DT_DBL				8		// 64 bit double
	VT_EMPTY,	// #define DT_BLOB				9		// blob, maximum 65535
	VT_I1,		// #define DT_CHAR				10		// 8 bit signed integer value
	VT_UI1,		// #define DT_UCHAR			11		// 8 bit unsigned integer value
	VT_EMPTY,		// #define DT_INT64			12		// 64 bit signed integer value
	VT_EMPTY,		// #define DT_UINT64			13		// 64 bit unsigned integer value
	VT_EMPTY,	// #define DT_CV_TIME			14		// 64 bit cv time (second + usecond)
};

/**
 *  ���캯��.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCItem::COPCItem() : m_ExceptionListMutex(), m_bValid(false)
{
	m_pOPCGroup = NULL;
	memset(m_szDescription, 0, sizeof(m_szDescription));
	m_nLength		= 0;
	m_nDataType = DATA_TYPE_DEFAULT;
	m_hItem = NULL;
	m_vt	= VT_EMPTY;
	m_dwHClient = g_dwClientHandle++;
	m_nOldQuality = QUALITY_GOOD;
}

/**
 *  ��������.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCItem::~COPCItem()
{
	ACE_GUARD(ACE_Thread_Mutex, mon, m_ExceptionListMutex);
	// ��ȡ�쳣����
	std::list<EXCEPTION_INFO*>::iterator iter = m_ExceptionList.begin();
	
	while(iter != m_ExceptionList.end())
	{
		EXCEPTION_INFO *pExceptionInfo = *iter;
		iter = m_ExceptionList.erase(iter);
		delete pExceptionInfo;
	}
}

/**
 *  ��������ע����쳣����.
 *
 *
 *  @version     07/10/2008  shijunpu  Initial Version.
 */
long COPCItem::TriggerAllExceptions( QUALITY_STATE nQuality )
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, mon, m_ExceptionListMutex, EC_ICV_DA_GUARD_RETURN);	
	std::list<EXCEPTION_INFO*>::iterator iter = m_ExceptionList.begin();
	
	for ( ; iter != m_ExceptionList.end(); iter++)
	{
		EXCEPTION_INFO *pExceptionInfo = *iter;
		pExceptionInfo->bTrigger = true;
	}
	
	TriggerExceptions(nQuality);
	
	return PK_SUCCESS;	
}

/**
 *  �����쳣����.
 *
 *
 *  @version     08/11/2008  shijunpu  Initial Version.
 */

long COPCItem::TriggerExceptions( QUALITY_STATE nQuality )
{
	return 0;
	/*
	int nData = 0;
	CDataArea* pDataArea = NULL;
	long nStatus = PK_SUCCESS;

 	CHECK_INIT_FALSE_RETURN_ERR(g_bDITInitDone);
 	pDataArea = g_pDIT->GetDataArea();
 	CHECK_INIT_FALSE_RETURN_ERR(pDataArea);
	 	
 	TCV_TimeStamp cvTimeStamp;
	pDataArea->GetDataBlockData_i(m_nBlockNumber, sizeof(TCV_TimeStamp), 
		(void*)&cvTimeStamp, m_nBlockSize - sizeof(TCV_TimeStamp));

	CExceptionArea* pExceptionArea = g_pDIT->GetExceptionArea();
	CHECK_INIT_FALSE_RETURN_ERR(pExceptionArea);

	std::list<EXCEPTION_INFO*>::iterator iter = m_ExceptionList.begin();
	
	for ( ; iter != m_ExceptionList.end(); iter++)
	{
		EXCEPTION_INFO *pExceptionInfo = *iter;

		if (!pExceptionInfo->bTrigger)
		{
			continue;
		}

		// ��ӵ�DIT���쳣���ݶ���
		EXCEPTION_DESC_EX ExceptionDesc;
		ExceptionDesc.chType = pExceptionInfo->chLbhType;
		ExceptionDesc.nIpn = pExceptionInfo->nLbhIPN;
		ExceptionDesc.cvTimeStamp = cvTimeStamp;

		if (nQuality.nQuality == QUALITY_BAD)
		{
			ExceptionDesc.nQuality = *(unsigned short*)&nQuality;
		}
		else
		{
			ExceptionDesc.nQuality = *(unsigned short*)&nQuality;

			long nBlockSize = 0;
			nStatus = pDataArea->GetDataBlockSize_i(m_nBlockNumber, &nBlockSize);
			int nByteCanRead = nBlockSize - sizeof(TCV_TimeStamp) - pExceptionInfo->nByteOffset;
			// �޸Ķ�ȡ���ֽ���
			int nByteToRead = (nByteCanRead < pExceptionInfo->nExcDataSize) ? nByteCanRead : pExceptionInfo->nExcDataSize;
	
			switch(pExceptionInfo->chEguType)
			{				
			case EGU_B:
			case EGU_X:
				ExceptionDesc.nDataSize = nByteToRead;
				ExceptionDesc.pszData = new char[ExceptionDesc.nDataSize];
				pDataArea->GetDataBlockData_i(m_nBlockNumber, ExceptionDesc.nDataSize, ExceptionDesc.pszData, pExceptionInfo->nByteOffset);
				break;

			case EGU_A:
				char *pData;
				pData = new char[nByteToRead];
				double fData;
				pDataArea->GetDataBlockData_i(m_nBlockNumber, nByteToRead, pData, pExceptionInfo->nByteOffset);
				nStatus = Data2Double(pData, pExceptionInfo->chOpt, &fData);
				delete[] pData;
				// ���ݽض�
				if (DOUBLE_GREATER_THAN(fData, pExceptionInfo->fHigh))
				{
					fData = pExceptionInfo->fHigh;
				}
				
				if (DOUBLE_LESS_THAN(fData, pExceptionInfo->fLow))
				{
					fData = pExceptionInfo->fLow;
				}
				ExceptionDesc.nDataSize = sizeof(double);
				ExceptionDesc.pszData = new char[ExceptionDesc.nDataSize];
				memcpy(ExceptionDesc.pszData, &fData, sizeof(double));
				break;

			case EGU_D:
				// ��ȡnByteOffset����ֵ
				pDataArea->GetDataBlockData_i(m_nBlockNumber, nByteToRead, &nData, pExceptionInfo->nByteOffset);			
				// ��ȡָ��bit����ֵ
				// ��ȡ����������pszData�ĵ�һ���ַ���ֵΪ0��1
				nData = (nData >> pExceptionInfo->nBitOffset) & 0x0001;
				ExceptionDesc.nDataSize = 1;
				ExceptionDesc.pszData = new char[ExceptionDesc.nDataSize];
				ExceptionDesc.pszData[0] = (char)nData;
				break;
			}
		}

		pExceptionInfo->bTrigger = false;

		pExceptionArea->EnQueue(&ExceptionDesc);
        LH_DEBUG(_("Trigger exception data!"));//�����쳣����
	}

	return PK_SUCCESS;	
	*/
}

/**
 *  ����쳣����.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
/*
 long COPCItem::ExamExceptions( CDataArea* pDataArea, const char* pDeviceData, int nByteCount, QUALITY_STATE nOldQuality, QUALITY_STATE nNewQuality )
{
	long nStatus = PK_SUCCESS;
	char* pOldData = NULL;
	
	if (pDataArea == NULL || pDeviceData == NULL)
	{
		return EC_ICV_DA_INVALID_PARAMETER;
	}
	
	if (nOldQuality.nQuality != QUALITY_GOOD)
	{
		nStatus = pDataArea->SetDataBlockData_i(m_nBlockNumber, nByteCount, (void *)(pDeviceData));//TODO:�����ӿ��޸ĺ����ҲҪ��
		TriggerAllExceptions(nNewQuality);
	}
	else
	{
		pOldData = new char[nByteCount];
		ACE_Auto_Array_Ptr<char> autoPtr(pOldData);

		pDataArea->GetDataBlockData_i(m_nBlockNumber, nByteCount, (void*)pOldData);
		pDataArea->SetDataBlockData_i(m_nBlockNumber, nByteCount, (void*)pDeviceData);
		
		CExceptionArea* pExceptionArea = g_pDIT->GetExceptionArea();
		if (pExceptionArea == NULL)
		{
			return EC_ICV_DA_SHARED_MEM_NOT_INIT;
		}
		
		ACE_GUARD_RETURN(ACE_Thread_Mutex, mon, m_ExceptionListMutex, EC_ICV_DA_GUARD_RETURN);
		std::list<EXCEPTION_INFO*>::iterator iter = m_ExceptionList.begin();
		for ( ; iter != m_ExceptionList.end(); iter++)
		{
			int nOldData = 0, nNewData = 0;
			EXCEPTION_INFO *pExceptionInfo = *iter;
			
			long nBlockSize = 0, nByteToRead = 0;
			nStatus = pDataArea->GetDataBlockSize_i(m_nBlockNumber, &nBlockSize);
			long nByteCanRead = nBlockSize - sizeof(TCV_TimeStamp) - pExceptionInfo->nByteOffset;
			
			// �޸Ķ�ȡ���ֽ���
			nByteToRead = (nByteCanRead < pExceptionInfo->nExcDataSize) ? nByteCanRead : pExceptionInfo->nExcDataSize;
			
			switch(pExceptionInfo->chEguType)
			{
			case EGU_A:
			case EGU_X:
			case EGU_B:
				// ���ݷ����˱仯��֪ͨPDB
				if (memcmp((void*)(pOldData + pExceptionInfo->nByteOffset), 
					(void*)(pDeviceData + pExceptionInfo->nByteOffset),
					nByteToRead) != 0)
				{
					pExceptionInfo->bTrigger = true;
				}
				break;
				
			case EGU_D:
				//*
				
				if (pOldData[0] != pDeviceData[0])
				{
					pExceptionInfo->bTrigger = true;
				}
				break;
			}
		}
		
		TriggerExceptions(nNewQuality);
	}
	
	return PK_SUCCESS;
}
*/

/**
 *  ����Ϣ����ʧ��ʱ�������ݿ���������֪ͨ�쳣����.
 *
 *  @param  -[in]  bool  bTimeout: [comment]
 *
 *  @version     07/10/2008  shijunpu  Initial Version.
 */
long COPCItem::ProcessReadError( bool bTimeout /*= true*/ )
{
	unsigned short nNewQuality;

	
	((QUALITY_STATE*)&nNewQuality)->nQuality = QUALITY_BAD;
	
	if (bTimeout)  // ��ʱ����
	{
		((QUALITY_STATE*)&nNewQuality)->nSubStatus = SS_COMM_FAILURE;
	}
	else
	{
		((QUALITY_STATE*)&nNewQuality)->nSubStatus = SS_DEVICE_FAILURE;
	}
	
	((QUALITY_STATE*)&nNewQuality)->nLimit = LIMIT_NOT_LIMITED;
	((QUALITY_STATE*)&nNewQuality)->nLeftOver = 0;
	
	if (nNewQuality != m_nOldQuality)
	{
		// ����ʱ���
		ACE_Time_Value cvTimeStamp = ACE_OS::gettimeofday();
		VARIANT varVal;
		SetQualityBad(*(QUALITY_STATE*)&nNewQuality);
	}
	
	// ֪ͨ�쳣����
	if (((QUALITY_STATE*)&m_nOldQuality)->nQuality == QUALITY_GOOD)
	{
		TriggerAllExceptions(*(QUALITY_STATE*)&nNewQuality);
	}
	
	return PK_SUCCESS;
}

/**
 *  �����������������Ϊdatablock������Ӧ������
 *
 *
 *  @version     08/04/2009  ��־Ȩ&½��  Initial Version.
 */
long COPCItem::SetDataType( const char* szDataType )
{
	int i = 0;
	for (; i < DATA_TYPE_MAX_INDEX; i++)
	{
		//�����������������Ҷ�Ӧ��datatypeֵ
		if(ACE_OS::strcasecmp(szDataType, m_szOPCDataTypeArray[i]) == 0)
		{
			m_nDataType = static_cast<DataType>(i);
			break;
		}
	}
	// Ĭ��ΪDefault��������
	if (i == DATA_TYPE_MAX_INDEX)
	{
		m_nDataType = DATA_TYPE_DEFAULT;
	}
	return PK_SUCCESS;
}

/**
 *  ����group������items
 *
 *
 *  @version     08/04/2009  ��־Ȩ&½��  Initial Version.
 */
long COPCItem::AddOPCItem()
{
	USES_CONVERSION;
	if(!m_pOPCGroup || !m_pOPCGroup->m_pOPCServer || !m_pOPCGroup->m_pIOPCItemMgt)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of device or m_pIOPCItemMgt is NULL when adding OPC item."));//����OPC��ʱ�����豸��m_pIOPCItemMgtָ��Ϊ�գ�
		return EC_ICV_DA_GENERAL_ERROR;
	}
    
	OPCITEMRESULT *pItemResult = NULL;
	HRESULT *pErrors = NULL;
	OPCITEMDEF ItemDef;
	ItemDef.szAccessPath = T2W(m_strAccessPath.c_str());
	ItemDef.szItemID = T2W(m_strName.c_str());
	ItemDef.bActive = TRUE;
	ItemDef.hClient = m_dwHClient;
	ItemDef.dwBlobSize = 0;
	ItemDef.pBlob = NULL;
	ItemDef.vtRequestedDataType = m_nOPCDataType[m_nDataType];
	//TestItem[nTestItem].hClient = ItemDef.hClient;
	
	HRESULT hr = m_pOPCGroup->m_pIOPCItemMgt->AddItems(1, &ItemDef, &pItemResult, &pErrors);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("[%s@%s%][ItemID=%s] AddItem failed."), 
			m_pOPCGroup->m_pOPCServer->m_szName, m_pOPCGroup->m_pOPCServer->m_strCurrentServer.c_str(), m_strName.c_str());
		return EC_ICV_DA_GENERAL_ERROR;
	}
	else if (hr == S_FALSE)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("AddItem [%s] Failed ERROR: %x"), m_strName.c_str(), *pErrors);
	}

	if (pErrors != NULL && (*pErrors) == S_OK)
	{
		SetValid(true);
	}

	// record unique handle for this item
	//���item�ľ�������ں���Ķ�д����
	m_hItem = pItemResult->hServer;
	m_vt = pItemResult->vtCanonicalDataType;
	if(m_vt == VT_EMPTY)
		m_vt = VT_BSTR;

	if(pItemResult)
		::CoTaskMemFree(pItemResult);

	if(pErrors)
		::CoTaskMemFree(pErrors);
	return PK_SUCCESS;
}

/**
 *  ����group������items
 *
 *
 *  @version     08/04/2009  ��־Ȩ&½��  Initial Version.
 */
long COPCItem::RemoveOPCItem()
{
	if(!m_pOPCGroup || !m_pOPCGroup->m_pOPCServer)
	{
        //ɾ��OPC��ʱ�����豸���豸��Ӧ���豸��ָ��Ϊ�գ�
		Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of device or corresponding device group is NULL when deleting OPC item."));
		return EC_ICV_DA_GENERAL_ERROR;
	}

	if(!m_pOPCGroup->m_pIOPCItemMgt || m_hItem == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of m_pIOPCItemMgt or m_hItem is NULL when deleting OPC item."));//ɾ��OPC��ʱ����m_pIOPCItemMgtָ���m_hItemΪ�գ�
		return EC_ICV_DA_GENERAL_ERROR;
	}

	USES_CONVERSION;
	
	OPCITEMRESULT *pItemResult = NULL;
	HRESULT *pErrors = NULL;	
	HRESULT hr = m_pOPCGroup->m_pIOPCItemMgt->RemoveItems(1, &m_hItem, &pErrors);
	if(FAILED(hr))
	{
		//ShowError(hr,"AddItem()");
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("[%s@%s%][ItemID=%s] RemoveOPCItem failed."), m_pOPCGroup->m_pOPCServer->m_szName, 
			m_pOPCGroup->m_pOPCServer->m_strCurrentServer.c_str(), m_strName.c_str());
		return EC_ICV_DA_GENERAL_ERROR;
	}
	else if (hr == S_FALSE)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RemoveOPCItem [%s] Failed ERROR: %x"), m_strName.c_str(), *pErrors);
	}

	m_hItem = NULL;	
	
	if(pErrors)
		::CoTaskMemFree(pErrors);
	return PK_SUCCESS;
}

long COPCItem::Variant2String(VARIANT &varVal, string &strVal)
{
	char szValue[MAX_TAGDATA_LEN] = { 0 };
//��������������chVal�е����ݣ�����ֵ��varVal
	switch (varVal.vt)
	{
	case VT_EMPTY:
		szValue[0] = '\0';
		break;
	case VT_BOOL:
		sprintf(szValue, "%d", V_BOOL(&varVal)?1:0);//*(static_cast<bool*>(chVal));
		break;
	case VT_I1:
		sprintf(szValue, "%d", V_I1(&varVal));
		break;
	case VT_UI1:
		sprintf(szValue, "%d", V_UI1(&varVal));
		break;
	case VT_R4:
		sprintf(szValue, "%f", V_R4(&varVal));
		break;
	case VT_I4:
		sprintf(szValue, "%d", V_I4(&varVal));
		break;
	case VT_UI4:
		sprintf(szValue, "%d", V_UI4(&varVal));
		break;

	case VT_R8:
		sprintf(szValue, "%f", V_R8(&varVal));
		break;
    case VT_I2:
		sprintf(szValue, "%d", V_I2(&varVal));
		break;
    case VT_UI2:
		sprintf(szValue, "%d", V_UI2(&varVal));
		break;
    case VT_BSTR:
		{
			USES_CONVERSION;
			string strData = W2T(varVal.bstrVal);
			//nLen = strData.size() + 1;
			PKStringHelper::Snprintf(szValue, sizeof(szValue), "%s", strData.c_str());
		}
		break;
	default:
		break;
	}
	strVal = szValue;

	return 0;
}

/**
 *  ����ȡ����datablock������д�뵽dit��
 *
 *
 *  @version     08/04/2009  ��־Ȩ&½��  Initial Version.
 *  @version	2/19/2013  shijunpu  ����ڴ�й¶����.
 */
long COPCItem::SetValue(VARIANT &val, FILETIME &ftTimeStamp, WORD wQuality, HRESULT hErr )
{
	ACE_Time_Value timeStamp = ACE_Time_Value(ftTimeStamp);
	string strVal;
	Variant2String(val, strVal);
	int nPKQuality = 0;
	if(QUALITY_IS_GOOD(wQuality))
		nPKQuality = 0;
	Drv_SetTagData_Text(m_pTag, (char *)strVal.c_str(), timeStamp.sec(), timeStamp.usec() / 1000, nPKQuality);
	return PK_SUCCESS;
}

/**
 *  ����ȡ����datablock������д�뵽dit��
 *
 *
 *  @version     08/04/2009  ��־Ȩ&½��  Initial Version.
 *  @version	2/19/2013  shijunpu  ����ڴ�й¶����.
 */
long COPCItem::SetQualityBad(QUALITY_STATE Quality)
{
	int nPKQuality = 0;
	if(Quality.nQuality == QUALITY_GOOD)
		nPKQuality = 0;
	else
		nPKQuality = Quality.nQuality;
	Drv_SetTagData_Text(m_pTag, "", 0, 0, nPKQuality);

	m_nOldQuality = *(unsigned short *)&Quality;
	//pDataArea->SetDataBlockStatus_i(m_nBlockNumber, *((unsigned short*)&Quality));

	//TriggerAllExceptions(*(QUALITY_STATE* )m_nOldQuality);

	return PK_SUCCESS;
}

/**
 *  ͬ��д����ָ������ش������.
 *
 *  @param  -[in]  OUTPUT_DESC_EX *output: [����ָ���]
 *
 *  @version     08/03/2009  chenzhiquan  Initial Version.
 */
long COPCItem::SyncWriteOPCItem( OUTPUT_DESC_EX *output )
{
	HRESULT hr = S_OK;
	
	if(!m_pOPCGroup)
	{
        //SyncWriteOPCItem OPC��ʱ�����豸ָ��Ϊ�գ�
		Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of device is NULL when SyncWrite OPC item."));
		return EC_ICV_DA_GENERAL_ERROR;
	}

	if(!m_pOPCGroup->m_pIOPCSyncIO || m_hItem == NULL)
	{
        //SyncWriteOPCItem OPC��ʱ����m_pIOPCSyncIOָ���m_hItemΪ�գ�
		Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of m_pIOPCSyncIO or m_hItem is NULL when SyncWrite OPC item."));
		return EC_ICV_DA_GENERAL_ERROR;
	}

	VARIANT varVal;
	long lErr = BuildVariant(output, varVal);

	if (lErr != PK_SUCCESS)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("%s: BuildVariant Error Return Code = %d"), m_strName.c_str(), lErr);
		return lErr;
	}
	
	HRESULT *pErrors = NULL;
    hr = m_pOPCGroup->m_pIOPCSyncIO->Write(1, &m_hItem, &varVal, &pErrors);

	//д��ʧ��
	if (hr == S_FALSE)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("ִ�п���,����:%s, ��ַ:%s: Write Error Return S_FALSE, Error Code = %d"), 
			m_pTag->szName, m_strName.c_str(), pErrors[0]);	
		lErr = pErrors[0];
	}
	else if (hr != S_OK)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("ִ�п���,����:%s, ��ַ:%s: Write Failed (hr != S_OK), Return Handle = %d"), 
			m_pTag->szName, m_strName.c_str(), hr);
		lErr = (long)hr;
	}

	//���д�����ַ�������Ҫ��BuildVariantΪ�ַ�������Ŀռ��ͷŵ�
	if (m_vt == VT_BSTR)
		::SysFreeString(V_BSTR(&varVal));
	::CoTaskMemFree(pErrors);

	return lErr;
}

/**
 *  �첽д����ָ������ش������.
 *
 *  @param  -[in]  OUTPUT_DESC_EX *output: [����ָ���]
 *
 *  @version     08/03/2009  chenzhiquan  Initial Version.
 */
long COPCItem::AsyncWriteOPCItem( OUTPUT_DESC_EX *output )
{
	HRESULT hr = S_OK;
	
	if(!m_pOPCGroup)
	{
		//AsyncWriteOPCItem OPC��ʱ�����豸ָ��Ϊ�գ�
        Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of device is NULL when AsyncWrite OPC item."));
		return EC_ICV_DA_GENERAL_ERROR;
	}

	if(!m_pOPCGroup->m_pIOPCAsyncIO2 || m_hItem == NULL)
	{
		//AsyncWriteOPCItem OPC��ʱ����m_pIOPCAsyncIO2ָ���m_hItemΪ�գ�
        Drv_LogMessage(PK_LOGLEVEL_ERROR,_("Find the pointer of m_pIOPCAsyncIO2 or m_hItem is NULL when AsyncWrite OPC item."));
		return EC_ICV_DA_GENERAL_ERROR;
	}

	VARIANT varVal;

	long lErr = PK_SUCCESS;
	lErr = BuildVariant(output, varVal);

	if (lErr != PK_SUCCESS)
	{
        Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Generate VARIANT variable error! ErrCode=%d."), lErr);
		return lErr;
	}	
    
	HRESULT *pErrors = NULL;
	DWORD dwCancelID = 1;

    hr = m_pOPCGroup->m_pIOPCAsyncIO2->Write(1, &m_hItem, &varVal, output->nCmdId, &dwCancelID, &pErrors);

	//д��ʧ��
	if (hr == S_FALSE)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("%s: Write Error Return Error Code = %d"), m_strName.c_str(), pErrors[0]);	
		lErr = pErrors[0];
	}
	else if (hr != S_OK)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("%s: Write Failed Return Handle = %d"), m_strName.c_str(), hr);
		lErr = hr;
	}

	//���д�����ַ�������Ҫ��BuildVariantΪ�ַ�������Ŀռ��ͷŵ�
	if (m_vt == VT_BSTR)
		::SysFreeString(V_BSTR(&varVal));
	::CoTaskMemFree(pErrors);
	
	return lErr;
}

/**
 *  ����output�е����ͺ�������������Ӧ��variant
 *
 *
 *  @version     08/03/2009  chenzhiquan  Initial Version.
 */
long COPCItem::BuildVariant(OUTPUT_DESC_EX * output, VARIANT &varVal )
{
	char *szTagVal = output->szValue;
	
	USES_CONVERSION;
	::VariantInit(&varVal);
	V_VT(&varVal) = m_vt;

	//��������������chVal�е����ݣ�����ֵ��varVal
	switch (m_vt)
	{
	case VT_BOOL:
		V_BOOL(&varVal) = ::atoi(szTagVal);//*(static_cast<bool*>(chVal));
		break;
	case VT_I1:
		V_I1(&varVal) = ::atoi(szTagVal);
		break;
	case VT_UI1:
		V_UI1(&varVal) = ::atoi(szTagVal);
		break;
	case VT_R4:
		V_R4(&varVal) = ::atof(szTagVal);
		break;
	case VT_I4:
		V_I4(&varVal) = ::atoi(szTagVal);
		break;
	case VT_UI4:
		V_UI4(&varVal) = ::atoi(szTagVal);
		break;

	case VT_R8:
		V_R8(&varVal) = ::atof(szTagVal);
		break;
    case VT_I2:
		V_I2(&varVal) = ::atoi(szTagVal);
		break;
    case VT_UI2:
		V_UI2(&varVal) = ::atoi(szTagVal);
		break;
	case VT_EMPTY:
	case VT_BSTR:
		{
			V_BSTR(&varVal) = ::SysAllocString(T2W(szTagVal));
		}
		break;
	default:
		{
			m_vt = VT_BSTR;
			V_VT(&varVal) = VT_BSTR;
			V_BSTR(&varVal) = ::SysAllocString(T2W(szTagVal));
		}
		break;
	}
	
	//HRESULT hr = ::VariantChangeType(&varVal, &varVal, 0, m_vt);

	////����ת��ʧ��
	//if (hr != S_OK)
	//{
	//	Drv_LogMessage(PK_LOGLEVEL_ERROR, _("VariantChangeType Error[EC=%d], [vt: %d -> %d]"), hr, varVal.vt, m_vt));//return error;
	//	return hr;
	//}
	//   
	return PK_SUCCESS;
}

/**
 *  �������˿��Ź�����ָ��ʱ��ʱ����δ�仯datablock��quality����Ϊuncertain
 *
 *
 *  @version     08/03/2009  chenzhiquan  Initial Version.
 */
long COPCItem::SetAsyncTimeout()
{
	QUALITY_STATE quality = {0};

	//������ݱ����Ͳ��������������κβ���
	if (m_nOldQuality != QUALITY_GOOD)
	{
		return PK_SUCCESS;
	}

	quality.nQuality = QUALITY_UNCERTAIN;
	quality.nSubStatus = SS_LAST_USABLE;
	//pDataArea->SetDataBlockStatus_i(m_nBlockNumber, *((unsigned short*)&quality));

	//�����쳣
	//quality.nQuality = QUALITY_UNCERTAIN;
	//TriggerAllExceptions(quality);
	return PK_SUCCESS;
}

/**
 *  ����datablock��quality
 *
 *
 *  @version     08/03/2009  chenzhiquan  Initial Version.
 */
/*
long COPCItem::SetBlockQuality( QUALITY_STATE Quality )
{
	if (m_nOldQuality != QUALITY_GOOD)
	{
		SetTagQuality(m_nOldQuality);
		return PK_SUCCESS;
	}
	
	//�����ǰquality��good�����败�������쳣
	SetTagQuality(Quality);
	m_nOldQuality = Quality;
	//pDataArea->SetDataBlockStatus_i(m_nBlockNumber, *((unsigned short*)&Quality));

	TriggerAllExceptions(m_nOldQuality);
	return PK_SUCCESS;
}*/

DataType COPCItem::GetDataTypeByVarType( VARTYPE vt )
{
	for (int i = 0; i < DATA_TYPE_MAX_INDEX; i++)
	{
		if (vt == m_nOPCDataType[i])
		{
			return static_cast<DataType>(i);
		}
	}
	return DATA_TYPE_DEFAULT;
}

void COPCItem::GetVARIANTByType(VARIANT &val, _variant_t &vaRet)
{
	try
	{
		vaRet = val;
		switch(m_nDataType)
		{
		case DATA_TYPE_DEFAULT:
			break;
		case DATA_TYPE_BOOL:
			vaRet.ChangeType(VT_BOOL);
			break;
		case DATA_TYPE_CHAR:
			vaRet.ChangeType(VT_I1);
			break;
		case DATA_TYPE_BYTE:
			vaRet.ChangeType(VT_UI1);
			break;
		case DATA_TYPE_FLOAT:
			vaRet.ChangeType(VT_R4);
			break;
		case DATA_TYPE_LONG:
			vaRet.ChangeType(VT_I4);
			break;
		case DATA_TYPE_DWORD:
			vaRet.ChangeType(VT_UI4);
			break;
		case DATA_TYPE_DOUBLE:
			vaRet.ChangeType(VT_R8);
			break;
		case DATA_TYPE_SHORT:
			vaRet.ChangeType(VT_I2);
			break;
		case DATA_TYPE_WORD:
			vaRet.ChangeType(VT_UI2);
			break;
		case DATA_TYPE_STRING:
			vaRet.ChangeType(VT_BSTR);
			break;
		default:
			break;
		}
	}
	catch (_com_error *e)
	{
		//TODO: Return Original VARIANT on Exception
	}
	catch (...) //catch�����쳣����һ��catch��_com_error�޷�catch����
	{
	}
}

char * COPCItem::GetVariantPtr(VARIANT &val)
{
	char * valptr = reinterpret_cast<char*>(&V_INT(&val));
	switch(val.vt)
	{
	case VT_BOOL:
		valptr = reinterpret_cast<char*>(&V_BOOL(&val));
		break;
	case VT_I1:
		valptr = reinterpret_cast<char*>(&V_I1(&val));
		break;
	case VT_UI1:
		valptr = reinterpret_cast<char*>(&V_UI1(&val));
		break;
	case VT_R4:
		valptr = reinterpret_cast<char*>(&V_R4(&val));
		break;
	case VT_I4:
		valptr = reinterpret_cast<char*>(&V_I4(&val));
		break;
	case VT_UI4:
		valptr = reinterpret_cast<char*>(&V_UI4(&val));
		break;

	case VT_R8:
		valptr = reinterpret_cast<char*>(&V_R8(&val));
		break;
    case VT_I2:
		valptr = reinterpret_cast<char*>(&V_I2(&val));
		break;
    case VT_UI2:
		valptr = reinterpret_cast<char*>(&V_UI2(&val));
		break;
    default:
		break;
	}
	return valptr;

}
