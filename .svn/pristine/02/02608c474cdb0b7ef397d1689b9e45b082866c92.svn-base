
#include "S7Drv.h"
#include "S7Device.h"

#include "math.h"
#include "pkcomm/pkcomm.h"
#include <memory.h>
#include <cstring>
#include <string.h> // for sprintf
#include <stdlib.h>
#include <cstdio>
#include "nodave.h"
#include<algorithm>
#include <string>
using namespace std;

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	CS7Device *pS7Device = new CS7Device(pDevice);
	pDevice->pUserData[NUSERDATA_INDEX_S7DEVICE] = pS7Device;

	char *szPLCTypeAndProtocol = pDevice->szParam1; // PLC类型
	char *szSlotNo = pDevice->szParam2; // CPU所在槽号
	char *szRackNo = pDevice->szParam3; // CPU所在机架

	vector<string> vecPLCTypeANdProtocal = PKStringHelper::StriSplit(szPLCTypeAndProtocol, ":");
	string strPLCType = vecPLCTypeANdProtocal[0];
	string strPLCProtocol = "";
	if (vecPLCTypeANdProtocal.size() > 1)
		strPLCProtocol = vecPLCTypeANdProtocal[1];

	int nPLCType = SIMENS_PLC_TYPE_200;
	if (strstr(szPLCTypeAndProtocol, "1200") != 0)
	{
		nPLCType = SIMENS_PLC_TYPE_1200;
		strPLCType = "S7-1200";
	}
	else if (strstr(szPLCTypeAndProtocol, "400") != 0)
	{
		nPLCType = SIMENS_PLC_TYPE_400;
		strPLCType = "S7-400";
	}
	else if (strstr(szPLCTypeAndProtocol, "300") != 0)
	{
		nPLCType = SIMENS_PLC_TYPE_300;
		strPLCType = "S7-300";
	}
	else if (strstr(szPLCTypeAndProtocol, "200") != 0)
	{
		nPLCType = SIMENS_PLC_TYPE_200;
		strPLCType = "S7-200";
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "Device[Name:%s,ConnParams:%s] param1(PLC Type):%s is NOT valid, support:200/300/400/1200，represent to:S7 200/S7 300/S7 400/S7 1200,default:S7 300", 
			pDevice->szName, pDevice->szConnParam,	szPLCTypeAndProtocol);
		nPLCType = SIMENS_PLC_TYPE_300;
		strPLCType = "S7-300";
	}

	string strPLCProtocolName = "unknown";
	transform(strPLCProtocol.begin(), strPLCProtocol.end(), strPLCProtocol.begin(), ::tolower);
	if (strPLCProtocol.find("mpi") != string::npos)
	{
		pS7Device->m_nProtocolNo = daveProtoMPI;
		strPLCProtocolName = "tcp";
	}
	else if (strPLCProtocol.find("ppi") != string::npos)
	{
		pS7Device->m_nProtocolNo = daveProtoPPI;
		strPLCProtocolName = "ppi";
	}
	else
	{
		pS7Device->m_nProtocolNo = daveProtoISOTCP;
		strPLCProtocolName = "tcp";
	}
	int nSlotNo = ::atoi(szSlotNo);
	if (nSlotNo <= 0)
		nSlotNo = 1;

	int nRackNo = ::atoi(szRackNo);
	if (nRackNo < 0)
		nRackNo = 0;
	
	// 保存PLC类型
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device[Name:%s,ConnParams:%s], param1( PLC type):%s, param2(Slot No):%s. Parsed DeviceType:%s,Protocol:%s, SLOT NO:%d, RACK NO:%d", 
		pDevice->szName, pDevice->szConnParam, strPLCType.c_str(), strPLCProtocolName.c_str(), szSlotNo, strPLCType.c_str(), nSlotNo, nRackNo);

	pS7Device->m_strPLCType = strPLCType;
	pS7Device->m_nPLCType = nPLCType;
	pS7Device->m_nMPIAddress = 2;	// default
	pS7Device->m_nRackNo = nRackNo;
	pS7Device->m_nSlotNo = nSlotNo;

	// 进行自组块处理，将所有的tag点自组块成BLOCK
	vector<DRVGROUP *> vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 0;
	groupOption.nMaxBytesOneGroup = MAX_BLOCK_BYTE_NUM;

	vector<PKTAG *> vecTags;
	for (int i = 0; i < pDevice->nTagNum; i++)
		vecTags.push_back(pDevice->ppTags[i]);
	TagsToGroups(vecTags, &groupOption, vecTagGroup);
	vecTags.clear();

	for(int i = 0; i < vecTagGroup.size(); i ++)
	{
		DRVGROUP *pTagGroup = vecTagGroup[i];
		if (pTagGroup->vecTags.size() <= 0)
			continue;

		// 因为西门子的字符串，分配长度为8，实际上占用了10个字节，前两个字节是头，第一个字节表示字符窜长度8，第二个是实际内容长度（如7，abcdefg）。
		// 必须使用第二个字节，因为内存中可能存的是abcdefgh，实际上我们只要前7个
		// 在分配请求空间时，如果包含字符串，那么Group的结束长度必须 >= tag点结束长度+2字节。不能用最后一个tag判断，因为最后可能是bool不能涵盖2个字节
		for(int iTag = 0; iTag < pTagGroup->vecTags.size(); iTag ++)
		{
			vector<PKTAG *> &vecTag = pTagGroup->vecTags;
			PKTAG *pTag = vecTag[iTag];
			if(pTag->nDataType == TAG_DT_TEXT)
			{
				if( pTagGroup->nEndAddrBit < pTag->nEndBit + 16)
				{
					pTagGroup->nEndAddrBit = pTag->nEndBit + 16;
					pTagGroup->nLenBits = pTagGroup->nEndAddrBit - pTagGroup->nStartAddrBit + 1;
				}
			}
		}

		CalcGroupRegisterInfo(pTagGroup, 8); // 西门子的dave接口，都是以字节为单位计算的

		PKTIMER timerInfo;
		timerInfo.nPeriodMS = pTagGroup->nPollRate;
		timerInfo.pUserData[PUSERDATA_INDEX_TAGGROUP] = pTagGroup;
		void *pTimerHandle = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器
	}

	pS7Device->OnInitDevice(pDevice);
	return 0;
}

