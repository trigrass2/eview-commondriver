
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

#define EC_ICV_DA_INVALID_PARAMETER 					15004 // 非法参数
#define EC_ICV_DA_SHARED_MEM_NOT_INIT   				15002 // 共享内存未初始化


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


// 控制命令记录结构
class OUTPUT_DESC_EX
{ 	
public:
	PKTAG *pTag;			// 控制命令类型：写控制命令，注册异常数据，注销异常数据，停止驱动
	char szValue[MAX_TAGDATA_LEN];				// 控制命令的值.转换为字符串格式
	int  nCmdId;

	OUTPUT_DESC_EX()
	{
		pTag = NULL;
		nCmdId = 0;
		memset(szValue, 0, sizeof(szValue));
	}
};

// 异常数据信息. 暂时不用
typedef struct _EXCEPTION_INFO
{
	long nByteOffset;	// Byte偏移
	long nBitOffset;	// Word内的位偏移
	char chOpt;			// 硬件类型
	char chLbhType;		// 注册的异常数据类型
	char chEguType;		// EGU类型
	bool bTrigger;		// 是否触发异常数据
	long nLbhIPN;		// 异常数据标记
	long nExcDataSize;		// 异常数据的长度
	double fLow;		// 最大值
	double fHigh;		// 最小值
} EXCEPTION_INFO;


#endif //_OPC_DRIVER_H_ 