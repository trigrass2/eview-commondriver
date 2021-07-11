#include "AutoGroup_ObjDev.h"
#include <algorithm>
#include <string>
#include <map>
#include "pkdriver/pkdrvcmn.h"
#include <ace/ACE.h>
#include <memory.h>
#include <cstring>
#include <stdlib.h>
#include "math.h"
#include <stdarg.h>
#include "opcdrv.h"
#define EC_ICV_BUFFER_TOO_SMALL                     105

using namespace std;
const int DEFAULT_SCAN_INTERVAL = 1000; // ms

unsigned int CAutoGroupObjDev::m_nAutoGrpNum = 0;
bool compareTagByPollRate(PKTAG* left, PKTAG* right)
{
	// 本比较仅仅可以用于数字地址的转换，不能用于字符串地址 的转换
	return left->nPollRate < right->nPollRate; // 起始地址位的比较,借用nData1字段
}
CAutoGroupObjDev::CAutoGroupObjDev(vector<PKTAG *> &vecTags)
{
	// 利用stl对tag进行排序（按照起始地址排序）
	for (int i = 0; i < vecTags.size(); i++)
	{
		PKTAG *pTag = vecTags[i];
		m_vecAllDevTags.push_back(pTag);
	}

	// 根据每个tag点的扫描周期排序，以便成组时同一个周期的为一组，不同周期的为不同的组
	std::sort(m_vecAllDevTags.begin(), m_vecAllDevTags.end(), compareTagByPollRate);
}

CAutoGroupObjDev::~CAutoGroupObjDev(void)
{
}

// 根据每个设备每个组允许的最大元素个数，以及每个元素的nPollRate是否相同进行分组
// nIsGroup:是否需要分组，nMaxItemsOfGroup，一个分组允许的最大的字节数。
void CAutoGroupObjDev::AutoGroupTags(GroupVector &vecGroups, GROUP_OPTION *pGroupOption)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "AutoGroupTags");
	if(m_vecAllDevTags.size() <= 0)
		return;

	// 先按照点的类型分成不同的组
	DRVGROUP *pTagGroup = NULL;
	int nCurGroupPollRate = -1;
	int nIsGroupTag = 1;
	int nMaxItemsOfGroup = 500; // 每各组最多多少个Item
	if(pGroupOption)
	{
		nMaxItemsOfGroup = pGroupOption->nMaxItemsOfGroup;
		nIsGroupTag = pGroupOption->nIsGroupTag;
	}

	for(int iTag = 0; iTag < m_vecAllDevTags.size(); iTag ++)
	{
		PKTAG *&pTag = m_vecAllDevTags[iTag];

		// 如果该组长度超出上限
		if(!pTagGroup || pTagGroup->vecTags.size() > nMaxItemsOfGroup || pTag->nPollRate != pTagGroup->nPollRate)
		{
			if(pTagGroup) // 将已存在的Group保存起来
			{
				if(pTagGroup->nPollRate <= 0)
					pTagGroup->nPollRate = DEFAULT_SCAN_INTERVAL;
				vecGroups.push_back(pTagGroup);
			}

			// 需要一个新的Group对象
			pTagGroup = new DRVGROUP();
			sprintf(pTagGroup->szAutoGroupName,  "group_%d", ++ m_nAutoGrpNum);
		}

		if (pTagGroup->vecTags.size() == 0)
			pTagGroup->nPollRate = pTag->nPollRate;		
		pTag->pData1 = NULL;
		pTagGroup->vecTags.push_back(pTag);
	}

	if(pTagGroup->nPollRate <= 0)
		pTagGroup->nPollRate = DEFAULT_SCAN_INTERVAL;

	if(!pTagGroup->vecTags.empty())
		vecGroups.push_back(pTagGroup);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "AutoGroupTags_end");
}

// 会修改pDevTags的值
long TagsToGroups(vector<PKTAG *> &vecAllDevTags, GROUP_OPTION *pGroupOption,  GroupVector &vecTagGroup)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "TagsToGroups");
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
	char szQuality[MAX_TAGDATA_LEN] = { 0 };
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