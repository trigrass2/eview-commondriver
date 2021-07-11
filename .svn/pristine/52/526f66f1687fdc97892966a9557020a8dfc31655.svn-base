/**************************************************************
 *  Filename:    DeviceGroup.cpp
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备组信息实体类.
 *
 *  @author:     shijunpu
 *  @version     05/28/2008  shijunpu  Initial Version
 *  @version     09/17/2011  zhucongfeng  添加方法GetCLSIDByProgID，修改CreateOPCServer中，获取ClsID的方法，CLSIDFromProgID失败后，调用方法
										  CLSIDFromProgID，若二者都失败，再查一次注册表（此次查询无益，纯属多余），都失败，报错
 *  @version	1/17/2013  shijunpu  修改无法连接opc server的问题
**************************************************************/
#include "ace/OS.h"
#include "opcdrv.h"
#include "OPCServer.h"
#include "OpcServerTask.h"
#include "CommHelper.h"
#include "pkcomm/pkcomm.h"
#include <windows.h>  
#include <atlbase.h>   
#include <atlconv.h>
#include "tchar.h"
#include "opcenum_i.c"
#include <atlstr.h>
//#include "gettext/libintl.h"
#include "pkdriver/pkdrvcmn.h"
#include "OPCServerTask.h"
#include "AutoGroup_ObjDev.h"

using namespace std;

#define _(STRING) (STRING)
enum OPCSERVER_TYPE
{
	LOCAL_SERVER	= 1,
	REMOTE_SERVER	=2
};

/**
 *  构造函数.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCServer::COPCServer()
{
	m_bStop = false;
	memset(m_szName, 0, sizeof(m_szName));
	memset(m_szDescription, 0, sizeof(m_szDescription));
	
	m_pOPCServerTask = NULL;
	m_nReconnectTime = 3000;	// 默认重连间隔为3秒
	m_bIsBackup = false;		//默认无备机
	m_nRetries = 3;				//默认重试3次
	m_nNumOfRetries = -1;
	m_tvLastConnect = ACE_Time_Value::zero;
	m_pIOPCServer = NULL;
	m_pIOPCCommon = NULL;
	m_bConnected = false;
	m_pDevice = NULL;
	memset(m_clsidString, 0x00, sizeof(m_clsidString));

	m_bAsync = true;			// 缺省为同步方式
	m_bDataFromCache = false;	// 从设备读取而不是缓存读取
	m_fDeadband = 0.0f;			// 死区
}

/**
 *  析构函数.
 *
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
COPCServer::~COPCServer()
{
	StopOPCServerTask();
}

/**
 *  启动线程
 *
 *
 *  @version     07/09/2008  shijunpu  Initial Version.
 */
long COPCServer::StartOPCServerTask()
{
	// 将存放在pDevice中的OPC服务名，复制到OPC Server上
	string strDevConn = m_pDevice->szConnParam;
	int nPos = strDevConn.find("@");
	string strOPCName = "";
	m_bIsBackup = false;
	m_strMainServer = m_strBackupServer = "";
	if(nPos != string::npos)
	{
		strOPCName = strDevConn.substr(0, nPos);
		// @后面是主备的IP,如果有主备则是以,或者/隔开.	SimOPCServer@127.0.0.1,192.168.10.2
		string strHosts = strDevConn.substr(nPos + 1);
		vector<string> vecHost = PKStringHelper::StriSplit(strHosts, ",/:"); //
		if(vecHost.size() > 0)
			m_strMainServer = vecHost[0];
		if(vecHost.size() > 1)
		{
			m_strBackupServer = vecHost[1];
			m_bIsBackup = true;
		}
	}
	else // 没有@
		strOPCName = strDevConn;

	strncpy(m_szName, strOPCName.c_str(), sizeof(m_szName) - 1);
	m_strCurrentServer = m_strMainServer;

	m_bDataFromCache = false;
	if(strlen(m_pDevice->szParam1) > 0)
		m_bDataFromCache = ::atoi(m_pDevice->szParam1);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OPC Server:%s, 从设备的参数1读取数据方式为（0：设备，1：缓存）:%d", m_szName, m_bDataFromCache);

	m_bAsync = false;
	if(strlen(m_pDevice->szParam2) > 0)
		m_bAsync = ::atoi(m_pDevice->szParam2);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OPC Server:%s, 从设备的参数2读取I/O方式为（0：同步，1：异步）:%d", m_szName, m_bAsync);

	m_fDeadband = 0.0f;
	if(strlen(m_pDevice->szParam3) > 0)
		m_fDeadband = ::atof(m_pDevice->szParam3);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "OPC Server:%s, 从设备的参数3读取死区为:%f", m_szName, m_fDeadband);

	// rebuild the groups by m_pDevice
	ReuildGroupsByDevice();

	// 数据块扫描线程
	if(!m_pOPCServerTask)
	{
		m_pOPCServerTask = new COPCServerTask(this);
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "m_pOPCServerTask->activate()_start");
		m_pOPCServerTask->activate();
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "m_pOPCServerTask->activate()_end");
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "m_pOPCServerTask->activate()");
	return PK_SUCCESS;
}

