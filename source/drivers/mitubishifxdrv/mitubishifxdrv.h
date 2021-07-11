
#ifndef _MODBUS_DRIVER_H_
#define _MODBUS_DRIVER_H_

#include "pkdriver/pkdrvcmn.h"
#include "memory.h"
#include <vector>
using namespace std;

#define PROTOCOL_TCP_PACKAGE_HEADER_LEN			6 
#define PROTOCOL_RTU_PACKAGE_HEADER_LEN			3

// ������Ԫ������
#define  BLOCK_TYPE_NAME_X				"X"		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_Y				"Y"		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_M				"M"		// �����̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_S				"S"		// ״̬,λ��Ԫ����
#define  BLOCK_TYPE_NAME_L				"L"		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_F				"F"		// ������,λ��Ԫ����
#define  BLOCK_TYPE_NAME_B				"B"		// ���Ӽ̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_V				"V"		// ���ؼ̵���,λ��Ԫ����
#define  BLOCK_TYPE_NAME_CS				"CS"	// �������͸��ټ�����,����,λ��Ԫ����
#define  BLOCK_TYPE_NAME_CC				"CC"	// �������͸��ټ�����,��Ȧ,λ��Ԫ����
#define  BLOCK_TYPE_NAME_TS				"TS"	// ��ʱ��,���㣬λ��Ԫ����
#define  BLOCK_TYPE_NAME_TC				"TC"	// ��ʱ��,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_NAME_SS				"SS"	// �ۼƶ�ʱ��,���㣬λ��Ԫ����
#define  BLOCK_TYPE_NAME_SC				"SC"	// �ۼƶ�ʱ��,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_NAME_SB				"SB"	// ��������̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_NAME_S				"S"		// �����̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_NAME_DX				"DX"	// ֱ������̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_NAME_DY				"DY"	// ֱ������̵���,��Ȧ��λ��Ԫ����

#define  BLOCK_TYPE_NAME_TN				"TN"	// ��ʱ��,��ǰֵ������Ԫ����
#define  BLOCK_TYPE_NAME_SN				"SN"	// �ۼƼĴ���,��ǰֵ,����Ԫ����
#define  BLOCK_TYPE_NAME_CN				"CN"	// �������͸��ټ�����,��ǰֵ,����Ԫ����
#define  BLOCK_TYPE_NAME_D				"D"		// ���ݼĴ���,����Ԫ����
#define  BLOCK_TYPE_NAME_Z				"Z"		// ��ַ�Ĵ���,����Ԫ����
#define  BLOCK_TYPE_NAME_W				"W"		// ���ӼĴ���,����Ԫ����
#define  BLOCK_TYPE_NAME_SW				"SW"	// ��������̵���,��Ȧ������Ԫ����

// ���岻ͬ����Ԫ���Ĵ���
#define  BLOCK_TYPE_CODE_UNKNOWN		0x00		// δ֪
#define  BLOCK_TYPE_CODE_X				0x9C		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_Y				0x9D		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_M				0x90		// �����̵���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_S				0x98		// ״̬,λ��Ԫ����
#define  BLOCK_TYPE_CODE_L				0x92		// ����̵���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_F				0x93		// �����Ĵ���,����Ԫ����
#define  BLOCK_TYPE_CODE_B				0xA0		// ���Ӽ̵���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_V				0x94		// ���ؼĴ���,λ��Ԫ����
#define  BLOCK_TYPE_CODE_TS				0xC1		// ��ʱ��,����,λ��Ԫ����
#define  BLOCK_TYPE_CODE_TC				0xC0		// ��ʱ��,��Ȧ,λ��Ԫ����
#define  BLOCK_TYPE_CODE_CS				0xC4		// ������,����,λ��Ԫ����
#define  BLOCK_TYPE_CODE_CC				0xC3		// ������,��Ȧ,λ��Ԫ����
#define  BLOCK_TYPE_CODE_SS				0xC7		// �ۼƶ�ʱ��,���㣬λ��Ԫ����
#define  BLOCK_TYPE_CODE_SC				0xC6		// �ۼƶ�ʱ��,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_CODE_SB				0xA1		// ��������̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_CODE_S				0x98		// �����̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_CODE_DX				0xA2		// ֱ������̵���,��Ȧ��λ��Ԫ����
#define  BLOCK_TYPE_CODE_DY				0xA3		// ֱ������̵���,��Ȧ��λ��Ԫ����

