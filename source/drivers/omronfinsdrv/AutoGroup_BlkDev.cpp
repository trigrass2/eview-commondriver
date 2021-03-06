#include "ace/ACE.h"
#include "AutoGroup_BlkDev.h"
#include <algorithm>
#include <string>
#include <map>
#include "pkdriver/pkdrvcmn.h"
#include <memory.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "math.h"
#define EC_ICV_BUFFER_TOO_SMALL                     105

#ifdef _MSC_VER
#define		snprintf	_snprintf
#endif

using namespace std;
const int DEFAULT_SCAN_INTERVAL = 1000; // ms

// ！！！下面的函数需要由各个驱动实现
extern int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits);

unsigned int CAutoGroupBlkDev::m_nAutoGrpNum = 0;
CAutoGroupBlkDev::CAutoGroupBlkDev(vector<PKTAG *> &vecTags)
{
	for (int i = 0; i < vecTags.size(); i++)
	{
		PKTAG *pTag = vecTags[i];
		m_vecAllDevTags.push_back(pTag);

	}
}

CAutoGroupBlkDev::~CAutoGroupBlkDev(void)
{
}

// tag地址格式：DB3:B1,DB3:X1.7,DB3:W1,DB3:D1，DB可以用I、Q、PI、PQ、M。T:1,C:1
// 取地址的第一部分，即冒号、点号、#号、逗号前 
int GetBlockName(PKTAG *pTag, char *szBlockName, int nBlockNameLen)
{
	int nStartBits, nEndBits;
	return AnalyzeTagAddr_Continuous(pTag->szAddress, pTag->nLenBits, szBlockName, nBlockNameLen,&nStartBits, &nEndBits);
}

// DB3:B10,	DB4:X1.1,DB3:D2, DB3:10（字符串）

// 支持的地址格式：按位类型包括：DB3:B10,DB4:X1.1,DB3:D2, DB3:10
// 输入：szAddressInDevice：变量地址，按字类型：D2025,D2026.2,TN2,TN2.1。按位类型：X1,Y2,TS2
// 输入：nLenBits，根据tag数据类型计算得到的tag值位数
// 输出：*pnStartBits、*pnEndBits, 相对于相对整个物理块（如AI、DO、D、DB1，而不是重组的某个AI内的某个Group）内的起始位、结束位（含结束位，如16则起始位0，结束位15），以位为单位
int AnalyzeTagAddr_Continuous_Default(char *szAddressInDevice, int nLenBits, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// 该方法可能会被多线程调用，不能用静态变量
	char szAddress[PK_IOADDR_MAXLEN + 1] = {0};
	strncpy(szAddress, szAddressInDevice, sizeof(szAddress));

	char szBlockName[PK_DATABLOCKTYPE_MAXLEN] = {0};
	int nStartRegister = 0;
	// 无位数则认为起始地址时0
	char *pTmp = NULL;
	int nIndex = 0;
	const char *ptrAddr = ACE_OS::strtok_r(szAddress, ":.#,", &pTmp); // 	D2025,X2026,FS2027,D2025.1,etc... DB4:X1.1,C:1
	char cStartAddrType = '\0'; // B,D,W,X
	int	 nStartAddr = 0;
	while(ptrAddr)
	{
		if(nIndex  == 0) // DBn
			strncpy(szBlockName, ptrAddr, sizeof(szBlockName));
		else if(nIndex  == 1) // B1,D1,W1,X1
		{
			if(ptrAddr[0] == 'X' || ptrAddr[0] == 'b' || ptrAddr[0] == 'B' || ptrAddr[0] == 'W' || ptrAddr[0] == 'D')
			{
				cStartAddrType = ptrAddr[0];
				if(strlen(ptrAddr) > 1)
					nStartAddr = ::atoi(ptrAddr+1);
			}
			else // 1byte
				nStartAddr = ::atoi(ptrAddr); 
		}
		else if(nIndex  == 2) // .1
			*pnBitNo = ::atoi(ptrAddr);

		nIndex ++;
		ptrAddr = ACE_OS::strtok_r(NULL, ":", &pTmp);
	}

	nStartRegister -=1; // modbus1表示第一个，序号为0
	if(cStartAddrType == 'X' || cStartAddrType == 'b') // bit
		*pnStartBits = nStartAddr;
	else if(cStartAddrType == 'B')
		*pnStartBits = nStartAddr * 8;
	else if(cStartAddrType == 'W')
		*pnStartBits = nStartAddr * 16;
	else if(cStartAddrType == 'D')
		*pnStartBits = nStartAddr * 32;
	else
		*pnStartBits = nStartAddr * 8;

	if(*pnBitNo >= 0)
		*pnStartBits += *pnBitNo;

	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}

