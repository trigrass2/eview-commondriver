
#include "pkdriver/pkdrvcmn.h"
#include "S7Device.h"
#include "pkdriver/pkdrvcmn.h"
#include <vector>
using namespace std;

#include "nodave.h"
#include "log2.h"
#include "memory.h"


us ccrc(uc *b, int size) {
	us sum;
	int i, j, m, lll;
	//initialize for crc
	lll = 0xcf87;
	sum = 0x7e;
	for (j = 2; j <= size; j++) {
		for (m = 0; m <= 7; m++) {
			if ((lll & 0x8000) != 0) {
				lll = lll ^ 0x8408;
				lll = lll << 1;
				lll = lll + 1;
			}
			else {
				lll = lll << 1;
			}
		}
		sum = sum^lll;
	}
	for (j = 0; j < size; j++) {
		sum = sum ^ b[j];
		for (i = 0; i <= 7; i++) {
			if (sum & 0x01) {
				sum = sum >> 1;
				sum = sum ^ 0x8408;
			}
			else {
				sum = sum >> 1;
			}
		}
	}
	return sum;
}


int daveSendWithCRC3(daveInterface * di, uc* buffer, int length) {
	uc target[daveMaxRawLen];
	us crc;
	memcpy(target + 4, buffer, length);
	target[0] = 0x7e;
	if (target[10] == 0xB0) {
		target[1] = di->seqNumber + 1;
	}
	else {
		di->seqNumber += 0x11;
		if (di->seqNumber >= 0x88) di->seqNumber = 0;
		target[1] = di->seqNumber;
	}
	target[2] = (length);
	target[3] = 0xff - (length);
	//    crc=ccrc(target,length+4,startTab[length]);
	crc = ccrc(target, length + 4);
	target[4 + length] = crc % 256;
	target[5 + length] = crc / 256;
	target[6 + length] = 0x7e;
	di->ifwrite(di, (char*)target, length + 7);
	return 0;
}

int read1(daveInterface * di, uc* b) {
	int len, res;
	if (daveDebug & daveDebugByte)
		LOG1("enter read1\n");
	len = 0;
again:
	res = di->ifread(di, (char*)b, 5);
	if (res == 5) {
		if (b[4] == 0x7e) goto again;
		if (b[2] == 255 - b[3])  {
			len = b[2] + 7;
			//	    LOG2("need length %d\n",len);
			while (res < len) {
				res += di->ifread(di, (char*)(b + res), len - res);
			}
		}
	}
	//    LOG3("need length %d got %d\n",len,res);
	if (daveDebug & daveDebugByte)
		_daveDump("got", b, res);
	return res;
}


/*
	This initializes the MPI adapter. Step 7 version.
	*/
