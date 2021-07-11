// OmronMessage.cpp: implementation of the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "Message.h"
#include "AutoGroup_BlkDev.h"
#include "math.h"
#include <algorithm>
#include "string.h"
#include <cstring>
#include "stdio.h"
#include "stdlib.h"
using namespace std;

#define DEVICE_USERDATA_TRANSID_INDEX		0	// �����
#define	DEFAULT_RESPONSE_MAXLEN			4096

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessage::CMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	m_pDevice = pDevice;
	m_pTagGroup = pTagGroup;
	m_pMsgBuffer = new char[MAX_MESSAGE_BUFFER_LEN];
	m_nMsgBufferLen = 0;
}

CMessage::~CMessage()
{
	delete[] m_pMsgBuffer;
	m_nMsgBufferLen = 0;
}
char CMessage::hextostring(char hex)
{
	char temp[2] = { 0 };
	sprintf(temp, "%x", hex);
	if (temp[0] >= 'a' && temp[0] <= 'f')
	{
		temp[0] = toupper(temp[0]);
	}
	return temp[0];
}
// ���16���ֽڵ�TCPͷ��
int CMessage::BuildTcpHeader(char *szBuffer, int nBufferLen, int nCmdNo, int nMsgBodyLen)
{
	if (nBufferLen < TCP_HEADER_LENGTH)
		return -1;

	int nTotalLen = COMMAND_START_LENGTH + FINS_HEADER_LENGTH + nMsgBodyLen;

	char *szTcpHeader = szBuffer;
	szTcpHeader[0] = 0x46;	//fins
	szTcpHeader[1] = 0x49;
	szTcpHeader[2] = 0x4E;
	szTcpHeader[3] = 0x53;
	szTcpHeader[4] = 0x00;	//length��4bytes
	szTcpHeader[5] = 0x00;
	szTcpHeader[6] = 0x00;
	szTcpHeader[7] = 0x0E;
	szTcpHeader[8] = 0x00;	//command, 4bytes
	szTcpHeader[9] = 0x00;
	szTcpHeader[10] = 0x00;
	szTcpHeader[11] = nCmdNo; // 0x02;
	szTcpHeader[12] = 0x00;	//error code
	szTcpHeader[13] = 0x00;
	szTcpHeader[14] = 0x00;
	szTcpHeader[15] = 0x00;

	//ת��Ϊ16����
	szTcpHeader[6] = nTotalLen / 256;
	szTcpHeader[7] = nTotalLen % 256;

	return 0;
}

// ���16���ֽڵ�TCPͷ��
int CMessage::BuildFinsHeader(char *szBuffer, int nBufferLen)
{
	if (nBufferLen < FINS_HEADER_LENGTH)
		return -1;

	m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX] = m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX] ++;
	m_nTransId = m_pDevice->nUserData[DEVICE_USERDATA_TRANSID_INDEX];
	m_nTransId = m_nTransId % 256;

	char nServerNodeNo = ::atoi(m_pDevice->szParam1); // �豸����1��ŷ���˽ڵ�DNA1
	char nClientNodeNo = ::atoi(m_pDevice->szParam2); // �豸����2��ſͻ��˽ڵ�SNA1

	char *szFinsHeader = szBuffer;
	szFinsHeader[0] = (char)0x80;			//ICF
	szFinsHeader[1] = 0x00;			//RSV
	szFinsHeader[2] = 0x02;			//GCT
	szFinsHeader[3] = 0x00;			//DNA
	szFinsHeader[4] = nServerNodeNo;	//DNA1
	szFinsHeader[5] = 0x00;			//DNA2
	szFinsHeader[6] = 0x00;			//SNA
	szFinsHeader[7] = nClientNodeNo;	//SNA1
	szFinsHeader[8] = 0x00;			//SNA2
	szFinsHeader[9] = (char)m_nTransId;		//SID, ����ţ�������256��һ������

	return 0;
}

