
#include "pkdriver/pkdrvcmn.h"
#include "S7Device.h"
#include "pkdriver/pkdrvcmn.h"
#include <vector>
using namespace std;

#include "nodave.h"
#include "log2.h"
#include "memory.h"

int DECL2 _daveInitStep(daveInterface * di, int nr, uc *fix, int len, char * caller);


/*
Send a string of init data to the MPI adapter.
*/
int DECL2 _daveInitStep(daveInterface * di, int nr, uc *fix, int len, char * caller) {
	_daveSendSingle(di, STX);
	if (_daveReadSingle(di) != DLE){
		if (daveDebug & daveDebugInitAdapter)
			LOG3("%s %s no answer (DLE) from adapter.\n", di->name, caller);
		if (_daveReadSingle(di) != DLE){
			if (daveDebug & daveDebugInitAdapter)
				LOG3("%s %s no answer (DLE) from adapter.\n", di->name, caller);
			return nr;
		}
	}
	if (daveDebug & daveDebugInitAdapter)
		LOG4("%s %s step %d.\n", di->name, caller, nr);
	_daveSendWithCRC(di, fix, len);
	if (_daveReadSingle(di) != DLE) return nr + 1;
	if (daveDebug & daveDebugInitAdapter)
		LOG4("%s %s step %d.\n", di->name, caller, nr + 1);
	if (_daveReadSingle(di) != STX) return nr + 2;
	if (daveDebug & daveDebugInitAdapter)
		LOG4("%s %s step %d.\n", di->name, caller, nr + 2);
	_daveSendSingle(di, DLE);
	return 0;
}

/*
Initializes the MPI adapter.
*/
int DECL2 _daveInitAdapterMPI1(daveInterface * di) {
	uc b2[] = {
		0x01, 0x0D, 0x02,
	};
	//  us answ1[]={0x01,0x0D,0x20,'V','0','0','.','8','3'};
	//  us adapter0330[]={0x01,0x03,0x20,'E','=','0','3','3','0'};
	//  us answ2[]={0x01,0x03,0x20,'V','0','0','.','8','3'};
	us answ1[] = { 0x01, 0x10D, 0x20, 'V', '0', '0', '.', 0x138, 0x133 };
	us adapter0330[] = { 0x01, 0x03, 0x20, 'E', '=', '0', '3', 0x133, 0x130 };


	uc b3[] = {
		0x01, 0x03, 0x02, 0x27, 0x00, 0x9F, 0x01, 0x3C,
		0x00, 0x90, 0x01, 0x14, 0x00,
		0x00, 0x05,
		0x02,
		0x00, 0x1F, 0x02, 0x01, 0x01, 0x03, 0x80,
		//	^localMPI
	};
	uc v1[] = {
		0x01, 0x0C, 0x02,
	};
	int res;
	uc b1[daveMaxRawLen];
	if (daveDebug & daveDebugInitAdapter)
		LOG2("%s enter initAdapter(1).\n", di->name);

	res = _daveInitStep(di, 1, b2, sizeof(b2), "initAdapter()");
	if (res) {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() fails.\n", di->name);
		return -44;
	}

	res = _daveReadMPI(di, b1);
	_daveSendSingle(di, DLE);

	if (_daveMemcmp(answ1, b1, sizeof(answ1) / 2)) return 4;

	b3[16] = di->localMPI;

	if (di->speed == daveSpeed500k)
		b3[7] = 0x64;
	if (di->speed == daveSpeed1500k)
		b3[7] = 0x96;
	b3[15] = di->speed;
	res = _daveInitStep(di, 4, b3, sizeof(b3), "initAdapter()");
	if (res) {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() fails.\n", di->name);
		return -54;
	}
	/*
	The following extra lines seem to be necessary for
	TS adapter 6ES7 972-0CA33-0XAC:
	*/
	res = _daveReadMPI(di, b1);
	_daveSendSingle(di, DLE);
	if (!_daveMemcmp(adapter0330, b1, sizeof(adapter0330) / 2)) {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() found Adapter E=0330.\n", di->name);
		_daveSendSingle(di, STX);
		res = _daveReadMPI2(di, b1);
		_daveSendWithCRC(di, v1, sizeof(v1));
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() Adapter E=0330 step 7.\n", di->name);
		if (_daveReadSingle(di) != DLE) return 8;
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() Adapter E=0330 step 8.\n", di->name);
		res = _daveReadMPI(di, b1);
		if (res != 1 || b1[0] != STX) return 9;
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() Adapter E=0330 step 9.\n", di->name);
		_daveSendSingle(di, DLE);
		/* This needed the exact Adapter version:    */
		/* instead, just read and waste it */
		res = _daveReadMPI(di, b1);
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() Adapter E=0330 step 10.\n", di->name);
		_daveSendSingle(di, DLE);
		return 0;

	}
	else if (!_daveMemcmp(answ1, b1, sizeof(answ1) / 2)) {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() success.\n", di->name);
		di->users = 0;	/* there cannot be any connections now */
		return 0;
	}
	else {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() failed.\n", di->name);
		return -56;
	}
}


