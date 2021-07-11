
#include "pkdriver/pkdrvcmn.h"
#include "S7Device.h"
#include <vector>
using namespace std;

#include "nodave.h"
#include "log2.h"
#include "memory.h"


int DECL2 _daveNegPDUlengthRequest(daveConnection * dc, PDU *p);

int DECL2 _daveSendISOPacket(daveConnection * dc, int size) {
	//printf("511,size:%d, dc:%d\n", size, (int)dc);
	int res;
	size += 4;
	*(dc->msgOut + dc->partPos + 3) = size % 0x100;	//was %0xFF, certainly a bug
	*(dc->msgOut + dc->partPos + 2) = size / 0x100;
	*(dc->msgOut + dc->partPos + 1) = 0;
	*(dc->msgOut + dc->partPos + 0) = 3;
	if (daveDebug & daveDebugByte)
		_daveDump("send packet: ", dc->msgOut + dc->partPos, size);

	//printf("512\n", size, (int)dc);
	PKDEVICE *pDevice = (PKDEVICE *)(dc->iface->fd.wfd);
	//printf("513,dc->partPos:%d\n", dc->partPos);
	char *pSendBuff = (char *)dc->msgOut + dc->partPos;
	//printf("5133,device:%s, sendBuffPointer:%d, size:%d\n", pDevice->szName, (int)pSendBuff, size);
	//printf("go...\n");
	
	long lSentBytes = 0;
	try
	{
		lSentBytes = Drv_Send(pDevice, (char *)dc->msgOut + dc->partPos, size, 100);
	}
	catch (...)
	{
		printf("Drv_Send except,device:%s, sendBuffPointer:%d, size:%d\n", pDevice->szName, (int)pSendBuff, size);
	}
	//printf("514\n", size, (int)dc);
	return lSentBytes;
}

/*
	Protocol specific functions for ISO over TCP:
	*/
int DECL2 _daveReadOne(daveInterface * di, uc *b) {
	PKDEVICE *pDevice = (PKDEVICE *)(di->fd.rfd);
	long lRecvBytes = Drv_Recv(pDevice, (char *)b, 1, 100);
	return lRecvBytes;
}

/*
	Universal receive with timeout:
	*/
int DECL2 _daveTimedRecv(daveInterface * di, uc *b, int len){
	PKDEVICE * pDevice = (PKDEVICE *)(di->fd.rfd);
	long lRecvBytes = Drv_Recv(pDevice, (char *)b, len, di->timeout / 1000);
	return lRecvBytes;
}



/*
	Read one complete packet.
	*/
int DECL2 _daveReadISOPacket(daveInterface * di, uc *b) {
	int res, i, length, follow;
	uc lhdr[7];
	//printf("411\n");
	i = _daveTimedRecv(di, b, 4); // 先读取4个字节的包头，包括长度，如03 00 00 16，表示包总数据长度为0x16个字节
	//printf("412\n");
	if (i < 0) 
		return 0;

	//printf("413\n");
	res = i;
	if (res < 4) {
		if (daveDebug & daveDebugByte) {
			LOG2("res %d ", res);
			//printf("414\n");
			_daveDump("readISOpacket: short packet", b, res);
		}
		return (0); /* short packet */
	}

	//printf("415\n");
	length = b[3] + 0x100 * b[2];
	i = _daveTimedRecv(di, b + 4, length - 4); // 继续读取后面的包的数据
	//printf("416\n");

	res += i;
	if (daveDebug & daveDebugByte) {
		LOG3("readISOpacket: %d bytes read, %d needed\n", res, length);
		_daveDump("readISOpacket: packet", b, res);
	}
	follow = ((b[5] == 0xf0) && ((b[6] & 0x80) == 0));

	while (follow) {
		if (daveDebug & daveDebugByte) {
			LOG2("readISOpacket: more data follows %d\n", b[6]);
		}

		//printf("418\n");
		i = _daveTimedRecv(di, lhdr, 7);
		length = lhdr[3] + 0x100 * lhdr[2];
		if (daveDebug & daveDebugByte) {
			_daveDump("readISOpacket: follow %d %d", lhdr, i);
		}
		//printf("419\n");
		i = _daveTimedRecv(di, b + res, length - 7);
		if (daveDebug & daveDebugByte) {
			//printf("420\n");
			_daveDump("readISOpacket: follow %d %d", b + res, i);
		}
		res += i;
		follow = ((lhdr[5] == 0xf0) && ((lhdr[6] & 0x80) == 0));
	}
	return (res);
}