// ���8���ֽڵ�Э��ͷ��,�ټ���д�����ݵ�����
int CMessage::BuildFinsBody_WriteMessage(char *szBodyBuffer, int nBufferLen, PKTAG *pTag, char *szBinValue, int nBinValueLen)
{
	if (nBufferLen < MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinValueLen)
		return -1;

	int nStartBits = pTag->nData1 + m_pTagGroup->nStartAddrBit;// ����ڿ��ڵ���ʼ��ַλ����AI/DI)+�����ʼ��ַλ ������
	int nEndBits = pTag->nData2 + m_pTagGroup->nStartAddrBit; // ����ڿ��ڵĽ�����ַλ����AI/DI)+�����ʼ��ַλ ������

	// ��ʼ��ַ��Ҫ���Ƶĵ�ַ��������Ϊ��λ���ǣ�����������
	int nStartRegisterNo = nStartBits / 16; // ��ʼ�Ĵ���, WORD�� ��10bit����1
	char nStartRegisterHighByte = nStartRegisterNo / 256;  // ��Ϊ��λ�ļĴ�����ʼ��ַ�����ֽ�
	char nStartRegisterLowByte = nStartRegisterNo % 256;  // ��Ϊ��λ�ļĴ�����ʼ��ַ�����ֽ�
	char nStartRegisterBitOffset = nStartBits % 16;    // ���ڵ�ƫ�� bit offset

	int nEndRegisterNo = ceil(m_pTagGroup->nEndAddrBit / 16.0f); // ��ʼ�Ĵ���, WORD�� ȡ����ceil�������һ���ֽڶ�ȡ����, ��10bit����1
	int nRegisterNum = nEndRegisterNo - nStartRegisterNo;

	char *szBody = szBodyBuffer;
	szBody[0] = 0x01;
	szBody[1] = 0x02;
	szBody[2] = GetAreaCode();
	szBody[3] = nStartRegisterHighByte;
	szBody[4] = nStartRegisterLowByte;
	szBody[5] = nStartRegisterBitOffset;
	szBody[6] = nRegisterNum / 256;
	szBody[7] = nRegisterNum % 256;
	if (nBinValueLen > 0)
	{
		memcpy(szBody + MESSAGE_CMD_ADDRESS_REGNUM_LEN, szBinValue, nBinValueLen);
	}

	return 0;
}

// ���8���ֽڵ�Э��ͷ��
int CMessage::BuildFinsBody_ReadMessage(char *szBodyBuffer, int nBufferLen)
{
	if (nBufferLen < MESSAGE_CMD_ADDRESS_REGNUM_LEN)
		return -1;

	int nStartRegisterNo = m_pTagGroup->nStartAddrBit / 16; // ��ʼ�Ĵ���, WORD�� ��10bit����1
	char nStartRegisterHighByte = nStartRegisterNo / 256;  // ��Ϊ��λ�ļĴ�����ʼ��ַ�����ֽ�
	char nStartRegisterLowByte = nStartRegisterNo % 256;  // ��Ϊ��λ�ļĴ�����ʼ��ַ�����ֽ�
	char nStartRegisterBitOffset = m_pTagGroup->nStartAddrBit % 16;    // ���ڵ�ƫ�� bit offset

	int nEndRegisterNo = ceil(m_pTagGroup->nEndAddrBit / 16.0f); // ��ʼ�Ĵ���, WORD�� ȡ����ceil�������һ���ֽڶ�ȡ����, ��10bit����1
	int nRegisterNum = nEndRegisterNo - nStartRegisterNo;

	char *szBody = szBodyBuffer;
	szBody[0] = 0x30;
	szBody[1] = 0x31;
	szBody[2] = 0x30;
	szBody[3] = 0x31;

	char area1 = (GetAreaCode() >> 4) &0x0f;
	char area2 = GetAreaCode() & 0x0f;
	szBody[4] = hextostring(area1);
	szBody[5] = hextostring(area2);

	area1 = (nStartRegisterHighByte >> 4) & 0x0f;
	area2 = nStartRegisterHighByte & 0x0f;
	szBody[6] = hextostring(area1);
	szBody[7] = hextostring(area2);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "nStartRegisterNo %d", nStartRegisterNo);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "nStartRegisterBitOffset %d", nStartRegisterBitOffset);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "nEndRegisterNo %d", nEndRegisterNo);
	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "nRegisterNum %d", nRegisterNum);