#define  BLOCK_TYPE_CODE_TN				0xC2		// ��ʱ��,��ǰֵ,����Ԫ����
#define  BLOCK_TYPE_CODE_CN				0xC5		// ������,��ǰֵ,����Ԫ����
#define  BLOCK_TYPE_CODE_SN				0xC8		// �ۼƼĴ���,��ǰֵ,����Ԫ����
#define  BLOCK_TYPE_CODE_D				0xA8		// ���ݼĴ���,����Ԫ����
#define  BLOCK_TYPE_CODE_Z				0xCC		// ��ַ�Ĵ���,����Ԫ����
#define  BLOCK_TYPE_CODE_W				0xB4		// ���ӼĴ���,����Ԫ����
#define  BLOCK_TYPE_CODE_SW				0xB5		// ��������̵���,��Ȧ������Ԫ����

#define  BLOCK_LENTYPE_ID_UNDEFINED		0		// δ����
#define  BLOCK_LENTYPE_ID_BIT			1		// ������ͣ�λ��Ԫ��
#define  BLOCK_LENTYPE_ID_WORD			2		// ������ͣ�����Ԫ��

// ModbusЭ�鹦���붨��
#define COMMAND_READ_BULK			0x0401	// ������ȡ��������
#define COMMAND_WRITE_BULK			0x1401	// ����д�룬������
#define SUBCOMMAND_WORD				0x0000	// ��λΪ��λ��ȡ
#define SUBCOMMAND_BIT				0x0001	// ����Ϊ��λ��ȡ

#define	MAX_BULK_RW_WORDS			480		// ���һ�ζ�д���ֽ�����480������λ��7680�㣩�����֣�480���֣�

#define	MAX_REQUEST_PACKDATA_LEN	960		// �����ĳ��ȼ��ɡ�ÿ��������Ϊ960���ֽڣ�480*2��+��ͷβ������1024����
#define	MAX_RESPONSE_PACKDATA_LEN	960
#define	MAX_REQUEST_BUFLEN			2048	// 2�������ĳ��ȼ��ɡ�ÿ��������Ϊ960���ֽڣ�480*2��+��ͷβ������1024����
#define	MAX_RESPONSE_BUFLEN			2048
#define	BITS_PER_BYTE				8
#define	BITS_PER_WORD				16

#define REQ_HEAD_LEN				15		// �����������
#define PACKET_REQ_HEADFLAG			0x0050	// �����ͷ��ʶ��
#define PACKET_RESP_HEADFLAG		0x00D0	// �����ͷ��ʶ��
#define PACKET_PLC_NO				0xFF	// PLC���
#define PACKET_NETWORK_NO			0x00	// ������
#define PACKET_DESTMODULE_IONO		0x03FF  // ����Ŀ���ַIO���
#define PACKET_DESTMODULE_STATIONNO	0x00  // ����Ŀ���ַվ���

#pragma pack(push)  // ������1�ֽڶ��룬����PACKET_HEAD����10���ֽڶ�����9���ֽ�
#pragma pack(1)//�趨Ϊ4�ֽڶ���
typedef struct _PACKET_HEAD{
	unsigned short		nStartFlag; // always=0x5000
	unsigned char		nNetworkNo; // always=00 for E71
	unsigned char		nPLCNo;	// always=FF for E71
	unsigned short		nDestModuleIONo; // always=0xFF03
	unsigned char		nDestModuleStationNo;	// always=0x00 for E71
	unsigned short		nBodyDataLen;	// always= for E71
	_PACKET_HEAD()
	{
		nStartFlag=PACKET_REQ_HEADFLAG;
		nNetworkNo = PACKET_NETWORK_NO;
		nPLCNo = PACKET_PLC_NO;
		nDestModuleIONo = PACKET_DESTMODULE_IONO;
		nDestModuleStationNo = PACKET_DESTMODULE_STATIONNO;
		nBodyDataLen = 0;
	}
}PACKET_HEAD;

