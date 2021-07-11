
#ifndef _MODBUS_DRIVER_H_
#define _MODBUS_DRIVER_H_

#include "pkdriver/pkdrvcmn.h"
#include "memory.h"
#include <vector>
using namespace std;

#define PROTOCOL_TCP_PACKAGE_HEADER_LEN			6 
#define PROTOCOL_RTU_PACKAGE_HEADER_LEN			3

// 定义软元件名称
#define  BLOCK_TYPE_NAME_X				"X"		// 输入继电器,位软元器件
#define  BLOCK_TYPE_NAME_Y				"Y"		// 输出继电器,位软元器件
#define  BLOCK_TYPE_NAME_M				"M"		// 辅助继电器,位软元器件
#define  BLOCK_TYPE_NAME_S				"S"		// 状态,位软元器件
#define  BLOCK_TYPE_NAME_L				"L"		// 锁存继电器,位软元器件
#define  BLOCK_TYPE_NAME_F				"F"		// 报警器,位软元器件
#define  BLOCK_TYPE_NAME_B				"B"		// 链接继电器,位软元器件
#define  BLOCK_TYPE_NAME_V				"V"		// 边沿继电器,位软元器件
#define  BLOCK_TYPE_NAME_CS				"CS"	// 计数器和高速计数器,触点,位软元器件
#define  BLOCK_TYPE_NAME_CC				"CC"	// 计数器和高速计数器,线圈,位软元器件
#define  BLOCK_TYPE_NAME_TS				"TS"	// 定时器,触点，位软元器件
#define  BLOCK_TYPE_NAME_TC				"TC"	// 定时器,线圈，位软元器件
#define  BLOCK_TYPE_NAME_SS				"SS"	// 累计定时器,触点，位软元器件
#define  BLOCK_TYPE_NAME_SC				"SC"	// 累计定时器,线圈，位软元器件
#define  BLOCK_TYPE_NAME_SB				"SB"	// 链接特殊继电器,线圈，位软元器件
#define  BLOCK_TYPE_NAME_S				"S"		// 步进继电器,线圈，位软元器件
#define  BLOCK_TYPE_NAME_DX				"DX"	// 直接输入继电器,线圈，位软元器件
#define  BLOCK_TYPE_NAME_DY				"DY"	// 直接输出继电器,线圈，位软元器件

#define  BLOCK_TYPE_NAME_TN				"TN"	// 定时器,当前值，字软元器件
#define  BLOCK_TYPE_NAME_SN				"SN"	// 累计寄存器,当前值,字软元器件
#define  BLOCK_TYPE_NAME_CN				"CN"	// 计数器和高速计数器,当前值,字软元器件
#define  BLOCK_TYPE_NAME_D				"D"		// 数据寄存器,字软元器件
#define  BLOCK_TYPE_NAME_Z				"Z"		// 变址寄存器,字软元器件
#define  BLOCK_TYPE_NAME_W				"W"		// 链接寄存器,字软元器件
#define  BLOCK_TYPE_NAME_SW				"SW"	// 链接特殊继电器,线圈，字软元器件

// 定义不同的软元件的代码
#define  BLOCK_TYPE_CODE_UNKNOWN		0x00		// 未知
#define  BLOCK_TYPE_CODE_X				0x9C		// 输入继电器,位软元器件
#define  BLOCK_TYPE_CODE_Y				0x9D		// 输出继电器,位软元器件
#define  BLOCK_TYPE_CODE_M				0x90		// 辅助继电器,位软元器件
#define  BLOCK_TYPE_CODE_S				0x98		// 状态,位软元器件
#define  BLOCK_TYPE_CODE_L				0x92		// 锁存继电器,位软元器件
#define  BLOCK_TYPE_CODE_F				0x93		// 报警寄存器,字软元器件
#define  BLOCK_TYPE_CODE_B				0xA0		// 链接继电器,位软元器件
#define  BLOCK_TYPE_CODE_V				0x94		// 边沿寄存器,位软元器件
#define  BLOCK_TYPE_CODE_TS				0xC1		// 定时器,触点,位软元器件
#define  BLOCK_TYPE_CODE_TC				0xC0		// 定时器,线圈,位软元器件
#define  BLOCK_TYPE_CODE_CS				0xC4		// 计数器,触点,位软元器件
#define  BLOCK_TYPE_CODE_CC				0xC3		// 计数器,线圈,位软元器件
#define  BLOCK_TYPE_CODE_SS				0xC7		// 累计定时器,触点，位软元器件
#define  BLOCK_TYPE_CODE_SC				0xC6		// 累计定时器,线圈，位软元器件
#define  BLOCK_TYPE_CODE_SB				0xA1		// 链接特殊继电器,线圈，位软元器件
#define  BLOCK_TYPE_CODE_S				0x98		// 步进继电器,线圈，位软元器件
#define  BLOCK_TYPE_CODE_DX				0xA2		// 直接输入继电器,线圈，位软元器件
#define  BLOCK_TYPE_CODE_DY				0xA3		// 直接输出继电器,线圈，位软元器件

