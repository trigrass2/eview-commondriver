#ifndef COBJDEVICE_GROUP_BUILDER_HXX
#define COBJDEVICE_GROUP_BUILDER_HXX
#include "pkdriver/pkdrvcmn.h"
#include <vector>
#include <string>
#include <memory.h>

using namespace std;

#define PK_DATABLOCKNAME_MAXLEN			64
#define PK_DATABLOCKTYPE_MAXLEN			16

typedef struct _GROUP_OPTION
{
	int nIsGroupTag;
	int nMaxBytesOneGroup;
	_GROUP_OPTION()
	{
		nIsGroupTag = 1;
		nMaxBytesOneGroup = 1000;
	}
}GROUP_OPTION;

typedef struct _DRVGROUP
{
	char            szAutoGroupName[PK_DATABLOCKNAME_MAXLEN + 1];
	int				nLenBits;		// 长度
	int				nPollRate;		// 扫描周期，单位ms
	int				nFailCountRecent; // 最近失败次数，运行时需要
	vector<PKTAG *>		vecTags;		// 本组内的tag点数组指针
	bool bIsBulkOid;
	void *			pParam1; 
	void *			pParam2;
	_DRVGROUP()
	{
		memset(szAutoGroupName, 0, sizeof(szAutoGroupName));
		nLenBits = 0;
		nPollRate = 1000;
		nFailCountRecent = 0;
		pParam1 = pParam2 = NULL;
		bIsBulkOid = false;
		vecTags.clear();
	}
	~_DRVGROUP()
	{
		vecTags.clear();
	}
}DRVGROUP;
typedef std::vector<DRVGROUP *> GroupVector;

class CAutoGroupObjDev
{
public:
	CAutoGroupObjDev(vector<PKTAG *> &vecAllDevTags);
	virtual ~CAutoGroupObjDev(void);
	void AutoGroupTags(GroupVector &vecTagGrpInfo, GROUP_OPTION *pGroupOption);
public:
	vector<PKTAG *>			m_vecAllDevTags;
	static unsigned int m_nAutoGrpNum;
};

extern long TagsToGroups(vector<PKTAG *> &vecTags, GROUP_OPTION *pGroupOption, GroupVector &vecTagGroup);
extern long UpdateGroupData(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
extern long UpdateGroupQuality(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nQuality, const char *szQualityFormat, ...);
#endif