int DECL2 _daveInitAdapterMPI3(daveInterface * di)
{
	uc b2[] = { 0x7E, 0xFC, 0x9B, 0xCD, 0x7E };
	us adapter0330[] = { 0x01, 0x03, 0x20, 'E', '=', '0', '3', '3', '0' };
	uc v1[] = { 0x01, 0x0C, 0x02 };

	uc b3[] = {
		0x01, 0x03, 0x02, 0x17, 0x00, 0x9F, 0x01, 0x3C,
		0x00, 0x90, 0x01, 0x14, 0x00,	/* ^^^ MaxTsdr */
		0x00, 0x5,
		0x02,/* Bus speed */

		0x00, 0x1F, 0x05, 0x01, 0x01, 0x03, 0x80,/* from topserverdemo */
		/*^^ - Local mpi */
	};
	uc m4[] = { 0x7e, 0xca, 0x2e, 0x99, 0x7e };
	uc b55[] = { 0x01, 0x08, 0x02 };
	uc b1[daveMaxRawLen];

	int res, count;

	b3[16] = di->localMPI;
	if (di->speed == daveSpeed500k)
		b3[7] = 0x64;
	if (di->speed == daveSpeed1500k)
		b3[7] = 0x96;
	b3[15] = di->speed;
	count = 0;
again:
	count++;
	if (count > 20) return -2;
	di->seqNumber = 0x77;
	di->ifwrite(di, (char*)b2, sizeof(b2));
	res = di->ifread(di, (char*)b1, 5);
	if (res == 0) {
		di->ifwrite(di, (char*)b2, sizeof(b2));
		res = di->ifread(di, (char*)b1, 5);
	}
	if (res == 0) {
		di->ifwrite(di, (char*)b2, sizeof(b2));
		res = di->ifread(di, (char*)b1, 5);
	}
	if (daveDebug & daveDebugByte)
		_daveDump("got", b1, res);
	if (res == 5) {
		if (b1[1] == 0xCE) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("ok, I begin sequence\n");
			di->seqNumber = 0x77;
		}
		else if (b1[1] == 0xCA) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("refused.\n");
			goto again;
			//	    res=di->ifread(di, b1, 100);	//certainly nonsense after a jump
		}
		else if (b1[1] == 0xF8) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("refused.\n");
			di->ifwrite(di, (char*)m4, sizeof(m4));
			res = di->ifread(di, (char*)b1, 100);
			goto again;
		}
		else if (b1[1] == 0x8a) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("in sequence. set to 0x11\n");
			di->seqNumber = 0x0;
		}
		else if (b1[1] == 0x8b) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("in sequence. set to 0x22\n");
			di->seqNumber = 0x22;
		}
		else if (b1[1] == 0x8c) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("in sequence. set to 0x33\n");
			di->seqNumber = 0x33;
		}
		else if (b1[1] == 0x8d) {
			if (daveDebug & daveDebugInitAdapter)
				LOG1("in sequence. set to 0x44\n");
			di->seqNumber = 0x44;
		}
	}
	else return -1;
	daveSendWithCRC3(di, b3, sizeof(b3));
	read1(di, b1);
	if (!_daveMemcmp(adapter0330, b1 + 4, sizeof(adapter0330) / 2)) {
		if (daveDebug & daveDebugInitAdapter)
			LOG2("%s initAdapter() found Adapter E=0330.\n", di->name);
		daveSendWithCRC3(di, v1, sizeof(v1));
		read1(di, b1);
		return 0;
	}
	daveSendWithCRC3(di, b55, sizeof(b55));
	read1(di, b1);
	//    daveSendWithCRC3(di,b66,sizeof(b66));
	//    read1(di, b1);
	return 0;
}

int DECL2 _daveSendWithPrefix32(daveConnection * dc, int size) {
	uc fix[] = { 04, 0x80, 0x80, 0x0C, 0x03, 0x14 };
	fix[4] = dc->connectionNumber2;		// 1/10/05 trying Andrew's patch
	fix[5] = dc->connectionNumber;		// 1/10/05 trying Andrew's patch
	memcpy(dc->msgOut, fix, sizeof(fix));
	dc->msgOut[1] |= dc->MPIAdr;
	dc->msgOut[sizeof(fix)] = 0xF1;
	return daveSendWithCRC3(dc->iface, dc->msgOut, size + sizeof(fix));
}

int DECL2 _daveListReachablePartnersMPI3(daveInterface * di, char * buf) {
	uc b1[daveMaxRawLen];
	uc m1[] = { 1, 7, 2 };
	int res, len;
	daveSendWithCRC3(di, m1, sizeof(m1));
	res = read1(di, b1);
	if (daveDebug & daveDebugInitAdapter)
		LOG2("res:%d\n", res);
	if (140 == res){
		memcpy(buf, b1 + 10, 126);
		return 126;
	}
	else
		return 0;
}


/*
	Open connection to a PLC. This assumes that dc is initialized by
	daveNewConnection and is not yet used.
	(or reused for the same PLC ?)
	*/
