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
#define PK_TAGDATA_MAXLEN		4096

// ����������ĺ�����Ҫ�ɸ�������ʵ��
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

// tag��ַ��ʽ��DB3:B1,DB3:X1.7,DB3:W1,DB3:D1��DB������I��Q��PI��PQ��M��T:1,C:1
// ȡ��ַ�ĵ�һ���֣���ð�š���š�#�š�����ǰ 
int GetBlockName(PKTAG *pTag, char *szBlockName, int nBlockNameLen)
{
	int nStartBits, nEndBits;
	return AnalyzeTagAddr_Continuous(pTag->szAddress, pTag->nLenBits, szBlockName, nBlockNameLen,&nStartBits, &nEndBits);
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
		// ������ʼ��ַ
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
	// �Ȱ��յ�����ͷֳɲ�ͬ����
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

	// �������
	std::map<string, vector<PKTAG *> *>::iterator itMap = mapGrpTypeToTags.begin();
	for(; itMap != mapGrpTypeToTags.end(); itMap ++)
	{
		GroupTagsOfGroupType(itMap->first, *itMap->second, vecTagGrpInfo, pGroupOption);
		delete itMap->second;
	}
}

bool compareDrvTag(PKTAG* left, PKTAG* right)
{
	// ���ȽϽ��������������ֵ�ַ��ת�������������ַ�����ַ ��ת��
	return left->nStartBit < right->nStartBit; // ��ʼ��ַλ�ıȽ�,����nData1�ֶ�
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

	// std::sort(vecTags.begin(), vecTags.end(), TagCompFuctor()); // ����tag�����ʼλ���򣬲���tag�������������
	// ����stl��tag�������򣨰�����ʼ��ַ����
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
		// ������鳤�ȳ�������
		if(pTagGroup->vecTags.size() > 0 && (nIsGroupTag == 0 || pTag->nEndBit - pTagGroup->nStartAddrBit >= nMaxBytesOneGroup * 8))
		{
			vecGroups.push_back(pTagGroup);
			pTagGroup = new DRVGROUP();
			strncpy(pTagGroup->szHWBlockName, (char *)strGrpType.c_str(), sizeof(pTagGroup->szHWBlockName));
			sprintf(pTagGroup->szAutoGroupName,  "%s_group_%d", pTagGroup->szHWBlockName, ++m_nAutoGrpNum);
		}

		// ����ĵ�һ��Ԫ�أ���ôһ��Ҫ����
		if (0 == pTagGroup->vecTags.size())
		{
			// ���е�һ����ĵ�ַ,ȡ��Ϊ�����ʼ��ַ������������
			pTagGroup->nStartAddrBit = pTag->nStartBit;
			pTagGroup->nPollRate = pTag->nPollRate;
			if (0 == pTagGroup->nPollRate)
				pTagGroup->nPollRate = DEFAULT_SCAN_INTERVAL;
		}

		pTag->pData1 = pTagGroup;
		if (pTag->nPollRate > 0)
			pTagGroup->nPollRate = min(pTagGroup->nPollRate, pTag->nPollRate);
		// �п��ܳ��ֺ����tag��Ϊ�������͵ĳ��Ƚ�С����1λ����������ַ������ǰ���tagС����ǰ��������ַ�����double��
		if(pTagGroup->nEndAddrBit < pTag->nEndBit)
		{
			pTagGroup->nEndAddrBit = pTag->nEndBit;
			pTagGroup->nLenBits = pTagGroup->nEndAddrBit - pTagGroup->nStartAddrBit + 1;
		}

		pTagGroup->vecTags.push_back(pTag);
		// pTag->nData3 = (pTag->nData1 - pTagGroup->nStartAddrBit); // λ��ƫ��
	}

	if(!pTagGroup->vecTags.empty())
	{
		vecGroups.push_back(pTagGroup);
	}
}

// ���޸�pDevTags��ֵ
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