void CAutoGroupBlkDev::calcTagsStartAndEndBits()
{
	for(int iTag = 0; iTag < m_vecAllDevTags.size(); iTag ++)
	{
		PKTAG *pTag = m_vecAllDevTags[iTag];
		int nStartBits = 0;
		int nEndBits = 0;
		char szBlockName[PK_NAME_MAXLEN] = {0};
		int nRet = AnalyzeTagAddr_Continuous(pTag->szAddress, pTag->nLenBits, szBlockName, sizeof(szBlockName), &nStartBits, &nEndBits);
		// 计算起始地址
		pTag->nStartBit = nStartBits;
		pTag->nEndBit = nEndBits;
		//pTag->nData3 = nEndBits - nStartBits + 1;
	}
}

const char * CAutoGroupBlkDev::GenerateGroupName(const char *szBlockName)
{
	static char szGroupName[PK_DATABLOCKNAME_MAXLEN + 1];
	memset(szGroupName, 0, sizeof(szGroupName));
	sprintf(szGroupName, "%s_group_%u", szBlockName, m_nAutoGrpNum++);
	return szGroupName;
}

// GroupType. for modbus: ai/ao/di/do    for simens: db5/m/i....
void CAutoGroupBlkDev::AutoGroupTags(GroupVector &vecTagGrpInfo, GROUP_OPTION *pGroupOption)
{
	// 先按照点的类型分成不同的组
	std::map<std::string, vector<PKTAG *> *> mapGrpTypeToTags;
	for(int iTag = 0; iTag < m_vecAllDevTags.size(); iTag ++)
	{
		PKTAG *&pTag = m_vecAllDevTags[iTag];
		char szBlockName[PK_NAME_MAXLEN] = {0};
		int nRet = GetBlockName(pTag, szBlockName, sizeof(szBlockName));
		if(nRet != 0){
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "tag address(%s) cannot resolve block name", pTag->szAddress);
			continue;
		}

		bool bFound = false;
		map<std::string, vector<PKTAG *> *>::iterator itMap = mapGrpTypeToTags.begin(); // find(strBlockType);
		for(; itMap != mapGrpTypeToTags.end(); itMap ++)
		{
			if(itMap->first.compare(szBlockName) == 0)
			{
				bFound = true;
				break;
			}
		}

		vector<PKTAG *> *pTagVec = NULL;
		if(!bFound)
		{
			pTagVec = new vector<PKTAG *>();
			//mapGrpTypeToTags.insert(make_pair(strBlockType, pTagVec));
			mapGrpTypeToTags[szBlockName] = pTagVec;
		}
		else
			pTagVec = mapGrpTypeToTags[szBlockName];

		pTagVec->push_back(pTag);
	}

	// 成组操作
	std::map<string, vector<PKTAG *> *>::iterator itMap = mapGrpTypeToTags.begin();
	for(; itMap != mapGrpTypeToTags.end(); itMap ++)
	{
		GroupTagsOfGroupType(itMap->first, *itMap->second, vecTagGrpInfo, pGroupOption);
		delete itMap->second;
	}
}

