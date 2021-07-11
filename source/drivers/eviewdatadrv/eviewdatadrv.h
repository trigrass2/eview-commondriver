
#ifndef _EVIEW_DATA_H_
#define _EVIEW_DATA_H_

#include "pkdriver/pkdrvcmn.h"
#include <vector>
#include <string>
#include "pkdata/pkdata.h"

using namespace std;

class CRemoteEview
{
public:
	string m_strServerIp;
	PKHANDLE m_hRemoteEview;
	PKDATA * m_pDataArr;
	PKDEVICE *m_pDevice;

public:
	CRemoteEview(PKDEVICE *pDevice, char  *szServerIp)
	{
		m_pDevice = pDevice;
		m_strServerIp = szServerIp;
		m_hRemoteEview = NULL;
		m_pDataArr = NULL;
	}

	~CRemoteEview()
	{
		if (m_hRemoteEview)
		{
			pkExit(m_hRemoteEview);
			m_hRemoteEview = NULL;
		}
	}

	int InitRemoteEveiw()
	{
		if (m_strServerIp.empty())
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, remote eveiw server ip is EMPTY!", m_pDevice->szName);
			return -1;
		}
		m_hRemoteEview = pkInit((char *)m_strServerIp.c_str(), NULL, NULL);
		if (m_pDataArr)
			delete[] m_pDataArr;

		m_pDataArr = new PKDATA[m_pDevice->nTagNum];
		for (int iTag = 0; iTag < m_pDevice->nTagNum; iTag++)
		{
			PKDATA *pkData = &m_pDataArr[iTag];
			PKTAG *pTag = m_pDevice->ppTags[iTag];
			PKStringHelper::Safe_StrNCpy(pkData->szObjectPropName, pTag->szAddress, sizeof(pkData->szObjectPropName));
			PKStringHelper::Safe_StrNCpy(pkData->szFieldName, "", sizeof(pkData->szFieldName));

		}
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, remote eveiw server ip£º%s, tag count:%d, inited", m_pDevice->szName, m_strServerIp.c_str(), m_pDevice->nTagNum);
	}
};

#endif //_EVIEW_DATA_H_ 