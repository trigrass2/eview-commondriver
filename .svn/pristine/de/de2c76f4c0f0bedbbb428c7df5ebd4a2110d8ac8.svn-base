
#include "S7Device.h"
#include "math.h"
#include "AutoGroup_BlkDev.h"
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

/*
1.协议细节：
建立连接后获取pdu大小
连接建立之后，会发送MPI（这里为2）、机架号（这里为0）、槽号（第一个2，第二个1）获得PDU大小。
如果槽号给错误了，会收不到数据且会被PLC断开连接：
[022] 03 00 00 16 11 E0 00 00 00 01 00 C1 02 01 00 C2 02 01 02 C0 01 09 （槽号2，错误导致被PLC断开连接）
[022] 03 00 00 16 11 E0 00 00 00 01 00 C1 02 01 00 C2 02 01 01 C0 01 09 （槽号1）
对于上面槽号为1的请求，接受22个字节应答：
[022] 03 00 00 16 11 D0 00 01 00 01 00 C0 01 09 C1 02 01 00 C2 02 01 01

2.读取数值
数据：DB101:W305-W309的读取请求
【31】03 00 00 1f 02 f0 80 32 01 00 00 03 54 00 0e 00 00 04 01 12 0a 10 02 00 06 00 65 84 00 09 88
解释：
B1 B2：03 00 00 未知
B3 B4: 00 1F = 0x1F=31,读取报文总长度，从第一个字节到最后1个字节
B12 B13:03 54，序列号
B24 B25：00 06 = 06，读取的寄存器个数（？？）
B26 B27：00 65=0x0065=101=DB101
B28：84 = 0x84，读取的数据块类型为DB块
B29~B31：=00 09 88=0x0000988=2440,2440/8=305*8 读取的偏移量offset（bit为单位）

PLC回复报文：
【31】03 00 00 1f 02 f0 80 32 03 00 00 03 54 00 02 00 0a 00 00 04 01 ff 04 00 30 00 03 ff 03 eb 00
解释：
B1 B2：03 00 00 未知
B3 B4: 00 1F = 0x1F=31,读取报文总长度，从第一个字节到最后1个字节
B12 B13:03 54，序列号,应该等于请求的序列号
B16 B17:00 0a = 0x000A=10,读取请求寄存器个数（6）+4
B24 B25：00 30 = 0x0030=48=6*8，读取的位长度（字节长度6）
B26 ~最后：6个字节，数据值

【31】03 00 00 1f 02 f0 80 32 03 00 00 03 56 00 02 00 0a 00 00 04 01 ff 04 00 30 00 04 00 03 ec 00
*/
#define DEFAULT_REQUEST_MAXLEN				1024	
#define PK_TAGDATA_MAXLEN					4096


CS7Device::CS7Device(PKDEVICE *pDevice)
{
	m_pDevice = pDevice;
	m_strPLCType = "unknown";
	m_nPLCType = SIMENS_PLC_TYPE_300;
	m_nProtocolNo = daveProtoISOTCP;	// 协议，目前用的都是daveProtoISOTCP
	m_nMPIAddress = 2; // MPI地址？
	m_nRackNo = 0;		// 机架号，缺省为0，从0开始
	m_nSlotNo = 1;		// 槽号，缺省为1，从1开始. S7-200 Smart型号的槽号为1

	m_uTransID = 0;
	m_bNeedClearRecvBuffer = false;
	m_bBigEndian = true; // 西门子数据过来就是大端序的，所以标志一下需要转换 isBigEndian();
}

CS7Device::~CS7Device()
{

}

void CS7Device::CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess)
 {
	 if(lSuccess == PK_SUCCESS)
		 pTagGroup->nFailCountRecent = 0;
	 else
	 {
		 if(pTagGroup->nFailCountRecent > 3)	// 最近失败次数
		 {
			 char szTip[64] = {0};
			 sprintf(szTip, "read failcount:%d", pTagGroup->nFailCountRecent);
			 UpdateGroupQuality(pDevice, pTagGroup, TAG_QUALITY_COMMUNICATE_FAILURE, szTip);
			 pTagGroup->nFailCountRecent = 0; // 避免计数太大导致循环
			 Drv_Disconnect(pDevice);
		 }
		 else
			 pTagGroup->nFailCountRecent += 1;
	 }
 }