// ����Group����ʼ��ַ���ͼĴ������룩
// ����Group�Ĵ�����ʼ��ַ���Ĵ�����������ʼ��Чλ
// nRegisterLenBits ��Ϊ1��8��16
long CalcGroupRegisterInfo(DRVGROUP *pTagGroup, int nRegisterLenBits)
{
	// ������ʼ�Ĵ����������Ĵ������Ĵ�������
	pTagGroup->nBeginRegister = (int)(pTagGroup->nStartAddrBit / nRegisterLenBits); // ��ʼλ���ڼĴ�����0�ǵ�һ���Ĵ���������0��0...15����0���ڵ�0���Ĵ�����
	pTagGroup->nEndRegister = (int)(pTagGroup->nEndAddrBit/nRegisterLenBits); // ����λ���ڼĴ�����0...15Ӧ�÷���0,16...31Ӧ�÷���1�Ŷԣ�������2������λ��Ҫ��1�������λ8��ʾ���ǵڶ����ֽڵĵ�һ��λ��
	pTagGroup->nRegisterNum = pTagGroup->nEndRegister - pTagGroup->nBeginRegister + 1;
	
	// ���������ʼ�ͽ�����ַ��ȡ���ڼĴ�����һ���ĵ�1λ�������һ�������1λ
	pTagGroup->nStartAddrBit = pTagGroup->nBeginRegister * nRegisterLenBits; // ȡ���ڼĴ����ĵ�һλ
	pTagGroup->nEndAddrBit = (1 + pTagGroup->nEndRegister) * nRegisterLenBits  -1; // ȡ���ڼĴ��������һλ
	pTagGroup->nLenBits = pTagGroup->nEndAddrBit - pTagGroup->nStartAddrBit + 1;
	pTagGroup->nRegisterLenBits = nRegisterLenBits;
	return 0;
}

// ���������������
long UpdateGroupQuality(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nQuality, const char *szQualityFormat, ...)
{
	char szQuality[1024] = {0};
	//�������ָ��
	try
	{
		va_list	ap;   
		//��ʼ��ap
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

long UpdateGroupData(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *szBuffer, long lBufLen, short nStatus){
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
		pTag->nTimeSec = pTag->nTimeMilSec = 0; // ��ȡϵͳʱ��
		pTag->nQuality = nStatus; // �����ȸ�ֵ
		// �����������ڵ�һ���ֽڣ�����ڿ��׵�ַ����pTag->nData1��ʾtag�����ʼλ����һ��λ��
		int nTagStartBitInGroup = pTag->nStartBit - pTagGroup->nStartAddrBit; // �������������ľ��Ե�ַ�����������ڿ��ڵ���Ե�ַ
		int nStartByteInBlock = nTagStartBitInGroup /8; // ���ڵ���ʼ�ֽ�
		// ���������������һ���ֽڣ�����ڿ��׵�ַ����pTag->nData2��ʾtag��Ľ���λ�����һ��λ��
		int nTagEndBitInGroup = pTag->nEndBit - pTagGroup->nStartAddrBit;
		int nTagEndByteInBlock = nTagEndBitInGroup / 8;

		// �����һ���ֽڵ���ʼλ��������
		int nTagStartBitInByte = nTagStartBitInGroup %8;	
		// ����ռ�õ��ֽ���
		int nTagLenBytes = nTagEndByteInBlock - nStartByteInBlock + 1;

		if(pTag->nLenBits == 1) // ��λȡ��������D4001.1, DI/DO
		{				
			unsigned char ucValue = (*(unsigned char *)(szBuffer + nStartByteInBlock));
			unsigned char ucFinalValue = ((ucValue >> nTagStartBitInByte) & 0x01) != 0; // ȡnTagOffsetOfOneByteλ

			Drv_SetTagData_Binary(pTag, (void *)&ucFinalValue, 1);

			// ���ǵ��а�λд��Ҫ�����ｫλ���ڵģ�����Ϊ��λ�Ŀ飬��D�飩��ֵҲ�����������Ա����������д��
			//int nBitOrWord = pTagGroup->nUserData[TAGGROUP_NUSERDATA_INDEX_BITORWORD];
			//if(nBitOrWord == 0) // ����һ������Ϊ��λ�Ŀ飬��D��
			//{
				//pTagGroup->
			//}
		}
		else if (pTag->nLenBits % 8 != 0)
		{
			// һ��λһ��λ��ֵ��>>
		}
		else // �����ֽڿ�����8λ�ı���
		{
			Drv_SetTagData_Binary(pTag, (void *)(szBuffer + nStartByteInBlock), nTagLenBytes);
		}
	}

	Drv_UpdateTagsData(pDevice, pTagGroup->vecTags.data(), pTagGroup->vecTags.size());
	return 0;
}