#if 1
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "num %d", nStartRegisterLowByte&0xff);
	area1 = (nStartRegisterLowByte >> 4) & 0x0f;
	area2 = (nStartRegisterLowByte & 0x0f);
	szBody[8] = hextostring(area1);
	szBody[9] = hextostring(area2);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[8]);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[9]);

	area1 = (nStartRegisterBitOffset >> 4) & 0x0f;
	area2 = nStartRegisterBitOffset & 0x0f;
	szBody[10] = hextostring(area1);
	szBody[11] = hextostring(area2);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[10]);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[11]);

	//szBody[12] = hextostring(nRegisterNum / 256);
	//szBody[13] = hextostring(nRegisterNum % 256);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[12]);
//	Drv_LogMessage(PK_LOGLEVEL_NOTICE, "area %c", szBody[13]);
#endif
#if 0
	szBody[0] = 0x01;
	szBody[1] = 0x01;
	szBody[2] = GetAreaCode();
	szBody[3] = nStartRegisterHighByte;
	szBody[4] = nStartRegisterLowByte;
	szBody[5] = nStartRegisterBitOffset;
	szBody[6] = nRegisterNum / 256;
	szBody[7] = nRegisterNum % 256;
#endif
	return 0;
}

long CMessage::BuildHeartBeatMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup)
{
	//����cmd. ����Ϊ8���ֽ�+4���ֽڣ��ͻ��˺ţ�
	char *pMsgBuf = m_pMsgBuffer;
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_HEARTBEAT, MESSAGE_CMD_ADDRESS_REGNUM_LEN + 4); // 16���ֽ�
	pMsgBuf += TCP_HEADER_LENGTH;

	int nClientNo = ::atoi(pDevice->szParam1);
	memcpy(pMsgBuf, &nClientNo, 4);
	pMsgBuf += 4;

	m_nMsgBufferLen = TCP_HEADER_LENGTH + 4; // ��20���ֽ�
	return PK_SUCCESS;
}

long CMessage::BuildReadMessage(DRVGROUP *pTagGroup)
{
	//����cmd
	char *pMsgBuf = m_pMsgBuffer;
	char *startAddr = pMsgBuf;
	pMsgBuf[0]='@';
	pMsgBuf++;
	int count = 0;
	unsigned int uintNo = 0x3030;
	::memcpy(pMsgBuf,&uintNo,sizeof(unsigned int));
	pMsgBuf+=2;
	count += 2;

	unsigned int fixedHead = 0x4146;
	::memcpy(pMsgBuf,&fixedHead,sizeof(unsigned int));
	pMsgBuf+=2;
	count += 2;

	unsigned char delay = 0x30;
	::memcpy(pMsgBuf, &delay, sizeof(unsigned char));
	pMsgBuf += 1;
	count += 1;

	::memset(pMsgBuf, 0x30, 8);
	pMsgBuf += 8;
	count += 8;

	BuildFinsBody_ReadMessage(pMsgBuf, 12);  // 8���ֽ�
	pMsgBuf += 12;
	count += 12;

	unsigned char channelNum[4] = { 0x30,0x30,0x30,0x30 };
	::memcpy(pMsgBuf, channelNum, sizeof(channelNum));
	pMsgBuf += 4;
	count += 4;


	char fscH = (startAddr[0] >> 4) &0x0f;
	char fscL = startAddr[0] & 0x0f;
	for (int i = 0; i <count; i++)
	{
		fscH = fscH ^ (startAddr[i + 1] >> 4);
		fscL = fscL ^ (startAddr[i + 1] & 0x0f);
	}
	fscH = CMessage::hextostring(fscH);
	fscL = CMessage::hextostring(fscL);
#if 1
	::memcpy(pMsgBuf, &fscH, sizeof(char));
	pMsgBuf++;
	count++;

	::memcpy(pMsgBuf, &fscL, sizeof(char));
	pMsgBuf++;
	count++;


#endif
	char endSym = '*';
	::memcpy(pMsgBuf, &endSym, sizeof(char));
	pMsgBuf++;
	count++;

	endSym = '\r';
	::memcpy(pMsgBuf, &endSym, sizeof(char));
	pMsgBuf++;
	count++;
	m_nMsgBufferLen = count;
	for (int i = 0; i <= count; i++)
	{
		Drv_LogMessage(PK_LOGLEVEL_NOTICE,"%c",startAddr[i]);
	}
#if 0
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_READ, MESSAGE_CMD_ADDRESS_REGNUM_LEN); // 16���ֽ�
	pMsgBuf += TCP_HEADER_LENGTH;

	BuildFinsHeader(pMsgBuf, FINS_HEADER_LENGTH); // 10���ֽ�
	pMsgBuf += FINS_HEADER_LENGTH;

	BuildFinsBody_ReadMessage(pMsgBuf, MESSAGE_CMD_ADDRESS_REGNUM_LEN);  // 8���ֽ�

	m_nMsgBufferLen = TCP_HEADER_LENGTH + FINS_HEADER_LENGTH + MESSAGE_CMD_ADDRESS_REGNUM_LEN; // ��34���ֽ�
