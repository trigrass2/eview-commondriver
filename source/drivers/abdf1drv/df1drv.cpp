#include "ace/Time_Value.h"
#include "ace/OS.h"
#include "df1drv.h"
#include "math.h"
#include "AutoGroup_BlkDev.h"
#include <memory.h>
#include <cstring>
#include <string.h> 
#include <stdlib.h>
#include <string>
#include <cstdio>
#include "time.h"
#include "pkcomm/pkcomm.h"
#include "Message.h"

#define EC_ICV_INVALID_PARAMETER                    100
#define EC_ICV_DRIVER_DATABLOCK_TYPECANNOTWRITE		101
#define EC_ICV_DRIVER_DATABLOCK_UNKNOWNTYPE			103
#define EC_ICV_DRIVER_DATABLOCK_INVALIDCMDLENGTH	104
#define EC_ICV_BUFFER_TOO_SMALL                     105
#define	INT_MAX										2147483647

#define DEVPARAM_TRANSID							0	// 事务号
#define DEVPARAM_MAXBYTES_ONEBLOCK					1	// 每个块的最大字节数
#define DEVPARAM_CLEARFLAG							2	// 是否需要先清除缓冲区


const unsigned short dataFileN = 0x0700;		// 根据RSLinux的抓包推测出来的

unsigned short ABcrc16(unsigned char* crc16_data, int len)
{
	unsigned short crc_value = 0;
	for (int i = 0; i <= len; i++)
	{
		if (i == len)
		{
			crc_value ^= 0x03;
		}
		else
		{
			crc_value ^= crc16_data[i];
		}
		for (int j = 0; j < 8; j++)
		{
			if (crc_value & 0x01)
			{
				crc_value = (crc_value >> 1) & 0x7fff;
				crc_value ^= 0xA001;
				continue;
			}
			crc_value = (crc_value >> 1) & 0x7fff;
		}
	}
	return crc_value;
}
bool is_little_endian()
{
	const int i = 1;
	char *ch = (char*)&i;
	return ch[0] && 0x01;
}
void to_little_endian(void* src, size_t size)
{
	if (!is_little_endian())
	{
		char chTmp = '\0';
		for (size_t i = 0; i < size / 2; ++i)
		{
			chTmp = ((char *)src)[i];
			((char *)src)[i] = ((char *)src)[size - 1 - i];
			((char *)src)[size - 1 - i] = chTmp;
		}
	}
}

// 包的主体部分，不包含包头和包尾 10 02 [01 00 0f 00 00 00 01 01 00 02 00 07 00 0a 1e 02] 10 03 bb，中括号中的部分
char checkBcc(char szPackBody[], int size,int nflag)
{
	unsigned char bcc = 0;
	int flag = 0;
	for (int i = 0; i < size; i++)
	{
		if (*(szPackBody + i) == 0x10)
		{
			flag++;
		}
		if (flag >= 2)
		{
			flag = 0;
			continue;
		}
		if (flag <= 1)
		{
			bcc += (*(szPackBody + i));
			bcc &= 0xff;
			if (nflag)
			{
				Drv_LogMessage(PK_LOGLEVEL_NOTICE, "check %x", bcc);
			}
		}
	}
	bcc = (~bcc) + 1;
	if (nflag)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE, "check %x", bcc);
	}
	
	return bcc&0xff;
}

bool IsTagAddrssValid(PKDEVICE *pDevice, PKTAG *pTag)
{
	string strTagAddress = pTag->szAddress;
	vector<string> vecAddr = PKStringHelper::StriSplit(strTagAddress, ":");
	if (vecAddr.size() <= 1)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,tag:%s, address:%s INVALID, should:Nxx:yy, i.e. N10:31", pDevice->szName, pTag->szName, strTagAddress.c_str());
		return false;
	}
	if (vecAddr[0].length() <= 0 || vecAddr[1].length() <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,tag:%s, address:%s INVALID, should:Nxx:yy, i.e. N10:31", pDevice->szName, pTag->szName, strTagAddress.c_str());
		return false;
	}
	return true;
}

