
#ifndef _OPC_DRIVER_H_
#define _OPC_DRIVER_H_


#ifdef _WIN32
#	include <winsock2.h>
#else
#	include <stdint.h>
#	include <sys/types.h>
#	include <sys/time.h>
#endif /*_WIN32*/

#include "pkdriver/pkdrvcmn.h"
#include <vector>
using namespace std;

#define EC_ICV_DA_INVALID_PARAMETER 					15004 // �Ƿ�����
#define EC_ICV_DA_SHARED_MEM_NOT_INIT   				15002 // �����ڴ�δ��ʼ��


#define LH_LOG(X) PKLog.LogMessage X
#define LH_ERROR(X) LH_LOG((PK_LOGLEVEL_ERROR,X))
#define LH_INFO(X) LH_LOG((PK_LOGLEVEL_INFO,X))
#define LH_CRITICAL(X) LH_LOG((PK_LOGLEVEL_CRITICAL,X))

#define CHECK_NULL_PARAMETER_RETURN_ERR(X)	{if(!X) return EC_ICV_DA_INVALID_PARAMETER;}
#define CHECK_INIT_FALSE_RETURN_ERR(X)		{if(!X) return EC_ICV_DA_SHARED_MEM_NOT_INIT;}

#define MAX_TAGDATA_LEN	4096
struct TCV_TimeStamp
{
#ifdef _WIN32
	unsigned int tv_sec;         /* seconds */
	unsigned int tv_usec;        /* and microseconds */
#else
	uint32_t tv_sec;         /* seconds */
	uint32_t tv_usec;        /* and microseconds */
#endif//_WIN32

	/// Returns the value of the object as a timeval.
	operator timeval() const
	{
		timeval tv;
		tv.tv_sec = tv_sec;
		tv.tv_usec = tv_usec;
		return tv;
	}

	TCV_TimeStamp() : tv_usec(0), tv_sec(0)
	{
	}


	TCV_TimeStamp(const timeval &tv)
	{
		this->tv_sec = tv.tv_sec;
		this->tv_usec = tv.tv_usec;
	}

	/// Assign @ tv to this
	TCV_TimeStamp &operator = (const timeval &tv)
	{
		this->tv_sec = tv.tv_sec;
		this->tv_usec = tv.tv_usec;
		return *this;
	}
};


// ���������¼�ṹ
class OUTPUT_DESC_EX
{ 	
public:
	PKTAG *pTag;			// �����������ͣ�д�������ע���쳣���ݣ�ע���쳣���ݣ�ֹͣ����
	char szValue[MAX_TAGDATA_LEN];				// ���������ֵ.ת��Ϊ�ַ�����ʽ
	int  nCmdId;

	OUTPUT_DESC_EX()
	{
		pTag = NULL;
		nCmdId = 0;
		memset(szValue, 0, sizeof(szValue));
	}
};

// �쳣������Ϣ. ��ʱ����
typedef struct _EXCEPTION_INFO
{
	long nByteOffset;	// Byteƫ��
	long nBitOffset;	// Word�ڵ�λƫ��
	char chOpt;			// Ӳ������
	char chLbhType;		// ע����쳣��������
	char chEguType;		// EGU����
	bool bTrigger;		// �Ƿ񴥷��쳣����
	long nLbhIPN;		// �쳣���ݱ��
	long nExcDataSize;		// �쳣���ݵĳ���
	double fLow;		// ���ֵ
	double fHigh;		// ��Сֵ
} EXCEPTION_INFO;


#endif //_OPC_DRIVER_H_ 