/**
 *  启动线程
 *
 *
 *  @version     07/09/2008  shijunpu  Initial Version.
 */
long COPCServer::StopOPCServerTask()
{
	if(m_pOPCServerTask != NULL)
	{
		m_pOPCServerTask->wait();
		delete m_pOPCServerTask;
		m_pOPCServerTask = NULL;
	}

	RemoveAllGroups();
	ReleaseInterface();
	return PK_SUCCESS;
}


/**
 *   添加设备.
 *
 *  @param  -[in]  CDevice*  pDevice: [comment]
 *
 *  @version     05/30/2008  shijunpu  Initial Version.
 */
long COPCServer::AddGroup(COPCGroup* pGroup)
{
	CHECK_NULL_PARAMETER_RETURN_ERR(pGroup);

	// 检查设备是否已经存在
	std::list<COPCGroup*>::iterator iter = m_listGroup.begin();
	for ( ; iter != m_listGroup.end(); iter++)
	{
		COPCGroup* pGroupIT = *iter;
		if (strcmp(pGroupIT->m_szName, pGroup->m_szName) == 0)
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, _("Add added device"));
			return EC_ICV_DA_CONFIG_OVERLAP;
		}
	}

	// 设备不存在，添加设备
	pGroup->m_pOPCServer = this;
	m_listGroup.push_back(pGroup);

	return PK_SUCCESS;	
}

/**
 *  移除所有的设备.
 *
 *
 *  @version     07/09/2008  shijunpu  Initial Version.
 */
long COPCServer::RemoveAllGroups()
{
	COPCGroup* pDevice = NULL;
	
	// 遍历DeviceList，删除所有的设备
	std::list<COPCGroup*>::iterator iter = m_listGroup.begin();
	while(iter != m_listGroup.end())
	{
		pDevice = *iter;
		iter = m_listGroup.erase(iter);
		delete pDevice;
	}

	return PK_SUCCESS;
}

long COPCServer::ConnectToOPCServer()
{
	ACE_Time_Value now = ACE_OS::gettimeofday();

	ACE_Time_Value tvInterval;
	tvInterval.msec(m_nReconnectTime);

	if (now < m_tvLastConnect + tvInterval)
	{
		return EC_ICV_DA_IO_CONNECT_FAILED; //未到重连时间
	}

	if (m_nNumOfRetries >= m_nRetries)
	{
		// 冗余切换
		if (m_bIsBackup)
		{
			if (m_strCurrentServer == m_strMainServer)
			{
				m_strCurrentServer = m_strBackupServer;
			}
			else
			{
				m_strCurrentServer = m_strMainServer;
			}
			Drv_LogMessage(PK_LOGLEVEL_INFO, _("OPC Server switched, connected OPCServer:[%s@%s]"), m_szName, m_strCurrentServer.c_str());
		}
		else
		{
			m_strCurrentServer = m_strMainServer;
		}
		m_nNumOfRetries = 0;
	}

	m_nNumOfRetries ++;

	m_tvLastConnect = ACE_OS::gettimeofday();

	CreateOPCServer();

	if(IsConnected())
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, _("Connect to OPC Server [%s] successfully!"), m_szName);
		CreateOPCGroupAndItem();
		m_nNumOfRetries = 0;
		return PK_SUCCESS;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Connect to OPC Server [%s] failed! wait to reconnect next time!"), m_szName);
		return EC_ICV_DA_IO_CONNECT_FAILED;
	}

}