// 这是TAG地址，如N10:31,第10个文件的第31个地址（双字？）开始。N后表示第几个文件，N10是第10个文件，第31个字节（字？）开始读取2个字节的数据。返回？
// 这个地址是根据RSLinux读写PLC的协议包推测出来的
// N10:31,01 01 00 02 00 07 00 0a 1e 02
int phaseAddr(string rawAddr, char* pddr)
{
	char choice = rawAddr[0];
	char temp[10] = { 0 };
	int addrLen = 0;

	unsigned short packetOffset = atoi(rawAddr.substr(rawAddr.find(':') + 1).c_str());
	*pddr = (packetOffset % 10) % 255;
	*(pddr + 1) = (packetOffset % 10) / 255;
	pddr += 2;
	addrLen += 2;

	char totalTrans = 0;
	*pddr = (packetOffset % 10) + 1;
	pddr++;
	addrLen += 1;

	switch (choice)
	{
	case 'N':
		::memcpy(pddr, &dataFileN, 2);
		break;
	default:
		break;
	}
	pddr += 2;
	addrLen += 2;

	int pos = rawAddr.find(':') + 1;
	int fileNum = atoi(rawAddr.substr(1, pos).c_str());
	*pddr = fileNum / 255;
	*(pddr + 1) = fileNum % 255;
	if (fileNum % 255 == 0x10)
	{
		*(pddr + 2) = 0x10;
		pddr++;
		addrLen++;
	}
	pddr += 2;
	addrLen += 2;

	string num = rawAddr.substr(pos);
	int offset = atoi(num.c_str());
	//*pddr = offset - 1; // 这个字节必须给0，不能是块内地址偏移！
	*pddr = (offset - offset % 10); // N10:31--->1e, N10:6--->0, N10:7--->0
	pddr += 1;
	addrLen += 1;

	char readByte = 2;
	*pddr = readByte;
	addrLen += 1;

	return addrLen;
}

// 读取消息N10:31完整包如下：   
// 发送：10 02 01 00 0f 00 00 01 01 01 00 02 00 07 00 0a 1e 02 10 03 BA
// 发送：10 02 01 00 0f 00 00 00 01 01 00 02 00 07 00 0a 1e 02 10 03 bb，解析：10 02() 01(DST) 00(SRC) 0f(CMD) 00(STS) 00 00(TNS) 01 01 00 02 00 07 00 0a 1e 02		10 03		bb
// 接收：10 02 00 01 4f 00 00 01 21 00 10 03 8e
int BuildReadMsg(PKDEVICE *pDevice, char *szReadMsg, int nTransId, string addr)
{
	//10 02 01 00 0F 00 01 18 01
	char *pMsg = szReadMsg;
	unsigned short start = 0x0210; //10 02 报头， 10 02 DLE STX
	::memcpy(pMsg, &start, sizeof(start));
	pMsg += 2;

	char dst = 0x01;   //源地址，反了？，DST
	::memcpy(pMsg, &dst, sizeof(char));
	pMsg += 1;

	char src = 0x00; //目标地址  ,rslinx中配置， SRC
	::memcpy(pMsg, &src, sizeof(char));
	pMsg += 1;

	char cmd = 0x0f;     //CMD,unprotected read
	::memcpy(pMsg, &cmd, sizeof(char));
	pMsg += 1;

	char status = 0x00;  //状态位，STS
	::memcpy(pMsg, &status, sizeof(char));
	pMsg += 1;

	unsigned short tns = nTransId;  //事务号，TNS（2字节）
	::memcpy(pMsg, &tns, sizeof(tns));
	to_little_endian(pMsg, 2);
	pMsg += 2;

	// 下面是数据区，ADDR（2字节）和SIZE（1字节）
	char func = 0x01;   //func？
	::memcpy(pMsg, &func, sizeof(char));
	pMsg += 1;

#if 1
	char temp[20] = { 0 };
	int len = phaseAddr(addr, temp);  //地址解析，N10:31-->01 00 02 00 07 00 0a 1e 02， 9个字节
	::memcpy(pMsg, temp, len);
	pMsg += len;
#endif

	unsigned short end = 0x0310; // 包尾结束标志
	::memcpy(pMsg, &end, sizeof(end));
	pMsg += 2;

	char bcc = checkBcc(szReadMsg + 2, len + 9 - 2, 0) & 0x00ff;  //BCC校验码 10 02 与 10 03 之间数据，10 01 STN [10 02 DST(目标节点) SRC(源节点) CMD(命令字) STS(状态字节) TNS(2字节事务号) Data  10 03] BCC/CRC
	*pMsg = bcc;
	pMsg++;
	int nMsgLen = pMsg - szReadMsg;

	return nMsgLen;
}


