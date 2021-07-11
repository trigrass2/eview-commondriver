
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
	This initializes the MPI adapter. Andrew's version.
	*/
int DECL2 _daveInitAdapterMPI2(daveInterface * di)  /* serial interface */
{
	uc b3[] = {
		0x01, 0x03, 0x02, 0x17, 0x00, 0x9F, 0x01, 0x3C,
		0x00, 0x90, 0x01, 0x14, 0x00,	/* ^^^ MaxTsdr */
		0x00, 0x5,
		0x02,/* Bus speed */

		0x00, 0x0F, 0x05, 0x01, 0x01, 0x03, 0x80,/* from topserverdemo */
		/*^^ - Local mpi */
	};

	int res;
	uc b1[daveMaxRawLen];
	b3[16] = di->localMPI;
	if (di->speed == daveSpeed500k)
		b3[7] = 0x64;
	if (di->speed == daveSpeed1500k)
		b3[7] = 0x96;
	b3[15] = di->speed;

	res = _daveInitStep(di, 1, b3, sizeof(b3), "initAdapter()");

	res = _daveReadMPI(di, b1);
	if (daveDebug & daveDebugInitAdapter)
		LOG2("%s initAdapter() success.\n", di->name);
	_daveSendSingle(di, DLE);
	di->users = 0;	/* there cannot be any connections now */
	return 0;
}
/*
	Open connection to a PLC. This assumes that dc is initialized by
	daveNewConnection and is not yet used.
	(or reused for the same PLC ?)
	*/
int DECL2 _daveConnectPLCMPI2(daveConnection * dc) {
	int res;
	PDU p1;
	uc b1[daveMaxRawLen];

	uc b4[] = {
		0x00, 0x0d, 0x00, 0x03, 0xe0, 0x04, 0x00, 0x80,
		0x00, 0x02, 0x01, 0x06,
		0x01,
		0x00,
		0x00, 0x01,
		0x02, 0x03, 0x01, 0x00
		/*^^ MPI ADDR */
	};

	us t4[] = {
		0x00, 0x0c, 0x103, 0x103, 0xd0, 0x04, 0x00, 0x80,
		0x01, 0x06,
		0x00, 0x02, 0x00, 0x01, 0x02,
		0x03, 0x01, 0x00,
		0x01, 0x00, 0x10, 0x03, 0x4d
	};
	uc b5[] = {
		0x05, 0x01,
	};

	us t5[] = {
		0x00,
		0x0c,
		0x103, 0x103, 0x05, 0x01, 0x10, 0x03, 0x1b
	};

	b4[3] = dc->connectionNumber; // 1/10/05 trying Andrew's patch
	b4[sizeof(b4) - 3] = dc->MPIAdr;
	t4[15] = dc->MPIAdr;
	t4[sizeof(t4) / 2 - 1] ^= dc->MPIAdr; /* 'patch' the checksum	*/

	_daveInitStep(dc->iface, 1, b4, sizeof(b4), "connectPLC(2)");
	res = _daveReadMPI2(dc->iface, b1);
	if (_daveMemcmp(t4, b1, res)) {
		LOG2("%s daveConnectPLC() step 3 ends with 3.\n", dc->iface->name);
		return 3;
	}
	dc->connectionNumber2 = b1[3]; // 1/10/05 trying Andrew's patch

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 4.\n", dc->iface->name);
	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != DLE)) {
		LOG2("%s daveConnectPLC() step 4 ends with 4.\n", dc->iface->name);
		return 4;
	}

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 5.\n", dc->iface->name);
	_daveSendWithPrefix(dc, b5, sizeof(b5));
	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != DLE)) return 5;
	res = _daveReadMPI(dc->iface, b1);
	if ((res != 1) || (b1[0] != STX)) return 5;

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);
	_daveSendSingle(dc->iface, DLE);

	res = _daveReadMPI(dc->iface, b1);
	_daveSendSingle(dc->iface, DLE);
	if (dc->iface->protocol == daveProtoMPI4) _daveSendSingle(dc->iface, STX);
	if (_daveMemcmp(t5, b1, res)) return 6;

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);
	res = _daveNegPDUlengthRequest(dc, &p1);
	return 0;
}


int DECL2 _daveReadMPI2(daveInterface * di, uc *b) {
	uc b6;
	uc b2[daveMaxRawLen];
	uc fix[] = { 04, 0x80, 0x80, 0x0C, 0x03, 0x14, 5, 1, 0 };
	int res2, re;
	int res = _daveReadMPI(di, b);
	re = res;
	b6 = b[6];
again:
	if ((re >= 7) && (b6 == 0xF0)) {
		if ((daveDebug & daveDebugRawRead) != 0)
			LOG1("follow up expected\n");
		//	uc fix[]= {04,0x80,0x80,0x0C,0x03,0x14,0xB0,0,0};
		//	uc fix[]= {04,0x80,0x80,0x0C,0x03,0x14,5,1,0};
		/*
			uc m[3];
			m[0]=0xB1;
			m[1]=0x01;
			m[2]=nr;
			*/
		fix[8] = b[7];
		fix[1] = b[1];
		_daveSendSingle(di, DLE);
		_daveSendSingle(di, STX);
		_daveReadSingle(di);
		_daveSendWithCRC(di, fix, sizeof(fix));
		_daveReadSingle(di);
		_daveReadSingle(di);
		_daveSendSingle(di, STX);
		_daveSendSingle(di, DLE);
		//	_daveReadSingle(di);
		//	_daveReadSingle(di);
		res2 = _daveReadMPI(di, b2);
		b6 = b2[6];
		re = res2;
		memcpy(b + res - 3, b2 + 6, res2 - 9);
		res += res2 - 9;
		b[7]++;		// increase packet number for ack
		goto again;
	}
	if (res > 1) {
		_daveSendSingle(di, DLE);
		_daveSendSingle(di, STX);
	}

	return res;
}