/**
 *  判断OPC服务器是否连接成功
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
bool COPCServer::IsConnected()
{
	HRESULT hr = 0;
	OPCSERVERSTATUS *pSvrStat = NULL;
	DWORD dwStatus;

	bool bRet = true;

	//判断是否先前以连接过OPC服务器
	if(!m_pIOPCServer)
	{
		return false;
	}
	
	//得到服务器状态
	hr = m_pIOPCServer->GetStatus(&pSvrStat);
	if(hr != S_OK)
	{
		bRet = false;
	}

	//没有serverstate返回
	if (!pSvrStat)
	{
		return false;
	}
	
	dwStatus = pSvrStat->dwServerState;
	if (dwStatus == OPC_STATUS_RUNNING || dwStatus == OPC_STATUS_NOCONFIG || dwStatus == OPC_STATUS_TEST)
	{
		bRet = true;
	}
	else
	{
		bRet = false;
	}
		
	
	//释放资源，注意同时释放pSvrStat和里面的字符串
	if(pSvrStat != NULL)
	{
		::CoTaskMemFree(pSvrStat->szVendorInfo);
		::CoTaskMemFree(pSvrStat);
	}
	
	return bRet;
}

/**
 *  连接OPC服务器
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 *  @version	1/17/2013  shijunpu  修改无法重连opc server的问题，原因是代码逻辑有问题.
 */
long COPCServer::CreateOPCServer()
{
	//重连OPC服务器后，需要清除前一次连接的借口
	ReleaseInterface();

	HRESULT hr;	
	HKEY hk = HKEY_CLASSES_ROOT;
	HKEY hCLSID;
	DWORD dwR;
	LONG lsize = MAX_KEYLEN;
	WCHAR* szProgID = 0;
	
	CheckServerType();

	USES_CONVERSION;
	
	szProgID =  T2W((LPCTSTR)m_szName);
	hr = ::CLSIDFromProgID(szProgID, &m_clsID);

	BOOL bFound = FALSE;
	if(FAILED(hr))
	{
		bFound = GetCLSIDByProgID();
	}
	
	if(FAILED(hr) && (!bFound))
	{
		dwR = RegOpenKey(hk, m_szName, &hCLSID);
		if(dwR != ERROR_SUCCESS)
		{
			dwR = RegConnectRegistry(m_strCurrentServer.c_str(), HKEY_CLASSES_ROOT, &hk);
			if(dwR != ERROR_SUCCESS)
			{
				Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RegConnectRegistry Failed! [%s@%s] ,ret=%X"), m_szName, m_strCurrentServer.c_str(),dwR);
				return EC_ICV_DA_IO_CONNECT_FAILED;
			}

			if(::RegOpenKey(hk, m_szName, &hCLSID) != ERROR_SUCCESS)
			{
				RegCloseKey(hk);
				Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RegOpenKey [%s@%s]: Failed!"), m_szName, m_strCurrentServer.c_str());
				return EC_ICV_DA_IO_CONNECT_FAILED;
			}

			RegCloseKey(hk);
		}

		dwR = ::RegQueryValue(hCLSID, _T("CLSID"), m_clsidString, &lsize);
		if(dwR != ERROR_SUCCESS)
		{
			RegCloseKey(hCLSID);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Create OPCServer [%s@%s]: Open remote register failed!Get CLSID Failed!"), m_szName, m_strCurrentServer.c_str());
			return EC_ICV_DA_IO_CONNECT_FAILED; 
		}

		RegCloseKey(hCLSID);
		hr = CLSIDFromString(T2OLE(m_clsidString), &m_clsID);
		if (FAILED(hr))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Create OPCServer [%s@%s]: CLSIDFromString Failed!"), m_szName, m_strCurrentServer.c_str());
			return EC_ICV_DA_IO_CONNECT_FAILED; 
		}
		bFound = true;
	}
	
	if(!bFound && FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Create OPCServer [%s@%s]: CLSIDFromString Failed!"), m_szName, m_strCurrentServer.c_str());
		return EC_ICV_DA_IO_CONNECT_FAILED; 
	}
	
	WCHAR* pClsid = NULL;
	if(m_clsidString[0] == 0)
	{
		hr = StringFromCLSID(m_clsID, &pClsid);
		if(FAILED(hr))
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Create OPCServer [%s@%s]: StringFromCLSID Failed!hr=%d"), m_szName, m_strCurrentServer.c_str(), hr);
			return EC_ICV_DA_IO_CONNECT_FAILED;
		}
		else
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, _("Create OPCServer [%s@%s]: StringFromCLSID success!"), m_szName, m_strCurrentServer.c_str());
		WideCharToMultiByte(CP_ACP, 0, pClsid, -1, m_clsidString, MAX_KEYLEN, NULL, NULL);
		CoTaskMemFree(pClsid);
	}
	
	int nRet = CoInterfaces();
	if(nRet != PK_SUCCESS)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "连接OPC Server(%s)句柄失败", m_szName); // 已经在CoInterfaces中打印日志，此处不重复打印
		return nRet;
	}

	if(!m_pIOPCServer)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("RegisterCallBack: OPC device(NAME:%s) the m_pIOPCServer is NULL."), this->m_szName);
		return -1;
	}
	hr = m_pIOPCServer->QueryInterface(IID_IOPCCommon, (void**)&m_pIOPCCommon);
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Query the IOPCCommon handle failed[%s@%s], ErrorCode[%d]."),m_szName, m_strCurrentServer.c_str(), hr);
		m_pIOPCCommon = NULL;
	}
	else
	{
		m_pIOPCCommon->SetClientName(L"SST Win32 Simple Client");
	}

	m_bConnected = true;

	return nRet;
}