// 是否需要清楚标志位
void SetClearRecvBufferFlag(PKDEVICE *pDevice)
{
	pDevice->nUserData[DEVPARAM_CLEARFLAG] = 1;
}

bool GetClearRecvBufferFlag(PKDEVICE *pDevice)
{
	return pDevice->nUserData[DEVPARAM_CLEARFLAG] != 0;
}

void CheckBlockStatus(PKDEVICE *pDevice, DRVGROUP *pTagGroup, long lSuccess)
{
	if (lSuccess == PK_SUCCESS)
		pTagGroup->nFailCountRecent = 0;
	else
	{
		if (pTagGroup->nFailCountRecent > 3)	// 最近失败次数
		{
			char szTip[1024] = { 0 };
			sprintf(szTip, "read failcount:%d", pTagGroup->nFailCountRecent);
			UpdateGroupQuality(pDevice, pTagGroup, -1, szTip);
			pTagGroup->nFailCountRecent = 0; // 避免计数太大导致循环
		}
		else
			pTagGroup->nFailCountRecent += 1;
	}
}

PKDRIVER_EXPORTS long InitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDriver(driver:%s)", pDriver->szName);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "设备4个参数均未使用!");
	return 0;
}

PKDRIVER_EXPORTS long InitDevice(PKDEVICE *pDevice)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "InitDevice(device:%s)", pDevice->szName);
	pDevice->nUserData[DEVPARAM_TRANSID] = 0;				// 事务号

	// 获取到所有的tag点。需要在tag点存储块内偏移（位）、长度（位），组包含的tag点对象列表（以便计算）
	// 进行自组块处理，将所有的tag点自组块成BLOCK
	GroupVector vecTagGroup;
	GROUP_OPTION groupOption;
	groupOption.nIsGroupTag = 1;
	groupOption.nMaxBytesOneGroup = pDevice->nUserData[DEVPARAM_MAXBYTES_ONEBLOCK]; // modbus每种设备都有所不同，应该是作为参数传过来比较合适
	if (groupOption.nMaxBytesOneGroup <= 0) //如果没有输入，则取缺省230个字节
		groupOption.nMaxBytesOneGroup = 230;

	PKTIMER timerInfo;
	timerInfo.nPeriodMS =1000;
	timerInfo.pUserData[0] = NULL;
	void *pTimer = Drv_CreateTimer(pDevice, &timerInfo); // 设定定时器
	return 0;
}

/*
反初始化设备
*/
//#include "windows.h"
PKDRIVER_EXPORTS long UnInitDevice(PKDEVICE *pDevice)
{
	//Sleep(100000);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDevice(device:%s)", pDevice->szName);
	int nTimerNum = 0;
	////PKTIMER * pTimers = Drv_GetTimers(pDevice, &nTimerNum);
	////int i = 0;
	////for (i = 0; i < nTimerNum; i++)
	////{
	////	PKTIMER *pTimerInfo = &pTimers[i];
	////	DRVGROUP *pTagGroup = (DRVGROUP *)pTimerInfo->pUserData[0];
	////	delete pTagGroup;
	////	pTagGroup = NULL;
	////	Drv_DestroyTimer(pDevice, pTimerInfo);
	////}
	return 0;
}

PKDRIVER_EXPORTS long UnInitDriver(PKDRIVER *pDriver)
{
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "UnInitDriver(driver:%s)", pDriver->szName);
	return 0;
}

