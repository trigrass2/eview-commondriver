/**************************************************************
 *  Filename:    DeviceGroup.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: 设备组信息实体类.
 *
 *  @author:     lijingjing
 *  @version     05/28/2008  lijingjing  Initial Version
**************************************************************/

#ifndef _DEVICE_GROUP_H_
#define _DEVICE_GROUP_H_

#include "OPCGroup.h"
#include "CommHelper.h"
#include "opcda.h"
#include "opccomn.h"
#include "pkdriver/pkdrvcmn.h"

#define EC_ICV_DA_CONFIG_OVERLAP						15017 // 驱动配置出现重复
#define EC_ICV_DA_GUARD_RETURN  						15066 // 获取互斥锁失败
#define EC_ICV_DA_GENERAL_ERROR  						-1 // 
#define EC_ICV_DA_IO_CONNECT_FAILED 					15041 // 与设备建立连接失败

#define MAX_KEYLEN		256			// max key length of the Registry

#define RELEASE(X) {if(X){ X->Release(); X = NULL;}}

class COPCServerTask;
class COPCServer
{
public:
	COPCServer();
	virtual ~COPCServer();

	// 启动OPC通信任务
	long StartOPCServerTask();
	// 停止OPC通信任务
	long StopOPCServerTask();
	bool IsStop() { return m_bStop; }
	
	long ReuildGroupsByDevice();
	// 添加设备
	long AddGroup(COPCGroup* pGroup);

	bool IsConnected();

	long ConnectToOPCServer();

	// 设置所有的数据块的质量为BAD
	void SetAllBlockStatusBad(QUALITY_STATE &quality);
	int OnControl(OUTPUT_DESC_EX *pOutput);
	int SendControlCommand(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrVal, long lCmdId);

private:
	BOOL GetCLSIDByProgID();
	BOOL GetCLSIDByEnumOPCServers(WCHAR* szProgID);
	BOOL GetCLSIDByRegister();

	long CreateOPCServer();
	long CoInterfaces();

	long ReleaseInterface();

	// 移除所有的设备
	long RemoveAllGroups();
	
	long CreateOPCGroupAndItem();

	void CheckServerType();
	bool IsLocalIP(const std::string& strNode);
	void GetLocalIP(std::list<std::string>& lstLocalIP);
public:
	std::list<COPCGroup *>			m_listGroup;	// 设备列表

	ACE_Thread_Mutex				m_MutexActiveQueue;	// 活动消息队列锁

	char	m_szName[PK_NAME_MAXLEN];		// 设备组名称
	char	m_szDescription[PK_DESC_MAXLEN];	// 设备组描述
	bool	m_bStop;									// 驱动停止标志

	COPCServerTask	*m_pOPCServerTask;	// 数据块扫描线程

	long	m_nReconnectTime;		// 重连间隔时间
	string	m_strMainServer;		// 主机HostName
	string	m_strBackupServer;		// 备机HostName
	bool	m_bIsBackup;			// 是否存在备机

	string	m_strCurrentServer;

	long	m_nRetries;				// 配置的重试次数，超过次数后进行切换
	long	m_nNumOfRetries;		// 已重连次数

	// OPC Server

	IOPCServer* m_pIOPCServer;
	IOPCCommon *m_pIOPCCommon;
	IID			m_clsID;
	TCHAR		m_clsidString[MAX_KEYLEN];

	ACE_Time_Value	m_tvLastConnect;
	bool			m_bConnected;

	int				m_nServerType;
	PKDEVICE		*m_pDevice;

	// 下面几个属性是组（Group）的属性，为了简化组的配置就不配置在组中，而是配置在服务器中，让所有组共用同一个配置
	float	m_fDeadband;		// 死区
	bool	m_bDataFromCache;	// 是否从Cache中读取（0时为Device）
	bool	m_bAsync;			// 是否异步方式读取
};

#endif  // _DEVICE_GROUP_H_
