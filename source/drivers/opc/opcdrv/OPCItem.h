/**************************************************************
 *  Filename:    DataBlock.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 数据块信息实体类.
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

// 数据块读消息的状态
#define	 NO_READ_MESSAGE			0		// 没有读消息
#define  MESSAGE_IN_PROCESS			1		// 读消息正在处理中

#define  INVALID_BLOCK_NUMBER		-1		// DIT中数据块创建不成功时的数据块号

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

	// 触发所有注册的异常数据
	long TriggerAllExceptions(QUALITY_STATE nQuality);
	
	// 检查异常数据
	//long ExamExceptions(CDataArea* pDataArea, const char* pDeviceData, int nByteCount, QUALITY_STATE nOldQuality, QUALITY_STATE nNewQuality);

	// 触发需要触发的异常数据
	long TriggerExceptions(QUALITY_STATE nQuality);
	
	// 读消息处理失败时设置数据块质量
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
	COPCGroup *m_pOPCGroup;		// 所属设备指针
	//long	m_nBlockNumber;		// DIT中的数据块号
	//int		m_nBlockSize;		// 数据块的字节数, 包括时间戳在内

	string	m_strName;
	string  m_strAliasName;
	char	m_szDescription[PK_DESC_MAXLEN];	// 数据块描述

	int		m_nLength;			// 数据块长度

	DataType m_nDataType;

	string  m_strAccessPath;

	std::list<EXCEPTION_INFO*> m_ExceptionList; // 注册的异常数据
	ACE_Thread_Mutex	m_ExceptionListMutex;   // 异常数据信息列表锁
	static char*		m_szOPCDataTypeArray[];
	const static size_t		m_nOPCBlockSizeByType[DATA_TYPE_MAX_INDEX];
	static VARTYPE		m_nOPCDataType[DATA_TYPE_MAX_INDEX];

	OPCHANDLE		m_hItem;
	VARTYPE			m_vt;

	DWORD			m_dwHClient;
	unsigned short	m_nOldQuality;
	PKTAG			*m_pTag;	// 指向TAG的指针

private:
	bool m_bValid;
};

extern const char g_szDriverName[PK_NAME_MAXLEN];

#endif  // _DATABLOCK_H_