/*
Open connection to a PLC. This assumes that dc is initialized by
daveNewConnection and is not yet used.
(or reused for the same PLC ?)
*/
int DECL2 _daveConnectPLCMPI1(daveConnection * dc) {
	int res, len;
	PDU p1;
	uc * pcha;

	uc b4[] = {
		0x04, 0x80, 0x80, 0x0D, 0x00, 0x14,
		0xE0, 0x04, 0x00, 0x80, 0x00, 0x02,
		0x00,
		0x02,
		0x01, 0x00,
		0x01, 0x00,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	//7E 11 1F E0 04 82 00 0D 00 14   
	//E0 04 00 80 00 02   
	//01 0F  
	//01 00  
	//06 04 02  AA BB 00 00 CC DD  C0 A8 02 BC  01 03 18 87 7E 	
	//Step7//7E 00 1F E0 04 86 00 0D 00 14   E0 04 00 80 00 02   01 0F  01 00  06 04 02  AA BB 00 00 CC DD  C0 A8 02 BC  01 03 8C 60 7E 	

	us t4[] = {
		0x04, 0x80, 0x180, 0x0C, 0x114, 0x103, 0xD0, 0x04,	// 1/10/05 trying Andrew's patch
		0x00, 0x80,
		0x00, 0x02, 0x00, 0x02, 0x01,
		0x00, 0x01, 0x00,
	};

	uc b5[] = {
		0x05, 0x01,
	};
	us t5[] = {
		0x04,
		0x80,
		0x180, 0x0C, 0x114, 0x103, 0x05, 0x01,
	};
	b4[1] |= dc->MPIAdr;
	b4[5] = dc->connectionNumber; // 1/10/05 trying Andrew's patch

	//    b4R[1]|=dc->MPIAdr;	
	//    b4R[5]=dc->connectionNumber; // 1/10/05 trying Andrew's patch

	t4[1] |= dc->MPIAdr;
	t5[1] |= dc->MPIAdr;

	pcha = b4 + 16;
	len = 18;
	if (dc->routing) {
		b4[12] = 1;
		b4[13] = 11 + dc->routingData.PLCadrsize;
		*pcha = 6; pcha++;
		*pcha = dc->routingData.PLCadrsize; pcha++;
		*pcha = 2; pcha++;
		*pcha = (dc->routingData.subnetID1) / 0x100; pcha++;
		*pcha = (dc->routingData.subnetID1) % 0x100; pcha++;
		*pcha = (dc->routingData.subnetID2) / 0x100; pcha++;
		*pcha = (dc->routingData.subnetID2) % 0x100; pcha++;
		*pcha = (dc->routingData.subnetID3) / 0x100; pcha++;
		*pcha = (dc->routingData.subnetID3) % 0x100; pcha++;
		memcpy(pcha, dc->routingData.PLCadr, dc->routingData.PLCadrsize);
		pcha += dc->routingData.PLCadrsize;
		*pcha = 1; pcha++;
		*pcha = 0; pcha++;
		len = 27 + dc->routingData.PLCadrsize;
	}
	_daveInitStep(dc->iface, 1, b4, len, "connectPLC(1)");


	res = _daveReadMPI2(dc->iface, dc->msgIn);

	if (_daveMemcmp(t4, dc->msgIn, 10)) return 3;

	dc->connectionNumber2 = dc->msgIn[5]; // 1/10/05 trying Andrew's patch
	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC(1) step 4.\n", dc->iface->name);

	if (_daveReadSingle(dc->iface) != DLE) return 4;
	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 5.\n", dc->iface->name);
	_daveSendWithPrefix(dc, b5, sizeof(b5));
	if (_daveReadSingle(dc->iface) != DLE) return 5;
	if (_daveReadSingle(dc->iface) != STX) return 5;

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);
	_daveSendSingle(dc->iface, DLE);
	res = _daveReadMPI2(dc->iface, dc->msgIn);
	if (_daveMemcmp(t5, dc->msgIn, sizeof(t5) / 2)) return 6;
	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);
	res = _daveNegPDUlengthRequest(dc, &p1);
	return 0;
}