void COPCServer::CheckServerType()
{
	char strArray[PK_NAME_MAXLEN];
	memset(strArray, 0, sizeof(strArray));
	unsigned long num = PK_NAME_MAXLEN - 1;
	
	//获取计算机名称
	BOOL bRet = ::GetComputerName(strArray,&num);
	if(!bRet)
	{
        Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Failed to get the computer name."));//获取计算机名称失败
		m_nServerType = REMOTE_SERVER; //默认为Remote Server
		return ;
	}
	
	std::string strNode = strArray;
	
	//根据机器名称检测Server状态
	if(m_strCurrentServer == strNode || strNode == "" || IsLocalIP(strNode))
		m_nServerType = LOCAL_SERVER;
	else
		m_nServerType = REMOTE_SERVER;
	
	return;
}

bool COPCServer::IsLocalIP(const std::string& strNode)
{
	std::list<std::string> lstLocalIP;
	GetLocalIP(lstLocalIP);
	return lstLocalIP.end() != find(lstLocalIP.begin(), lstLocalIP.end(), strNode);
}

void COPCServer::GetLocalIP(std::list<std::string>& lstLocalIP)
{
	lstLocalIP.clear();
	char szHostName[PK_NAME_MAXLEN] = {0}; 
	struct hostent *pHostent = NULL; 

	if(gethostname(szHostName,sizeof(szHostName)) == 0) 
	{ 
		pHostent = gethostbyname(szHostName); 
		for(int i = 0; ((pHostent != NULL)&&(pHostent->h_addr_list[i] != NULL));i++) 
		{
			char* szIP = inet_ntoa(*(struct in_addr*)pHostent-> h_addr_list[i]);
			lstLocalIP.push_back(szIP);
		} 
	}
}