int CS7Device::OnInitDevice(PKDEVICE *pDevice)
{
	bool bBigEndian = isBigEndian();
	if (bBigEndian)
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device: %s, now runing on BigEndian CPU, should be ARM", pDevice->szName);
	else
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device: %s, now runing on LittleEndian CPU, should be Intel", pDevice->szName);

#ifdef _WIN32
	m_nfd.rfd = reinterpret_cast<HANDLE>(pDevice);
	m_nfd.wfd = reinterpret_cast<HANDLE>(pDevice);
#else
	m_nfd.rfd = reinterpret_cast<HANDLE>(pDevice);
	m_nfd.wfd = reinterpret_cast<HANDLE>(pDevice);
#endif

	int localPPI = 0;
	m_pDaveInterface = daveNewInterface(m_nfd, "IF1", localPPI, m_nProtocolNo, daveSpeed187k);
	m_pDaveConnection = daveNewConnection(m_pDaveInterface, m_nMPIAddress, m_nRackNo, m_nSlotNo);  // insert your rack and slot here
	if (m_pDaveConnection == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, daveNewConnection return NULL!!!", pDevice->szName);
	}

	return 0;
}

// DB5/I/Q....
int CS7Device::GetDaveBlockInfo(int nPLCType, char *szBlockName, int *pnDaveAreaType, int *pnIndex)
{
	*pnIndex = 1;
	char szBlockType[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	// 将DB5的前两个字母取出来
	if (strlen(szBlockName) > 2) // DB
	{
		strncpy(szBlockType, szBlockName, 2); // DB
		if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DB) == 0)
			*pnIndex = ::atoi(szBlockName + 2); // 块索引号
	}
	else
	{
		strcpy(szBlockType, szBlockName); // DB

	}
	string strBlockType = szBlockType;
	transform(strBlockType.begin(), strBlockType.end(), strBlockType.begin(), ::toupper);  //字母转小写
	strcpy(szBlockType, strBlockType.c_str());
	if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DB) == 0)
		*pnDaveAreaType = daveDB;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_I) == 0)
		*pnDaveAreaType = daveInputs;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_Q) == 0)
		*pnDaveAreaType = daveOutputs;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_COUNTER) == 0)
	{
		if (nPLCType == SIMENS_PLC_TYPE_200)
			*pnDaveAreaType = daveCounter200; //  IEC counters (200 family) 
		else
			*pnDaveAreaType = daveCounter;
	}
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_TIMER) == 0)
	{
		if (nPLCType == SIMENS_PLC_TYPE_200)
			*pnDaveAreaType = daveTimer200; // IEC timers (200 family)
		else
			*pnDaveAreaType = daveTimer;
	}
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_V) == 0)
		*pnDaveAreaType = daveV;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_P) == 0)	//direct peripheral access
		*pnDaveAreaType = daveP;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_PI) == 0)
		*pnDaveAreaType = daveInputs;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_PO) == 0)
		*pnDaveAreaType = daveOutputs;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_FLAGS) == 0)
	{
		if (nPLCType == SIMENS_PLC_TYPE_200)
			*pnDaveAreaType = daveSysFlags; // System flags of 200 family
		else
			*pnDaveAreaType = daveFlags;
	}
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_DI) == 0) // instance data blocks
		*pnDaveAreaType = daveDI;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_LOCAL) == 0)
		*pnDaveAreaType = daveLocal;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_S7_200_SYSINFO) == 0) // System info of 200 family
		*pnDaveAreaType = daveSysInfo;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_S7_200_AI) == 0) // analog inputs of 200 family
		*pnDaveAreaType = daveAnaIn;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_S7_200_AO) == 0) // analog outputs of 200 family 
		*pnDaveAreaType = daveAnaOut;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_S5_SYSDATA) == 0) //system data area?
		*pnDaveAreaType = daveSysDataS5;
	else if (PKStringHelper::StriCmp(szBlockType, BLOCK_TYPE_S5_RAWMEMORY) == 0)
		*pnDaveAreaType = daveRawMemoryS5;
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "block name invalid:%s", szBlockName);
		return -1;
	}

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
// 检查设备请求数据前是否已经取到PDU大小了
int CS7Device::CheckRequestPDU()
{
	if (m_pDaveConnection == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, OnTimer::daveConnection == NULL!!!! ", m_pDevice->szName);
		return -1;
	}

	// 对于PPI来说，调用不调用daveConnectPLC都没关系，所以如果是ppi，直接返回就可以了
	if (SIMENS_S7_PROTOCOL_TCP != this->m_nProtocolNo || m_pDaveConnection->TPDUsize > 0)
		return 0;

	int nRet = daveConnectPLC(m_pDaveConnection);
	if (0 == nRet) {
		Drv_LogMessage(PK_LOGLEVEL_INFO, "%s, Get PDU Size, connType(%s),connParam(%s), rackno:%d, slot:%d. PDU SIZE(大小):%d", 
			m_pDevice->szName, m_pDevice->szConnType, m_pDevice->szConnParam,m_nRackNo, m_nSlotNo, m_pDaveConnection->TPDUsize);
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "%s, Get PDU Size fail, connType(%s),connParam(%s), rackno:%d, slot:%d, 请检查槽号(check SLOT NO), PDU SIZE(大小):%d.", 
			m_pDevice->szName, m_pDevice->szConnType, m_pDevice->szConnParam, m_nRackNo, m_nSlotNo, m_pDaveConnection->TPDUsize);

	}

	return nRet;
}
/**
 *  设定的数据块定时周期到达时该函数被调用.
 *  
 *  可以在该函数中向设备发送请求读取实时数据或向设备发送控制指令.
 2.读取数值
 数据：DB101:W305-W309的读取请求
 【31】03 00 00 1f 02 f0 80 32 01 00 00 03 54 00 0e 00 00 04 01 12 0a 10 02 00 06 00 65 84 00 09 88
 解释：
 B1 B2：03 00 00 未知
 B3 B4: 00 1F = 0x1F=31,读取报文总长度，从第一个字节到最后1个字节
 B12 B13:03 54，序列号
 B24 B25：00 06 = 06，读取的寄存器个数（？？）
 B26 B27：00 65=0x0065=101=DB101
 B28：84 = 0x84，读取的数据块类型为DB块
 B29~B31：=00 09 88=0x0000988=2440,2440/8=305*8 读取的偏移量offset（bit为单位）

 PLC回复报文：
 【31】03 00 00 1f 02 f0 80 32 03 00 00 03 54 00 02 00 0a 00 00 04 01 ff 04 00 30 00 03 ff 03 eb 00
 解释：
 B1 B2：03 00 00 未知
 B3 B4: 00 1F = 0x1F=31,读取报文总长度，从第一个字节到最后1个字节
 B12 B13:03 54，序列号,应该等于请求的序列号
 B16 B17:00 0a = 0x000A=10,读取请求寄存器个数（6）+4
 B24 B25：00 30 = 0x0030=48=6*8，读取的位长度（字节长度6）
 B26 ~最后：6个字节，数据值
 */