// nodave在连接上之后，需要调用daveConnectPLC设置一些信息如PDU大小等，因此必须实现此回调函数
// 这里发送1个请求：
/*

连接建立之后，daveConnectPLC会发送一次请求。
MPI（这里为2）、机架号（这里为0）、槽号（第一个2，第二个1）获得PDU大小。
！！！！如果槽号给错误了，会收不到数据且会被PLC断开连接！！！！
[022] 03 00 00 16 11 E0 00 00 00 01 00 C1 02 01 00 C2 02 01 02 C0 01 09 （槽号2，错误导致被PLC断开连接）
[022] 03 00 00 16 11 E0 00 00 00 01 00 C1 02 01 00 C2 02 01 01 C0 01 09 （槽号1）
对于上面槽号为1的请求，接受22个字节应答：
[022] 03 00 00 16 11 D0 00 01 00 01 00 C0 01 09 C1 02 01 00 C2 02 01 01
*/
PKDRIVER_EXPORTS long OnDeviceConnStateChanged(PKDEVICE *pDevice, int nConnState)
{
	if (nConnState != 1)
	{
		CS7Device *pS7Device = (CS7Device *)pDevice->pUserData[NUSERDATA_INDEX_S7DEVICE];
		pS7Device->m_pDaveConnection->TPDUsize = 0; // 会触发重新发送握手信号
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s DisConnected, set TPDUSize == 0", pDevice->szName);
		return -1;
	}

	//daveConnection * dc = (daveConnection *)pDevice->pUserData[PUSERDATA_INDEX_DAVECONNECT];
	//if (dc == NULL) // 可能还没有初始化！
	//{
	//	Drv_LogMessage(PK_LOGLEVEL_INFO, "OnDeviceConnStateChanged device:%s, OnDeviceConnStateChanged::daveConnection == NULL!!!! from pDevice->pUserData[PUSERDATA_INDEX_DAVECONNECT]", pDevice->szName);
	//	return -1;
	//}

	//int nRet = daveConnectPLC(dc);
	//if(0==nRet) {
	//	Drv_LogMessage(PK_LOGLEVEL_INFO, "OnDeviceConnStateChanged, success to connect to device(%s), connType(%s),connParam(%s), PDU SIZE(大小):%d", pDevice->szName,pDevice->szConnType, pDevice->szConnParam, dc->TPDUsize);
	//}
	//else
	//{
	//	Drv_LogMessage(PK_LOGLEVEL_ERROR, "OnDeviceConnStateChanged, fail to connect device(%s), connType(%s),connParam(%s), RackNo:%d, SlotNo:%d.", pDevice->szName,pDevice->szConnType, pDevice->szConnParam, dc->rack);
	//}
	return 0;
}

/*
	反初始化设备
*/

PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{ 
	return 0;
}

/**
 *  设定的数据块定时周期到达时该函数被调用.
 *  
 *  可以在该函数中向设备发送请求读取实时数据或向设备发送控制指令.
 */
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	CS7Device *pS7Device = (CS7Device *)pDevice->pUserData[NUSERDATA_INDEX_S7DEVICE];
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[PUSERDATA_INDEX_TAGGROUP];
	if (pTagGroup == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s OnTimer,TagGroup == NULL!!! ERROR", pDevice->szName);
		return -1;
	}
	int nRet = pS7Device->OnTimer_ReadData(pDevice, pTagGroup);
	return nRet;
}

/**
 *  当有控制命令时该函数被调用.
 *  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
 *
 *  @version     12/11/2012    Initial Version.
 */
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	CS7Device *pS7Device = (CS7Device *)pDevice->pUserData[NUSERDATA_INDEX_S7DEVICE];
	int nRet = pS7Device->OnControl(pDevice, pTag, szStrValue, lCmdId);
	return nRet;
}