#define  BLOCK_TYPE_CODE_TN				0xC2		// 定时器,当前值,字软元器件
#define  BLOCK_TYPE_CODE_CN				0xC5		// 计数器,当前值,字软元器件
#define  BLOCK_TYPE_CODE_SN				0xC8		// 累计寄存器,当前值,字软元器件
#define  BLOCK_TYPE_CODE_D				0xA8		// 数据寄存器,字软元器件
#define  BLOCK_TYPE_CODE_Z				0xCC		// 变址寄存器,字软元器件
#define  BLOCK_TYPE_CODE_W				0xB4		// 链接寄存器,字软元器件
#define  BLOCK_TYPE_CODE_SW				0xB5		// 链接特殊继电器,线圈，字软元器件

#define  BLOCK_LENTYPE_ID_UNDEFINED		0		// 未定义
#define  BLOCK_LENTYPE_ID_BIT			1		// 块的类型：位软元件
#define  BLOCK_LENTYPE_ID_WORD			2		// 块的类型：字软元件

// Modbus协议功能码定义
#define COMMAND_READ_BULK			0x0401	// 批量读取，二进制
#define COMMAND_WRITE_BULK			0x1401	// 批量写入，二进制
#define SUBCOMMAND_WORD				0x0000	// 以位为单位读取
#define SUBCOMMAND_BIT				0x0001	// 以字为单位读取

#define	MAX_BULK_RW_WORDS			480		// 最大一次读写的字节数：480个，按位（7680点），按字（480个字）

#define	MAX_REQUEST_PACKDATA_LEN	960		// 最大包的长度即可。每个包长度为960个字节（480*2）+包头尾，按照1024估算
#define	MAX_RESPONSE_PACKDATA_LEN	960
#define	MAX_REQUEST_BUFLEN			2048	// 2个最大包的长度即可。每个包长度为960个字节（480*2）+包头尾，按照1024估算
#define	MAX_RESPONSE_BUFLEN			2048
#define	BITS_PER_BYTE				8
#define	BITS_PER_WORD				16

#define REQ_HEAD_LEN				15		// 到子命令结束
#define PACKET_REQ_HEADFLAG			0x0050	// 请求包头标识字
#define PACKET_RESP_HEADFLAG		0x00D0	// 请求包头标识字
#define PACKET_PLC_NO				0xFF	// PLC编号
#define PACKET_NETWORK_NO			0x00	// 网络编号
#define PACKET_DESTMODULE_IONO		0x03FF  // 请求目标地址IO编号
#define PACKET_DESTMODULE_STATIONNO	0x00  // 请求目标地址站编号

#pragma pack(push)  // 需设置1字节对齐，否则PACKET_HEAD会变成10个字节而不是9个字节
#pragma pack(1)//设定为4字节对齐
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
	
	unsigned char		ucStartL; // 起始地址低字节
	unsigned char		ucStartH;	// 起始地址高字节
	unsigned char		ucStartDummy;	// 起始地址不用的字节
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
	unsigned short		nErrCode;	// 00 表示成功
	char				szReqData[MAX_REQUEST_PACKDATA_LEN]; // 在读取成功时返回数据，读取失败时返回错误信息；在写入成功时无用，写入失败时返回错误信息
	_RESP_PACKET()
	{
		memset(szReqData, 0, sizeof(szReqData));
	}
}RESP_PACKET;

#pragma pack(pop)//恢复对齐状态

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
 *  字节顺序转换.
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


#endif //_MODBUS_DRIVER_H_ 