int CS7Device::OnTimer_ReadData(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{ 
	// 组织读取消息
	CheckRequestPDU();
	if (this->m_nProtocolNo == SIMENS_S7_PROTOCOL_TCP && m_pDaveConnection->TPDUsize <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "%s, Get PDU Size fail, connType(%s),connParam(%s), rackno:%d, slot:%d, 请检查槽号(check SLOT NO), PDU SIZE(大小):%d.",
			m_pDevice->szName, m_pDevice->szConnType, m_pDevice->szConnParam, m_nRackNo, m_nSlotNo, m_pDaveConnection->TPDUsize);
		return -1;
	}

	unsigned short uTransID = ++m_uTransID;

	// 先清空接收缓冲区，避免以前发的请求数据没有来得及接收还放在缓冲区，影响本次请求
	if(m_bNeedClearRecvBuffer)
		Drv_ClearRecvBuffer(pDevice);	
	//Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s OnTimer 1", pDevice->szName);
	int nDaveAreaType, nDaveAreaIndx;
	char *szPLCType = pDevice->szParam1; // S7: 200/300/400,1200同400
	int result = GetDaveBlockInfo(m_nPLCType, pTagGroup->szHWBlockName, &nDaveAreaType, &nDaveAreaIndx);


	int nBeginRegister = pTagGroup->nBeginRegister;
	int nRegisterNum = pTagGroup->nRegisterNum;

	//Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s OnTimer 3.1 BeginRegister:%d, RegisterNum:%d", pDevice->szName, nBeginRegister, nRegisterNum);

	int res=daveReadBytes(m_pDaveConnection, nDaveAreaType, nDaveAreaIndx, nBeginRegister, nRegisterNum, NULL); // 起始字节以0开始.每个寄存器计算时按照8位计算
	//Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s OnTimer 3", pDevice->szName);
	if(0==res) 
	{
		Drv_LogMessage(PK_LOGLEVEL_INFO, "updateblockdata device:%s,group:%s success,len:%d,tagcount:%d, first byte:%d", pDevice->szName, pTagGroup->szAutoGroupName, m_pDaveConnection->AnswLen, pTagGroup->vecTags.size(), *(unsigned char *)m_pDaveConnection->resultPointer);
		UpdateGroupData_Simens(pDevice, pTagGroup, (char *)m_pDaveConnection->resultPointer, m_pDaveConnection->AnswLen, 0);
		CheckBlockStatus(pDevice, pTagGroup, 0);
		return 0;
	}
	

	if(res == daveResTimeout)
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "(daveReadBytes) failed to read device:%s,group:%s,areatype:%s, fromregister: %d,len: %d,errcode:%d(timeout).", 
		pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->szHWBlockName, pTagGroup->nBeginRegister, pTagGroup->nRegisterNum, res);  
	else
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "(daveReadBytes) failed to read device:%s,group:%s,areatype:%s, fromregister %d,len: %d,errcode:%d.", 
		pDevice->szName, pTagGroup->szAutoGroupName, pTagGroup->szHWBlockName, pTagGroup->nBeginRegister, pTagGroup->nRegisterNum, res);  
	CheckBlockStatus(pDevice, pTagGroup, -1);
	//Drv_LogMessage(PK_LOGLEVEL_ERROR, "(daveReadBytes) failed end");
	return -1; 
}

