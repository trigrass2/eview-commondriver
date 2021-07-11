
#pragma once  
#include "ace/Basic_Types.h"

#include <string>
#include <vector>
#include <list>
using namespace std;

#define MAXBYTE_IOI_IN_SLICE 480	// 数据块分片字节大小
#define MAX_IOI_NAME_LEN	32


#define SYMBOL_SEGMENT_TYPE		0x91
#define ELEMENT_SEGMENT_ONEBYTE	0x28	// Element Segment DataLen(1Byte), e.g. 28 01 28 05,ABC[1,5]
#define ELEMENT_SEGMENT_TWOBYTE	0x29	// Element Segment DataLen(2Byte), e.g. 29 00 01 01,ABC[257]


// 表示一个地址, 该地址有层次结构, e.g. Program:MainProgram.Data1.Ch1Data1(三级, 以.号隔开)
class CIOI  
{
public:
	char			m_szAddrBuf[260];	// Save parsed address buffer
	
	long			m_lAddrBufSize;		// parsed address buffer length
	ACE_UINT16		m_uCount;			// 从m_strAStartAddress开始的N个数目

	long			m_lIndex;			// 在数组中的索引

	char * GetIOIBuff(long *pnActualBufLen); // 得到地址缓冲区, 形如 : 91 03 P R O 00 91 04 A B C D 00 <01 00 00 00 00 00>
									   //                        Symol Size    Symbol Size       Count Other            
									   // 返回缓冲区的长度
	
	long ParseIOIBuff(vector<string> & strASegs); // 得到地址缓冲区, 形如 : 91 03 P R O 00 91 04 A B C D 00 <01 00 00 00 00 00>
									   //                        Symol Size    Symbol Size       Count Other            
									   // 返回缓冲区的长度
	long ParseAdress(long lIndex, const char* szAddress, ACE_UINT16 uCount); // 将.号隔开的地址分析为

	CIOI();
	virtual ~CIOI();
	CIOI & operator=(const CIOI & symbolAddr)
	{
		m_uCount = symbolAddr.m_uCount;
	}

};

class CIOIGroup
{
public:
	// CIP地址列表

	list< list<void*>* > m_arrIoiSlice; 
	// 最近一次请求的序列号
	ACE_UINT32	 m_ulSeqCount; 

	// 添加CIP地址到分片列表中
	long		AddIOI_Slice(long lIndex, string strAdress, ACE_UINT16 uCount);

	// 清除列表中的所有分片和地址
	void		ClearAllIOI_Slice();

	// 获取地址分片的数目
	long		GetIOI_SliceCount();

	// 获取指定的分片
	list<void*> *	GetPtrList(long nSlice);

	// 获取指定的地址
	CIOI*		GetIOI(long lIndex);


	CIOIGroup();
	virtual ~CIOIGroup();

	int m_nMaxIoiInSlice;

protected:
	// 获取可用的分片
	list<void*>* GetAvailPtrList(); 
}; 
 