/**
 *  从服务器查询IOPCServer的句柄
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
long COPCServer::CoInterfaces()
{
	HRESULT hr;
	COSERVERINFO si;
	MULTI_QI qi;
	WCHAR* szProgID = 0;
	WCHAR* szNodeName = 0;
	
	qi.hr = 0;
	qi.pIID = &IID_IOPCServer;
	qi.pItf = 0;

	USES_CONVERSION;
	szProgID = T2W(m_szName);
	szNodeName = T2W(m_strCurrentServer.c_str());

	if(m_nServerType == REMOTE_SERVER)
	{
		si.dwReserved1 = 0;
		si.dwReserved2 = 0;
		si.pAuthInfo = 0;
		si.pwszName = szNodeName;
	}
	
	//本地OPCServer创建实例
	if(m_nServerType == LOCAL_SERVER)
	{
		hr = ::CoCreateInstanceEx(
			m_clsID,
			NULL,
			//				CLSCTX_ALL,
			CLSCTX_LOCAL_SERVER,
			NULL, 
			1,
			&qi );
		
		if(FAILED(hr))
		{
			if(qi.pItf != 0)
				qi.pItf->Release();
			
			hr = ::CoCreateInstanceEx(
				m_clsID,
				NULL,
				CLSCTX_ALL,
				NULL, 
				1,
				&qi );
		}
	}
	else
	{
		//远程OPCServer创建实例
		hr = ::CoCreateInstanceEx(
			m_clsID,
			NULL,
			CLSCTX_REMOTE_SERVER,
			&si, 
			1,
			&qi );
		
		if(FAILED(hr))
		{
			if(qi.pItf != 0)
				qi.pItf->Release();
			
			hr = ::CoCreateInstanceEx(
				m_clsID,
				NULL,
				CLSCTX_ALL,
				NULL, 
				1,
				&qi );
		}
	}
	
	if(FAILED(hr) || FAILED(qi.hr))
	{
		//连接服务器失败
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Connect tp OPC server[%s@%s] failed, ErrorCode[%d]."), m_szName, m_strCurrentServer.c_str(), hr);
		if(qi.pItf != 0)
			qi.pItf->Release();
		
		return EC_ICV_DA_IO_CONNECT_FAILED; // 连接失败后不重连本地
	}

	//释放以前保存的IOPCServer句柄
	RELEASE(m_pIOPCServer);
	
	hr = qi.pItf->QueryInterface(IID_IOPCServer, (void**)(&m_pIOPCServer));
	//查询IOPCServer句柄出错
	if(FAILED(hr))
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Query IOPCServer handle failed[%s@%s], ErrorCode[%d]."), m_szName, m_strCurrentServer.c_str(), hr);
		if(m_pIOPCServer != 0)
			m_pIOPCServer->Release();
		m_pIOPCServer = 0;
		return EC_ICV_DA_IO_CONNECT_FAILED; 
	}
	qi.pItf->Release();

	if(!m_pIOPCServer)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("CoInterfaces: OPC device(NAME:%s) the m_pIOPCServer is NULL."), this->m_szName);
		return -1;
	}
	//未在服务器上找到对应的IOPCServer接口
	if(!m_pIOPCServer)
	{
        //未在服务器上找到对应的IOPCServer接口[%s@%s], ErrorCode[%d].
        Drv_LogMessage(PK_LOGLEVEL_ERROR, _("Did Not found corresponding IOPCServer interface on the server[%s@%s], ErrorCode[%d]."), m_szName, m_strCurrentServer.c_str(), hr);
		return EC_ICV_DA_IO_CONNECT_FAILED; 
	}

	Drv_LogMessage(PK_LOGLEVEL_INFO, _("Connect to OPCServer %s@%s SUCCESS."), m_szName, m_strCurrentServer.c_str());

	return PK_SUCCESS;
}

/**
 *  释放从OPC服务器得到的interface 
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
long COPCServer::ReleaseInterface()
{
	if(m_pIOPCServer != 0)
	{
		QUALITY_STATE quality = {0};
		quality.nQuality = QUALITY_BAD;
		quality.nSubStatus = SS_NOT_CONNECTED;
		SetAllBlockStatusBad(quality);

		try
		{
			// because exception on some machine
			RELEASE(m_pIOPCCommon);
		}
		catch (...)
		{
			printf(_("RELEASE(m_pIOPCCommon) catch exception!"));
		}

		m_pIOPCServer = 0;
	}
	return PK_SUCCESS;
}

/**
 *  初始化各个OPC group
 *
 *
 *  @version     08/04/2009  陈志权&陆文  Initial Version.
 */