typedef struct _REQ_PACKET{
	PACKET_HEAD			header;
	unsigned short		nCPUTimer;	// always=0x1000 for E71
	unsigned short		nCommand;	// 
	unsigned short		nSubCommand;//
	
	unsigned char		ucStartL; // ��ʼ��ַ���ֽ�
	unsigned char		ucStartH;	// ��ʼ��ַ���ֽ�
	unsigned char		ucStartDummy;	// ��ʼ��ַ���õ��ֽ�
	unsigned char		ucCompCode;
	unsigned short		uCompNum;

	char				szReqData[MAX_REQUEST_PACKDATA_LEN];
	int					nTotalPackLen;
	_REQ_PACKET()
	{
		nCPUTimer = 0x0010; // 250ms * 0x10
		nCommand = nSubCommand = 0;
		nTotalPackLen = 0;
		ucStartL = ucStartDummy = ucStartH = ucCompCode = 0;
		uCompNum = 0;
	}
}REQ_PACKET;

typedef struct _RESP_PACKET{
	PACKET_HEAD			header;
	unsigned short		nErrCode;	// 00 ��ʾ�ɹ�
	char				szReqData[MAX_REQUEST_PACKDATA_LEN]; // �ڶ�ȡ�ɹ�ʱ�������ݣ���ȡʧ��ʱ���ش�����Ϣ����д��ɹ�ʱ���ã�д��ʧ��ʱ���ش�����Ϣ
	_RESP_PACKET()
	{
		memset(szReqData, 0, sizeof(szReqData));
	}
}RESP_PACKET;

#pragma pack(pop)//�ָ�����״̬

void StartAddr2Packet(REQ_PACKET *pPack, unsigned short uStartAddr)
{
	unsigned char *pTmp = (unsigned char *)&uStartAddr;
	pPack->ucStartL = *pTmp;
	pPack->ucStartDummy = 0;
	pPack->ucStartH = *(pTmp + 1);
}

int Packet2StartAddr(REQ_PACKET *pPack)
{
	unsigned short uStartAddr;
	unsigned char *pTmp = (unsigned char *)&uStartAddr;
	pPack->ucStartL = *pTmp;
	pPack->ucStartH = *(pTmp + 1);
	pPack->ucStartDummy = 0;
	return uStartAddr;
}

/**
 *  �ֽ�˳��ת��.
 *
 *  @param  -[in, out]  char*  pOrig: [comment]
 *  @param  -[in]  int  nLength: [comment]
 *
 *  @version     07/25/2008  lijingjing  Initial Version.
 */
inline long SwapByteOrder(char* pOrig, int nLength, int nWordBytes)
{
	int i = 0;
	int nSwapCount = 0;
	char chTemp;
	
	if (nWordBytes == 2)
	{
		nSwapCount = nLength / 2;
		for(i = 0; i < nSwapCount; i++)
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
		for(i = 0; i < nSwapCount; i++)
		{
			// ��0�͵�3���ֽڽ���
			chTemp = *(pOrig + 3);
			*(pOrig + 3) = *pOrig; 
			*pOrig = chTemp;

			// ��1�͵�2���ֽڽ���
			chTemp = *(pOrig + 2);
			*(pOrig + 2) = *(pOrig + 1);
			*(pOrig + 1) = chTemp;

			pOrig += 4;
		}
		// ����4���ֽڵĲ���
		if (nLength - nSwapCount * 4 == 2)
		{
			// ʣ�������ֽ�
			chTemp = *pOrig;
			*pOrig = *(pOrig + 1);
			*(pOrig + 1) = chTemp;
		}
		else if (nLength - nSwapCount * 4 == 3)
		{
			// ʣ�������ֽ�
			chTemp = *pOrig;
			*pOrig = *(pOrig + 2);
			*(pOrig + 2) = chTemp;
		}
	}
	return PK_SUCCESS;
}


#endif //_MODBUS_DRIVER_H_ 