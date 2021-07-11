#include <ace/ACE.h>
#include "AutoGroup_ObjDev_snmp.h"
#include <algorithm>
#include <string>
#include <map>
#include "pkdriver/pkdrvcmn.h"
#include "pkcomm/pkcomm.h"
#include <memory.h>
#include <cstring>
#include <stdlib.h>
#include "math.h"
#include <stdarg.h>
#define EC_ICV_BUFFER_TOO_SMALL                     105

using namespace std;
const int DEFAULT_SCAN_INTERVAL = 1000; // ms

unsigned int CAutoGroupObjDev::m_nAutoGrpNum = 0;
CAutoGroupObjDev::CAutoGroupObjDev(vector<PKTAG *> &vecTags)
{
	for (int i = 0; i < vecTags.size(); i++)
	{
		PKTAG *pTag = vecTags[i];
		m_vecAllDevTags.push_back(pTag);
	}
}

CAutoGroupObjDev::~CAutoGroupObjDev(void)
{
}

// 根据每个设备每个组允许的最大字节数（传入）分组，超过最大个数则成为新的一组
// nIsGroup:是否需要分组，nMaxBytesInGroup，一个分组允许的最大的字节数。
void CAutoGroupObjDev::AutoGroupTags(GroupVector &vecGroups, GROUP_OPTION *pGroupOption)
{
	// 先按照点的类型分成不同的组
	DRVGROUP *pTagGroup = new DRVGROUP();
	sprintf(pTagGroup->szAutoGroupName,  "group_%d", ++ m_nAutoGrpNum);
	pTagGroup->nLenBits = 0;

	int nIsGroupTag = 1;
	int nMaxBytesOneGroup = 1000;
	if(pGroupOption)
	{
		nMaxBytesOneGroup = pGroupOption->nMaxBytesOneGroup;
		nIsGroupTag = pGroupOption->nIsGroupTag;
	}
	int nMaxBits = nMaxBytesOneGroup * 8;

	for(int iTag = 0; iTag < m_vecAllDevTags.size(); iTag ++)
	{
		PKTAG *&pTag = m_vecAllDevTags[iTag];
		bool bIsBulkOid = false; //  bulk:1.3.111.2.802.1.1.24.1.1.2.0   or    1.3.111.2.802.1.1.24.1.1.2.0  是否是要批量
		if (strlen(pTag->szAddress) >= 5)
		{
			string strBulkPart = pTag->szAddress;
			strBulkPart = strBulkPart.substr(0, 5);
			if (PKStringHelper::StriCmp(strBulkPart.c_str(), "walk:") == 0)
				bIsBulkOid = true;
		}

		if (bIsBulkOid)
		{
			DRVGROUP *pTagGroup2 = new DRVGROUP();
			pTagGroup2->nLenBits = pTagGroup2->nLenBits + pTag->nLenBits;
			pTagGroup2->bIsBulkOid = true;
			pTagGroup2->vecTags.push_back(pTag);
			pTag->pData1 = pTagGroup2;
			pTagGroup2->nPollRate = min(pTagGroup2->nPollRate, pTag->nPollRate);
			if (0 == pTagGroup2->nPollRate)
				pTagGroup2->nPollRate = DEFAULT_SCAN_INTERVAL;

			sprintf(pTagGroup2->szAutoGroupName, "group_%d", ++m_nAutoGrpNum);

			vecGroups.push_back(pTagGroup2);
			continue;
		}

		// 如果该组长度超出上限
		if( pTagGroup->vecTags.size() > 0 && (nIsGroupTag == 0 || pTagGroup->nLenBits + pTag->nLenBits > nMaxBits) )
		{
			vecGroups.push_back(pTagGroup);
			pTagGroup = new DRVGROUP();
			sprintf(pTagGroup->szAutoGroupName,  "group_%d", ++ m_nAutoGrpNum);
			pTagGroup->nLenBits = 0;
		}

		pTagGroup->nLenBits = pTagGroup->nLenBits + pTag->nLenBits;
		pTagGroup->vecTags.push_back(pTag);
		pTag->pData1 = pTagGroup;
		pTagGroup->nPollRate = min(pTagGroup->nPollRate, pTag->nPollRate);
		if (0 == pTagGroup->nPollRate)
			pTagGroup->nPollRate = DEFAULT_SCAN_INTERVAL;
	}

	if(!pTagGroup->vecTags.empty())
		vecGroups.push_back(pTagGroup);
}

// 会修改pDevTags的值
long TagsToGroups(vector<PKTAG *> &vecAllDevTags, GROUP_OPTION *pGroupOption,  GroupVector &vecTagGroup)
{
	CAutoGroupObjDev objGrpBuilder(vecAllDevTags);
	objGrpBuilder.AutoGroupTags(vecTagGroup, pGroupOption);

	if (vecTagGroup.size() <= 0)
		return EC_ICV_BUFFER_TOO_SMALL;

	return PK_SUCCESS;
}

// 数据直接存放在每个pTag中，保存在
long UpdateGroupData(PKDEVICE *pDevice, DRVGROUP *pTagGroup){
	Drv_UpdateTagsData(pDevice, pTagGroup->vecTags.data(), pTagGroup->vecTags.size());
	return 0;
}

// 仅仅更新组的质量
long UpdateGroupQuality(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nQuality, const char *szQualityFormat, ...)
{
	char szQuality[1024] = {0};
	//定义变量指针
	try
	{
		va_list	ap;   
		//初始化ap
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