/**
 *  当有控制命令时该函数被调用.
 *  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
 *
 *  @version     12/11/2012    Initial Version.
 */
int CS7Device::OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	char szBinValue[PK_TAGDATA_MAXLEN] = {0};
	int nBinValueLen = 0;
	Drv_TagValStr2Bin(pTag, szStrValue, szBinValue, sizeof(szBinValue), &nBinValueLen);

	char *szTagName = pTag->szName;
	char *szAddress = pTag->szAddress;

	char szRequestBuff[DEFAULT_REQUEST_MAXLEN];
	memset(szRequestBuff,0, sizeof(szRequestBuff));
	long nStatus = PK_SUCCESS;
	int  i = 0;

	//Drv_LogMessage(PK_LOGLEVEL_DEBUG, "@@@收到控制命令：向设备(%s)的tag(%s)进行控制，地址:%s",pDevice->pszName, szTagName, szAddress);

	DRVGROUP *pTagGroup = (DRVGROUP *)pTag->pData1;
	int nDaveAreaType,nDaveAreaIndx;
	int result = GetDaveBlockInfo(m_nPLCType, pTagGroup->szHWBlockName, &nDaveAreaType, &nDaveAreaIndx);
	daveConnection *dc = m_pDaveConnection;
	if (dc == NULL)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s, OnControl::daveConnection == NULL!!!!", pDevice->szName);
		return 0;
	}

	int nStartBits = pTag->nStartBit;// 相对于块内的起始地址位（如AI/DI), 从0开始
	int nEndBits = pTag->nEndBit; // 相对于块内的结束地址位（如AI/DI)
	//int nLenBits = nEndBits - nStartBits + 1; // 长度（位表示）
	int nStartBytes = (int)ceil(nStartBits / 8.0f); // AO是2个字节为单位
	//int nByteNum = (int)ceil(nLenBits / 8.0f); // AO是2个字节为单位

	int nRet = 0;
	if(pTag->nLenBits > 1)
	{
		int nStartBytes = (int)(nStartBits / 8);
		int nEndBytes = (int)(nEndBits / 8);
		int nLenBytes = nEndBytes - nStartBytes + 1;
		if(nLenBytes > nBinValueLen){
			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!控制命令：向设备(%s)发送写tag(%s)请求， 需要数据长度:%d，仅提供长度:%d", 
				pDevice->szName, pTag->szName, nLenBytes, nBinValueLen);
		}
		nRet = daveWriteBytes(dc,nDaveAreaType,nDaveAreaIndx,nStartBytes,nBinValueLen,szBinValue);
	}
	else
	{
		nRet = daveWriteBits(dc,nDaveAreaType,nDaveAreaIndx,nStartBits,1,szBinValue);
	}

	//Drv_LogMessage(PK_LOGLEVEL_NOTICE, "!!!控制命令：向设备(%s)发送写tag(%s)请求成功", pDevice->pszName, pTag->szTagName);

	{
		// 对于设备为AO，但上位配置为DO（取AO的某一个位）；如果上位连续对某个AO（2个字节），
		// 在一个读写周期内，两个不同的位连续控制，会出现后面会覆盖前面的控制值

		//memcpy(pTag->szTagData, szBinValue, 1);
		//pTag->nTagDataLen = 1;
		//Drv_UpdateTagsData(hDevice,pTag,1,0,NULL);
		//OnTimer(hDevice, "block",pTagGroup,NULL);
	}
	return 0;
}

