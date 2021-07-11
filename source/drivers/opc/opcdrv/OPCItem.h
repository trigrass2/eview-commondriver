/**************************************************************
 *  Filename:    DataBlock.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: ���ݿ���Ϣʵ����.
 *
 *  @author:     lijingjing
 *  @version     05/28/2008  lijingjing  Initial Version
**************************************************************/

#ifndef _DATABLOCK_H_
#define _DATABLOCK_H_

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <list>
#include <map>
#include "Quality.h"
#include "CommHelper.h"
#include <wtypes.h>
#include <comutil.h>
#include "opcda.h"
#include "pkdriver/pkdrvcmn.h"
#include "opcdrv.h"

using namespace std;

// ���ݿ����Ϣ��״̬
#define	 NO_READ_MESSAGE			0		// û�ж���Ϣ
#define  MESSAGE_IN_PROCESS			1		// ����Ϣ���ڴ�����

#define  INVALID_BLOCK_NUMBER		-1		// DIT�����ݿ鴴�����ɹ�ʱ�����ݿ��

enum DataType
{
	DATA_TYPE_DEFAULT = 0,
	DATA_TYPE_BOOL = 1,
	DATA_TYPE_BYTE = 2,
	DATA_TYPE_CHAR = 3,
	DATA_TYPE_FLOAT = 4,
	DATA_TYPE_LONG = 5,
	DATA_TYPE_DOUBLE = 6,
	DATA_TYPE_SHORT = 7,
	DATA_TYPE_STRING = 8,
	DATA_TYPE_WORD = 9,
	DATA_TYPE_DWORD = 10,
	DATA_TYPE_MAX_INDEX = 11,
};

class COPCGroup;
class COPCServerTask;
class OUTPUT_DESC_EX;

class COPCItem
{
public:
	COPCItem();
	virtual ~COPCItem();

	// ��������ע����쳣����
	long TriggerAllExceptions(QUALITY_STATE nQuality);
	
	// ����쳣����
	//long ExamExceptions(CDataArea* pDataArea, const char* pDeviceData, int nByteCount, QUALITY_STATE nOldQuality, QUALITY_STATE nNewQuality);

	// ������Ҫ�������쳣����
	long TriggerExceptions(QUALITY_STATE nQuality);
	
	// ����Ϣ����ʧ��ʱ�������ݿ�����
	long ProcessReadError(bool bTimeout = true);

	long SetDataType(const char* szDataType);
	
	long AddOPCItem();
	long RemoveOPCItem();

	long SetValue(VARIANT &val, FILETIME &ftTimeStamp, WORD wQuality, HRESULT hErr);
	long SetQualityBad(QUALITY_STATE Quality);

	long SyncWriteOPCItem(OUTPUT_DESC_EX *output);
	long AsyncWriteOPCItem(OUTPUT_DESC_EX *output);

	long SetAsyncTimeout();

	long BuildVariant(OUTPUT_DESC_EX * output, VARIANT &Val );

	DataType GetDataTypeByVarType(VARTYPE vt);

	char * GetVariantPtr(VARIANT &val);

	void GetVARIANTByType(VARIANT &val, _variant_t &vaRet);

	void SetValid(bool bValid) { m_bValid = bValid; }
	bool IsValid(void) { return m_bValid; }

	long Variant2String(VARIANT &val, string &strVal);
public:
	COPCGroup *m_pOPCGroup;		// �����豸ָ��
	//long	m_nBlockNumber;		// DIT�е����ݿ��
	//int		m_nBlockSize;		// ���ݿ���ֽ���, ����ʱ�������

	string	m_strName;
	string  m_strAliasName;
	char	m_szDescription[PK_DESC_MAXLEN];	// ���ݿ�����

	int		m_nLength;			// ���ݿ鳤��

	DataType m_nDataType;

	string  m_strAccessPath;

	std::list<EXCEPTION_INFO*> m_ExceptionList; // ע����쳣����
	ACE_Thread_Mutex	m_ExceptionListMutex;   // �쳣������Ϣ�б���
	static char*		m_szOPCDataTypeArray[];
	const static size_t		m_nOPCBlockSizeByType[DATA_TYPE_MAX_INDEX];
	static VARTYPE		m_nOPCDataType[DATA_TYPE_MAX_INDEX];

	OPCHANDLE		m_hItem;
	VARTYPE			m_vt;

	DWORD			m_dwHClient;
	unsigned short	m_nOldQuality;
	PKTAG			*m_pTag;	// ָ��TAG��ָ��

private:
	bool m_bValid;
};

extern const char g_szDriverName[PK_NAME_MAXLEN];

#endif  // _DATABLOCK_H_