int DECL2 _daveConnectPLCMPI3(daveConnection * dc) {
	int res, mpi, len;
	PDU p1;
	uc b1[daveMaxRawLen];
	uc * pcha;

	uc e18[] = { 0x04, 0x82, 0x00, 0x0d, 0x00, 0x14,
		0xe0, 0x04, 0x00, 0x80, 0x00, 0x02,
		0x00, 0x02,
		0x01,
		0x00,
		0x01, 0x00,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	//Step7
	//7E 00 1F E0 
	//04 86 00 0D 00 14   
	//E0 04 00 80 00 02   
	//01 
	//0F  
	// 01 00  
	// 06 04 02  sizes
	// AA BB 00 00 CC DD  
	// C0 A8 02 BC  
	// 01 03 8C 60 7E 	

	uc b5[] = {
		0x05, 0x01,
	};

	mpi = dc->MPIAdr;
	e18[1] |= dc->MPIAdr;

	pcha = e18 + 16;
	len = 18;
	if (dc->routing) {
		e18[12] = 1;
		e18[13] = 11 + dc->routingData.PLCadrsize;
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
		len = 27 + dc->routingData.PLCadrsize;
	}
	*pcha = dc->communicationType; pcha++;
	*pcha = 0; pcha++; // rack+slot ?

	daveSendWithCRC3(dc->iface, e18, len);
	read1(dc->iface, b1);

	dc->connectionNumber2 = b1[9];
	dc->connectionNumber = 0x14;

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 3.\n", dc->iface->name);

	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 4.\n", dc->iface->name);

	_daveSendWithPrefix31(dc, b5, sizeof(b5));
	read1(dc->iface, b1);
	if (daveDebug & daveDebugConnect)
		LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);
	res = _daveNegPDUlengthRequest(dc, &p1);
	return 0;
}

/*
	This adds a prefix to a string and theen sends it
	after doubling DLEs in the String
	and adding DLE,ETX and bcc.
	*/
int DECL2 _daveSendWithPrefix31(daveConnection * dc, uc *b, int size)
{
	uc target[daveMaxRawLen];
	uc fix[] = { 04, 0x80, 0x80, 0x0C, 0x03, 0x14 };
	fix[4] = dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
	fix[5] = dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
	memcpy(target, fix, sizeof(fix));
	memcpy(target + sizeof(fix), b, size);
	target[1] |= dc->MPIAdr;
	memcpy(target + sizeof(fix), b, size);
	return daveSendWithCRC3(dc->iface, target, size + sizeof(fix));
}

/*
	Executes part of the dialog necessary to send a message:
	*/
int DECL2 _daveSendDialog3(daveConnection * dc, int size)
{
	if (size > 5){
		dc->needAckNumber = dc->messageNumber;
		dc->msgOut[dc->iface->ackPos - dc->PDUstartI + dc->PDUstartO + 1] = _daveIncMessageNumber(dc);
	}
	_daveSendWithPrefix32(dc, size);
	return 0;
}

int DECL2 _daveSendMessageMPI3(daveConnection * dc, PDU * p) {
	if (daveDebug & daveDebugExchange) {
		LOG2("%s enter _daveSendMessageMPI3\n", dc->iface->name);
	}
	if (_daveSendDialog3(dc, 2 + p->hlen + p->plen + p->dlen)) {
		LOG2("%s *** _daveSendMessageMPI3 error in _daveSendDialog.\n", dc->iface->name);
		//	return -1;	
	}
	if (daveDebug & daveDebugExchange) {
		LOG3("%s _daveSendMessageMPI send done. needAck %x\n", dc->iface->name, dc->needAckNumber);
	}
	return 0;
}

/*
	Sends an ackknowledge message for the message number nr:
	*/
int DECL2 _daveSendAckMPI3(daveConnection * dc, int nr)
{
	uc m[3];
	if (daveDebug & daveDebugPacket)
		LOG3("%s sendAck for message %d \n", dc->iface->name, nr);
	m[0] = 0xB0;
	m[1] = 0x01;
	m[2] = nr;
	return _daveSendWithPrefix31(dc, m, 3);
}