bool compareDrvTag(PKTAG* left, PKTAG* right)
{
	// 本比较仅仅可以用于数字地址的转换，不能用于字符串地址 的转换
	return left->nStartBit < right->nStartBit; // 起始地址位的比较,借用nData1字段
}
// GroupType. for modbus: ai/ao/di/do    for simens: db5/m/i....
void CAutoGroupBlkDev::GroupTagsOfGroupType(string strGrpType, vector<PKTAG *> & vecTags, GroupVector &vecGroups, GROUP_OPTION *pGroupOption)
{
	int nIsGroupTag = 1;
	int nMaxBytesOneGroup = 1000;
	if(pGroupOption)
	{
		nMaxBytesOneGroup = pGroupOption->nMaxBytesOneGroup;
		nIsGroupTag = pGroupOption->nIsGroupTag;
	}
	int nMaxBits = nMaxBytesOneGroup * 8;

	// std::sort(vecTags.begin(), vecTags.end(), TagCompFuctor()); // 根据tag点的起始位排序，不分tag点所在组的类型
	// 利用stl对tag进行排序（按照起始地址排序）
	vector<PKTAG *> tagsVector;
	for(int iTag = 0; iTag < vecTags.size(); iTag ++)
	{
		PKTAG *pTag = vecTags[iTag];
		tagsVector.push_back(pTag);
	}
	std::sort(tagsVector.begin(), tagsVector.end(), compareDrvTag);
	vecTags.clear();
	for(int iTag = 0; iTag < tagsVector.size(); iTag ++)
	{
		PKTAG *pTag = tagsVector[iTag];
		vecTags.push_back(pTag);
	}

	DRVGROUP *pTagGroup = new DRVGROUP();

	strncpy(pTagGroup->szHWBlockName, (char *)strGrpType.c_str(), sizeof(pTagGroup->szHWBlockName));
	sprintf(pTagGroup->szAutoGroupName,  "%s_group_%d", pTagGroup->szHWBlockName, ++ m_nAutoGrpNum);

	for(int iTag = 0; iTag < vecTags.size(); iTag ++)
	{
		PKTAG *pTag = vecTags[iTag];
		// 如果该组长度超出上限;
		if(pTagGroup->vecTags.size() > 0 && (nIsGroupTag == 0 || pTag->nEndBit - pTagGroup->nStartAddrBit >= nMaxBytesOneGroup * 8))
		{
			vecGroups.push_back(pTagGroup);
			pTagGroup = new DRVGROUP();
			strncpy(pTagGroup->szHWBlockName, (char *)strGrpType.c_str(), sizeof(pTagGroup->szHWBlockName));
			sprintf(pTagGroup->szAutoGroupName,  "%s_group_%d", pTagGroup->szHWBlockName, ++m_nAutoGrpNum);
		}

		// 该组的第一个元素，那么一定要加入;
		if (0 == pTagGroup->vecTags.size())
		{
			// 组中第一个点的地址,取作为组的起始地址，并生成组名;
			pTagGroup->nStartAddrBit = pTag->nStartBit;
			pTagGroup->nPollRate = pTag->nPollRate;
			if (0 == pTagGroup->nPollRate)
				pTagGroup->nPollRate = DEFAULT_SCAN_INTERVAL;
		}

		pTag->pData1 = pTagGroup;
		if (pTag->nPollRate > 0)
			pTagGroup->nPollRate = min(pTagGroup->nPollRate, pTag->nPollRate);
		// 有可能出现后面的tag因为数据类型的长度较小（如1位），结束地址反而比前面的tag小（如前面可能是字符串或double）
		if(pTagGroup->nEndAddrBit < pTag->nEndBit)
		{
			pTagGroup->nEndAddrBit = pTag->nEndBit;
			pTagGroup->nLenBits = pTagGroup->nEndAddrBit - pTagGroup->nStartAddrBit + 1;
		}
		pTagGroup->vecTags.push_back(pTag);
		// pTag->nData3 = (pTag->nData1 - pTagGroup->nStartAddrBit); // 位的偏移
	}

	if(!pTagGroup->vecTags.empty())
	{
		vecGroups.push_back(pTagGroup);
	}
}

// 会修改pDevTags的值
long TagsToGroups(vector<PKTAG *> &vecAllDevTags, GROUP_OPTION *pGroupOption, GroupVector &vecTagGroup)
{
	CAutoGroupBlkDev objGrpBuilder(vecAllDevTags);
	objGrpBuilder.calcTagsStartAndEndBits();
	objGrpBuilder.AutoGroupTags(vecTagGroup, pGroupOption);
	if (vecTagGroup.size() <= 0)
		return EC_ICV_BUFFER_TOO_SMALL;

	//std::copy(vecTagGrpInfo.begin(), vecTagGrpInfo.end(), pTagGrps);

	return PK_SUCCESS;
}

// 计算Group的起始地址（和寄存器对齐）
// 计算Group寄存器起始地址、寄存器个数、初始有效位
// nRegisterLenBits 可为1、8、16
long CalcGroupRegisterInfo(DRVGROUP *pTagGroup, int nRegisterLenBits)
{
	// 计算起始寄存器、结束寄存器、寄存器个数;
	pTagGroup->nBeginRegister = (int)(pTagGroup->nStartAddrBit / nRegisterLenBits); // 起始位所在寄存器。0是第一个寄存器，按字0。0...15返回0，在第0个寄存器；
	pTagGroup->nEndRegister = (int)(pTagGroup->nEndAddrBit/nRegisterLenBits); // 结束位所在寄存器。0...15应该返回0,16...31应该返回1才对，而不是2。结束位需要加1，如结束位8表示的是第二个字节的第一个位。
	pTagGroup->nRegisterNum = pTagGroup->nEndRegister - pTagGroup->nBeginRegister + 1;
	
	// 调整组的起始和结束地址，取所在寄存器第一个的第1位，和最后一个的最后1位
	pTagGroup->nStartAddrBit = pTagGroup->nBeginRegister * nRegisterLenBits; // 取所在寄存器的第一位;
	pTagGroup->nEndAddrBit = (1 + pTagGroup->nEndRegister) * nRegisterLenBits  -1; // 取所在寄存器的最后一位;
	pTagGroup->nLenBits = pTagGroup->nEndAddrBit - pTagGroup->nStartAddrBit + 1;
	pTagGroup->nRegisterLenBits = nRegisterLenBits;
	return 0;
}