int DECL2 _daveDisconnectPLCMPI(daveConnection * dc)
{
	int res;
	uc m[] = {
		0x80
	};
	uc b1[daveMaxRawLen];

	_daveSendSingle(dc->iface, STX);

	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != DLE)) {
		if (daveDebug & daveDebugPrintErrors)
			LOG2("%s *** no DLE before send.\n", dc->iface->name);
		return -1;
	}
	_daveSendWithPrefix(dc, m, 1);

	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != DLE)) {
		if (daveDebug & daveDebugPrintErrors)
			LOG2("%s *** no DLE after send.\n", dc->iface->name);
		return -2;
	}

	_daveSendSingle(dc->iface, DLE);

	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != STX)) return 6;
	if (daveDebug & daveDebugConnect)
		LOG2("%s daveDisConnectPLC() step 6.\n", dc->iface->name);
	res = _daveReadMPI(dc->iface, b1);
	if (daveDebug & daveDebugConnect)
		_daveDump("got", b1, 10);
	_daveSendSingle(dc->iface, DLE);
	return 0;
}


/*
It seems to be better to complete this subroutine even if answers
from adapter are not as expected.
*/
int DECL2 _daveDisconnectAdapterMPI(daveInterface * di) {
	int res;
	uc m2[] = {
		1, 4, 2
	};

	uc b1[daveMaxRawLen];
	if (daveDebug & daveDebugInitAdapter)
		LOG2("%s enter DisconnectAdapter()\n", di->name);
	_daveSendSingle(di, STX);
	res = _daveReadMPI(di, b1);
	/*    if ((res!=1)||(b1[0]!=DLE)) return -1; */
	_daveSendWithCRC(di, m2, sizeof(m2));
	if (daveDebug & daveDebugInitAdapter)
		LOG2("%s daveDisconnectAdapter() step 1.\n", di->name);
	res = _daveReadMPI(di, b1);
	/*    if ((res!=1)||(b1[0]!=DLE)) return -2; */
	res = _daveReadMPI(di, b1);
	/*    if ((res!=1)||(b1[0]!=STX)) return -3; */
	if (daveDebug & daveDebugInitAdapter)
		LOG2("%s daveDisconnectAdapter() step 2.\n", di->name);
	_daveSendSingle(di, DLE);
	di->ifread(di, (char *)b1, daveMaxRawLen);
	//    _daveReadChars(di, b1, tmo_normal, daveMaxRawLen);
	_daveSendSingle(di, DLE);
	if (daveDebug & daveDebugInitAdapter)
		_daveDump("got", b1, 10);
	return 0;
}