bool CS7Device::isBigEndian()
{
	int a = 1;
	if (((char*)&a)[sizeof(int) - 1] == 1) {
		return true;
	}
	else
		return false;
}

/**
*  字节顺序转换.
*
*  @param  -[in, out]  char*  pOrig: [comment]
*  @param  -[in]  int  nLength: [comment]
*
*  @version     07/25/2008  lijingjing  Initial Version.
*/
inline long SwapByteOrder(char* pOrig, int nLength)
{
	int i = 0;
	int nSwapCount = 0;
	int nWordBytes = 2;
	char chTemp;
	if (nLength <= 2)
		nWordBytes = 2;
	else
		nWordBytes = 4;

	if (nWordBytes == 2)
	{
		nSwapCount = nLength / 2;
		for (i = 0; i < nSwapCount; i++)
		{
			chTemp = *pOrig;
			*pOrig = *(pOrig + 1);
			*(pOrig + 1) = chTemp;
			pOrig += 2;
		}
	}
	else if (nWordBytes == 4)
	{
		nSwapCount = nLength / 4;
		for (i = 0; i < nSwapCount; i++)
		{
			// 第0和第3个字节交换
			chTemp = *(pOrig + 3);
			*(pOrig + 3) = *pOrig;
			*pOrig = chTemp;

			// 第1和第2个字节交换
			chTemp = *(pOrig + 2);
			*(pOrig + 2) = *(pOrig + 1);
			*(pOrig + 1) = chTemp;

			pOrig += 4;
		}
		// 不足4个字节的部分
		if (nLength - nSwapCount * 4 == 2)
		{
			// 剩余两个字节
			chTemp = *pOrig;
			*pOrig = *(pOrig + 1);
			*(pOrig + 1) = chTemp;
		}
		else if (nLength - nSwapCount * 4 == 3)
		{
			// 剩余三个字节
			chTemp = *pOrig;
			*pOrig = *(pOrig + 2);
			*(pOrig + 2) = chTemp;
		}
	}
	return PK_SUCCESS;
}