int DECL2 _daveConnectPLCTCP(daveConnection * dc) {
	int res, success, retries, i, px;
	uc b4[] = {
		0x11, 0xE0, 0x00,
		0x00, 0x00, 0x01, 0x00,
		0xC1, 2, 1, 0,
		0xC2, 2,
		dc->communicationType,
		(dc->slot | dc->rack << 5), // hope I got it right this time...
		0xC0, 1, 0x9,
	};

	uc b4R2[] = {			// for routing
		6 + 30 + 30 + 3,		// Length over all without this byte (6 byte fixed data, 30 bytes source TSAP (C1), 30 bytes dest TSAP (C2), 3 bytes TPDU size (C0))

		0xE0,		// TDPU Type CR = Connection Request (see RFC1006/ISO8073)
		0x00, 0x00,	// TPDU Destination Reference (unknown)
		0x00, 0x01,	// TPDU Source-Reference (my own reference, should not be zero)

		0x00,		// TPDU Class 0 and no Option

		0xC1,		// Parameter Source-TSAP
		28,		// Length of this parameter
		1,		// one block of data (???)
		0,		// Length for S7-Subnet-ID
		0,		// Length of PLC-Number
		2,		// Length of Function/Rack/Slot
		0, 0, 0, 0, 0, 0, 0, 0,	// empty Data 
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0,
		1,		// Function (1=PG,2=OP,3=Step7Basic)
		0,		// Rack (Bit 7-5) and Slot (Bit 4-0)

		0xC2,		// Parameter Destination-TSAP
		28,		// Length of this parameter 
		1,		// one block of data (???)

		6,		// Length for S7-Subnet-ID
		1,		// Length of PLC address
		2,		// Length of Function/Rack/Slot

		0x01, 0x52,		// first part of S7-Subnet-ID  (look into the S7Project/Network configuration)
		0x00, 0x00,		// fix always 0000 (reserved for later use ?)
		0x00, 0x13,		// second part of S7-Subnet-ID 
		// (see S7Project/Network configuration)

		0x01,			// PLC address

		0, 0, 0, 0, 0, 0, 0, 0,	// empty 
		0, 0, 0, 0, 0, 0, 0,

		dc->communicationType,		// Function (1=PG,2=OP,3=Step7Basic)
		(dc->slot | dc->rack << 5),	// Rack (Bit 7-5) and Slot (Bit 4-0) hope I got it right this time...

		0xC0,		// Parameter requested TPDU-Size
		1,		// Length of this parameter 
		0x9,		// requested TPDU-Size 8=256 Bytes, 9=512, a=1024 Bytes 
	};

	uc b243[] = {
		0x11, 0xE0, 0x00,
		0x00, 0x00, 0x01, 0x00,
		0xC1, 2, 'M', 'W',
		0xC2, 2, 'M', 'W',
		0xC0, 1, 9,
	};

	PDU p1;
	success = 0;
	retries = 0;
	dc->partPos = 0;

	if (dc->iface->protocol == daveProtoISOTCP243) {
		memcpy(dc->msgOut + 4, b243, sizeof(b243));
	}
	else if (dc->iface->protocol == daveProtoISOTCP) {
		if (!dc->routing) {
			memcpy(dc->msgOut + 4, b4, sizeof(b4));
			dc->msgOut[17] = dc->communicationType;  // (1=PG Communication,2=OP Communication,3=Step7Basic Communication)
			dc->msgOut[18] = dc->slot | dc->rack << 5; // hope I got it right this time...
		}
		else {
			if (daveDebug & daveDebugConnect) {
				_daveDump("routing data 1: ", (char*)&(dc->routingData), 30);
			}
			b4R2[41] = dc->routingData.PLCadrsize;

			px = 43;
			b4R2[px] = (dc->routingData.subnetID1) / 0x100;
			b4R2[px + 1] = (dc->routingData.subnetID1) % 0x100;
			b4R2[px + 2] = (dc->routingData.subnetID2) / 0x100;
			b4R2[px + 3] = (dc->routingData.subnetID2) % 0x100;
			b4R2[px + 4] = (dc->routingData.subnetID3) / 0x100;
			b4R2[px + 5] = (dc->routingData.subnetID3) % 0x100;

			memcpy(b4R2 + 49, dc->routingData.PLCadr, dc->routingData.PLCadrsize);

			memcpy(dc->msgOut + 4, b4R2, sizeof(b4R2));	// with routing over MPI

			//	    dc->msgOut[17]=dc->rack+1;			// this is probably wrong
			//	    dc->msgOut[18]=dc->slot;
		}
	}

	_daveSendISOPacket(dc, dc->msgOut[4] + 1);
	do {
		res = _daveReadISOPacket(dc->iface, dc->msgIn);
		if (daveDebug & daveDebugConnect) {
			LOG2("%s daveConnectPLC() step 1. ", dc->iface->name);
			_daveDump("got packet: ", dc->msgIn, res);
		}
		if ((res == 22 && !dc->routing) || (res == 48 && dc->routing) || (res == 74 && dc->routing)) { // 检查数据包长度是否合法
			success = 1;
			for (i = 6; i < res; i++) {
				if (dc->msgIn[i] == 0xc0) {// 最后一个C0个参数
					dc->TPDUsize = 128 << (dc->msgIn[i + 2] - 7); // 512
					if (daveDebug & daveDebugConnect) {
						LOG3("TPDU len %d = %d\n", dc->msgIn[i + 2], dc->TPDUsize);
					}
				}
			}
		}
		else {
			if (daveDebug & daveDebugPrintErrors){
				LOG2("%s error in daveConnectPLC() step 1. retrying...", dc->iface->name);
			}
		}
		retries++;
	} while ((success == 0) && (retries < 3));
	if (success == 0) return -1;

	retries = 0;
	do {
		res = _daveNegPDUlengthRequest(dc, &p1);
		if (res == 0) {
			return res;
		}
		else {
			if (daveDebug & daveDebugPrintErrors){
				LOG2("%s error in daveConnectPLC() step 1. retrying...\n", dc->iface->name);
			}
		}
		retries++;
	} while (retries < 3);
	return -1;
}


