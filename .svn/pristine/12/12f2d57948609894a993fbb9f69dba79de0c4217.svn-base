// IOI.cpp: implementation of the CIOI class.
//
//////////////////////////////////////////////////////////////////////

#include "IOI.h"
#include "pkcomm/pkcomm.h"
#include "cipdrv.h"

#define SAFE_DELETE(x) { if (x) delete x; x = NULL; }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOI::CIOI()
{
	// 从m_strAStartAddress开始的N个数目
	m_uCount = 1;

	memset(m_szAddrBuf, 0, sizeof(m_szAddrBuf));
	m_lAddrBufSize = 0;
}

CIOI::~CIOI()
{
	m_uCount = 1; // 从m_strAStartAddress开始的N个数目
}

// e.g. ABC[5,10].CDE.DEF[5].XYZ  
long CIOI::ParseIOIBuff(vector<string> & strASegs) // 得到地址缓冲区, 形如 : 91 03 P R O 00 91 04 A B C D 00 01 00 00 00 00 00
												 //                        Symol Size    Symbol Size       Count Other            
												 // 返回缓冲区的长度
{
	memset(m_szAddrBuf, 0, sizeof(m_szAddrBuf));
	char *pAdress = m_szAddrBuf;
	char *pStartAdress = m_szAddrBuf;

	// 跳过总长度字段（1个字节长度，值表示WORD个数，如0x05表示后面10个字节是地址总长度）
	ACE_UINT8 byTotalLen = 0xFF;
	memcpy(pAdress, &byTotalLen, sizeof(byTotalLen));
	pAdress += sizeof(byTotalLen);

	// every segment of symbol address
	for(int i = 0; i < (int)strASegs.size(); i ++)
	{
		string strAddrSegment = strASegs[i];

		// may be elment([5,6]) or symbolic(abc) segment
		if(strAddrSegment.substr(0, 1) == "[")
		{
			// Remove []
			while (!strAddrSegment.empty() && strAddrSegment.substr(0, 1) == "[")
				strAddrSegment.erase(0, 1);
			while (!strAddrSegment.empty() && strAddrSegment.substr(strAddrSegment.size()-1) == "]")
				strAddrSegment.erase(strAddrSegment.size()-1, 1);

			// Get Every Element
			strAddrSegment += ",";
			int nPosComma = strAddrSegment.find(',');
			while(nPosComma >= 0)
			{
				string strLeft = strAddrSegment.substr(0, nPosComma);
				int nIndex = ::atoi(strLeft.c_str());
				// twoByte Segment type 
				if(nIndex > 0xFF)
				{
					// TwoByte Segment type
					ACE_UINT16 uTmp = ELEMENT_SEGMENT_TWOBYTE;
					memcpy(pAdress, &uTmp, sizeof(ACE_UINT16));
					to_little_endian(pAdress, sizeof(ACE_UINT16));
					pAdress += sizeof(ACE_UINT16);

					// Value
					uTmp = nIndex;
					memcpy(pAdress, &uTmp, sizeof(ACE_UINT16));
					to_little_endian(pAdress, sizeof(ACE_UINT16));
					pAdress += sizeof(ACE_UINT16);
				}
				else // oneByte Segment type 
				{
					// OneByte Segment type
					ACE_UINT8 byTmp = ELEMENT_SEGMENT_ONEBYTE;
					memcpy(pAdress, &byTmp, sizeof(byTmp));
					pAdress += sizeof(byTmp);

					// Value
					byTmp = nIndex;
					memcpy(pAdress, &byTmp, sizeof(byTmp));
					pAdress += sizeof(byTmp);
				}
				strAddrSegment = strAddrSegment.substr(nPosComma + 1); //strAddrSegment.Mid(nPosComma + 1);
				nPosComma = strAddrSegment.find(',');
			}
			continue;
		} // if(strAddrSegment.Left(1).Compare("[") == 0)

		// Symbol address type
		ACE_UINT8 byAdressType = SYMBOL_SEGMENT_TYPE;
		memcpy(pAdress, &byAdressType, sizeof(byAdressType));
		pAdress += sizeof(byAdressType);

		// Symbol address size
		ACE_UINT8 byAdressSize = strAddrSegment.size();
		memcpy(pAdress, &byAdressSize, sizeof(byAdressSize));
		pAdress ++;

		// address content
		memcpy(pAdress, strAddrSegment.c_str(), byAdressSize);
		pAdress += byAdressSize;

		if((byAdressSize % 2) == 1) // if Len is Odd, pad ZERO
		{
			*pAdress = 0x00; // stirng end 'NULL'
			pAdress ++;
		}
	} // for(int i = 0; i < m_strAStartAddress.GetSize(); i ++)

	// 补上开始的总长度
	ACE_UINT16 uTotalLen = pAdress - pStartAdress;
	byTotalLen = uTotalLen / 2; // because of 2-BYTE word count
	memcpy(pStartAdress, &byTotalLen, sizeof(byTotalLen));
	to_little_endian(pStartAdress, sizeof(byTotalLen));

	m_lAddrBufSize = pAdress - pStartAdress;
	return 0;
}