// 处理包的转义字符，并返回转义后的包
int TranslatePackage(char *szOriginalPackBuf, int nOrignalPackLen, char *szTranslatedPackBuf, int &nTranslatedPackLen)
{
	char *pBufOrig = szOriginalPackBuf;
	char *pBufOrigEnd = szOriginalPackBuf + nOrignalPackLen;
	char *pDestPackBuf = szTranslatedPackBuf;
	while (pBufOrig < pBufOrigEnd) // 最后多判断1个数据，不过不太影响
	{
		if (*pBufOrig == 0x10 && *(pBufOrig + 1) == 0x10) // 转义的字符，2个算是1个10
		{
			*pDestPackBuf = *pBufOrig;
			pBufOrig += 2;
		}
		else
		{
			*pDestPackBuf = *pBufOrig;
			pBufOrig++;
		}
		pDestPackBuf++;
	}

	nTranslatedPackLen = pDestPackBuf - szTranslatedPackBuf;
	return nTranslatedPackLen;
}

//检查原始数据包的BCC是否正确 10 02 00 01 4f 00 00 01 21 00 10 03 8e, 00 01 4...21 00 需要计算BCC
bool CheckPackageBcc(char *pPackBuff, int nPackBufLen)
{
	char *pBccBegin = pPackBuff + 2;
	int nBccBufLen = nPackBufLen - 5;
	char nBccValInPack = pPackBuff[nPackBufLen - 1]; // 最后1个字节是BCC
	char nCalcBcc = checkBcc(pBccBegin, nBccBufLen, 0);
	if (nBccValInPack == nCalcBcc)
		return true;
	return false;
}