// 西门子PLC是大端序的数据，0x12345678，传过来的二进制数据为：12 34 56 78
int CS7Device::UpdateTagDataWithByteOrder(PKDEVICE *pDevice, PKTAG *pTag, char *szDataBuff, int nDataBuffLen)
{
	// 西门子PLC是大端序的数据，0x12345678，传过来的二进制数据为：12 34 56 78.此时不用做转换
	int nRet = 0;

	// 下面这些数据类型不需要做大小端转换
	if (pTag->nDataType == TAG_DT_BOOL || pTag->nDataType == TAG_DT_CHAR || pTag->nDataType == TAG_DT_UCHAR || pTag->nDataType == TAG_DT_TEXT || pTag->nDataType == TAG_DT_BLOB)
	{
		nRet = Drv_SetTagData_Binary(pTag, (void *)(szDataBuff), nDataBuffLen);
		return nRet;
	}

    // 下面是超过2个字节的数值，本机无论大小端序都需要做处理。大小端序的转换在后续pkdrvcmn会处理
	if (pTag->nDataLen < nDataBuffLen)
		nDataBuffLen = pTag->nDataLen;

	if (nDataBuffLen <= 0 || nDataBuffLen >= 8)
	{
		nRet = Drv_SetTagData_Binary(pTag, (void *)(szDataBuff), nDataBuffLen);
		return nRet;
	}

	char szBuffOrdered[8];
	memcpy(szBuffOrdered, szDataBuff, nDataBuffLen);

	// 做字节转换
	SwapByteOrder(szBuffOrdered, nDataBuffLen);
	nRet = Drv_SetTagData_Binary(pTag, (void *)szBuffOrdered, nDataBuffLen);
	return nRet;
}

// 因为西门子的字符串，分配长度为8，实际上占用了10个字节，前两个字节是头，第一个字节表示字符窜长度8，第二个是实际内容长度（如7，abcdefg）。
// 必须使用第二个字节，因为内存中可能存的是abcdefgh，实际上我们只要前7个
long CS7Device::UpdateGroupData_Simens(PKDEVICE *pDevice, DRVGROUP *pTagGroup, const char *szBuffer, long lBufLen, short nStatus)
{
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
		PKTAG *pTag = pTagGroup->vecTags[iTag];
		pTag->nQuality = nStatus; // 质量先赋值
		// 计算数据所在第一个字节（相对于块首地址）
		int nStartBitInGroup = pTag->nStartBit - pTagGroup->nStartAddrBit; // 由相对于物理块的绝对地址，计算出相对于块内的相对地址
		int nStartByteInBlock = nStartBitInGroup /8; // 块内的起始字节
		// 计算数据所在最胡一个字节（相对于块首地址）
		int nEndBitInGroup = pTag->nEndBit - pTagGroup->nStartAddrBit;
		int nEndByteInBlock = nEndBitInGroup / 8;

		// 计算第一个字节的起始位（余数）
		int nStartBitInByte = nStartBitInGroup %8;	
		// 计算占用的字节数
		int nLenBytes = nEndByteInBlock - nStartByteInBlock + 1;

		if(pTag->nLenBits == 1) // 按位取，可能是DB3:D1.1, DI/DO
		{				
			unsigned char ucValue = (*(unsigned char *)(szBuffer + nStartByteInBlock));
			unsigned char ucFinalValue = ((ucValue >> nStartBitInByte) & 0x01) != 0; // 取nTagOffsetOfOneByte位

			Drv_SetTagData_Binary(pTag, &ucFinalValue, 1);
			//pTag->nDataLen = 1;
		}
		else // 按字节访问
		{
			//pTag->nDataLen = nLenBytes;
			if (pTag->nDataType != TAG_DT_TEXT && pTag->nDataType != TAG_DT_BLOB)
			{
				UpdateTagDataWithByteOrder(pDevice, pTag, (char *)(szBuffer + nStartByteInBlock), nLenBytes);
			}
			else // pTag->nDataType == TAG_DT_TEXT
			{
				const char *pTmp = szBuffer + nStartByteInBlock;
				unsigned char nStrLen = *(pTmp + 1);
				if(nStrLen > pTag->nDataLen)
					nStrLen = pTag->nDataLen;

				Drv_SetTagData_Binary(pTag, (void *)(szBuffer + nStartByteInBlock + 2), nStrLen);
			}
		}
	}

	Drv_UpdateTagsData(pDevice, pTagGroup->vecTags.data(), pTagGroup->vecTags.size());
	return 0;
}