// 将.号隔开的地址分析为, e.g. ABC[5,10].CDE.DEF[5].XYZ
long CIOI::ParseAdress(long lIndex, const char* szAddress, ACE_UINT16 uCount)
{
	m_lIndex = lIndex;

	string strAddress(szAddress);

	vector<string> strASegs;
 	if(uCount <= 0 || strAddress.empty())
		return -1;

	strAddress += ".";
	int nPos = strAddress.find('.');
	while(nPos >= 0)
	{
		// Get A Segment
		string strSegment = strAddress.substr(0, nPos);
		// strSegment maybe ABC[5,10], maybe ABC, we should continue parsing
		// if ABC[5,10], at most one []
		int nPosT = strSegment.find('[');
		string strElementSeg;
		if(nPosT >= 0)
		{
			strElementSeg = strSegment.substr(nPosT);
			strSegment = strSegment.substr(0, nPosT);
		}

		// Insert Symbolic Segment First
		strASegs.push_back(strSegment);
		
		// Then Insert Element Segment if have
		if(nPosT >= 0)
			strASegs.push_back(strElementSeg);
		
		strAddress = strAddress.substr(nPos + 1);
		nPos = strAddress.find('.');
	}

	m_uCount = uCount;

	return ParseIOIBuff(strASegs);
}

char * CIOI::GetIOIBuff(long *plActualBufLen) // 得到地址缓冲区, 形如 : 91 03 P R O 00 91 04 A B C D 00 01 00 00 00 00 00
															   //                        Symol Size    Symbol Size       Count Other            
															   // 返回缓冲区的长度
{
	*plActualBufLen = m_lAddrBufSize;
	return m_szAddrBuf;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOIGroup::CIOIGroup()
{
	m_ulSeqCount = 0; // 最近一次请求的序列号
	m_nMaxIoiInSlice = 16;
}

CIOIGroup::~CIOIGroup()
{
	ClearAllIOI_Slice();
}

list<void*> * CIOIGroup::GetAvailPtrList()
{
	if (m_arrIoiSlice.size() > 0)
	{
		list<void*> *pLast = m_arrIoiSlice.back();
		if ((int)pLast->size() < m_nMaxIoiInSlice)
			return pLast;
	}
	
	list<void*> *pList = new list<void*>;
	m_arrIoiSlice.push_back(pList);
	
	return pList;
}

long CIOIGroup::AddIOI_Slice( long lIndex, string strAdress, ACE_UINT16 uCount )
{
	list<void*> *pList = GetAvailPtrList();

	CIOI *pSymbolAddress = new CIOI();
 	if(0 == pSymbolAddress->ParseAdress(lIndex, strAdress.c_str(), uCount))
 	{
 		pList->push_back(pSymbolAddress);
 		return 0;
 	}
	return -1;	
}

CIOI* CIOIGroup::GetIOI(long lIndex)
{
	list< list<void*>* >::const_iterator it = m_arrIoiSlice.begin(); 
	for (; it != m_arrIoiSlice.end(); ++it)
	{
		list<void*> *pList = *it;
		list<void*>::const_iterator itList = pList->begin();
		for (; itList != pList->end(); ++itList)
		{
			CIOI *pIoi = (CIOI *)(*itList);
 			if (pIoi->m_lIndex == lIndex)
 				return pIoi;
		}
	}

	return NULL;
}

void CIOIGroup::ClearAllIOI_Slice()
{
	list< list<void*>* >::const_iterator it = m_arrIoiSlice.begin(); 
	for (; it != m_arrIoiSlice.end(); ++it)
	{
		list<void*> *pList = *it;
		list<void*>::const_iterator itList = pList->begin();
		for (; itList != pList->end(); ++itList)
		{
			CIOI *p = (CIOI *)(*itList);
			SAFE_DELETE(p);
		}
		
		pList->clear();
		SAFE_DELETE(pList);
	}
	
	m_arrIoiSlice.clear();
}

long CIOIGroup::GetIOI_SliceCount()
{
	return m_arrIoiSlice.size();
}

list<void*> * CIOIGroup::GetPtrList( long nSlice )
{
	if (nSlice >= (long)m_arrIoiSlice.size())
		return NULL;

	list< list<void*>* >::const_iterator it = m_arrIoiSlice.begin(); 
	for (int i=0; i<nSlice; ++i)
		++it;
	return *it;
}