// 仅仅更新组的质量;
long UpdateGroupQuality(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nQuality, const char *szQualityFormat, ...)
{
	char szQuality[1024] = {0};
	//定义变量指针;
	try
	{
		va_list	ap;   
		//初始化ap;
		va_start(ap, szQualityFormat);   
		int len = vsnprintf(szQuality, sizeof(szQuality), szQualityFormat, ap);
		va_end(ap);
	}
	catch (...)
	{
	}

	for(int i = 0;i < pTagGroup->vecTags.size(); i ++)
	{
		pTagGroup->vecTags[i]->nQuality = nQuality;
	}
	return  Drv_UpdateTagsData(pDevice, pTagGroup->vecTags.data(), pTagGroup->vecTags.size());
}

long UpdateGroupData(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *szBuffer, long lBufLen, short nStatus)
{
	//取整函数;
	int nGroupEndByte = ceil(pTagGroup->nRegisterNum * pTagGroup->nRegisterLenBits / 8.0f);
	if(nGroupEndByte > lBufLen)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "UpdateGroupData(group:%s,tagnum:%d), should %d bytes, actual %d bytes", 
			pTagGroup->szAutoGroupName, pTagGroup->vecTags.size(), nGroupEndByte, lBufLen);
		return -1;
	}

	int nTagNum = pTagGroup->vecTags.size();
	int iTagData = 0;
	for(int iTag = 0; iTag < pTagGroup->vecTags.size(); iTag ++)
	{
		PKTAG *pTag =  pTagGroup->vecTags[iTag];
		pTag->nTimeSec = pTag->nTimeMilSec = 0; // 获取系统时间;
		pTag->nQuality = nStatus; // 质量先赋值;
		// 计算数据所在第一个字节（相对于块首地址）。pTag->nData1表示tag点的起始位（第一个位）
		int nTagStartBitInGroup = pTag->nStartBit - pTagGroup->nStartAddrBit; // 由相对于物理块的绝对地址，计算出相对于块内的相对地址;
		int nStartByteInBlock = nTagStartBitInGroup /8; // 块内的起始字节;
		// 计算数据所在最胡一个字节（相对于块首地址）。pTag->nData2表示tag点的结束位（最后一个位）
		int nTagEndBitInGroup = pTag->nEndBit - pTagGroup->nStartAddrBit;
		int nTagEndByteInBlock = nTagEndBitInGroup / 8;

		// 计算第一个字节的起始位（余数）;
		int nTagStartBitInByte = nTagStartBitInGroup %8;	
		// 计算占用的字节数;
		int nTagLenBytes = nTagEndByteInBlock - nStartByteInBlock + 1;
		 
		if(pTag->nLenBits == 1) // 按位取，可能是D4001.1, DI/DO
		{				
			unsigned char ucValue = (*(unsigned char *)(szBuffer + nStartByteInBlock));
			unsigned char ucFinalValue = ((ucValue >> nTagStartBitInByte) & 0x01) != 0; // 取nTagOffsetOfOneByte位

			Drv_SetTagData_Binary(pTag, &ucFinalValue, 1);
			//pTag->nDataLen = 1;

			// 考虑到有按位写的要求，这里将位所在的（以字为单位的块，如D块）的值也保存下来，以便后续按照字写入
			//int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
			//if(nBitOrWord == 0) // 这是一个以字为单位的块，如D块
			//{
				//pTagGroup->
			//}
		}
		else if (pTag->nLenBits % 8 != 0)
		{
			// 一个位一个位赋值，>>
		}
		else // 整个字节拷贝，8位的倍数
		{
			Drv_SetTagData_Binary(pTag, (void *)(szBuffer + nStartByteInBlock), nTagLenBytes);
			//pTag->nDataLen = nTagLenBytes;
		}
	}

	Drv_UpdateTagsData(pDevice, pTagGroup->vecTags.data(), pTagGroup->vecTags.size());
	return 0;
}