long COPCServer::CreateOPCGroupAndItem()
{
	COPCGroup* pDevice = NULL;
	
	// 遍历DeviceList，对每个设备建立OPC中相应的组
	std::list<COPCGroup*>::iterator iter = m_listGroup.begin();
	while(iter != m_listGroup.end())
	{
		pDevice = *iter;
		pDevice->AddOPCGroupAndItems();

		//如果是异步调用，初始化时发出异步读请求
		//if (pDevice->m_bAsync)
		//	pDevice->AsyncReadOPCGroup();

		iter++;
	}
	
	iter = m_listGroup.begin();
	while(iter != m_listGroup.end())
	{
		pDevice = *iter;
		//pDevice->AddOPCGroup();
		
		//如果是异步调用，初始化时发出异步读请求
		if (pDevice->m_bAsync)
			pDevice->AsyncReadOPCGroup();
		
		iter++;
	}
	
	return PK_SUCCESS;
}

void COPCServer::SetAllBlockStatusBad( QUALITY_STATE &quality )
{
	if (!m_bConnected)
	{
		return; // 已经设置过连接状态
	}
	m_bConnected = false;

	COPCGroup* pGroup = NULL;
	
	// 遍历DeviceList，对每个设备建立OPC中相应的组
	std::list<COPCGroup*>::iterator iter = m_listGroup.begin();
	while(iter != m_listGroup.end())
	{
		pGroup = *iter;
		pGroup->SetAllBlockStatusBad(quality);
		
		iter++;
	}
}

BOOL COPCServer::GetCLSIDByProgID()
{
	WCHAR* szProgID = 0;
	BOOL bFound = FALSE;
	
	USES_CONVERSION;
	
	//根据OPCserver名称获取ClassID
	szProgID =  T2W(m_szName);
	bFound = GetCLSIDByEnumOPCServers(szProgID);
	if(!bFound)
		bFound = GetCLSIDByRegister();

	return bFound;
}

BOOL COPCServer::GetCLSIDByEnumOPCServers(WCHAR* szProgID)
{
	BOOL bFound = FALSE;

	CLSID clsid = CLSID_OpcServerList;			//在opcenum_i.c中定义
	IID IIDOPCServerList=IID_IOPCServerList;    //在opcenum_i.c中定义
	CLSID catid = CATID_OPCDAServer20;			//CATID_OPCDAServer20;    //OPC数据访问服务器2.0组件目录
    IOPCEnumGUID *pEnumGUID;
	HRESULT hr = 0;
    IOPCServerList *gpOPC = NULL;	
    DWORD clsctx = CLSCTX_REMOTE_SERVER;

	if(m_nServerType == LOCAL_SERVER)
	{
		clsctx = CLSCTX_LOCAL_SERVER;    //本地服务

		// 创建OPC服务器的浏览器对象
		hr = CoCreateInstance(clsid, NULL, clsctx, IIDOPCServerList, (void**)&gpOPC);
		if(FAILED(hr) || !gpOPC)
		{
			if(gpOPC)
				gpOPC->Release();

			return bFound;
		}
    
		// 查询OPC DA 2.0 组件目录接口指针	
		hr = gpOPC->EnumClassesOfCategories(1, &catid, 1, &catid, (IEnumGUID**)&pEnumGUID); 
		if(FAILED(hr))
		{
			if(gpOPC)
				gpOPC->Release();

			return bFound;
		}
		
		//获得支持OPC DA2.0数据服务器的CLSID
		unsigned long c;
		while (S_OK == pEnumGUID->Next(1, &clsid, &c)) 
		{
			LPOLESTR pszProgID;
			LPOLESTR pszUserType;
			hr = gpOPC->GetClassDetails(clsid, &pszProgID, &pszUserType);
			CString str1 = pszProgID;
			CString str2 = szProgID;
			if(str1 == str2)
			{
				bFound = TRUE;
				m_clsID = clsid;
			}

			CoTaskMemFree(pszProgID);
			CoTaskMemFree(pszUserType);

			if(bFound)
				break;
		}
		
		if (pEnumGUID)
			pEnumGUID->Release();

		//释放接口
		if(gpOPC)
			gpOPC->Release();
	}
	else
	{
		clsctx = CLSCTX_REMOTE_SERVER;    //远程服务;

		COSERVERINFO si;
		MULTI_QI qi;
		
		qi.hr = 0;
		qi.pIID = &IIDOPCServerList;
		qi.pItf = 0;

		USES_CONVERSION;
		WCHAR* szNodeName = T2W(m_strCurrentServer.c_str());

		si.dwReserved1 = 0;
		si.dwReserved2 = 0;
		si.pAuthInfo = 0;
		si.pwszName = szNodeName;

		//远程OPCServer创建实例
		hr = ::CoCreateInstanceEx(
				clsid,
				NULL,
				clsctx,
				&si, 
				1,
				&qi );

		if(FAILED(hr) || FAILED(qi.hr))
		{
			if(qi.pItf != 0)
				qi.pItf->Release();
			return bFound;
		}

		gpOPC =(IOPCServerList*)qi.pItf; //获得IOPCServerList接口

		// 查询OPC DA 2.0 组件目录接口指针	
		hr = gpOPC->EnumClassesOfCategories(1, &catid, 1, &catid, (IEnumGUID**)&pEnumGUID); 
		if(FAILED(hr))
		{
			if(qi.pItf != 0)
				qi.pItf->Release();
			
			return bFound;
		}
		
		//获得支持OPC DA2.0数据服务器的CLSID
		unsigned long c;
		while (S_OK == pEnumGUID->Next(1, &clsid, &c)) 
		{
			LPOLESTR pszProgID;
			LPOLESTR pszUserType;
			hr = gpOPC->GetClassDetails(clsid, &pszProgID, &pszUserType);
			CString str1 = pszProgID;
			CString str2 = szProgID;
			if(str1 == str2)
			{
				bFound = TRUE;
				m_clsID = clsid;
			}

			CoTaskMemFree(pszProgID);
			CoTaskMemFree(pszUserType);

			if(bFound)
				break;
		}

		if (pEnumGUID)
			pEnumGUID->Release();

		if(qi.pItf != 0)
			qi.pItf->Release();
	}

	return bFound;
}