/*
This doesn't work yet. I'm not sure whether it is possible to get that
list after having connected to a PLC.
*/
int DECL2 _daveListReachablePartnersMPI(daveInterface * di, char * buf) {
	uc b1[daveMaxRawLen];
	uc m1[] = { 1, 7, 2 };
	int res;
	res = _daveInitStep(di, 1, m1, sizeof(m1), "listReachablePartners()");
	if (res) return 0;
	res = _daveReadMPI(di, b1);
	//    LOG2("res %d\n", res);	
	if (136 == res){
		_daveSendSingle(di, DLE);
		memcpy(buf, b1 + 6, 126);
		return 126;
	}
	else
		return 0;
}


int DECL2 _daveExchangeMPI(daveConnection * dc, PDU * p) {
	_daveSendMessageMPI(dc, p);
	dc->AnswLen = 0;
	return _daveGetResponseMPI(dc);
}


/*
Sends a message and gets ackknowledge:
*/
int DECL2 _daveSendMessageMPI(daveConnection * dc, PDU * p) {
	if (daveDebug & daveDebugExchange) {
		LOG2("%s enter _daveSendMessageMPI\n", dc->iface->name);
	}
	if (_daveSendDialog2(dc, 2 + p->hlen + p->plen + p->dlen)) {
		LOG2("%s *** _daveSendMessageMPI error in _daveSendDialog.\n", dc->iface->name);
		//	return -1;	
	}
	if (daveDebug & daveDebugExchange) {
		LOG3("%s _daveSendMessageMPI send done. needAck %x\n", dc->iface->name, dc->needAckNumber);
	}

	if (_daveReadSingle(dc->iface) != STX) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG2("%s *** _daveSendMessageMPI no STX after _daveSendDialog.\n", dc->iface->name);
		}
		if (_daveReadSingle(dc->iface) != STX) {
			if (daveDebug & daveDebugPrintErrors) {
				LOG2("%s *** _daveSendMessageMPI no STX after _daveSendDialog.\n", dc->iface->name);
			}
			return -2;
		}
		else {
			if (daveDebug & daveDebugPrintErrors) {
				LOG2("%s *** _daveSendMessageMPI got STX after retry.\n", dc->iface->name);
			}
		}
	}
	_daveSendSingle(dc->iface, DLE);
	_daveGetAck(dc);
	_daveSendSingle(dc->iface, DLE);
	return 0;
}



int DECL2 _daveGetResponseMPI(daveConnection *dc) {
	int res;
	res = _daveReadSingle(dc->iface);
	if (res != STX) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG2("%s *** _daveGetResponseMPI no STX before answer data.\n", dc->iface->name);
		}
		res = _daveReadSingle(dc->iface);
	}
	_daveSendSingle(dc->iface, DLE);
	if (daveDebug & daveDebugExchange) {
		LOG2("%s _daveGetResponseMPI receive message.\n", dc->iface->name);
	}
	res = _daveReadMPI2(dc->iface, dc->msgIn);
	/*	LOG3("%s *** _daveExchange read result %d.\n", dc->iface->name, res); */
	if (res <= 0) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG2("%s *** _daveGetResponseMPI no answer data.\n", dc->iface->name);
		}
		return -3;
	}
	/*	This is NONSENSE!
	if (daveDebug & daveDebugExchange) {
	LOG3("%s _daveGetResponseMPI got %d bytes\n", dc->iface->name, dc->AnswLen);
	}
	*/
	if (_daveReadSingle(dc->iface) != DLE) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG2("%s *** _daveGetResponseMPI: no DLE.\n", dc->iface->name);
		}
		return -5;
	}
	_daveSendAck(dc, dc->msgIn[dc->iface->ackPos + 1]);
	if (_daveReadSingle(dc->iface) != DLE) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG2("%s *** _daveGetResponseMPI: no DLE after Ack.\n", dc->iface->name);
		}
		return -6;
	}
	return 0;
}