#endif
	return PK_SUCCESS;
}

// ��֯д��Ϣ
long CMessage::BuildWriteMessage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, PKTAG *pTag, char *szBinBuf, int nBinBufLen)
{
	char *pMsgBuf = m_pMsgBuffer;
	BuildTcpHeader(pMsgBuf, TCP_HEADER_LENGTH, COMMAND_WRITE, MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinBufLen); // 16���ֽ�
	pMsgBuf += TCP_HEADER_LENGTH;

	BuildFinsHeader(pMsgBuf, FINS_HEADER_LENGTH); // 10���ֽ�
	pMsgBuf += FINS_HEADER_LENGTH;

	BuildFinsBody_WriteMessage(pMsgBuf, MESSAGE_CMD_ADDRESS_REGNUM_LEN, pTag, szBinBuf, nBinBufLen); // 8���ֽ� + Ҫд������ݵĳ���

	m_nMsgBufferLen = TCP_HEADER_LENGTH + FINS_HEADER_LENGTH + MESSAGE_CMD_ADDRESS_REGNUM_LEN + nBinBufLen; // ��34+д���ʵ�ʳ���  �� �ֽ�

	return PK_SUCCESS;
}


//���������
char CMessage::GetAreaCode()
{
	char areaCode = '0';
	string strAreaType = m_pTagGroup->szHWBlockName;
	std::transform(strAreaType.begin(), strAreaType.end(), strAreaType.begin(), ::toupper); // ��Ϊ��д��ĸ

	if (strAreaType.compare(AREA_NAME_CIO_WORD) == 0)
	{
		areaCode = (char)0xB0;
	}
	else if (strAreaType.compare(AREA_NAME_WR_WORD) == 0)
	{
		areaCode = (char)0xB1;
	}
	else if (strAreaType.compare(AREA_NAME_HR_WORD) == 0)
	{
		areaCode = (char)0xB2;
	}
	else if (strAreaType.compare(AREA_NAME_AR_WORD) == 0)
	{
		areaCode = (char)0xB3;
	}
	else if (strAreaType.compare(AREA_NAME_TIMER_PV) == 0 || strAreaType.compare(AREA_NAME_COUNTER_PV) == 0)
	{
		areaCode = (char)0x89;
	}
	else if (strAreaType.compare(AREA_NAME_TIMER_CF) == 0 || strAreaType.compare(AREA_NAME_COUNTER_CF) == 0)
	{
		areaCode = (char)0x09;
	}
	else if (strAreaType.compare(AREA_NAME_DM_WORD) == 0)
	{
		areaCode = (char)0x82;
	}
	else if (strAreaType.compare(AREA_NAME_IR_DWORD) == 0)
	{
		areaCode = (char)0xDC;
	}
	else if (strAreaType.compare(AREA_NAME_DR_WORD) == 0)
	{
		areaCode = (char)0xBC;
	}
	else if (strAreaType.compare(AREA_NAME_EM_CURRENT_WORD) == 0)
	{
		areaCode = (char)0x98;
	}
	else if (strAreaType.compare(AREA_NAME_EM_WORD) == 0)
	{
		areaCode = (char)0xA0;
	}
	else
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "�豸:%s, ŷķ��PLC���������Ͳ���:%s", m_pDevice->szName, strAreaType.c_str());
	}

	return areaCode;
}