// 支持的地址格式：DB1.DBX70.0/DB1.DBB70.0/DB1.DBD70/DB1.DBW70.1。亦支持:DB1.DBX.70.0。其中点号都可以用冒号或#号代替
// DB1表示块号；后面的表示起始地址，DBX70,X70:按位；DBB70，B70：按字节；DBW，W70：按字；DBD70，D70：双字。再之后的.号表示起始地址开始后的第几位
// 支持的块：DB/C/T/V/I/Q,I 1.0 | Q 1.7 | M 10.1,IB 1,IW 1,ID 1,PIB 0,PIW 0,PID 0 ,T 1,C1,
// 输入：szAddressInDevice：变量地址
// 输入：nLenBits，根据tag类型计算得到的tag值位数
// 输出：szBlockName, nBlockNameLen，解析得到的块名称
// 输出：*pnStartBits、*pnEndBits, 相对于相对整个物理块（如AI、DO、D、DB1，而不是重组的某个AI内的某个Group）内的起始位、结束位（含结束位，如16则起始位0，结束位15），以位为单位
int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameBufLen, int *pnStartBits, int *pnEndBits)
{
	*pnStartBits = 0;
	*pnEndBits = 0;
	int nBitNo;
	int *pnBitNo = &nBitNo;
	*pnBitNo = -1;

	// 复制地址，并去掉#和：号。该方法可能会被多线程调用，不能用静态变量
	char szAddress[PK_IOADDR_MAXLEN + 1] = { 0 };
	char *pDest = szAddress;
	char *pTmp = szAddressInDevice;
	while (*pTmp != '\0')
	{
		*pTmp = toupper(*pTmp); //先转为大写

		if (*pTmp == ' ') // 空格则直接跳过，忽略不计
		{
		}
		else if (*pTmp == '#' || *pTmp == ':') // 将#、:号、空格替换成.号
			*pDest = '.';
		else
			*pDest = *pTmp;
		pTmp++;
		pDest++;
	}

	pTmp = szAddress; // 指向字符串的首地址

					  // 找到第一个.号前的就是块名（DB1.DBD0），如果是I1.0，则是数字前
	pDest = szBlockName;
	bool bIsDBBlock = false;
	if (strstr(pTmp, "DB") == pTmp)
		bIsDBBlock = true;
	else
		bIsDBBlock = false;
	while (*pTmp != '\0')
	{
		// DB块找到.号之前作为块名DB1.DBD0。非DB块找到第一个是阿拉伯数字的字符或.号，之前的就是块名I1.0,IB1.0，ID1.0，IW1
		if (*pTmp == '.')
			break;
		if (!bIsDBBlock && (*pTmp >= '0' && *pTmp <= '9')) // 找到第一个是阿拉伯数字的字符，之前的就是块名
			break;
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}
	if (strlen(szBlockName) == 0)//
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no block type name! exit...", szAddressInDevice);
		return -1;
	}
	if (*pTmp == '.') // 跳过.号，指向下一个字符
		pTmp++;

	// 剩余：DB块：DBW25/W25/DBX25/X25。非DB块：1.0/B1.0/W1.0/D1
	// 找到块名后面的块的数值类型(DBW/W,DBD/D,DBX/X,DBB/B)，直到结束或者有.号。
	// 找到第一个.号或数字前的就是块名

	if (bIsDBBlock) {	// DB块的话先跳过前面的DB,DBW25--->W25
		if (strstr(pTmp, "DB") == pTmp)
			pTmp = pTmp + 2;
	}

	// 后面的处理就相同了，无论DB还是非DB，都是D[B|W|无]X.Y
	char chRegisterType = 'B'; // 缺省为字节B，如I、Q
	char szRegisterDataType[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szRegisterDataType;
	while (*pTmp != '\0' && *pTmp != '.')
	{
		if (*pTmp >= '0' && *pTmp <= '9') // 找到第一个是阿拉伯数字的字符，之前的就是块名
			break;
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}
	if (strlen(szRegisterDataType) == 0)//如果第一个字符是阿拉伯数字,包括I1.0,Q1.1等情况
		chRegisterType = 'B';
	else if (PKStringHelper::StriCmp(szRegisterDataType, "D") == 0 || PKStringHelper::StriCmp(szRegisterDataType, "DW") == 0)
		chRegisterType = 'D';
	else if (PKStringHelper::StriCmp(szRegisterDataType, "W") == 0)
		chRegisterType = 'W';
	else if (PKStringHelper::StriCmp(szRegisterDataType, "B") == 0)
		chRegisterType = 'B';
	else if (PKStringHelper::StriCmp(szRegisterDataType, "X") == 0)
		chRegisterType = 'X';
	else
	{
		chRegisterType = 'B';
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, register datatype:%s,do not support,e.g.DB1.DBB70, support as Byte !", szAddressInDevice, szRegisterDataType);
		// return -2;
	}

	if (*pTmp == '.') // 跳过.号，指向下一个字符
		pTmp++;

	char szRegisterStart[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szRegisterStart;
	while (*pTmp != '\0' && *pTmp != '.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}
	if (strlen(szRegisterStart) == 0)//如果第一个字符是阿拉伯数字，那么就认为是D块
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no startAddress", szAddressInDevice);
		return -3;
	}
	// 找到地址后面的位号，有.号
	if (*pTmp == '.') // 跳过.号自己
		pTmp++;

	char szBitNo[PK_DATABLOCKTYPE_MAXLEN] = { 0 };
	pDest = szBitNo;
	while (*pTmp != '\0' && *pTmp != '.') // 只要不结束并且不等于.号则赋值
	{
		*pDest = *pTmp; // 复制字符
		pTmp++;
		pDest++;
	}

	// 计算起始地址。西门子的地址是从0开始的，所以不需要-1
	// DB1.DBW1，其中的W表示的是数据的长度，并不是数据类型。所有类型都是按字节
	int	 nStartAddr = ::atoi(szRegisterStart);
	if (chRegisterType == 'X') // bit
		*pnStartBits = nStartAddr * 8;
	else if (chRegisterType == 'B')
		*pnStartBits = nStartAddr * 8;
	else if (chRegisterType == 'W')
		*pnStartBits = nStartAddr * 8;
	else if (chRegisterType == 'D')
		*pnStartBits = nStartAddr * 8;
	else
	{
		*pnStartBits = nStartAddr * 8;
		// Drv_LogMessage(PK_LOGLEVEL_ERROR, "parse tag addr:%s, no valid datatype", szAddressInDevice);
		// return -4;
	}

	// 计算起始位，对于字地址有效
	if (strlen(szBitNo) > 0)
	{
		int nBitNo = 0;
		nBitNo = ::atoi(szBitNo);
		if (nBitNo >= 0)
			*pnStartBits += nBitNo;
	}

	*pnEndBits = *pnStartBits + nLenBits - 1;

	return 0;
}