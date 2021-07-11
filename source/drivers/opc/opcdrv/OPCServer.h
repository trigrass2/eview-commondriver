/**************************************************************
 *  Filename:    DeviceGroup.h
 *  Copyright:   Shanghai Peak InfoTech Co., Ltd.
 *
 *  Description: �豸����Ϣʵ����.
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

#define EC_ICV_DA_CONFIG_OVERLAP						15017 // �������ó����ظ�
#define EC_ICV_DA_GUARD_RETURN  						15066 // ��ȡ������ʧ��
#define EC_ICV_DA_GENERAL_ERROR  						-1 // 
#define EC_ICV_DA_IO_CONNECT_FAILED 					15041 // ���豸��������ʧ��

#define MAX_KEYLEN		256			// max key length of the Registry

#define RELEASE(X) {if(X){ X->Release(); X = NULL;}}

class COPCServerTask;
class COPCServer
{
public:
	COPCServer();
	virtual ~COPCServer();

	// ����OPCͨ������
	long StartOPCServerTask();
	// ֹͣOPCͨ������
	long StopOPCServerTask();
	bool IsStop() { return m_bStop; }
	
	long ReuildGroupsByDevice();
	// ����豸
	long AddGroup(COPCGroup* pGroup);

	bool IsConnected();

	long ConnectToOPCServer();

	// �������е����ݿ������ΪBAD
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

	// �Ƴ����е��豸
	long RemoveAllGroups();
	
	long CreateOPCGroupAndItem();

	void CheckServerType();
	bool IsLocalIP(const std::string& strNode);
	void GetLocalIP(std::list<std::string>& lstLocalIP);
public:
	std::list<COPCGroup *>			m_listGroup;	// �豸�б�

	ACE_Thread_Mutex				m_MutexActiveQueue;	// ���Ϣ������

	char	m_szName[PK_NAME_MAXLEN];		// �豸������
	char	m_szDescription[PK_DESC_MAXLEN];	// �豸������
	bool	m_bStop;									// ����ֹͣ��־

	COPCServerTask	*m_pOPCServerTask;	// ���ݿ�ɨ���߳�

	long	m_nReconnectTime;		// �������ʱ��
	string	m_strMainServer;		// ����HostName
	string	m_strBackupServer;		// ����HostName
	bool	m_bIsBackup;			// �Ƿ���ڱ���

	string	m_strCurrentServer;

	long	m_nRetries;				// ���õ����Դ�������������������л�
	long	m_nNumOfRetries;		// ����������

	// OPC Server

	IOPCServer* m_pIOPCServer;
	IOPCCommon *m_pIOPCCommon;
	IID			m_clsID;
	TCHAR		m_clsidString[MAX_KEYLEN];

	ACE_Time_Value	m_tvLastConnect;
	bool			m_bConnected;

	int				m_nServerType;
	PKDEVICE		*m_pDevice;

	// ���漸���������飨Group�������ԣ�Ϊ�˼�������þͲ����������У����������ڷ������У��������鹲��ͬһ������
	float	m_fDeadband;		// ����
	bool	m_bDataFromCache;	// �Ƿ��Cache�ж�ȡ��0ʱΪDevice��
	bool	m_bAsync;			// �Ƿ��첽��ʽ��ȡ
};

#endif  // _DEVICE_GROUP_H_