// ����һ��������, ���ܰ���1���������������ݰ�
// nRequestTransID, ��д����������,�����ж��ǲ����յ��˸ôε�����Ӧ��
bool CMessage::ParseOnePackage(PKDEVICE *pDevice, DRVGROUP *pTagGroup, char *pOnePackBuf, int nOnePackLen, unsigned short nRequestTransID, time_t tmRequest, bool bReadResponse)
{
	Drv_LogMessage(PK_LOGLEVEL_INFO, "�յ�������Ӧ�����豸��:%s,������%s, �����:%d��", pDevice->szName, pTagGroup->szAutoGroupName, nRequestTransID);

	// �������ݿ�����ݡ�����OnDatablockTimer�Ķ�ʱ���ж�ȡ�����ݺ󣬵��øýӿڸ��������Ա�������ݿ��ȡ
	if (bReadResponse)
		UpdateGroupData(pDevice, pTagGroup, pOnePackBuf + READ_REPONSE_HEADER_LENGTH, nOnePackLen - READ_REPONSE_HEADER_LENGTH, TAG_QUALITY_GOOD);

	return true;
}

// �����Ƿ��յ�Ӧ������
bool CMessage::ProcessRecvData(PKDEVICE *pDevice, DRVGROUP *pTagGroup, int nTransId, bool bReadResponse, time_t tmRequest)
{
	bool bFoundPackWithTransId = false; // �Ƿ��ҵ��˸�����ŵ�Ӧ���
	// ���豸����Ӧ����Ϣ, ÿ���յ�û�����ݿ��Խ����˾����
	int nCurRecvLen = 0;
	char szResponse[DEFAULT_RESPONSE_MAXLEN] = { 0 };
	while (true)
	{
		long lRecvBytes = Drv_Recv(pDevice, szResponse + nCurRecvLen, sizeof(szResponse) - nCurRecvLen, pDevice->nRecvTimeout);
		if (lRecvBytes <= 0)
		{
			break;
		}

		char *pCurBuf = szResponse;
		nCurRecvLen += lRecvBytes;
		bool bGetOne = true;
		while (bGetOne) // ѭ����������������е����еİ�
		{
			char *pOnePackBuf = NULL; // �ҵ���һ�����İ�ͷָ��
			int nOnePackLen = 0; // �ҵ��İ��ĳ���
			bGetOne = GetOnePackage(pCurBuf, nCurRecvLen, pOnePackBuf, nOnePackLen); // ��ͷ����ʼ�ҵ�һ����
			if (!bGetOne) // һ����Ҳû���ҵ�
			{
				break;
			}

			bool bFoundPackThisTime = ParseOnePackage(pDevice, pTagGroup, pOnePackBuf, nOnePackLen, nTransId, tmRequest, bReadResponse); // һ����һ���������н�������
			if (bFoundPackThisTime)
				bFoundPackWithTransId = true;
		}

		if (bFoundPackWithTransId) // ����Ѿ��ҵ��������Ӧ����ô�Ͳ���Ҫ��������������
			break;
	}

	return bFoundPackWithTransId;
}

// ��ͷ����ʼ�ҵ�һ����, ͬʱָ��ǰ�ƣ�������С
// ����ҵ����򷵻�һ������ָ��ͳ���
bool CMessage::GetOnePackage(char *& pCurBuf, int &lCurRecvLen, char *&pOnePackBuf, int nOnePackLen)
{
	while (true)
	{
		if (lCurRecvLen < TCP_HEADER_LENGTH) // С����С���ȣ�ֱ�ӷ���ʧ��
			return false;

		unsigned short nBodyLen = 0;
		memcpy(&nBodyLen, pCurBuf + 6, sizeof(short));
		int nTotalPackLen = nBodyLen + 8;
		if (pCurBuf[0] == 0x46 && pCurBuf[1] == 0x49 && pCurBuf[2] == 0x4E && pCurBuf[3] == 0x53)
		{
			//if (nTotalPackLen > nOnePackLen) // ������治�����κ�һ�����������ݻ�������
			//	return false; //<?
			if (nTotalPackLen <= nOnePackLen) // ������治�����κ�һ�����������ݻ�������
					return false;

			// ����1�������ķ��Ϲ���İ�
			pOnePackBuf = pCurBuf;
			nOnePackLen = nTotalPackLen;

			pCurBuf += nTotalPackLen;
			lCurRecvLen -= nTotalPackLen;
			return true;
		}
		else // δ�ҵ�һ�������İ�, ��ǰ����1���ֽ�
		{
			pCurBuf += 1;
			lCurRecvLen -= 1;
		}

	}
	return false;
}