// 在接收到的数据缓冲区中，找到一个10 02....10 03 BCC的包，包含包头、包尾、BCC正确且事务号为HopeTransId，应答的功能码为0x4F(nHopeTransId)
// 如果找到了包，返回的pHopePackageHeader指向10 02，nHopePackLen是包含10 02...10 03 BCC的总长度
// 请求：10 02 01 00 0f 00 00 01 01 01 00 02 00 07 00 0a 1e 02 10 03 BA
// 应答应该是:10 02 00 01 4f 00 00 01 21 00 10 03 8e
int FindPackage(PKDEVICE *pDevice, char *szReadMsg, int nReadMsgLen, int nHopeCmd, int nHopeTransId, char *szHopePackBuf, int &nHopePackLen, char *&pCurBuf)
{
	bool bFoundValidPack = false;
	nHopePackLen = 0;
	char *pOnePackHead = szReadMsg;
	int nOnePackLen = 0;
	char *pPackEnd = NULL;
	pCurBuf = szReadMsg;
	char *pBufEnd = szReadMsg + nReadMsgLen;
	while (pCurBuf <= pBufEnd)
	{
		bool bFoundPack = false;
		if (*pCurBuf == 0x10 && *(pCurBuf + 1) == 0x02) //报头
		{
			pOnePackHead = pCurBuf; // 找到了包头
			pCurBuf += 2; // 10 02
		}
		else if (*pCurBuf == 0x10 && *(pCurBuf + 1) == 0x03) //报尾
		{
			if (pOnePackHead)
			{
				pPackEnd = pCurBuf + 3;
				nOnePackLen = pPackEnd - pOnePackHead; // 包的总长度
				pCurBuf += 3; // 10 03 BCC
				bFoundPack = true;
			}
		}
		else
			pCurBuf++; //继续下个字节

		if (!bFoundPack)
			continue;

		// 校验包长度是否合法
		if (nOnePackLen < 13) // 一个包应该等于13，考虑到有10需要转义为两个10的现象存在，这个包应该大于13
		{
			char szHex[1024];
			unsigned int nHexStringLen = 0;
			PKStringHelper::HexDumpBuf(pOnePackHead, nOnePackLen, szHex, sizeof(szHex), &nHexStringLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,recv a response package:%s, len:%d < 13, INVALID!", pDevice->szName, szHex, nHexStringLen);
			continue;
		}

		bool bCheckBcc = CheckPackageBcc(pOnePackHead, nOnePackLen);
		if (!bCheckBcc)
		{
			char szHex[1024];
			unsigned int nHexStringLen = 0;
			PKStringHelper::HexDumpBuf(pOnePackHead, nOnePackLen, szHex, sizeof(szHex), &nHexStringLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,recv a response package:%s, len:%, BCC INVALID!", pDevice->szName, szHex, nHexStringLen);
			continue;
		}

		// 对数据包进行转义！
		int nPackLen = TranslatePackage(pOnePackHead, nOnePackLen, szHopePackBuf, nHopePackLen); // 得到1个转以后的数据包
		if (nHopePackLen != 13)
		{
			char szHex[1024];
			unsigned int nHexStringLen = 0;
			PKStringHelper::HexDumpBuf(szHopePackBuf, nHopePackLen, szHex, sizeof(szHex), &nHexStringLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,recv a response package, 转以后:%s, len:% != 13!", pDevice->szName, szHex, nHexStringLen);
			continue;
		}

		// 功能码必须为0x4F, 10 02 00 01 4f 00 00 01 21 00 10 03 8e
		char nCmdNo = *(szHopePackBuf + 4);
		if (nCmdNo != nHopeCmd)
		{
			char szHex[1024];
			unsigned int nHexStringLen = 0;
			PKStringHelper::HexDumpBuf(szHopePackBuf, nHopePackLen, szHex, sizeof(szHex), &nHexStringLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,recv a response package, translated:%s, len:% , cmd:%d != hope transno:%d!", pDevice->szName, szHex, nHexStringLen, nCmdNo, nHopeCmd);
			continue;
		}

		// 事务号必须相等, 10 02 00 01 4f 00 00 01 21 00 10 03 8e
		unsigned short nTransIdInPack = 0;
		memcpy(&nTransIdInPack, szHopePackBuf + 6, 2);
		if (nTransIdInPack != nHopeTransId)
		{
			char szHex[1024];
			unsigned int nHexStringLen = 0;
			PKStringHelper::HexDumpBuf(szHopePackBuf, nHopePackLen, szHex, sizeof(szHex), &nHexStringLen);
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,recv a response package translated:%s, len:%d , tansNo:%d != Hope transno:%d!", pDevice->szName, szHex, nHexStringLen, nTransIdInPack, nHopeTransId);
			continue;
		}

		// 下面是合法的数据包
		bFoundValidPack = true;
		break;
	}

	if (bFoundValidPack) // 找到合法的包
		return 0;
	else
		return -1; // 未找到合法的数据包
}
/**
*  设定的数据块定时周期到达时该函数被调用.
*
*  @return PK_SUCCESS: 执行成功
*
*  @version     12/11/2008    Initial Version..
*/
// TAG地址：N10:31, N10:32......
// 事务号大约2分钟内不会重复
PKDRIVER_EXPORTS long OnTimer(PKDEVICE *pDevice, PKTIMER *timerInfo)
{
	// 组织读取消息
	DRVGROUP *pTagGroup = (DRVGROUP *)timerInfo->pUserData[0];
	for (int iTag = 0; iTag < pDevice->nTagNum; iTag++)
	{
		PKTAG *pTag = pDevice->ppTags[iTag];
		bool bTagValid = IsTagAddrssValid(pDevice, pTag);
		if (!bTagValid)
			continue;

		// 事务号
		pDevice->nUserData[DEVPARAM_TRANSID] ++;
		if (pDevice->nUserData[DEVPARAM_TRANSID] >= 65536)
			pDevice->nUserData[DEVPARAM_TRANSID] = 0;
		int nTransNo = pDevice->nUserData[DEVPARAM_TRANSID];

		char szReadMsg[512];
		int nReadMsgLen = BuildReadMsg(pDevice, szReadMsg, nTransNo, pTag->szAddress);
		long nSentBytes = Drv_Send(pDevice, szReadMsg, nReadMsgLen, 300);
		if (nSentBytes != nReadMsgLen)
		{
			Drv_LogMessage(PK_LOGLEVEL_ERROR, "device:%s,NO %d/%d, tag:%s, send read message failed, Need to Send:%d, Sent:%d", pDevice->szName, iTag, pDevice->nTagNum, pTag->szName, nReadMsgLen, nSentBytes);
			continue;
		}
		else
		{
			Drv_LogMessage(PK_LOGLEVEL_INFO, "device:%s,NO %d/%d, tag:%s, send read message success, Sent:%d", pDevice->szName, iTag, pDevice->nTagNum, pTag->szName, nSentBytes);
		}

		char szRecvBuff[1024] = { 0 };
		ACE_Time_Value tvBegin = ACE_OS::gettimeofday();

		// 接收的包中需要包含10 02 .... 10 03 BCC,并且事务号等于nTrans
		int nRecvBytes = 0;
		bool bFoundTagDataPack = false; // 是否已经有该点的数据包
		while (true && !bFoundTagDataPack)
		{
			ACE_Time_Value tvNow = ACE_OS::gettimeofday();
			ACE_Time_Value tvSpan = tvNow - tvBegin; // 已经过得时间
			int nTimeoutMS = 8000 - tvSpan.msec(); // 每次最多等待3秒. PLC-5通过网络转之后，2秒都收不到数据
			if (nTimeoutMS < 0 || nTimeoutMS > 100 * 1000) // 超时用尽还没有满足条件则跳过, 或者修改了系统时间
				break;

			if (nRecvBytes >= 1024)
				break;

			int nRecvOnce = Drv_Recv(pDevice, (char*)szRecvBuff, 1024 - nRecvBytes, nTimeoutMS);
			if (nRecvOnce <= 0)
				continue;

			Drv_LogMessage(PK_LOGLEVEL_NOTICE, "device:[%s] transId %d recv the requst(part data) len: %d", pDevice->szName, iTag, nRecvOnce);

			nRecvBytes += nRecvOnce;
			char *pCurBuff = (char *)szRecvBuff;
			while (pCurBuff < szRecvBuff + nRecvBytes) // 找到所有的数据包
			{
				char szHopePackBuf[1024];
				int nHopePackLen = 0;
				char *pNextBuff = NULL;
				int nRet = FindPackage(pDevice, pCurBuff, nRecvBytes, 0x4F, nTransNo, szHopePackBuf, nHopePackLen, pNextBuff);
				if (nRet == 0) // 找到想要的包了！10 02 00 01 4f 00 00 01 18 00 10 03 97
				{
					char *szData = szHopePackBuf + 8; // 18 00 是数据，2
					Drv_SetTagData_Binary(pTag, szData, 2); // tag点必须配置为short或者unsigned short类型
					unsigned short nData;
					memcpy(&nData, szData, 2);
					int nRet = Drv_UpdateTagsData(pDevice, pDevice->ppTags, pDevice->nTagNum);
					if (nRet == 0)
						Drv_LogMessage(PK_LOGLEVEL_INFO, "!!!device:[%s],tag:%s[No.%d of %d], recv a data response package, packlen:%d, data:%02x %02x, to int16:%d", 
						pDevice->szName, pTag->szName, iTag, pDevice->nTagNum, nHopePackLen, *szData, *(szData + 1), nData);
					else
						Drv_LogMessage(PK_LOGLEVEL_ERROR, "!!!device:[%s],tag:%s[No.%d of %d], recv a data response package, packlen:%d, data:%02x %02x, to int16:%d, but Drv_UpdateTagsData return:%d",
						pDevice->szName, pTag->szName, iTag, pDevice->nTagNum, nHopePackLen, *szData, *(szData + 1), nData, nRet);

					// 更新数据
					bFoundTagDataPack = true;
					break;
				}

				pCurBuff = pNextBuff;
			} // while (pCurBuff < szRecvBuff + nRecvBytes) // 找到所有的数据包
		} // while (true)
	} // for (int iTag = 0; iTag < pDevice->nTagNum; iTag++)
	return 0;
}

/**
*  当有控制命令时该函数被调用.
*  @param  -[in]  szStrValue，变量值，除blob外都已经已经转换为字符串。blob转换为base64编码
*
*  @version     12/11/2008    Initial Version.
*/
PKDRIVER_EXPORTS long OnControl(PKDEVICE *pDevice, PKTAG *pTag, const char *szStrValue, long lCmdId)
{
	return 0;
}

int AnalyzeTagAddr_Continuous(char *szAddressInDevice, int nLenBits, char *szBlockName, int nBlockNameLen, int *pnStartBits, int *pnEndBits)
{
	return 0;
}