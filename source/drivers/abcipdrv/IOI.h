
#pragma once  
#include "ace/Basic_Types.h"

#include <string>
#include <vector>
#include <list>
using namespace std;

#define MAXBYTE_IOI_IN_SLICE 480	// ���ݿ��Ƭ�ֽڴ�С
#define MAX_IOI_NAME_LEN	32


#define SYMBOL_SEGMENT_TYPE		0x91
#define ELEMENT_SEGMENT_ONEBYTE	0x28	// Element Segment DataLen(1Byte), e.g. 28 01 28 05,ABC[1,5]
#define ELEMENT_SEGMENT_TWOBYTE	0x29	// Element Segment DataLen(2Byte), e.g. 29 00 01 01,ABC[257]


// ��ʾһ����ַ, �õ�ַ�в�νṹ, e.g. Program:MainProgram.Data1.Ch1Data1(����, ��.�Ÿ���)
class CIOI  
{
public:
	char			m_szAddrBuf[260];	// Save parsed address buffer
	
	long			m_lAddrBufSize;		// parsed address buffer length
	ACE_UINT16		m_uCount;			// ��m_strAStartAddress��ʼ��N����Ŀ

	long			m_lIndex;			// �������е�����

	char * GetIOIBuff(long *pnActualBufLen); // �õ���ַ������, ���� : 91 03 P R O 00 91 04 A B C D 00 <01 00 00 00 00 00>
									   //                        Symol Size    Symbol Size       Count Other            
									   // ���ػ������ĳ���
	
	long ParseIOIBuff(vector<string> & strASegs); // �õ���ַ������, ���� : 91 03 P R O 00 91 04 A B C D 00 <01 00 00 00 00 00>
									   //                        Symol Size    Symbol Size       Count Other            
									   // ���ػ������ĳ���
	long ParseAdress(long lIndex, const char* szAddress, ACE_UINT16 uCount); // ��.�Ÿ����ĵ�ַ����Ϊ

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
	// CIP��ַ�б�

	list< list<void*>* > m_arrIoiSlice; 
	// ���һ����������к�
	ACE_UINT32	 m_ulSeqCount; 

	// ���CIP��ַ����Ƭ�б���
	long		AddIOI_Slice(long lIndex, string strAdress, ACE_UINT16 uCount);

	// ����б��е����з�Ƭ�͵�ַ
	void		ClearAllIOI_Slice();

	// ��ȡ��ַ��Ƭ����Ŀ
	long		GetIOI_SliceCount();

	// ��ȡָ���ķ�Ƭ
	list<void*> *	GetPtrList(long nSlice);

	// ��ȡָ���ĵ�ַ
	CIOI*		GetIOI(long lIndex);


	CIOIGroup();
	virtual ~CIOIGroup();

	int m_nMaxIoiInSlice;

protected:
	// ��ȡ���õķ�Ƭ
	list<void*>* GetAvailPtrList(); 
}; 
 