BOOL COPCServer::GetCLSIDByRegister()
{
	BOOL bFound = FALSE;
	USES_CONVERSION;

	// search the registry for OPC entries
	HKEY hk = HKEY_CLASSES_ROOT;
	LPCTSTR strNode = m_strCurrentServer.c_str();
	
	if( strNode )
	{//HKEY_CLASSES_ROOT
		DWORD dwR = RegConnectRegistry ((LPTSTR)strNode, HKEY_CLASSES_ROOT, &hk);
		if( dwR != ERROR_SUCCESS )
		{
			//CString errormessage;
			//errormessage.Format(_T("打开服务器“%s”上的注册表失败，错误码是：%u"), m_serverInfo.strHostName, dwR);
			//MessageBox(errormessage);
			return bFound;
		}
	}
	
	TCHAR key[MAX_KEYLEN];
	strcpy(key, m_szName);

	HKEY hProgID;
	TCHAR dummy[MAX_KEYLEN];
	long lRegReturn = RegOpenKey(hk, key, &hProgID );
	if(lRegReturn==ERROR_SUCCESS )
	{
		LONG size=MAX_KEYLEN;
		if(RegQueryValue(hProgID, _T("CLSID"), dummy, &size)==ERROR_SUCCESS )
		{
			HRESULT hr = CLSIDFromString(T2OLE(dummy), &m_clsID);
			if(!FAILED(hr))
				bFound = TRUE;
		}
		RegCloseKey( hProgID );
	}
	
	RegCloseKey(hk);

	return bFound;
}

int COPCServer::SendControlCommand(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	// 发送消息给相应的OPC线程，以便在其线程控制执行
	OUTPUT_DESC_EX output;
	output.nCmdId = lCmdId;
	output.pTag = pTag;
	strncpy(output.szValue, szStrValue, sizeof(output.szValue) - 1);

	int nMsgLen = sizeof(OUTPUT_DESC_EX) + sizeof(int); // 最后1个'\0'需要放进去
	ACE_Message_Block *pMsg = new ACE_Message_Block(nMsgLen);

	// Command No
	int nCmdNo = 1;
	memcpy(pMsg->wr_ptr(), &nCmdNo, sizeof(int));
	pMsg->wr_ptr(sizeof(int));

	// OUTPUT_DESC_EX
	memcpy(pMsg->wr_ptr(), &output, sizeof(OUTPUT_DESC_EX));
	pMsg->wr_ptr(sizeof(OUTPUT_DESC_EX));

	m_pOPCServerTask->putq(pMsg);
	//this->reactor()->notify(this, ACE_Event_Handler::WRITE_MASK); 

	return 0;
}

