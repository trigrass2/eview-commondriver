#pragma once

#include "ace/Time_Value.h"
#include "ace/OS.h"
#include "math.h"
#include <memory.h>
#include <cstring>
#include <string.h> 
#include <stdlib.h>
#include <string>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "pkdriver/pkdrvcmn.h"
#include <vector>
#include <string.h>
#include "ace/Basic_Types.h"
#include "AutoGroup_BlkDev.h"
#include "cipdrv.h"
using namespace std;


class CIPDevice{
public:
	CIPDevice(PKDEVICE *pDevice);
	~CIPDevice();
	void ResetAttributes();

public:
	PKDEVICE *		m_pDevice;
	unsigned short				m_nTransId;		// ����ţ������CIPͷ����8���û��Զ����ֽ���
	int				m_nPLCModel;	// Logix 5000, or SLC500
	// ��������RegisterSession�����з���
	unsigned int	m_nSessionId;	// RegisterSession ���ص�����š�����0˵���Ѿ�ע��Ự��ϡ����ص�һ������0

public:
	// ��ForwardOpen��ʹ�úͷ��������ֵ��
	// m_nO2TConnID����0, �Ƿ���RegisterSession��ReadDataǰ���ɹ�������OpenForward����ĵ���
	int			m_nSlotOfCPU;	//CPU���ڲۺţ�һ����0����ģ����������1��0��RSLinuxռ���ˣ�
	// ��Origninator�����ģ�Targetʹ��.
	ACE_UINT32	m_nT2OConnID;
	ACE_UINT16	m_nT2OConnSN;
	ACE_UINT16	m_nOVendorID;
	ACE_UINT32	m_nOSN;
	ACE_UINT32	m_nT2OAPI;

	// ��Target������Originatorʹ��
	ACE_UINT32	m_nO2TConnID;
	ACE_UINT16	m_nO2TConnSN;
	ACE_UINT32	m_nO2TAPI;

public:
	int Build_Command_RegisterSession_Packet(PKDEVICE *pDevice, char *szReadMsg, unsigned short nTransId);
	int Build_Command_UnRegisterSession_Packet(PKDEVICE *pDevice, char *szReadMsg, unsigned short nTransId);
	int Build_Command_SendRRData_Packet(PKDEVICE *pDevice, char *szRRDataBuff, int nRRDataBufLen, char *szOutRequestBuffer, int &nOutBuffLen);

	
public: // for Logix5000
	int OnTimer_Logix5000_ReadTags(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
	int Build_Command_SendUnitData_Packet(PKDEVICE *pDevice, char *szInDataBuff, int nInDataLen, char *szOutRequestBuffer, int &nOutRequestBufLen);
	int Build_SendRRData_ForwardOpenService_Request_Packet(PKDEVICE *pDevice, char *szReadMsg, unsigned short nTransId);
	int Build_Service_MultiRequest_Packet(PKDEVICE *pDevice, vector<string> &vecMultiServiceBuffer, char *szRequestBuffer, int &nAllServiceBuffLen);
	int Build_Service_ReadFragData_Packet(PKDEVICE *pDevice, PKTAG *pTag, char *szOutRequestBuff, int &nOutServiceBuffLen);
	int Build_Service_UnconnectedSend_Packet(PKDEVICE *pDevice, char *szInContentBuff, int nInContentBuffLen, char *szOutRequestBuffer, int &nOutBuffLen);
	int Build_SendUnitData_Cmd_MultiRequest_ReadData_Service_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutMultiReadBuffer, int &nOutMultiReadBufLen);

	
public: // for SLC500
	int OnTimer_SLC500_ReadTags(PKDEVICE *pDevice, DRVGROUP *pTagGroup);
	int OnParsePCCCReadDataReply(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *&pBuff, int nHopePackLen, int nSendDataRelyDataLen);
	int OnParse4BReadDataReply(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *&pBuff, int nHopePackLen, int nSendDataRelyDataLen);
	int GetFileType_SLC500(PKDEVICE *pDevice, DRVGROUP *pTagGroup, ACE_UINT8 &nFileType, ACE_UINT8 &nFileNumber, ACE_UINT16 &nStartAddrInFile, ACE_UINT8 &nBytesToRead);
	int Build_SLC500_Service_PCCCReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen);
	int Build_SLC500_Service_UnConnectedSend_RSLinuxUnknown_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen);
	int Build_SLC500_SendRRData_UnConnectedSend_ReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen);
	int Build_SLC500_SendRRData_RSLinuxLike_ReadData_Packet(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *szOutRequestBuffer, int &nOutBuffLen);

public:
	int FindPackage(PKDEVICE *pDevice, char *szReadMsg, int nReadMsgLen, int nHopeCmd, unsigned short nHopeTransId, char *szHopePackBuf, int &nHopePackLen, char *&pCurBuf);
	unsigned short GetTransactionId(PKDEVICE *pDevice);
	int RecvAndGetHopedReplyPacket(PKDEVICE *pDevice, int nHopeCmdNo, unsigned short nTransId, int nRecvTimeoutMS, char *szHopePackBuf, int &nHopePackLen);
	int CheckRegisterSession(PKDEVICE *pDevice);
	int UnRegisterSession(PKDEVICE *pDevice);
	int CheckForwardOpenSuccess(PKDEVICE *pDevice);
};