//#define CRC
#ifdef CRC
int testcrc(unsigned char *b, int size, int start)
{
	unsigned short sum, s2;
	int i, j;
	unsigned char *b1 = b;
	sum = start;

	for (j = 0; j < size - 2; j++) {
		//	LOG2("I calc: %x.\n", sum);
		sum = sum ^ (b1[j]);
		//	LOG2("after xor data: %x.\n", sum);
		s2 = sum;
		for (i = 0; i <= 7; i++) {
			if (sum & 0x1) {
				sum = sum >> 1;
				sum = sum ^ 0x8408;
			}
			else
				sum = sum >> 1;
			//	    LOG2("loop: %x.\n", sum);
		}
	}
	/*
		if (
		((sum /256)==b[size-1]) &&
		((sum %256)==b[size-2])
		) {
		printf ("found 1 %04x \n",start);
		return 1;
		}
		*/
	if (
		((sum % 256) == b[size - 2]) &&
		((sum / 256) == b[size - 1])
		){
		printf("found 2 %04x %d\n", start, size - 6);
		startTab[size - 6] = start;
		return 1;
	}
	return 0;
}
#endif

int DECL2 _daveGetResponseMPI3(daveConnection *dc) {
	int res, count;
	if (daveDebug & daveDebugExchange)
		LOG1("enter _daveGetResponseMPI3\n");
	count = 0;
	dc->msgIn[10] = 0;
	do {
		//	res=dc->iface->ifread(dc->iface, dc->msgIn, 400);
		res = read1(dc->iface, dc->msgIn);
		count++;
	} while ((count < 5) && (dc->msgIn[10] != 0xF1));
	if (dc->msgIn[10] == 0xF1) {
		dc->iface->seqNumber = dc->msgIn[1];
		_daveSendAckMPI3(dc, dc->msgIn[dc->iface->ackPos + 1]);
#ifdef CRC	
		if (startTab[res - 7] == 0) {
			for (count = 0; count < 0xffff; count++)
				testcrc(dc->msgIn, res - 1, count);
		}
#endif
		return 0;
	}
	return -10;
}


int DECL2 _daveExchangeMPI3(daveConnection * dc, PDU * p) {
	_daveSendMessageMPI3(dc, p);
	dc->AnswLen = 0;
	return _daveGetResponseMPI3(dc);
}

int DECL2 _daveDisconnectPLCMPI3(daveConnection * dc)
{
	//    uc m[]={
	//        0x80
	//    };
	uc fix[] = { 04, 0x82, 0x0, 0x0C, 0x03, 0x14, 0x80 };
	fix[4] = dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
	fix[5] = dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
	//    _daveSendWithPrefix31(dc, m, 1);	
	fix[1] |= dc->MPIAdr;
	daveSendWithCRC3(dc->iface, fix, sizeof(fix));
	read1(dc->iface, dc->msgIn);
	return 0;
}
/*
	It seems to be better to complete this subroutine even if answers
	from adapter are not as expected.
	*/
int DECL2 _daveDisconnectAdapterMPI3(daveInterface * di) {
	//    uc m3[]={
	//        0x80
	//    };
	uc m2[] = {
		1, 4, 2
	};
	uc b[daveMaxRawLen];
	//    uc m4[]={0x7e,0xca,0x2e,0x99,0x7e};
	//    _daveSendWithPrefix31(di, m3, sizeof(m3));
	//    read1(di,b);
	daveSendWithCRC3(di, m2, sizeof(m2));
	read1(di, b);
#ifdef CRC
	printf("\n\n\n\nus startTab[]={");
	for (res = 0; res < 255; res++) {
		printf("0x%04x , // %d\n", startTab[res], res);
	}
	printf("}\n\n\n\n");
#endif    
	//    di->ifwrite(di, m4, 5);
	return 0;
}