int COPCServer::OnControl(OUTPUT_DESC_EX *pOutput)
{
	COPCItem* pOPCItem = (COPCItem*)pOutput->pTag->pData1;
	int nStatus = 0;
	if (pOPCItem)
	{
		if(!pOPCItem->m_pOPCGroup->m_pOPCServer->IsConnected())
		{
			//opc服务器未连接
			Drv_LogMessage(PK_LOGLEVEL_ERROR, _("OPC Server [%s] Not Connected!"), pOPCItem->m_pOPCGroup->m_pOPCServer->m_szName);
			//if (pOutput->bACK)
			//{
			//	ReturnWriteAck(nCmdId, DDA_SEND_FAILURE);
			//}
			return EC_ICV_DA_IO_CONNECT_FAILED;
		}

		//同步或异步写数据，根据返回结果写反馈
		if(pOPCItem->m_pOPCGroup->m_bAsync)
		{
			nStatus = pOPCItem->AsyncWriteOPCItem(pOutput);
		}
		else //if(pDataBlock && pOutput->bACK)
		{
			nStatus = pOPCItem->SyncWriteOPCItem(pOutput);
		}
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, _("DataBlock Handler NULL!"));
	}

	return 0;
}

long COPCServer::ReuildGroupsByDevice()
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "ReuildGroupsByDevice");
	// 获取到所有的tag点
	// 进行自组块处理，将所有的tag点自组块成BLOCK
	vector<DRVGROUP *> vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxItemsOfGroup = 500; // 根据pdf文档判断，每次可以读取480个字的大小

	vector<PKTAG *> vecTags;
	for (int i = 0; i < m_pDevice->nTagNum; i++)
		vecTags.push_back(m_pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();

	// 对于每个组，增加到Server中去！
	for(vector<DRVGROUP *>::iterator itGroup = vecTagGroup.begin(); itGroup != vecTagGroup.end(); itGroup ++)
	{
		DRVGROUP *pDrvGroup = *itGroup;
		COPCGroup *pOPCGroup = new COPCGroup();
		pDrvGroup->pParam1 = pOPCGroup;
		pOPCGroup->m_bAsync = m_bAsync;
		pOPCGroup->m_bDataFromCache = m_bDataFromCache;
		pOPCGroup->m_pOPCServer = this;
		pOPCGroup->m_fDeadband = m_fDeadband;
		pOPCGroup->m_nPollRate = pDrvGroup->nPollRate;
		strncpy(pOPCGroup->m_szName, pDrvGroup->szAutoGroupName, sizeof(pOPCGroup->m_szName) - 1);
		this->AddGroup(pOPCGroup);

		// 将每一个Item加入到Group中去
		for(int iTag = 0; iTag < pDrvGroup->vecTags.size(); iTag ++)
		{
			COPCItem *pOPCItem = new COPCItem();
			PKTAG *pTag = pDrvGroup->vecTags[iTag];
			pTag->pData1 = pOPCItem;
			pOPCItem->m_pTag = pTag;
			pOPCItem->m_pOPCGroup = pOPCGroup;

			pOPCItem->m_strName = pOPCItem->m_strAliasName = pTag->szName;
			strncpy(pOPCItem->m_szDescription, pTag->szAddress, sizeof(pOPCItem->m_szDescription) - 1);
			//vector<string> vecItemIdAndAccess = PKStringHelper::StriSplit(pTag->szAddress, "&");
			//if(vecItemIdAndAccess.size() > 0)
			//	pOPCItem->m_strName = vecItemIdAndAccess[0];
			//if(vecItemIdAndAccess.size() > 1)
			//	pOPCItem->m_strAccessPath = vecItemIdAndAccess[1];
			pOPCItem->m_strName = pTag->szAddress;
			pOPCItem->m_strAccessPath = "";
			pOPCItem->m_vt = VT_EMPTY;
			pOPCGroup->AddItem(pOPCItem);
			pOPCGroup->m_vecTags.push_back(pTag);	// 将该组的所有的变量保存到vector，以便批量更新
		}
	}
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "ReuildGroupsByDevice_end");
	return 0;
}