#define ISOTCPminPacketLength 16
int DECL2 _daveGetResponseISO_TCP(daveConnection * dc) {
	int res;
	res = _daveReadISOPacket(dc->iface, dc->msgIn);
	if (res == 7) {
		if (daveDebug & daveDebugByte)
			LOG1("CPU sends funny 7 byte packets.\n");
		res = _daveReadISOPacket(dc->iface, dc->msgIn);
	}
	if (res == 0) return daveResTimeout;
	if (res < ISOTCPminPacketLength) return  daveResShortPacket;
	return 0;
}


/*
	Executes the dialog around one message:
	*/
int DECL2 _daveExchangeTCP(daveConnection * dc, PDU * p) {
	PKDEVICE *pDevice = (PKDEVICE *)(dc->iface->fd.rfd);
	CS7Device *pS7Device = (CS7Device *)pDevice->pUserData[NUSERDATA_INDEX_S7DEVICE];
	if (pS7Device->m_nProtocolNo == SIMENS_S7_PROTOCOL_TCP && dc->TPDUsize <= 0)
	{
		Drv_LogMessage(PK_LOGLEVEL_ERROR, "连接时, 在OnDeviceConnStateChanged中必须先获取到PDU大小!!!");
		return -1;
	}

	int res, totLen, sLen;
	//if (dc == NULL || p == NULL)
		//printf("310 ERROR\n");

	//printf("311\n");
	if (daveDebug & daveDebugExchange) {
		LOG2("%s enter _daveExchangeTCP\n", dc->iface->name);
	}

	//    _daveSendISOPacket(dc,3+p->hlen+p->plen+p->dlen);
	int nTryTimes = 3; // by sjp
	dc->partPos = 0;
	totLen = p->hlen + p->plen + p->dlen;
	while (totLen && nTryTimes > 0)
	{
		nTryTimes--;
		//printf("3121\n");

		if (totLen > dc->TPDUsize) { // 1次要发送的包的内容超过了一个PDU大小, 需要分开发送
			sLen = dc->TPDUsize;
			*(dc->msgOut + dc->partPos + 6) = 0x00;
		}
		else
		{
			sLen = totLen;
			*(dc->msgOut + dc->partPos + 6) = 0x80;
		}
		*(dc->msgOut + dc->partPos + 5) = 0xf0;
		*(dc->msgOut + dc->partPos + 4) = 0x02;
		//printf("312\n");
		_daveSendISOPacket(dc, 3 + sLen);
		//printf("313\n");
		totLen -= sLen;
		dc->partPos += sLen;
	}

	if (nTryTimes <= 0)
		return daveResTimeout;

	//printf("314\n");
	res = _daveReadISOPacket(dc->iface, dc->msgIn);
	//printf("315\n");
	if (res == 7) { // 有时会收到7个多余的数据
		if (daveDebug & daveDebugByte)
			LOG1("CPU sends funny 7 byte packets.\n");
		//printf("3111\n");
		res = _daveReadISOPacket(dc->iface, dc->msgIn);
	}
	//printf("316\n");
	if (daveDebug & daveDebugExchange) {
		LOG3("%s _daveExchangeTCP res from read %d\n", dc->iface->name, res);
	}
	if (res == 0) 
		return daveResTimeout;
	if (res <= ISOTCPminPacketLength) 
		return  daveResShortPacket;
	return 0;
}