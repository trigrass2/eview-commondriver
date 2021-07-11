/*
 Part of Libnodave, a free communication libray for Siemens S7 200/300/400 via
 the MPI adapter 6ES7 972-0CA22-0XAC
 or  MPI adapter 6ES7 972-0CA23-0XAC
 or  TS adapter 6ES7 972-0CA33-0XAC
 or  MPI adapter 6ES7 972-0CA11-0XAC,
 IBH/MHJ-NetLink or CPs 243, 343 and 443

 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2002..2005

 S5 basic communication parts (C) Andrew Rostovtsew 2004.

 Libnodave is free software; you can redistribute it and/or modify
 it under the terms of the GNU Library General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 Libnodave is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with Libnodave; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "nodave.h"
#include "protoNLPro.h"
#include "protoIBH.h"
#include <stdio.h>
#include "log2.h"
#include <string.h>
#include "pkdriver/pkdrvcmn.h"

#define DUMPLIMIT 11132

//#define DAVE_HAVE_MEMCPY  // normally we have

extern int DECL2 _daveConnectPLCTCP(daveConnection * dc);
extern int DECL2 _daveExchangeTCP(daveConnection * dc, PDU * p);
extern int DECL2 _daveGetResponseISO_TCP(daveConnection * dc);

#ifdef LINUX
//#define HAVE_SELECT
#include <time.h>
#include <sys/time.h>

#include <unistd.h>
#define daveWriteFile(a,b,c,d) d=write(a,b,c)
#endif

int daveDebug = 0;

#ifdef _WIN32
#include <winsock2.h>
#endif
#include "openS7online.h"	// We can use the Siemens transport dlls only on Windows


void setTimeOut(daveInterface * di, int tmo)
{
	//COMMTIMEOUTS cto;
#ifdef DEBUG_CALLS
	//LOG3("setTimeOut(di:%p, time:%d)\n",	di,tmo);
	//FLUSH;
#endif	    
#ifdef _WIN32
	//    if(di->fd.connectionType==daveSerialConnection) {

	//GetCommTimeouts(di->fd.rfd, &cto);
	//cto.ReadIntervalTimeout=0;
	//cto.ReadTotalTimeoutMultiplier=0;
	//cto.ReadTotalTimeoutConstant=tmo/1000;
	//SetCommTimeouts(di->fd.rfd,&cto);
	//    } else if(di->fd.connectionType==daveTcpConnection) {  
	//    }
#endif
}


int DECL2 stdread(daveInterface * di,
	char * buffer,
	int length) {
	PKDEVICE *pDevice = (PKDEVICE *)(di->fd.wfd); 
	long lRecvBytes = 0;
	try
	{
		lRecvBytes = Drv_Recv(pDevice, (char *)buffer, length, 1000);
	}
	catch (...)
	{
		printf("Drv_Send except,device:%s, sendBuffPointer:%d, size:%d\n", pDevice->szName, (int)buffer, length);
	}
	return lRecvBytes;
}

int DECL2 stdwrite(daveInterface * di, char * buffer, int length) {
	PKDEVICE *pDevice = (PKDEVICE *)(di->fd.wfd);
	long lSentBytes = 0;
	try
	{
		lSentBytes = Drv_Send(pDevice, (char *)buffer, length, 100);
	}
	catch (...)
	{
		printf("Drv_Send except,device:%s, sendBuffPointer:%d, size:%d\n", pDevice->szName, (int)buffer, length);
	}
	 
	return lSentBytes;
}
/*
	Setup a new interface structure from an initialized serial interface's handle and a name.
	*/
daveInterface * DECL2 daveNewInterface(_daveOSserialType nfd, char * nname, int localMPI, int protocol, int speed){
	daveInterface * di = (daveInterface *)calloc(1, sizeof(daveInterface));
#ifdef DEBUG_CALLS
	LOG7("daveNewInterface(fd.rfd:%d fd.wfd:%d name:%s local MPI:%d protocol:%d PB speed:%d)\n",
		nfd.rfd, nfd.wfd, nname, localMPI, protocol, speed);
	FLUSH;
#endif	
	if (di) {
		//	di->name=nname;
		strncpy(di->realName, nname, 20);
		di->name = di->realName;
		di->fd = nfd;
		di->localMPI = localMPI;
		di->protocol = protocol;
		di->timeout = 1000000; /* 1 second */
		di->nextConnection = 0x14;
		di->speed = speed;

		di->getResponse = _daveGetResponseISO_TCP;
		di->ifread = stdread;
		di->ifwrite = stdwrite;
		di->initAdapter = _daveReturnOkDummy;
		di->connectPLC = _daveReturnOkDummy2;
		di->disconnectPLC = _daveReturnOkDummy2;
		di->disconnectAdapter = _daveReturnOkDummy;
		di->listReachablePartners = _daveListReachablePartnersDummy;
		switch (protocol) {
		case daveProtoMPI:
			di->initAdapter = _daveInitAdapterMPI1;
			di->connectPLC = _daveConnectPLCMPI1;
			di->disconnectPLC = _daveDisconnectPLCMPI;
			di->disconnectAdapter = _daveDisconnectAdapterMPI;
			di->exchange = _daveExchangeMPI;
			di->sendMessage = _daveSendMessageMPI;
			di->getResponse = _daveGetResponseMPI;
			di->listReachablePartners = _daveListReachablePartnersMPI;
			break;

		case daveProtoMPI2:
		case daveProtoMPI4:
			di->initAdapter = _daveInitAdapterMPI2;
			di->connectPLC = _daveConnectPLCMPI2;
			di->disconnectPLC = _daveDisconnectPLCMPI;
			di->disconnectAdapter = _daveDisconnectAdapterMPI;
			di->exchange = _daveExchangeMPI;
			di->sendMessage = _daveSendMessageMPI;
			di->getResponse = _daveGetResponseMPI;
			di->listReachablePartners = _daveListReachablePartnersMPI;
			di->nextConnection = 0x3;
			break;

		case daveProtoMPI3:
			di->initAdapter = _daveInitAdapterMPI3;
			di->connectPLC = _daveConnectPLCMPI3;
			di->disconnectPLC = _daveDisconnectPLCMPI3;
			di->disconnectAdapter = _daveDisconnectAdapterMPI3;
			di->exchange = _daveExchangeMPI3;
			di->sendMessage = _daveSendMessageMPI3;
			di->getResponse = _daveGetResponseMPI3;
			di->listReachablePartners = _daveListReachablePartnersMPI3;
			di->nextConnection = 0x3;
			break;

		case daveProtoISOTCP:
		case daveProtoISOTCP243:
			di->getResponse = _daveGetResponseISO_TCP;
			di->connectPLC = _daveConnectPLCTCP;
			di->exchange = _daveExchangeTCP;
			break;

		case daveProtoPPI:
			di->getResponse = _daveGetResponsePPI;
			di->exchange = _daveExchangePPI;
			di->connectPLC = _daveConnectPLCPPI;
			di->timeout = 150000; /* 0.15 seconds */
			break;
		case daveProtoMPI_IBH:
			di->exchange = _daveExchangeIBH;
			di->connectPLC = _daveConnectPLC_IBH;
			di->disconnectPLC = _daveDisconnectPLC_IBH;
			di->sendMessage = _daveSendMessageMPI_IBH;
			di->getResponse = _daveGetResponseMPI_IBH;
			di->listReachablePartners = _daveListReachablePartnersMPI_IBH;
			break;
		case daveProtoPPI_IBH:
			di->exchange = _daveExchangePPI_IBH;
			di->connectPLC = _daveConnectPLCPPI;
			di->sendMessage = _daveSendMessageMPI_IBH;
			di->getResponse = _daveGetResponsePPI_IBH;
			di->listReachablePartners = _daveListReachablePartnersMPI_IBH;
			break;
#ifdef _WIN32
		case daveProtoS7online:
			di->exchange = _daveExchangeS7online;
			di->connectPLC = _daveConnectPLCS7online;
			di->sendMessage = _daveSendMessageS7online;
			di->getResponse = _daveGetResponseS7online;
			di->listReachablePartners = _daveListReachablePartnersS7online;
			di->disconnectPLC = _daveDisconnectPLCS7online;
			//		di->disconnectAdapter=_daveDisconnectAdapterS7online;
#endif
			break;
		case daveProtoAS511:
			di->connectPLC = _daveConnectPLCAS511;
			di->disconnectPLC = _daveDisconnectPLCAS511;
			di->exchange = _daveFakeExchangeAS511;
			di->sendMessage = _daveFakeExchangeAS511;
			break;
		case daveProtoNLpro:
			di->initAdapter = _daveInitAdapterNLpro;
			di->connectPLC = _daveConnectPLCNLpro;
			di->disconnectPLC = _daveDisconnectPLCNLpro;
			di->disconnectAdapter = _daveDisconnectAdapterNLpro;
			di->exchange = _daveExchangeNLpro;
			di->sendMessage = _daveSendMessageNLpro;
			di->getResponse = _daveGetResponseNLpro;
			di->listReachablePartners = _daveListReachablePartnersNLpro;
			break;

		}
#ifdef _WIN32
		setTimeOut(di, di->timeout);
#endif
	}
	return di;
}

daveInterface * DECL2 davePascalNewInterface(_daveOSserialType* nfd, char * nname, int localMPI, int protocol, int speed){
#ifdef DEBUG_CALLS
	LOG7("davePascalNewInterface(fd.rfd:%d fd.wfd:%d name:%s local MPI:%d protocol:%d PB speed:%d)\n",
		nfd->rfd, nfd->wfd, nname, localMPI, protocol, speed);
	FLUSH;
#endif	
	return daveNewInterface(*nfd, nname, localMPI, protocol, speed);
}

/*
	debugging:
	set debug level by setting variable daveDebug. Debugging is split into
	several topics. Output goes to stderr.
	The file descriptor is written after the module name, so you may
	distinguish messages from multiple connections.

	naming: all stuff begins with dave... to avoid conflicts with other
	namespaces. Things beginning with _dave.. are not intended for
	public use.
	*/



void DECL2 daveSetDebug(int nDebug) {
#ifdef DEBUG_CALLS
	LOG2("daveSetDebug(%d)\n", nDebug);
	FLUSH;
#endif	
	daveDebug = nDebug;
}

int DECL2 daveGetDebug() {
#ifdef DEBUG_CALLS
	LOG1("daveGetDebug()\n");
	FLUSH;
#endif	
	return daveDebug;
}
/**
	C# interoperability:
	**/
void DECL2 daveSetTimeout(daveInterface * di, int tmo) {
#ifdef DEBUG_CALLS
	LOG3("daveSetTimeOut(di:%p, time:%d)\n", di, tmo);
#endif	    
	di->timeout = tmo;
#ifdef _WIN32
	setTimeOut(di, tmo);
#endif    
}

int DECL2 daveGetTimeout(daveInterface * di) {
#ifdef DEBUG_CALLS
	LOG2("daveGetTimeOut(di:%p)\n", di);
	FLUSH;
#endif	    
	return di->timeout;
}

char * DECL2 daveGetName(daveInterface * di) {
#ifdef DEBUG_CALLS
	LOG2("daveGetName(di:%p)\n", di);
	FLUSH;
#endif	    
	return di->name;
}


int DECL2 daveGetMPIAdr(daveConnection * dc) {
#ifdef DEBUG_CALLS
	LOG2("daveGetMPIAdr(dc:%p)\n", dc);
	FLUSH;
#endif	    
	return dc->MPIAdr;
}

int DECL2 daveGetAnswLen(daveConnection * dc) {
#ifdef DEBUG_CALLS
	LOG2("daveGetAnswLen(dc:%p)\n", dc);
	FLUSH;
#endif	    
	return dc->AnswLen;
}

int DECL2 daveGetMaxPDULen(daveConnection * dc) {
#ifdef DEBUG_CALLS
	LOG2("daveGetMaxPDULen(dc:%p)\n", dc);
	FLUSH;
#endif	    
	return dc->maxPDUlength;
}

/**
	PDU handling:
	**/

/*
	set up the header. Needs valid header pointer
	*/
void DECL2 _daveInitPDUheader(PDU * p, int type) {
	memset(p->header, 0, sizeof(PDUHeader));
	if (type == 2 || type == 3)
		p->hlen = 12;
	else
		p->hlen = 10;
	p->param = p->header + p->hlen;
	((PDUHeader*)p->header)->P = 0x32;
	((PDUHeader*)p->header)->type = type;
	p->dlen = 0;
	p->plen = 0;
	p->udlen = 0;
	p->data = NULL;
	p->udata = NULL;
}

/*
	add parameters after header, adjust pointer to data.
	needs valid header
	*/
void DECL2 _daveAddParam(PDU * p, uc * param, us len) {
#ifdef DEBUG_CALLS
	LOG4("_daveAddParam(PDU:%p, param %p, len:%d)\n", p, param, len);
	FLUSH;
#endif
	p->plen = len;
#ifdef DAVE_HAVE_MEMCPY
	memcpy(p->param, param, len);
#else
	int i;
	for (i = 0; i < len; i++) p->param[i] = param[i];
#endif
	((PDUHeader2*)p->header)->plenHi = len / 256;
	((PDUHeader2*)p->header)->plenLo = len % 256;
	//    ((PDUHeader*)p->header)->plen=daveSwapIed_16(len);
	p->data = p->param + len;
	p->dlen = 0;
}

/*
	add data after parameters, set dlen
	needs valid header,parameters
	*/
void DECL2 _daveAddData(PDU * p, void * data, int len) {
#ifdef DEBUG_CALLS
	LOG4("_daveAddData(PDU:%p, data %p, len:%d)\n", p, data, len);
	//    _daveDumpPDU(p);
	FLUSH;
#endif
	uc * dn = p->data + p->dlen;
	p->dlen += len;
#ifdef DAVE_HAVE_MEMCPY
	memcpy(dn, data, len);
#else
	int i; uc * d = (uc*)data;
	for (i = 0; i < len; i++) p->data[p->dlen + i] = d[i];
#endif
	((PDUHeader2*)p->header)->dlenHi = p->dlen / 256;
	((PDUHeader2*)p->header)->dlenLo = p->dlen % 256;
	//    ((PDUHeader*)p->header)->dlen=daveSwapIed_16(p->dlen);
}

/*
	add values after value header in data, adjust dlen and data count.
	needs valid header,parameters,data,dlen
	*/
void DECL2 _daveAddValue(PDU * p, void * data, int len) {
	us dCount;
	uc * dtype;
#ifdef DEBUG_CALLS
	LOG4("_daveAddValue(PDU:%p, data %p, len:%d)\n", p, data, len);
	_daveDumpPDU(p);
	FLUSH;
#endif

	dtype = p->data + p->dlen - 4 + 1;			/* position of first byte in the 4 byte sequence */

	dCount = p->data[p->dlen - 4 + 2 + 1];
	dCount += 256 * p->data[p->dlen - 4 + 2];

	if (daveDebug & daveDebugPDU)
		LOG2("dCount: %d\n", dCount);
	if (*dtype == 4) {	/* bit data, length is in bits */
		dCount += 8 * len;
	}
	else if (*dtype == 9) {	/* byte data, length is in bytes */
		dCount += len;
	}
	else if (*dtype == 3) {	/* bit data, length is in bits */
		dCount += len;
	}
	else {
		if (daveDebug & daveDebugPDU)
			LOG2("unknown data type/length: %d\n", *dtype);
	}
	if (p->udata == NULL) p->udata = p->data + 4;
	p->udlen += len;
	if (daveDebug & daveDebugPDU)
		LOG2("dCount: %d\n", dCount);

	p->data[p->dlen - 4 + 2] = dCount / 256;
	p->data[p->dlen - 4 + 2 + 1] = dCount % 256;

	_daveAddData(p, data, len);
}

/*
	add data in user data. Add a user data header, if not yet present.
	*/
void DECL2 _daveAddUserData(PDU * p, uc * da, int len) {
	uc udh[] = { 0xff, 9, 0, 0 };
	if (p->dlen == 0) {
		if (daveDebug & daveDebugPDU)
			LOG1("adding user data header.\n");
		_daveAddData(p, udh, sizeof(udh));
	}
	_daveAddValue(p, da, len);
}

void DECL2 davePrepareReadRequest(daveConnection * dc, PDU *p) {
	uc pa[] = { daveFuncRead, 0 };
#ifdef DEBUG_CALLS
	LOG3("davePrepareReadRequest(dc:%p PDU:%p)\n", dc, p);
	FLUSH;
#endif	    
	p->header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
}

PDU * DECL2 daveNewPDU() {
	PDU * p;
	p = (PDU*)malloc(sizeof(PDU));
#ifdef DEBUG_CALLS
	LOG2("daveNewPDU() = %p\n", p);
	FLUSH;
#endif	        
	return p;
}


void DECL2 davePrepareWriteRequest(daveConnection * dc, PDU *p) {
	uc pa[] = { daveFuncWrite, 0 };
#ifdef DEBUG_CALLS
	LOG3("davePrepareWriteRequest(dc:%p PDU:%p)\n", dc, p);
	FLUSH;
#endif	    
	p->header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
	p->dlen = 0;
}

void DECL2 daveAddToReadRequest(PDU *p, int area, int DBnum, int start, int byteCount, int isBit) {
	uc pa[] = {
		0x12, 0x0a, 0x10,
		0x02,		/* 1=single bit, 2=byte, 4=word */
		0, 0,		/* length in bytes */
		0, 0,		/* DB number */
		0,		/* area code */
		0, 0, 0		/* start address in bits */
	};

	if ((area == daveAnaIn) || (area == daveAnaOut) /*|| (area==daveP)*/) {
		pa[3] = 4;
		start *= 8;			/* bits */
	}
	else if ((area == daveTimer) || (area == daveCounter) || (area == daveTimer200) || (area == daveCounter200)) {
		pa[3] = area;
	}
	else {
		if (isBit) {
			pa[3] = 1;
		}
		else {
			start *= 8;			/* bit address of byte */
		}
	}

	pa[4] = byteCount / 256;
	pa[5] = byteCount & 0xff;
	pa[6] = DBnum / 256;
	pa[7] = DBnum & 0xff;
	pa[8] = area;
	pa[11] = start & 0xff;
	pa[10] = (start / 0x100) & 0xff;
	pa[9] = start / 0x10000;

	p->param[1]++;
	memcpy(p->param + p->plen, pa, sizeof(pa));
	p->plen += sizeof(pa);

	((PDUHeader2*)p->header)->plenHi = p->plen / 256;
	((PDUHeader2*)p->header)->plenLo = p->plen % 256;

	p->data = p->param + p->plen;
	p->dlen = 0;
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
}

void DECL2 daveAddVarToReadRequest(PDU *p, int area, int DBnum, int start, int byteCount) {
#ifdef DEBUG_CALLS
	LOG6("daveAddVarToReadRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d)\n",
		p, daveAreaName(area), DBnum, start, byteCount);
	FLUSH;
#endif	    	

	daveAddToReadRequest(p, area, DBnum, start, byteCount, 0);
}

void DECL2 daveAddBitVarToReadRequest(PDU *p, int area, int DBnum, int start, int byteCount) {
	daveAddToReadRequest(p, area, DBnum, start, byteCount, 1);
}

void DECL2 daveAddToWriteRequest(PDU *p, int area, int DBnum, int start, int byteCount,
	void * buffer,
	uc * da,
	int dasize,
	uc * pa,
	int pasize
	) {
	uc saveData[1024];
#ifdef DEBUG_CALLS
	LOG7("daveAddToWriteRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
		p, daveAreaName(area), DBnum, start, byteCount, buffer);
	//    _daveDumpPDU(p);
	FLUSH;
#endif

	if ((area == daveTimer) || (area == daveCounter) || (area == daveTimer200) || (area == daveCounter200)) {
		pa[3] = area;
		pa[4] = ((byteCount + 1) / 2) / 0x100;
		pa[5] = ((byteCount + 1) / 2) & 0xff;
	}
	else if ((area == daveAnaIn) || (area == daveAnaOut)) {
		pa[3] = 4;
		pa[4] = ((byteCount + 1) / 2) / 0x100;
		pa[5] = ((byteCount + 1) / 2) & 0xff;
	}
	else {
		pa[4] = byteCount / 0x100;
		pa[5] = byteCount & 0xff;
	}
	pa[6] = DBnum / 256;
	pa[7] = DBnum & 0xff;
	pa[8] = area;
	pa[11] = start & 0xff;
	pa[10] = (start / 0x100) & 0xff;
	pa[9] = start / 0x10000;
	if (p->dlen % 2) {
		_daveAddData(p, da, 1);
	}
	p->param[1]++;
#ifdef DAVE_HAVE_MEMCPY
	if (p->dlen){
		memcpy(saveData, p->data, p->dlen);
		memcpy(p->data + pasize, saveData, p->dlen);
	}
	memcpy(p->param + p->plen, pa, pasize);
#else
	int i;
	if (p->dlen){
		for (i = 0; i < p->dlen; i++) saveData[i] = p->data[i];
		for (i = 0; i < p->dlen; i++) p->data[i + pasize] = saveData[i];
	}
	for (i = 0; i < pasize; i++) p->param[i + p->plen] = pa[i];
#endif 
	p->plen += pasize;

	((PDUHeader2*)p->header)->plenHi = p->plen / 256;
	((PDUHeader2*)p->header)->plenLo = p->plen % 256;

	p->data = p->param + p->plen;
	_daveAddData(p, da, dasize);
	_daveAddValue(p, buffer, byteCount);
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
}

void DECL2 daveAddVarToWriteRequest(PDU *p, int area, int DBnum, int start, int byteCount, void * buffer) {
	uc da[] = { 0, 4, 0, 0, };
	uc pa[] = {
		0x12, 0x0a, 0x10,
		0x02,		/* unit (for count?, for consistency?) byte */
		0, 0,		/* length in bytes */
		0, 0,		/* DB number */
		0,		/* area code */
		0, 0, 0		/* start address in bits */
	};
#ifdef DEBUG_CALLS
	LOG7("daveAddVarToWriteRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
		p, daveAreaName(area), DBnum, start, byteCount, buffer);
	FLUSH;
#endif	    	

	daveAddToWriteRequest(p, area, DBnum, 8 * start, byteCount, buffer, da, sizeof(da), pa, sizeof(pa));
}


void DECL2 daveAddBitVarToWriteRequest(PDU *p, int area, int DBnum, int start, int byteCount, void * buffer) {
	uc da[] = { 0, 3, 0, 0, };
	uc pa[] = {
		0x12, 0x0a, 0x10,
		0x01,		/* single bit */
		0, 0,		/* insert length in bytes here */
		0, 0,		/* insert DB number here */
		0,		/* change this to real area code */
		0, 0, 0		/* insert start address in bits */
	};
#ifdef DEBUG_CALLS
	LOG7("daveAddBitVarToWriteRequest(PDU:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
		p, daveAreaName(area), DBnum, start, byteCount, buffer);
	FLUSH;
#endif	    	

	daveAddToWriteRequest(p, area, DBnum, start, byteCount, buffer, da, sizeof(da), pa, sizeof(pa));
}

/*
	Get the eror code from a PDU, if one.
	*/
int DECL2 daveGetPDUerror(PDU * p) {
#ifdef DEBUG_CALLS
	LOG2("daveGetPDUerror(PDU:%p\n", p);
	FLUSH;
#endif	    	
	if (p->header[1] == 2 || p->header[1] == 3) {
		return daveGetU16from(p->header + 10);
	}
	else
		return 0;
}

/*
	Sets up pointers to the fields of a received message.
	*/
int DECL2 _daveSetupReceivedPDU(daveConnection * dc, PDU * p) {
	int res; /* = daveResCannotEvaluatePDU; */
	p->header = dc->msgIn + dc->PDUstartI;
	res = 0;
	if (p->header[1] == 2 || p->header[1] == 3) { // 第9个字节
		p->hlen = 12;
		res = 256 * p->header[10] + p->header[11];
	}
	else {
		p->hlen = 10;
	}
	p->param = p->header + p->hlen;

	p->plen = 256 * p->header[6] + p->header[7];
	p->data = p->param + p->plen;
	p->dlen = 256 * p->header[8] + p->header[9];
	p->udlen = 0;
	p->udata = NULL;
	if (daveDebug & daveDebugPDU)
		_daveDumpPDU(p);
	return res;
}

int DECL2 _daveTestResultData(PDU * p) {
	int res; /*=daveResCannotEvaluatePDU;*/
	if ((p->data[0] == 255) && (p->dlen > 4)) {
		res = daveResOK;
		p->udata = p->data + 4;
		p->udlen = p->data[2] * 0x100 + p->data[3];
		if (p->data[1] == 4) {
			p->udlen >>= 3;	/* len is in bits, adjust */
		}
		else if (p->data[1] == 9) {
			/* len is already in bytes, ok */
		}
		else if (p->data[1] == 3) {
			/* len is in bits, but there is a byte per result bit, ok */
		}
		else {
			if (daveDebug & daveDebugPDU)
				LOG2("fixme: what to do with data type %d?\n", p->data[1]);
			res = daveResUnknownDataUnitSize;
		}
	}
	else {
		res = p->data[0];
	}
	return res;
}

int DECL2 _daveTestReadResult(PDU * p) {
	if (p->param[0] != daveFuncRead) return daveResUnexpectedFunc;
	return _daveTestResultData(p);
}

int DECL2 _daveTestPGReadResult(PDU * p) {
	int pres = 0;
	if (p->param[0] != 0) return daveResUnexpectedFunc;
	if (p->plen == 12) pres = (256 * p->param[10] + p->param[11]);
	if (pres == 0)return _daveTestResultData(p); else return pres;
}

int DECL2 _daveTestWriteResult(PDU * p) {
	int res;/* =daveResCannotEvaluatePDU; */
	if (p->param[0] != daveFuncWrite) return daveResUnexpectedFunc;
	if ((p->data[0] == 255)) {
		res = daveResOK;
	}
	else
		res = p->data[0];
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
	return res;
}

/*****
	Utilities:
	****/
/*
	This is an extended memory compare routine. It can handle don't care and stop flags
	in the sample data. A stop flag lets it return success.
	*/
int DECL2 _daveMemcmp(us * a, uc *b, size_t len) {
	unsigned int i;
	us * a1 = (us*)a;
	uc * b1 = (uc*)b;
	for (i = 0; i < len; i++){
		if (((*a1) & 0xff) != *b1) {
			if (((*a1) & 0x100) != 0x100) {
				//		LOG3("want:%02X got:%02X\n",*a1,*b1);
				return i + 1;
			}
			if (((*a1) & 0x200) == 0x200) return 0;
		}
		a1++;
		b1++;
	}
	return 0;
}

/*
	Hex dump:
	*/
void DECL2 _daveDump(char * name, void*b, int len) {//void DECL2 _daveDump(char * name,uc*b,int len) {
	int j;
	LOG2("%s:                             ", name);
	if (len > daveMaxRawLen) len = daveMaxRawLen; 	/* this will avoid to dump zillions of chars */
	if (len > DUMPLIMIT) len = DUMPLIMIT; 		/* this will avoid large dumps */
	for (j = 0; j < len; j++){
		if ((j & 0xf) == 0) LOG_2("\n                            %x:", j);
		LOG_2("0x%02X,", ((uc*)(b))[j]);
	}
	LOG_1("\n");
}

/*
	Hex dump PDU:
	*/
void DECL2 _daveDumpPDU(PDU * p) {
	int i, dl;
	uc * pd;
	_daveDump("PDU header", p->header, p->hlen);
	LOG3("plen: %d dlen: %d\n", p->plen, p->dlen);
	if (p->plen > 0) _daveDump("Parameter", p->param, p->plen);
	if (p->dlen > 0) _daveDump("Data     ", p->data, p->dlen);
	if ((p->plen == 2) && (p->param[0] == daveFuncRead)) {
		pd = p->data;
		for (i = 0; i < p->param[1]; i++) {
			_daveDump("Data hdr ", pd, 4);

			dl = 0x100 * pd[2] + pd[3];
			if (pd[1] == 4) dl /= 8;
			pd += 4;
			_daveDump("Data     ", pd, dl);
			if (i < p->param[1] - 1) dl = dl + (dl % 2);  	// the PLC places extra bytes at the end of all 
			// but last result, if length is not a multiple 
			// of 2
			pd += dl;
		}
	}
	else if ((p->header[1] == 1) &&/*(p->plen==2)&&*/(p->param[0] == daveFuncWrite)) {
		pd = p->data;
		for (i = 0; i < p->param[1]; i++) {
			_daveDump("Write Data hdr ", pd, 4);

			dl = 0x100 * pd[2] + pd[3];
			if (pd[1] == 4) dl /= 8;
			pd += 4;
			_daveDump("Data     ", pd, dl);
			if (i < p->param[1] - 1) dl = dl + (dl % 2);  	// the PLC places extra bytes at the end of all 
			// but last result, if length is not a multiple 
			// of 2
			pd += dl;
		}
	}
	else {
		/*
			if(p->dlen>0) {
			if(p->udlen==0)
			_daveDump("Data     ",p->data,p->dlen);
			else
			_daveDump("Data hdr ",p->data,4);
			}
			if(p->udlen>0) _daveDump("result Data ",p->udata,p->udlen);
			*/
	}
	if ((p->header[1] == 2) || (p->header[1] == 3)) {
		LOG2("error: %s\n", daveStrerror(daveGetPDUerror(p)));
	}
}

/*
	name Objects:
	*/
char * DECL2 daveBlockName(uc bn) {
	//#ifdef DEBUG_CALLS
	LOG2("daveBlockName(bn:%d)\n", bn);
	FLUSH;
	//#endif	    	
	switch (bn) {
	case daveBlockType_OB: return "OB";
	case daveBlockType_DB: return "DB";
	case daveBlockType_SDB: return "SDB";
	case daveBlockType_FC: return "FC";
	case daveBlockType_SFC: return "SFC";
	case daveBlockType_FB: return "FB";
	case daveBlockType_SFB: return "SFB";
	default:return "unknown block type!";
	}
}

char * DECL2 daveAreaName(uc n) {
#ifdef DEBUG_CALLS
	LOG2("daveAreaName(n:%d)\n", n);
	FLUSH;
#endif	    	
	switch (n) {
	case daveSysInfo:	return "System info mem.area of 200 family";
	case daveSysFlags:	return "System flags of 200 family";
	case daveAnaIn:		return "analog inputs of 200 family";
	case daveAnaOut:	return "analog outputs of 200 family";

	case daveP:		return "Peripheral I/O";
	case daveInputs:	return "Inputs";
	case daveOutputs:	return "Outputs";
	case daveDB:		return "DB";
	case daveDI:		return "DI (instance data)";
	case daveFlags:		return "Flags";
	case daveLocal:		return "local data";
	case daveV:		return "caller's local data";
	case daveCounter:	return "S7 counters";
	case daveTimer:		return "S7 timers";
	case daveCounter200:	return "IEC counters";
	case daveTimer200:	return "IEC timers";
	default:return "unknown area!";
	}
}

/*
	Functions to load blocks from PLC:
	*/
void DECL2 _daveConstructUpload(PDU *p, char blockType, int blockNr) {
	uc pa[] = { 0x1d,
		0, 0, 0, 0, 0, 0, 0, 9, 0x5f, 0x30, 0x41, 48, 48, 48, 48, 49, 65 };
	pa[11] = blockType;
	sprintf((char*)(pa + 12), "%05d", blockNr);
	pa[17] = 'A';
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
}

void DECL2 _daveConstructDoUpload(PDU * p, int uploadID) {
	uc pa[] = { 0x1e, 0, 0, 0, 0, 0, 0, 1 };
	pa[7] = uploadID;
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
}

void DECL2 _daveConstructEndUpload(PDU * p, int uploadID) {
	uc pa[] = { 0x1f, 0, 0, 0, 0, 0, 0, 1 };
	pa[7] = uploadID;
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
}

uc paInsert[] = {		// sended after transmission of a complete block,
	// I guess this makes the CPU link the block into a program.
	0x28, 0, 0, 0, 0, 0, 0, 0xFD, 0, 0x0A, 1, 0, 0x30, 0x42, 0x30, 0x30, 0x30, 0x30, 0x34, 0x50, // block type code and number	
	0x05, '_', 'I', 'N', 'S', 'E',
};

uc paMakeRun[] = {
	0x28, 0, 0, 0, 0, 0, 0, 0xFD, 0, 0x00, 9, 'P', '_', 'P', 'R', 'O', 'G', 'R', 'A', 'M'
};

uc paCompress[] = {
	0x28, 0, 0, 0, 0, 0, 0, 0xFD, 0, 0x00, 5, '_', 'G', 'A', 'R', 'B'
};

uc paMakeStop[] = {
	0x29, 0, 0, 0, 0, 0, 9, 'P', '_', 'P', 'R', 'O', 'G', 'R', 'A', 'M'
};

uc paCopyRAMtoROM[] = {
	0x28, 0, 0, 0, 0, 0, 0, 0xfd, 0, 2, 'E', 'P', 5, '_', 'M', 'O', 'D', 'U'
};

int DECL2 daveStop(daveConnection * dc) {
	int res;
	PDU p, p2;
#ifdef DEBUG_CALLS
	LOG2("daveStop(dc:%p)\n", dc);
	FLUSH;
#endif	  
	if (dc->iface->protocol == daveProtoAS511) {
		return daveStopS5(dc);
	}
	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 1);
	_daveAddParam(&p, paMakeStop, sizeof(paMakeStop));
	res = _daveExchange(dc, &p);
	if (res == daveResOK) {
		res = _daveSetupReceivedPDU(dc, &p2);
		if (daveDebug & daveDebugPDU) {
			_daveDumpPDU(&p2);
		}
	}
	return res;
}

int DECL2 daveStart(daveConnection*dc) {
	int res;
	PDU p, p2;
#ifdef DEBUG_CALLS
	LOG2("daveStart(dc:%p)\n", dc);
	FLUSH;
#endif	    	
	if (dc->iface->protocol == daveProtoAS511) {
		return daveStartS5(dc);
	}
	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 1);
	_daveAddParam(&p, paMakeRun, sizeof(paMakeRun));
	res = _daveExchange(dc, &p);
	if (res == daveResOK) {
		res = _daveSetupReceivedPDU(dc, &p2);
		if (daveDebug & daveDebugPDU) {
			_daveDumpPDU(&p2);
		}
	}
	return res;
}

int DECL2 daveCopyRAMtoROM(daveConnection * dc) {
	int res;
	PDU p, p2;
#ifdef DEBUG_CALLS
	LOG2("davecopyRAMtoROM(dc:%p)\n", dc);
	FLUSH;
#endif	  
	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 1);
	_daveAddParam(&p, paCopyRAMtoROM, sizeof(paCopyRAMtoROM));
	res = _daveExchange(dc, &p);
	if (res == daveResOK) {
		res = _daveSetupReceivedPDU(dc, &p2);  /* possible problem, Timeout */
		if (daveDebug & daveDebugPDU) {
			_daveDumpPDU(&p2);
		}
	}
	return res;
}



/*
	Build a PDU with user data ud, send it and prepare received PDU.
	*/
int DECL2 daveBuildAndSendPDU(daveConnection * dc, PDU*p2, uc *pa, int psize, uc *ud, int usize) {
	int res;
	PDU p;
	uc nullData[] = { 0x0a, 0, 0, 0 };
	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 7);
	_daveAddParam(&p, pa, psize);
	if (ud != NULL) _daveAddUserData(&p, ud, usize); else
		if (usize != 0) _daveAddData(&p, nullData, 4);
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(&p);
	}
	res = _daveExchange(dc, &p);
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveExchange(): %d\n", res);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, p2);
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p2);
	}
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveSetupReceivedPDU(): %04X\n", res);
	if (res != daveResOK) return res;
	res = _daveTestPGReadResult(p2);
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveTestPGReadResult(): %04X\n", res);
	return res;
}

int DECL2 daveListBlocksOfType(daveConnection * dc, uc type, daveBlockEntry * buf) {
	int res, i, len;
	PDU p2;
	uc * buffer = (uc*)buf;
	uc pa[] = { 0, 1, 18, 4, 17, 67, 2, 0 };
	uc da[] = { '0', '0' };
	uc pam[] = { 0, 1, 18, 8, 0x12, 0x43, 2, 1, 0, 0, 0, 0 };
#ifdef DEBUG_CALLS
	LOG4("ListBlocksOfType(dc:%p type:%d buf:%p)\n", dc, type, buf);
	FLUSH;
#endif	    	
	da[1] = type;
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), da, sizeof(da));
	if (res != daveResOK) return -res;
	len = 0;
	while (p2.param[9] != 0) {
		if (buffer != NULL) memcpy(buffer + len, p2.udata, p2.udlen);
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len += p2.udlen;
		printf("more data\n");
		res = daveBuildAndSendPDU(dc, &p2, pam, sizeof(pam), NULL, 1);
		if (res != daveResOK) return res; 	// bugfix from Natalie Kather
	}


	if (res == daveResOK) {
		if (buffer != NULL) memcpy(buffer + len, p2.udata, p2.udlen);
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len += p2.udlen;
	}
	else {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("daveListBlocksOfType: %d=%s\n", res, daveStrerror(res));
	}
	dc->AnswLen = len;
	res = len / sizeof(daveBlockEntry);
	for (i = 0; i < res; i++) {
		buf[i].number = daveSwapIed_16(buf[i].number);
	}
	return res;
}

/*
	doesn't work on S7-200
	*/
int DECL2 daveGetOrderCode(daveConnection * dc, char * buf) {
	int res = 0;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 68, 1, 0 };
	uc da[] = { 1, 17, 0, 1 };  /* SZL-ID 0x111 index 1 */
#ifdef DEBUG_CALLS
	LOG3("daveGetOrderCode(dc:%p buf:%p)\n", dc, buf);
	FLUSH;
#endif	    	
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), da, sizeof(da));
	if (res != daveResOK) return res; // similar to bugfix from Natalie Kather
	if (buf) {
		memcpy(buf, p2.udata + 10, daveOrderCodeSize);
		buf[daveOrderCodeSize] = 0;
	}
	return res;
}

int DECL2 daveReadSZL(daveConnection * dc, int ID, int index, void * buffer, int buflen) {
	int res, len, cpylen;
	int pa7;
	//    int pa6;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 68, 1, 0 };
	uc da[] = { 1, 17, 0, 1 };

	uc pam[] = { 0, 1, 18, 8, 18, 68, 1, 1, 0, 0, 0, 0 };
	//    uc dam[]={10,0,0,0};

#ifdef DEBUG_CALLS
	LOG5("daveReadSZL(dc:%p, ID:%d, index:%d, buffer:%p)\n", dc, ID, index, buffer);
	FLUSH;
#endif	    	
	da[0] = ID / 0x100;
	da[1] = ID % 0x100;
	da[2] = index / 0x100;
	da[3] = index % 0x100;
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), da, sizeof(da));
	if (res != daveResOK) return res; // similar to bugfix from Natalie Kather

	len = 0;
	pa7 = p2.param[7];
	//    pa6=p2.param[6];
	while (p2.param[9] != 0) {
		if (buffer != NULL) {
			cpylen = p2.udlen;
			if (len + cpylen > buflen) cpylen = buflen - len;
			if (cpylen > 0) memcpy((uc *)buffer + len, p2.udata, cpylen);
		}
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len += p2.udlen;
		pam[7] = pa7;
		//		res=daveBuildAndSendPDU(dc, &p2,pam, sizeof(pam), NULL, sizeof(dam));
		res = daveBuildAndSendPDU(dc, &p2, pam, sizeof(pam), NULL, 1);
		if (res != daveResOK) return res; // similar to bugfix from Natalie Kather
	}


	if (res == daveResOK) {
		if (buffer != NULL) {
			cpylen = p2.udlen;
			if (len + cpylen > buflen) cpylen = buflen - len;
			if (cpylen > 0) memcpy((uc *)buffer + len, p2.udata, cpylen);
		}
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len += p2.udlen;
	}
	dc->AnswLen = len;
	return res;
}

int DECL2 daveGetBlockInfo(daveConnection * dc, daveBlockInfo *dbi, uc type, int number) {
	int res;
	uc pa[] = { 0, 1, 18, 4, 17, 67, 3, 0 };	   /* param */
	uc da[] = { '0', 0, '0', '0', '0', '1', '0', 'A' };
	PDU p2;
	sprintf((char*)(da + 2), "%05d", number);
	da[1] = type;
	da[7] = 'A';
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), da, sizeof(da));
	if (res != daveResOK) return res; // similar to bugfix from Natalie Kather
	if ((dbi != NULL) && (p2.udlen == sizeof(daveBlockInfo))) {
		memcpy(dbi, p2.udata, p2.udlen);
		dbi->number = daveSwapIed_16(dbi->number);
		dbi->length = daveSwapIed_16(dbi->length);
	}
	return res;
}

int DECL2 daveListBlocks(daveConnection * dc, daveBlockTypeEntry * buf) {
	int res, i;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 67, 1, 0 };
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), NULL, 1/*da, sizeof(da)*/);
	if (res != daveResOK) return res; // similar to bugfix from Natalie Kather
	res = p2.udlen / sizeof(daveBlockTypeEntry);
	if (buf) {
		memcpy(buf, p2.udata, p2.udlen);
		for (i = 0; i < res; i++) {
			buf[i].count = daveSwapIed_16(buf[i].count);
		}
	}
	return res;
}

int DECL2 daveReadManyBytes(daveConnection * dc, int area, int DBnum, int start, int len, void * buffer) {
	int res, pos, readLen;
	uc * pbuf;
	pos = 0;
	if (buffer == NULL) return daveResNoBuffer;
	pbuf = (uc*)buffer;
	res = daveResInvalidLength; //the only chance to return this is when len<=0
	while (len > 0) {
		if (len > dc->maxPDUlength - 18) readLen = dc->maxPDUlength - 18; else readLen = len;
		res = daveReadBytes(dc, area, DBnum, start, readLen, pbuf);
		if (res != 0) return res;
		len -= readLen;
		start += readLen;
		pbuf += readLen;
	}
	return res;
}

/*
	Read len bytes from PLC memory area "area", data block DBnum.
	Return the Number of bytes read.
	If a buffer pointer is provided, data will be copied into this buffer.
	If it's NULL you can get your data from the resultPointer in daveConnection long
	as you do not send further requests.
	*/
int DECL2 daveReadBytes(daveConnection * dc, int area, int DBnum, int start, int len, void * buffer){
	PDU p1, p2;
	int res;
	//printf("111\n");
#ifdef DEBUG_CALLS
	LOG7("daveReadBytes(dc:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
		dc, daveAreaName(area), DBnum, start, len, buffer);
	FLUSH;
#endif
	//printf("112\n");
	if (dc->iface->protocol == daveProtoAS511) {
		return daveReadS5Bytes(dc, area, DBnum, start, len/*, buffer*/);
	}
	//printf("113\n");
	dc->AnswLen = 0;	// 03/12/05
	dc->resultPointer = NULL;
	dc->_resultPointer = NULL;
	p1.header = dc->msgOut + dc->PDUstartO;
	//printf("114\n");
	davePrepareReadRequest(dc, &p1);
	//printf("115\n");
	daveAddVarToReadRequest(&p1, area, DBnum, start, len);
	//printf("116\n");
	res = _daveExchange(dc, &p1);
	//printf("117n");
	if (res != daveResOK)
		return res;
	//printf("118\n");
	res = _daveSetupReceivedPDU(dc, &p2);
	//printf("119\n");
	if (daveDebug & daveDebugPDU)
		LOG3("_daveSetupReceivedPDU() returned: %d=%s\n", res, daveStrerror(res));
	if (res != daveResOK)
		return res;

	//printf("121\n");
	res = _daveTestReadResult(&p2);
	//printf("122\n");
	if (daveDebug & daveDebugPDU)
		LOG3("_daveTestReadResult() returned: %d=%s\n", res, daveStrerror(res));
	if (res != daveResOK)
		return res;
	//printf("123\n");

	if (p2.udlen == 0) {
		return daveResCPUNoData;
	}
	/*
		copy to user buffer and setup internal buffer pointers:
		*/
	//printf("124\n");
	if (buffer != NULL)
		memcpy(buffer, p2.udata, p2.udlen);

	//printf("125\n");
	dc->resultPointer = p2.udata;
	dc->_resultPointer = p2.udata;
	dc->AnswLen = p2.udlen;
	return res;
}

/*
	Read len BITS from PLC memory area "area", data block DBnum.
	Return the Number of bytes read.
	If a buffer pointer is provided, data will be copied into this buffer.
	If it's NULL you can get your data from the resultPointer in daveConnection long
	as you do not send further requests.
	*/
int DECL2 daveReadBits(daveConnection * dc, int area, int DBnum, int start, int len, void * buffer){
	PDU p1, p2;
	int res;
#ifdef DEBUG_CALLS
	LOG7("daveReadBits(dc:%p area:%s area number:%d start address:%d byte count:%d buffer:%p)\n",
		dc, daveAreaName(area), DBnum, start, len, buffer);
	FLUSH;
#endif	    	
	dc->resultPointer = NULL;
	dc->_resultPointer = NULL;
	dc->AnswLen = 0;
	p1.header = dc->msgOut + dc->PDUstartO;
	davePrepareReadRequest(dc, &p1);
	daveAddBitVarToReadRequest(&p1, area, DBnum, start, len);

	res = _daveExchange(dc, &p1);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (daveDebug & daveDebugPDU)
		LOG3("_daveSetupReceivedPDU() returned: %d=%s\n", res, daveStrerror(res));
	if (res != daveResOK) return res;

	res = _daveTestReadResult(&p2);
	if (daveDebug & daveDebugPDU)
		LOG3("_daveTestReadResult() returned: %d=%s\n", res, daveStrerror(res));
	if (res != daveResOK) return res;
	if (daveDebug & daveDebugPDU)
		LOG2("got %d bytes of data\n", p2.udlen);
	if (p2.udlen == 0) {
		return daveResCPUNoData;
	}
	if (buffer != NULL) {
		if (daveDebug & daveDebugPDU)
			LOG2("copy %d bytes to buffer\n", p2.udlen);
		memcpy(buffer, p2.udata, p2.udlen);
	}
	dc->resultPointer = p2.udata;
	dc->_resultPointer = p2.udata;
	dc->AnswLen = p2.udlen;
	return res;
}

/*
	Execute a predefined read request. Store results into the resultSet structure.
	*/
int DECL2 daveExecReadRequest(daveConnection * dc, PDU *p, daveResultSet* rl){
	PDU p2;
	uc * q;
	daveResult * cr, *c2;
	int res, i, len, rlen;
#ifdef DEBUG_CALLS
	LOG4("daveExecReadRequest(dc:%p, PDU:%p, rl:%p\n", dc, p, rl);
	FLUSH;
#endif	    	
	dc->AnswLen = 0;	// 03/12/05
	dc->resultPointer = NULL;
	dc->_resultPointer = NULL;
	res = _daveExchange(dc, p);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != daveResOK) return res;
	res = _daveTestReadResult(&p2);
	if (res != daveResOK) return res;
	i = 0;
	if (rl != NULL) {
		cr = (daveResult*)calloc(p2.param[1], sizeof(daveResult));
		rl->numResults = p2.param[1];
		rl->results = cr;
		c2 = cr;
		q = p2.data;
		rlen = p2.dlen;
		while (i < p2.param[1]) {
			/*	    printf("result %d: %d  %d %d %d\n",i, *q,q[1],q[2],q[3]); */
			if ((*q == 255) && (rlen>4)) {
				len = q[2] * 0x100 + q[3];
				if (q[1] == 4) {
					len >>= 3;	/* len is in bits, adjust */
				}
				else if (q[1] == 9) {
					/* len is already in bytes, ok */
				}
				else if (q[1] == 3) {
					/* len is in bits, but there is a byte per result bit, ok */
				}
				else {
					if (daveDebug & daveDebugPDU)
						LOG2("fixme: what to do with data type %d?\n", q[1]);
				}
			}
			else {
				len = 0;
			}
			/*	    printf("Store result %d length:%d\n", i, len); */
			c2->length = len;
			if (len > 0){
				c2->bytes = (uc*)malloc(len);
				memcpy(c2->bytes, q + 4, len);
			}
			c2->error = daveUnknownError;

			if (q[0] == 0xFF) {
				c2->error = daveResOK;
			}
			else
				c2->error = q[0];

			/*	    printf("Error %d\n", c2->error); */
			q += len + 4;
			rlen -= len;
			if ((len % 2) == 1) {
				q++;
				rlen--;
			}
			c2++;
			i++;
		}
	}
	return res;
}

/*
	Execute a predefined write request.
	*/
int DECL2 daveExecWriteRequest(daveConnection * dc, PDU *p, daveResultSet* rl){
	PDU p2;
	uc * q;
	daveResult * cr, *c2;
	int res, i;
#ifdef DEBUG_CALLS
	LOG4("daveExecWriteRequest(dc:%p, PDU:%p, rl:%p\n", dc, p, rl);
	FLUSH;
#endif	    	
	res = _daveExchange(dc, p);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != daveResOK) return res;
	res = _daveTestWriteResult(&p2);
	if (res != daveResOK) return res;
	if (rl != NULL) {
		cr = (daveResult*)calloc(p2.param[1], sizeof(daveResult));
		rl->numResults = p2.param[1];
		rl->results = cr;
		c2 = cr;
		q = p2.data;
		i = 0;
		while (i < p2.param[1]) {
			/*		printf("result %d: %d  %d %d %d\n",i, *q,q[1],q[2],q[3]); */
			c2->error = daveUnknownError;
			if (q[0] == 0x0A) {	/* 300 and 400 families */
				c2->error = daveResItemNotAvailable;
			}
			else if (q[0] == 0x03) {	/* 200 family */
				c2->error = daveResItemNotAvailable;
			}
			else if (q[0] == 0x05) {
				c2->error = daveAddressOutOfRange;
			}
			else if (q[0] == 0xFF) {
				c2->error = daveResOK;
			}
			else if (q[0] == 0x07) {
				c2->error = daveWriteDataSizeMismatch;
			}
			/*		    printf("Error %d\n", c2->error); */
			q++;
			c2++;
			i++;
		}
	}
	return res;
}


int DECL2 daveUseResult(daveConnection * dc, daveResultSet * rl, int n){
	daveResult * dr;
#ifdef DEBUG_CALLS
	LOG4("daveUseResult(dc:%p, result set:%p, number:%d)\n", dc, rl, n);
#endif	    	
	if (rl == NULL) {
#ifdef DEBUG_CALLS
		LOG1("invalid resultSet \n");
		FLUSH;
#endif
		return daveEmptyResultSetError;
	}
#ifdef DEBUG_CALLS
	LOG2("result set has %d results\n", rl->numResults);
	FLUSH;
#endif        
	if (rl->numResults == 0) return daveEmptyResultSetError;
	if (n >= rl->numResults) return daveEmptyResultSetError;
	dr = &(rl->results[n]);
	if (dr->error != 0) return dr->error;
	if (dr->length <= 0) return daveEmptyResultError;

	dc->resultPointer = dr->bytes;
	dc->_resultPointer = dr->bytes;
	return 0;
}

void DECL2 daveFreeResults(daveResultSet * rl){
	daveResult * r;
	int i;
#ifdef DEBUG_CALLS
	LOG2("daveFreeResults(%p)", rl);
#endif	        
	if (rl == NULL) {
#ifdef DEBUG_CALLS
		LOG1("no Results,ready\n");
#endif	            
		return;	// make it NULL safe
	}
	/*    printf("result set: %p\n",rl); */
	for (i = 0; i < rl->numResults; i++) {
		r = &(rl->results[i]);
		/*	printf("result: %p bytes at:%p\n",r,r->bytes); */
		if (r->bytes != NULL) free(r->bytes);
	}
#ifdef DEBUG_CALLS
	LOG2(" free'd %d results\n", rl->numResults);
#endif	       
	free(rl->results);	// fix from Renato Gartmann     
	rl->numResults = 0;
	/*    free(rl);	*/ /* This is NOT malloc'd by library but in the application's memory space! */
}

int DECL2 daveGetErrorOfResult(daveResultSet *rs, int number) {
	return rs->results[number].error;
}

/*
	This will setup a new connection structure using an initialized
	daveInterface and PLC's MPI address.
	*/
daveConnection * DECL2 daveNewConnection(daveInterface * di, int MPI, int rack, int slot) {
	daveConnection * dc = (daveConnection *)calloc(1, sizeof(daveConnection));
	if (dc) {
		dc->iface = di;
		dc->MPIAdr = MPI;

		dc->rack = rack;
		dc->slot = slot;

		dc->maxPDUlength = 1920;				// assume an (unreal?) maximum
		dc->connectionNumber = di->nextConnection;	// 1/10/05 trying Andrew's patch

		dc->PDUnumber = 0xFFFE;			// just a start value; // test!
		dc->messageNumber = 0;
		dc->communicationType = davePGCommunication;

		switch (di->protocol) {
		case daveProtoMPI:		/* my first Version of MPI */
			dc->PDUstartO = 8;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 8;	/* position of PDU in incoming messages */
			di->ackPos = 6;		/* position of 0xB0 in ack packet */
			dc->maxPDUlength = 240;	/* limit because we still cannot assemble a PDU transported in multiple packets */
			break;
		case daveProtoMPI3:		/* Step 7 Version of MPI */
			dc->PDUstartO = 8;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 12;	/* position of PDU in incoming messages */
			di->ackPos = 10;		/* position of 0xB0 in ack packet */
			dc->maxPDUlength = 240;	/* limit because we still cannot assemble a PDU transported in multiple packets */
			break;
		case daveProtoMPI2:		/* Andrew's Version of MPI */
		case daveProtoMPI4:		/* Andrew's Version of MPI with extra STX */
			dc->PDUstartO = 6;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 6;	/* position of PDU in incoming messages */
			di->ackPos = 4;		/* position of 0xB0 in ack packet */
			dc->maxPDUlength = 240;	/* limit because we still cannot assemble a PDU transported in multiple packets */
			break;

		case daveProtoNLpro:	/* Deltalogic NetLink Pro */
			dc->PDUstartO = 6;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 8;	/* position of PDU in incoming messages */
			di->ackPos = 4;		/* position of 0xB0 in ack packet */
			break;

		case daveProtoPPI:
			dc->PDUstartO = 3;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 7;	/* position of PDU in incoming messages */
			break;

		case daveProtoISOTCP:
		case daveProtoISOTCP243:
			dc->PDUstartO = 7;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 7;	/* position of PDU in incoming messages */
			di->timeout = 1500000;
			break;
		case daveProtoMPI_IBH:
			//		dc->maxPDUlength=240;	// limit for NetLink as reported by AFK. Not needed any more. Now, a PDU can be split into multiple IBH packets.
			dc->PDUstartI = sizeof(IBHpacket) + sizeof(MPIheader);
			dc->PDUstartO = sizeof(IBHpacket) + sizeof(MPIheader); // 02/01/2005	
			break;
		case daveProtoPPI_IBH:
			//	        dc->maxPDUlength=240;	// limit for NetLink as reported by AFK
			dc->PDUstartI = 14; // sizeof(IBHpacket)+7;	
			dc->PDUstartO = 13;// sizeof(IBHpacket)+7; // 02/01/2005	
			break;

		case daveProtoAS511:
			dc->PDUstartI = 0;
			dc->PDUstartO = 0;
			break;

		case daveProtoUserTransport:
			dc->PDUstartI = 0;
			dc->PDUstartO = 0;
			break;
		case daveProtoS7online:
			dc->PDUstartI = 80;
			dc->PDUstartO = 80;
			break;

		default:
			dc->PDUstartO = 8;	/* position of PDU in outgoing messages */
			dc->PDUstartI = 8;	/* position of PDU in incoming messages */
			fprintf(stderr, "Unknown protocol on interface %s\n", di->name);
		}
#ifdef _WIN32	
		setTimeOut(di, di->timeout);
#endif
	}
	return dc;
}

void DECL2 daveSetRoutingDestination(daveConnection * dc, int subnet1, int subnet3, int adrsize, uc* plcadr) {
	memset(&(dc->routingData), 0, sizeof(daveRoutingData));
	dc->routing = 1;
	dc->routingData.subnetID1 = subnet1;
	dc->routingData.subnetID2 = 0;
	dc->routingData.subnetID3 = subnet3;
	dc->routingData.PLCadrsize = adrsize;
	memcpy(&(dc->routingData.PLCadr), plcadr, adrsize);
}

void DECL2 daveSetCommunicationType(daveConnection * dc, int communicationType) {
	dc->communicationType = communicationType;
}


int DECL2 daveWriteManyBytes(daveConnection * dc, int area, int DBnum, int start, int len, void * buffer){
	int res, pos, writeLen;
	uc * pbuf;
	pos = 0;
	if (buffer == NULL) return daveResNoBuffer;
	pbuf = (uc*)buffer;
	res = daveResInvalidLength; //the only chance to return this is when len<=0
	while (len > 0) {
		if (len > dc->maxPDUlength - 28) writeLen = dc->maxPDUlength - 28; else writeLen = len;
		res = daveWriteBytes(dc, area, DBnum, start, writeLen, pbuf);
		if (res != 0) return res;
		len -= writeLen;
		start += writeLen;
		pbuf += writeLen;
	}
	return res;
}

int DECL2 daveWriteBytes(daveConnection * dc, int area, int DB, int start, int len, void * buffer) {
	PDU p1, p2;
	int res;
	if (dc->iface->protocol == daveProtoAS511) {
		return daveWriteS5Bytes(dc, area, DB, start, len, buffer);
	}
	p1.header = dc->msgOut + dc->PDUstartO;
	davePrepareWriteRequest(dc, &p1);
	daveAddVarToWriteRequest(&p1, area, DB, start, len, buffer);
	res = _daveExchange(dc, &p1);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != daveResOK) return res;
	res = _daveTestWriteResult(&p2);
	return res;
}

int DECL2 daveWriteBits(daveConnection * dc, int area, int DB, int start, int len, void * buffer) {
	PDU p1, p2;
	int res;
	p1.header = dc->msgOut + dc->PDUstartO;
	davePrepareWriteRequest(dc, &p1);
	daveAddBitVarToWriteRequest(&p1, area, DB, start, len, buffer);
	res = _daveExchange(dc, &p1);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != 0) return res;
	res = _daveTestWriteResult(&p2);
	return res;
}

/*
	Simplified single bit set:
	*/
int DECL2 daveSetBit(daveConnection * dc, int area, int DB, int byteAdr, int bitAdr) {
	int a = 1;
	return daveWriteBits(dc, area, DB, 8 * byteAdr + bitAdr, 1, &a);
}
/*
	Simplified single bit clear:
	*/
int DECL2 daveClrBit(daveConnection * dc, int area, int DB, int byteAdr, int bitAdr) {
	int a = 0;
	return daveWriteBits(dc, area, DB, 8 * byteAdr + bitAdr, 1, &a);
}


int DECL2 initUpload(daveConnection * dc, char blockType, int blockNr, int * uploadID){
	PDU p1, p2;
	int res;
	if (daveDebug & daveDebugUpload) {
		LOG1("****initUpload\n");
	}
	p1.header = dc->msgOut + dc->PDUstartO;
	_daveConstructUpload(&p1, blockType, blockNr);
	res = _daveExchange(dc, &p1);
	if (daveDebug & daveDebugUpload) {
		LOG2("error:%d\n", res);
		FLUSH;
	}
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != daveResOK) return res;
	*uploadID = p2.param[7];
	return 0;
}


int DECL2 doUpload(daveConnection*dc, int * more, uc**buffer, int*len, int uploadID){
	PDU p1, p2;
	int res, netLen;
	p1.header = dc->msgOut + dc->PDUstartO;
	_daveConstructDoUpload(&p1, uploadID);
	res = _daveExchange(dc, &p1);
	if (daveDebug & daveDebugUpload) {
		LOG2("error:%d\n", res);
		FLUSH;
	}
	*more = 0;
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	*more = p2.param[1];
	if (res != daveResOK) return res;
	//    netLen=p2.data[1] /* +256*p2.data[0]; */ /* for long PDUs, I guess it is so */;
	netLen = p2.data[1] + 256 * p2.data[0]; /* some user confirmed my guess... */;
	if (*buffer) {
		memcpy(*buffer, p2.data + 4, netLen);
		*buffer += netLen;
		if (daveDebug & daveDebugUpload) {
			LOG2("buffer:%p\n", *buffer);
			FLUSH;
		}
	}
	*len += netLen;
	return res;
}

int DECL2 endUpload(daveConnection*dc, int uploadID){
	PDU p1, p2;
	int res;

	p1.header = dc->msgOut + dc->PDUstartO;
	_daveConstructEndUpload(&p1, uploadID);

	res = _daveExchange(dc, &p1);
	if (daveDebug & daveDebugUpload) {
		LOG2("error:%d\n", res);
		FLUSH;
	}
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	return res;
}

/*
	error code to message string conversion:
	*/
char * DECL2 daveStrerror(int code) {
	switch (code) {
	case daveResOK: return "ok";
	case daveResMultipleBitsNotSupported:return "the CPU does not support reading a bit block of length<>1";
	case daveResItemNotAvailable: return "the desired item is not available in the PLC";
	case daveResItemNotAvailable200: return "the desired item is not available in the PLC (200 family)";
	case daveAddressOutOfRange: return "the desired address is beyond limit for this PLC";
	case daveResCPUNoData: return "the PLC returned a packet with no result data";
	case daveUnknownError: return "the PLC returned an error code not understood by this library";
	case daveEmptyResultError: return "this result contains no data";
	case daveEmptyResultSetError: return "cannot work with an undefined result set";
	case daveResCannotEvaluatePDU: return "cannot evaluate the received PDU";
	case daveWriteDataSizeMismatch: return "Write data size error";
	case daveResNoPeripheralAtAddress: return "No data from I/O module";
	case daveResUnexpectedFunc: return "Unexpected function code in answer";
	case daveResUnknownDataUnitSize: return "PLC responds with an unknown data type";

	case daveResShortPacket: return "Short packet from PLC";
	case daveResTimeout: return "Timeout when waiting for PLC response";
	case daveResNoBuffer: return "No buffer provided";
	case daveNotAvailableInS5: return "Function not supported for S5";

	case 0x8000: return "function already occupied.";
	case 0x8001: return "not allowed in current operating status.";
	case 0x8101: return "hardware fault.";
	case 0x8103: return "object access not allowed.";
	case 0x8104: return "context is not supported. Step7 says:Function not implemented or error in telgram.";
	case 0x8105: return "invalid address.";
	case 0x8106: return "data type not supported.";
	case 0x8107: return "data type not consistent.";
	case 0x810A: return "object does not exist.";
	case 0x8301: return "insufficient CPU memory ?";
	case 0x8402: return "CPU already in RUN or already in STOP ?";
	case 0x8404: return "severe error ?";
	case 0x8500: return "incorrect PDU size.";
	case 0x8702: return "address invalid.";;
	case 0xd002: return "Step7:variant of command is illegal.";
	case 0xd004: return "Step7:status for this command is illegal.";
	case 0xd0A1: return "Step7:function is not allowed in the current protection level.";
	case 0xd201: return "block name syntax error.";
	case 0xd202: return "syntax error function parameter.";
	case 0xd203: return "syntax error block type.";
	case 0xd204: return "no linked block in storage medium.";
	case 0xd205: return "object already exists.";
	case 0xd206: return "object already exists.";
	case 0xd207: return "block exists in EPROM.";
	case 0xd209: return "block does not exist/could not be found.";
	case 0xd20e: return "no block present.";
	case 0xd210: return "block number too big.";
		//	case 0xd240: return "unfinished block transfer in progress?";  // my guess
	case 0xd240: return "Coordination rules were violated.";
		/*  Multiple functions tried to manipulate the same object.
			Example: a block could not be copied,because it is already present in the target system
			and
			*/
	case 0xd241: return "Operation not permitted in current protection level.";
		/**/	case 0xd242: return "protection violation while processing F-blocks. F-blocks can only be processed after password input.";
		case 0xd401: return "invalid SZL ID.";
		case 0xd402: return "invalid SZL index.";
		case 0xd406: return "diagnosis: info not available.";
		case 0xd409: return "diagnosis: DP error.";
		case 0xdc01: return "invalid BCD code or Invalid time format?";
		default: return "no message defined!";
	}
}

/*
	Copy an internal String into an external string buffer. This is needed to interface
	with Visual Basic. Maybe it is helpful elsewhere, too.
	*/
void DECL2 daveStringCopy(char * intString, char * extString) {
	strncpy(extString, intString, 255);	// arbritray limit. I hope each external string has at least this 
	// capacity
}

/*
	I'm not quite sure whether this is all correct, but it seems to work for all numbers I tested
	*/
float DECL2 daveGetKGAt(daveConnection * dc, int pos) {
	char kgExponent;
	int sign;
	union {
		uc b[4];
		int mantissa;
	} f;
	union {
		int a;
		float f;
	} v;
	uc* p = dc->_resultPointer + pos;
	kgExponent = *p;
	p++;
#ifdef DAVE_LITTLE_ENDIAN    
	f.b[3] = 0;
	f.b[2] = *p;
	p++;
	f.b[1] = *p;
	p++;
	f.b[0] = *p;
	sign = (f.b[2] & 0x80);
	f.b[2] &= 0x7f;
#else        
	f.b[0] = 0;
	f.b[1] = *p;
	p++;
	f.b[2] = *p;
	p++;
	f.b[3] = *p;
	sign = (f.b[1] & 0x80);
	f.b[1] &= 0x7f;
#endif    
	p++;
	LOG3("daveGetKG(dc:%p, mantissa:0x%08X)\n", dc, f.mantissa);
	if (sign) {
		f.mantissa = f.mantissa ^ 0xffffffff;
		f.mantissa = f.mantissa + 0x00800000;
	}
	v.f = f.mantissa;
	if (sign) {
		v.f = -v.f;
	}
	LOG5("daveGetKG(dc:%p, mantissa:0x%08X exponent:0x%02X %0.8f)\n", dc, f.mantissa, kgExponent, v.f);
	while (kgExponent > 23) {
		v.f = v.f*2.0;
		kgExponent--;
	}
	while (kgExponent < 23) {
		v.f = v.f / 2.0;
		kgExponent++;
	}
	LOG2("daveGetKG(%08X)\n", v.a);
	v.f = -v.f;
	LOG2("daveGetKG(%08X)\n", v.a);
	v.f = -v.f;
#ifdef DEBUG_CALLS
	LOG3("daveGetKG(dc:%p, result:%0.6f)\n", dc, v.f);
	FLUSH;
#endif	    
	return (v.f);
}

float DECL2 daveGetKG(daveConnection * dc) {
	float f;
	f = daveGetKGAt(dc, ((int)dc->resultPointer - (int)dc->_resultPointer));
	dc->resultPointer += 4;
	return f;
}

float DECL2 daveGetFloat(daveConnection * dc) {
	union {
		float a;
		uc b[4];
	} f;
#ifdef DAVE_LITTLE_ENDIAN    
	f.b[3] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[0] = *(dc->resultPointer);
#else        
	f.b[0] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	f.b[3] = *(dc->resultPointer);
#endif    
	dc->resultPointer++;
#ifdef DEBUG_CALLS
	LOG3("daveGetFloat(dc:%p, result:%0.6f)\n", dc, f.a);
	FLUSH;
#endif	    
	return (f.a);
}

float DECL2 daveGetFloatAt(daveConnection * dc, int pos) {
	union {
		float a;
		uc b[4];
	} f;
	uc* p = (uc*)dc->_resultPointer;
	p += pos;
#ifdef DAVE_LITTLE_ENDIAN
	f.b[3] = *p; p++;
	f.b[2] = *p; p++;
	f.b[1] = *p; p++;
	f.b[0] = *p;
#else    
	f.b[0] = *p; p++;
	f.b[1] = *p; p++;
	f.b[2] = *p; p++;
	f.b[3] = *p;
#endif    
	return (f.a);
}

float DECL2 toPLCfloat(float ff) {
#ifdef DAVE_LITTLE_ENDIAN    
	union {
		float a;
		uc b[4];
	} f;
	uc c;

	f.a = ff;
	c = f.b[0];
	f.b[0] = f.b[3];
	f.b[3] = c;
	c = f.b[1];
	f.b[1] = f.b[2];
	f.b[2] = c;

	//    f.a=ff;  //fixed bug found by luca at ventisei
#ifdef DEBUG_CALLS
	LOG3("toPLCfloat(%0.6f) = %0.6f\n", ff, f.a);
	FLUSH;
#endif	    
	return (f.a);
#else    
#ifdef DEBUG_CALLS
	LOG3("toPLCfloat(%0.6f) = %0.6f\n", ff, ff);
	FLUSH;
#endif	    
	return ff;
#endif        
}

int DECL2 daveToPLCfloat(float ff) {
	union {
		float a;
		uc b[4];
		int c;
	} f;
#ifdef DAVE_LITTLE_ENDIAN    
	uc c;
	f.a = ff;
	c = f.b[0];
	f.b[0] = f.b[3];
	f.b[3] = c;
	c = f.b[1];
	f.b[1] = f.b[2];
	f.b[2] = c;
#else    
	f.a = ff;
#endif    
#ifdef DEBUG_CALLS
	LOG3("toPLCfloat(%0.6f) = %08x\n", ff, f.c);
	FLUSH;
#endif	    
	return (f.c);
}

int DECL2 daveToKG(float ff) {
	union {
		uc b[4];
		int c;
	} f, f2;
	char kgExponent = 23;
	LOG2("daveToKG(%0.8f)\n", ff);
	if (ff == 0.0) {
		f.c = 0;
		return 0;
	}
	f2.c = (int)ff;	// attention! what does this cast? I do want to take the integer part of that float, NOT reinterpret the bit pattern as int!
	LOG4("daveToKG(mantissa:0x%08X exponent:0x%02X %0.8f)\n", f2.c, kgExponent, ff);
	while (f2.c > 0x00400000){
		ff /= 2;
		f2.c = (int)ff;	// attention! what does this cast? I do want to take the integer part of that float, NOT reinterpret the bit pattern as int!
		kgExponent++;
	}
	while (f2.c < 0x00400000){
		ff *= 2;
		f2.c = (int)ff;  	// attention! what does this cast? I do want to take the integer part of that float, NOT reinterpret the bit pattern as int!
		kgExponent--;
	}
	LOG4("daveToKG(mantissa:0x%08X exponent:0x%02X %0.8f)\n", f2.c, kgExponent, ff);
	f.b[0] = kgExponent;
#ifdef DAVE_LITTLE_ENDIAN    
	f.b[1] = f2.b[2];
	f.b[2] = f2.b[1];
	f.b[3] = f2.b[0];
#else    
	f.b[3] = f2.b[3];
	f.b[2] = f2.b[2];
	f.b[1] = f2.b[1];
#endif    
#ifdef DEBUG_CALLS
	LOG3("daveToKG(%0.6f) = %08x\n", ff, f.c);
	FLUSH;
#endif	    
	return (f.c);
}


short DECL2 daveSwapIed_16(short ff) {
#ifdef DAVE_LITTLE_ENDIAN
	union {
		short a;
		uc b[2];
	} f;
	uc c;
	f.a = ff;
	c = f.b[0];
	f.b[0] = f.b[1];
	f.b[1] = c;
	return (f.a);
#else
	//    printf("Here we are in BIG ENDIAN!!!\n");
	return (ff);
#endif    
}

int DECL2 daveSwapIed_32(int ff) {
#ifdef DAVE_LITTLE_ENDIAN
	union {
		int a;
		uc b[4];
	} f;
	uc c;
	f.a = ff;
	c = f.b[0];
	f.b[0] = f.b[3];
	f.b[3] = c;
	c = f.b[1];
	f.b[1] = f.b[2];
	f.b[2] = c;
	return f.a;
#else
	//    printf("Here we are in BIG ENDIAN!!!\n");
	return ff;
#endif       
}

/**
	Timer and Counter conversion functions:
	**/
/*
	get time in seconds from current read position:
	*/
float DECL2 daveGetSeconds(daveConnection * dc) {
	uc b[2], a;
	float f;
	b[1] = *(dc->resultPointer)++;
	b[0] = *(dc->resultPointer)++;
	f = b[0] & 0xf;
	f += 10 * ((b[0] & 0xf0) >> 4);
	f += 100 * (b[1] & 0xf);
	a = ((b[1] & 0xf0) >> 4);
	switch (a) {
	case 0: f *= 0.01; break;
	case 1: f *= 0.1; break;
	case 3: f *= 10.0; break;
	}
	return (f);
}
/*
	get time in seconds from random position:
	*/
float DECL2 daveGetSecondsAt(daveConnection * dc, int pos) {
	float f;
	uc b[2], a;
	uc* p = (uc*)dc->_resultPointer;
	p += pos;
	b[1] = *p;
	p++;
	b[0] = *p;
	f = b[0] & 0xf;
	f += 10 * ((b[0] & 0xf0) >> 4);
	f += 100 * (b[1] & 0xf);
	a = ((b[1] & 0xf0) >> 4);
	switch (a) {
	case 0: f *= 0.01; break;
	case 1: f *= 0.1; break;
	case 3: f *= 10.0; break;
	}
	return (f);
}
/*
	get counter value from current read position:
	*/
int DECL2 daveGetCounterValue(daveConnection * dc) {
	uc b[2];
	int f;
	b[1] = *(dc->resultPointer)++;
	b[0] = *(dc->resultPointer)++;
	f = b[0] & 0xf;
	f += 10 * ((b[0] & 0xf0) >> 4);
	f += 100 * (b[1] & 0xf);
	return (f);
}
/*
	get counter value from random read position:
	*/
int DECL2 daveGetCounterValueAt(daveConnection * dc, int pos){
	int f;
	uc b[2];
	uc* p = (uc*)dc->_resultPointer;
	p += pos;
	b[1] = *p;
	p++;
	b[0] = *p;
	f = b[0] & 0xf;
	f += 10 * ((b[0] & 0xf0) >> 4);
	f += 100 * (b[1] & 0xf);
	return (f);
}

/*
	dummy functions for protocols not providing a specific function:
	*/

int DECL2 _daveReturnOkDummy(daveInterface * di){
	return 0;
}

int DECL2 _daveReturnOkDummy2(daveConnection * dc){
	return 0;
}

int DECL2 _daveListReachablePartnersDummy(daveInterface * di, char * buf) {
	return 0;
}

/*
	MPI specific functions:
	*/

/*
	This writes a single chracter to the serial interface:
	*/

void DECL2 _daveSendSingle(daveInterface * di,	/* serial interface */
	uc c  			/* chracter to be send */
	)
{
	di->ifwrite(di, (char*)&c, 1);
}

int DECL2 _daveReadSingle(daveInterface * di) {
	char res;
	int i;
	i = di->ifread(di, &res, 1);
	if ((daveDebug & daveDebugSpecialChars) != 0)
		LOG3("readSingle %d chars. 1st %02X\n", i, res);
	if (i == 1) return res;
	return 0;
}

int DECL2 _daveReadMPI(daveInterface * di, uc *b) {
	int res = 0, state = 0, nr_read;
	uc bcc = 0;
rep:
	{
		nr_read = di->ifread(di, (char*)(b + res), 1);
		if (nr_read == 0) return 0;
		res += nr_read;
		if ((res == 1) && (*(b + res - 1) == DLE)) {
			if ((daveDebug & daveDebugSpecialChars) != 0)
				LOG1("readMPI single DLE!\n");
			return 1;
		}
		if ((res == 1) && (*(b + res - 1) == STX)) {
			if ((daveDebug & daveDebugSpecialChars) != 0)
				LOG1("readMPI single STX!\n");
			return 1;
		}
		if (*(b + res - 1) == DLE) {
			if (state == 0) {
				state = 1;
				/*		    if ((daveDebug & daveDebugSpecialChars)!=0)
							LOG1("readMPI 1st DLE in data.\n")
							;
							*/
			}
			else if (state == 1) {
				state = 0;
				res--;		/* forget this DLE, it is the second of a pair */
				/*		    if ((daveDebug & daveDebugSpecialChars)!=0)
								LOG1("readMPI 2nd DLE in data.\n")
								;
								*/
			}
		}
		if (state == 3) {
			if ((daveDebug & daveDebugSpecialChars) != 0)
				LOG4("readMPI: packet size %d, got BCC: %x. I calc: %x\n", res, *(b + res - 1), bcc);
			if ((daveDebug & daveDebugRawRead) != 0)
				_daveDump("answer", b, res);
			return res;
		}
		else {
			bcc = bcc ^ (*(b + res - 1));
		}

		if (*(b + res - 1) == ETX) if (state == 1) {
			state = 3;
			if ((daveDebug & daveDebugSpecialChars) != 0)
				LOG1("readMPI: DLE ETX,packet end.\n");
		}
		goto rep;
	}
}

int DECL2 _daveGetAck(daveConnection * dc) {
	int res;
	daveInterface * di = dc->iface;
	int nr = dc->needAckNumber;
	uc b1[daveMaxRawLen];
	if (daveDebug & daveDebugPacket)
		LOG2("%s enter getAck ack\n", di->name);
	res = _daveReadMPI(di, b1);
	if (res < 0) return res - 10;
	if (res != di->ackPos + 6) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG4("%s *** getAck wrong length %d for ack. Waiting for %d\n dump:", di->name, res, nr);
			_daveDump("wrong ack:", b1, res);
		}
		return -1;
	}
	if (b1[di->ackPos] != 0xB0) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG3("%s *** getAck char[6] %x no ack\n", di->name, b1[di->ackPos + 2]);
		}
		return -2;
	}
	if (b1[di->ackPos + 2] != nr) {
		if (daveDebug & daveDebugPrintErrors) {
			LOG4("%s *** getAck got: %d need: %d\n", di->name, b1[di->ackPos + 2], nr);
		}
		return -3;
	}
	return 0;
}


#define tmo_normal 95000
/*
	This reads up to max chracters when it can get them and returns the number:
	*/
int DECL2 _daveReadChars2(daveInterface * di,	/* serial interface */
	uc *b, 		/* a buffer */
	int max		/* limit */
	)
{
	return di->ifread(di, (char*)b, max);
}

/*
	This sends a string after doubling DLEs in the String
	and adding DLE,ETX and bcc.
	*/
int DECL2 _daveSendWithCRC(daveInterface * di, /* serial interface */
	uc *b, 		 /* a buffer containing the message */
	int size		 /* the size of the string */
	)
{
	uc target[daveMaxRawLen];
	int i, targetSize = 0;
	int bcc = DLE^ETX; /* preload */
	for (i = 0; i < size; i++) {
		target[targetSize] = b[i]; targetSize++;
		if (DLE == b[i]) {
			target[targetSize] = DLE;
			targetSize++;
		}
		else
			bcc = bcc^b[i];	/* The doubled DLE effectively contributes nothing */
	};
	target[targetSize] = DLE;
	target[targetSize + 1] = ETX;
	target[targetSize + 2] = bcc;
	targetSize += 3;
	//    daveWriteFile(di->fd.wfd, target, targetSize, wr);
	di->ifwrite(di, (char*)target, targetSize);
	if (daveDebug & daveDebugPacket)
		_daveDump("_daveSendWithCRC", target, targetSize);
	return 0;
}

/*
	This adds a prefix to a string and theen sends it
	after doubling DLEs in the String
	and adding DLE,ETX and bcc.
	*/
int DECL2 _daveSendWithPrefix(daveConnection * dc, uc *b, int size)
{
	uc target[daveMaxRawLen];
	uc fix[] = { 04, 0x80, 0x80, 0x0C, 0x03, 0x14 };
	uc fix2[] = { 0x00, 0x0c, 0x03, 0x03 };
	if (dc->iface->protocol == daveProtoMPI2) {
		fix2[2] = dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
		fix2[3] = dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
		memcpy(target, fix2, sizeof(fix2));
		memcpy(target + sizeof(fix2), b, size);
		return _daveSendWithCRC(dc->iface, target, size + sizeof(fix2));
	}
	else {
		fix[4] = dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
		fix[5] = dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
		memcpy(target, fix, sizeof(fix));
		memcpy(target + sizeof(fix), b, size);
		target[1] |= dc->MPIAdr;
		//	target[2]|=dc->iface->localMPI;
		memcpy(target + sizeof(fix), b, size);
		return _daveSendWithCRC(dc->iface, target, size + sizeof(fix));
	}
}

int DECL2 _daveSendWithPrefix2(daveConnection * dc, int size)
{
	uc fix[] = { 04, 0x80, 0x80, 0x0C, 0x03, 0x14 };
	uc fix2[] = { 0x00, 0x0C, 0x03, 0x03 };

	if (dc->iface->protocol == daveProtoMPI2) {
		fix2[2] = dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
		fix2[3] = dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
		memcpy(dc->msgOut, fix2, sizeof(fix2));
		dc->msgOut[sizeof(fix2)] = 0xF1;
		return _daveSendWithCRC(dc->iface, dc->msgOut, size + sizeof(fix2));
	}
	else if (dc->iface->protocol == daveProtoMPI) {
		fix[4] = dc->connectionNumber2;		// 1/10/05 trying Andrew's patch
		fix[5] = dc->connectionNumber;		// 1/10/05 trying Andrew's patch
		memcpy(dc->msgOut, fix, sizeof(fix));
		dc->msgOut[1] |= dc->MPIAdr;
		//	dc->msgOut[2]|=dc->iface->localMPI; //???
		dc->msgOut[sizeof(fix)] = 0xF1;
		/*	if (daveDebug & daveDebugPacket)
				_daveDump("_daveSendWithPrefix2",dc->msgOut,size+sizeof(fix)); */
		return _daveSendWithCRC(dc->iface, dc->msgOut, size + sizeof(fix));
	}
	return -1; /* shouldn't happen. */
}

/*
	Sends an ackknowledge message for the message number nr:
	*/
int DECL2 _daveSendAck(daveConnection * dc, int nr)
{
	uc m[3];
	if (daveDebug & daveDebugPacket)
		LOG3("%s sendAck for message %d \n", dc->iface->name, nr);
	m[0] = 0xB0;
	m[1] = 0x01;
	m[2] = nr;
	return _daveSendWithPrefix(dc, m, 3);
}

/*
	Handle MPI message numbers in a central place:
	*/
int DECL2 _daveIncMessageNumber(daveConnection * dc) {
	int res = dc->messageNumber++;
	if (daveDebug & daveDebugPacket)
		LOG2("_daveIncMessageNumber new number %d \n", dc->messageNumber);
	if ((dc->messageNumber) == 0) dc->messageNumber = 1;
	return res;
}
/*
	Executes part of the dialog necessary to send a message:
	*/
int DECL2 _daveSendDialog2(daveConnection * dc, int size)
{
	int a;
	_daveSendSingle(dc->iface, STX);
	if (_daveReadSingle(dc->iface) != DLE) {
		if (daveDebug & daveDebugPrintErrors)
			LOG2("%s *** no DLE before send.\n", dc->iface->name);
		_daveSendSingle(dc->iface, DLE);
		if (_daveReadSingle(dc->iface) != DLE) {
			if (daveDebug & daveDebugPrintErrors)
				LOG2("%s retry*** no DLE before send.\n", dc->iface->name);
			return -1;
		}
	}
	if (size > 5){
		dc->needAckNumber = dc->messageNumber;
		dc->msgOut[dc->iface->ackPos + 1] = _daveIncMessageNumber(dc);
	}
	_daveSendWithPrefix2(dc, size);
	a = _daveReadSingle(dc->iface);
	if (a != DLE) {
		LOG3("%s *** no DLE after send(1) %02x.\n", dc->iface->name, a);
		a = _daveReadSingle(dc->iface);
		if (a != DLE) {
			LOG3("%s *** no DLE after send(2) %02x.\n", dc->iface->name, a);
			_daveSendWithPrefix2(dc, size);
			a = _daveReadSingle(dc->iface);
			if (a != DLE) {
				LOG3("%s *** no DLE after resend(3) %02x.\n", dc->iface->name, a);
				_daveSendSingle(dc->iface, STX);
				a = _daveReadSingle(dc->iface);
				if (a != DLE) {
					LOG2("%s *** no DLE before resend.\n", dc->iface->name);
					return -1;
				}
				else {
					_daveSendWithPrefix2(dc, size);
					a = _daveReadSingle(dc->iface);
					if (a != DLE) {
						LOG2("%s *** no DLE before resend.\n", dc->iface->name);
						return -1;
					}
					else {
						LOG2("%s *** got DLE after repeating whole transmisson.\n", dc->iface->name);
						return 0;
					}
				}
			}
			else
				LOG3("%s *** got DLE after resend(3) %02x.\n", dc->iface->name, a);
		}

	}
	return 0;
}


/*
	Changes:
	07/19/04 removed unused vars.
	*/

/*
	Changes:
	07/19/04 added return values in daveInitStep and daveSendWithPrefix2.
	09/09/04 applied patch for variable Profibus speed from Andrew Rostovtsew.
	*/

/* PPI specific functions: */
#define tmo_normalPPI 140000

int DECL2 _daveSendLengthAndIt(daveInterface * di, int len, uc *szContent) {
	// 长度区域， 68 长度 长度 68  共4个字节！如：68 1B 1B 68 
	uc c[] = { 104, 0, 0, 104 };
	c[1] = len;
	c[2] = len;

	// 内容区域，如：02 00 6C 32 01 00 00 FF FF 00 0E 00 00 04 01 12 0A 10 02 00 02 00 01 84 00 00 08 6F 16 
	int i;
	int size = len;
	unsigned char * b = szContent;
	us sum = 0;
	for (i = 0; i < size; i++) {
		sum += b[i];
	}
	sum = sum & 0xff;
	b[size] = sum;
	size++;
	b[size] = SYN;
	size++;

	unsigned char szSendMsg[1024];
	memcpy(szSendMsg, c, 4);
	memcpy(szSendMsg + 4, b, size);
	int nRet = di->ifwrite(di, (char *)szSendMsg, 4 + size);
	if ((daveDebug & daveDebugByte) != 0) {
		_daveDump("I send", szSendMsg, 4 + size);
	}
	return nRet;
}

void DECL2 _daveSendIt(daveInterface * di, uc * b, int size) {
	int i;
	us sum = 0;
	for (i = 0; i < size; i++) {
		sum += b[i];
	}
	sum = sum & 0xff;
	b[size] = sum;
	size++;
	b[size] = SYN;
	size++;
	di->ifwrite(di, (char*)b, size);

	if ((daveDebug & daveDebugByte) != 0) {
		LOG2("send %d\n", i);
		_daveDump("I send", b, size);
	}
}

int DECL2 _daveSendRequestData(daveConnection * dc, int alt) {
	// 先组织第一个字节 0x10
	uc b[] = { DLE, 0, 0, 0x5C, 0, 0 };
	b[1] = dc->MPIAdr;
	b[2] = dc->iface->localMPI;
	if (alt) 
		b[3] = 0x7c; 
	else 
		b[3] = 0x5c;

	// 在组织后面的字节:02 00 5C 5E 16
	int size = sizeof(b) - 3;
	unsigned char *szContent = b + 1;
	int i;
	us sum = 0;
	for (i = 0; i < size; i++) {
		sum += szContent[i];
	}
	sum = sum & 0xff;
	szContent[size] = sum;
	size++;
	szContent[size] = SYN;
	size++;

	int nRet = dc->iface->ifwrite(dc->iface, (char*)b, 6);
	return nRet;
}


/*
	"generic" functions calling the protocol specific ones (or the dummies)
	*/
int DECL2 daveInitAdapter(daveInterface * di) {
	return di->initAdapter(di);
}

int DECL2 daveConnectPLC(daveConnection * dc) {
	return dc->iface->connectPLC(dc);
}

int DECL2 daveDisconnectPLC(daveConnection * dc) {
	return dc->iface->disconnectPLC(dc);
}

int DECL2 daveDisconnectAdapter(daveInterface * di) {
	return di->disconnectAdapter(di);
}

int DECL2 _daveExchange(daveConnection * dc, PDU *p) {
	int res;
	//printf("200\n");
	if ((p->header[4] == 0) && (p->header[5] == 0)) { /* do not number already numbered PDUs 12/10/04 */
		dc->PDUnumber++;
		if (daveDebug & daveDebugExchange) {
			LOG2("_daveExchange PDU number: %d\n", dc->PDUnumber);
		}
		p->header[5] = dc->PDUnumber % 256;	// test!
		p->header[4] = dc->PDUnumber / 256;	// test!
	}
	//printf("201\n");
	res = dc->iface->exchange(dc, p);
	//printf("202\n");
	if (((daveDebug & daveDebugExchange) != 0) || ((daveDebug & daveDebugErrorReporting) != 0)) {
		LOG2("result of exchange: %d\n", res);
	}
	return res;
}

int DECL2 daveSendMessage(daveConnection * dc, PDU *p) {
	return dc->iface->sendMessage(dc, p);
}

int DECL2 daveListReachablePartners(daveInterface * di, char * buf) {
	return di->listReachablePartners(di, buf);
}

int DECL2 daveGetResponse(daveConnection * dc) {
	return dc->iface->getResponse(dc);
}

/**
	Newer conversion routines. As the terms WORD, INT, INTEGER etc have different meanings
	for users of different programming languages and compilers, I choose to provide a new
	set of conversion routines named according to the bit length of the value used. The 'U'
	or 'S' stands for unsigned or signed.
	**/
/*
	Get a value from the position b points to. B is typically a pointer to a buffer that has
	been filled with daveReadBytes:
	*/
int DECL2 daveGetS8from(uc *b) {
	char* p = (char*)b;
	return *p;
}

int DECL2 daveGetU8from(uc *b) {
	return *b;
}

int DECL2 daveGetS16from(uc *b) {
	union {
		short a;
		uc b[2];
	} u;
#ifdef DAVE_LITTLE_ENDIAN    
	u.b[1] = *b;
	b++;
	u.b[0] = *b;
#else
	u.b[0] = *b;
	b++;
	u.b[1] = *b;
#endif
	return u.a;
}

int DECL2 daveGetU16from(uc *b) {
	union {
		unsigned short a;
		uc b[2];
	} u;
#ifdef DAVE_LITTLE_ENDIAN    
	u.b[1] = *b;
	b++;
	u.b[0] = *b;
#else
	u.b[0] = *b;
	b++;
	u.b[1] = *b;
#endif    
	return u.a;
}

int DECL2 daveGetS32from(uc *b) {
	union {
		int a;
		uc b[4];
	} u;
#ifdef DAVE_LITTLE_ENDIAN
	u.b[3] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[0] = *b;
#else
	u.b[0] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[3] = *b;
#endif
	return u.a;
}

unsigned int DECL2 daveGetU32from(uc *b) {
	union {
		unsigned int a;
		uc b[4];
	} u;
#ifdef DAVE_LITTLE_ENDIAN
	u.b[3] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[0] = *b;
#else
	u.b[0] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[3] = *b;
#endif    
	return u.a;
}

float DECL2 daveGetFloatfrom(uc *b) {
	union {
		float a;
		uc b[4];
	} u;
#ifdef DAVE_LITTLE_ENDIAN
	u.b[3] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[0] = *b;
#else
	u.b[0] = *b;
	b++;
	u.b[1] = *b;
	b++;
	u.b[2] = *b;
	b++;
	u.b[3] = *b;
#endif    
	return u.a;
}

/*
	Get a value from the current position in the last result read on the connection dc.
	This will increment an internal pointer, so the next value is read from the position
	following this value.
	*/
int DECL2 daveGetS8(daveConnection * dc) {
	char * p;
#ifdef DEBUG_CALLS
	LOG2("daveGetS8(dc:%p)\n", dc);
	FLUSH;
#endif	
	p = (char *)dc->resultPointer;
	dc->resultPointer++;
	return *p;
}

int DECL2 daveGetU8(daveConnection * dc) {
	uc * p;
#ifdef DEBUG_CALLS
	LOG2("daveGetU8(dc:%p)\n", dc);
	FLUSH;
#endif	
	p = dc->resultPointer;
	dc->resultPointer++;
	return *p;
}

int DECL2 daveGetS16(daveConnection * dc) {
	union {
		short a;
		uc b[2];
	} u;
#ifdef DEBUG_CALLS
	LOG2("daveGetS16(dc:%p)\n", dc);
	FLUSH;
#endif	
#ifdef DAVE_LITTLE_ENDIAN
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[0] = *(dc->resultPointer);
#else
	u.b[0] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
#endif
	dc->resultPointer++;
	return u.a;
}

int DECL2 daveGetU16(daveConnection * dc) {
	union {
		unsigned short a;
		uc b[2];
	} u;
#ifdef DEBUG_CALLS
	LOG2("daveGetU16(dc:%p)\n", dc);
	FLUSH;
#endif	    
#ifdef DAVE_LITTLE_ENDIAN
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[0] = *(dc->resultPointer);
#else
	u.b[0] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
#endif    
	dc->resultPointer++;
	return u.a;
}

int DECL2 daveGetS32(daveConnection * dc) {
	union {
		int a;
		uc b[4];
	} u;
#ifdef DEBUG_CALLS
	LOG2("daveGetS32(dc:%p)\n", dc);
	FLUSH;
#endif	    
#ifdef DAVE_LITTLE_ENDIAN
	u.b[3] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[0] = *(dc->resultPointer);
#else
	u.b[0] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[3] = *(dc->resultPointer);
#endif
	dc->resultPointer++;
	return u.a;
}

unsigned int DECL2 daveGetU32(daveConnection * dc) {
	union {
		unsigned int a;
		uc b[4];
	} u;
#ifdef DEBUG_CALLS
	LOG2("daveGetU32(dc:%p)\n", dc);
	FLUSH;
#endif	    

#ifdef DAVE_LITTLE_ENDIAN
	u.b[3] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[0] = *(dc->resultPointer);
#else
	u.b[0] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[1] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[2] = *(dc->resultPointer);
	dc->resultPointer++;
	u.b[3] = *(dc->resultPointer);
#endif    
	dc->resultPointer++;
	return u.a;
}
/*
	Get a value from a given position in the last result read on the connection dc.
	*/
int DECL2 daveGetS8At(daveConnection * dc, int pos) {
	char * p = (char *)(dc->_resultPointer);
	p += pos;
	return *p;
}

int DECL2 daveGetU8At(daveConnection * dc, int pos)  {
	uc * p = (uc *)(dc->_resultPointer);
	p += pos;
	return *p;
}

int DECL2 daveGetS16At(daveConnection * dc, int pos) {
	union {
		short a;
		uc b[2];
	} u;
	uc * p = (uc *)(dc->_resultPointer);
	p += pos;
#ifdef DAVE_LITTLE_ENDIAN
	u.b[1] = *p;
	p++;
	u.b[0] = *p;
#else
	u.b[0] = *p;
	p++;
	u.b[1] = *p;
#endif
	return u.a;
}

int DECL2 daveGetU16At(daveConnection * dc, int pos) {
	union {
		unsigned short a;
		uc b[2];
	} u;
	uc * p = (uc *)(dc->_resultPointer);
	p += pos;
#ifdef DAVE_LITTLE_ENDIAN
	u.b[1] = *p;
	p++;
	u.b[0] = *p;
#else
	u.b[0] = *p;
	p++;
	u.b[1] = *p;
#endif    
	return u.a;
}

int DECL2 daveGetS32At(daveConnection * dc, int pos) {
	union {
		int a;
		uc b[4];
	} u;
	uc * p = dc->_resultPointer;
	p += pos;
#ifdef DAVE_LITTLE_ENDIAN    
	u.b[3] = *p;
	p++;
	u.b[2] = *p;
	p++;
	u.b[1] = *p;
	p++;
	u.b[0] = *p;
#else
	u.b[0] = *p;
	p++;
	u.b[1] = *p;
	p++;
	u.b[2] = *p;
	p++;
	u.b[3] = *p;
#endif
	return u.a;
}

unsigned int DECL2 daveGetU32At(daveConnection * dc, int pos) {
	union {
		unsigned int a;
		uc b[4];
	} u;
	uc * p = (uc *)(dc->_resultPointer);
	p += pos;
#ifdef DAVE_LITTLE_ENDIAN    
	u.b[3] = *p;
	p++;
	u.b[2] = *p;
	p++;
	u.b[1] = *p;
	p++;
	u.b[0] = *p;
#else
	u.b[0] = *p;
	p++;
	u.b[1] = *p;
	p++;
	u.b[2] = *p;
	p++;
	u.b[3] = *p;
#endif    
	return u.a;
}
/*
	put one byte into buffer b:
	*/
uc * DECL2 davePut8(uc *b, int v) {
	*b = v & 0xff;
	b++;
	return b;
}

uc * DECL2 davePut16(uc *b, int v) {
	union {
		short a;
		uc b[2];
	} u;
	u.a = v;
#ifdef DAVE_LITTLE_ENDIAN    
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
#endif
	b++;
	return b;
}

uc * DECL2 davePut32(uc *b, int v) {
	union {
		int a;
		uc b[2];
	} u;
	u.a = v;
#ifdef DAVE_LITTLE_ENDIAN        
	*b = u.b[3];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[3];
#endif    
	b++;
	return b;
}

uc * DECL2 davePutFloat(uc *b, float v) {
	union {
		float a;
		uc b[2];
	} u;
	u.a = v;
#ifdef DAVE_LITTLE_ENDIAN        
	*b = u.b[3];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[3];
#endif
	b++;
	return b;
}

void DECL2 davePut8At(uc *b, int pos, int v) {
	union {
		short a;
		uc b[2];
	} u;
	u.a = v;
	b += pos;
	*b = v & 0xff;
}

void DECL2 davePut16At(uc *b, int pos, int v) {
	union {
		short a;
		uc b[2];
	} u;
	u.a = v;
	b += pos;
#ifdef DAVE_LITTLE_ENDIAN        
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
#endif    
}

void DECL2 davePut32At(uc *b, int pos, int v) {
	union {
		int a;
		uc b[2];
	} u;
	u.a = v;
	b += pos;
#ifdef DAVE_LITTLE_ENDIAN        
	*b = u.b[3];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[3];
#endif    
}

void DECL2 davePutFloatAt(uc *b, int pos, float v) {
	union {
		float a;
		uc b[2];
	} u;
	u.a = v;
	b += pos;
#ifdef DAVE_LITTLE_ENDIAN        
	*b = u.b[3];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[0];
#else    
	*b = u.b[0];
	b++;
	*b = u.b[1];
	b++;
	*b = u.b[2];
	b++;
	*b = u.b[3];
#endif    
}
/*
	"passive mode" functions. Needed to "simulate" an S7 PLC.
	*/
userReadFunc readCallBack = NULL;
userWriteFunc writeCallBack = NULL;

void DECL2 _daveConstructReadResponse(PDU * p) {
	uc pa[] = { 4, 1 };
	uc da[] = { 0xFF, 4, 0, 0 };
	_daveInitPDUheader(p, 3);
	_daveAddParam(p, pa, sizeof(pa));
	_daveAddData(p, da, sizeof(da));
}

void DECL2 _daveConstructBadReadResponse(PDU * p) {
	uc pa[] = { 4, 1 };
	uc da[] = { 0x0A, 0, 0, 0 };
	_daveInitPDUheader(p, 3);
	_daveAddParam(p, pa, sizeof(pa));
	_daveAddData(p, da, sizeof(da));
}

void DECL2 _daveConstructWriteResponse(PDU * p) {
	uc pa[] = { 5, 1 };
	uc da[] = { 0xFF };
	_daveInitPDUheader(p, 3);
	_daveAddParam(p, pa, sizeof(pa));
	_daveAddData(p, da, sizeof(da));
}

void DECL2 _daveHandleRead(PDU * p1, PDU * p2) {
	int result;
	uc * userBytes = NULL; //is this really better than reading from a dangling pointer?
	int bytes = 0x100 * p1->param[6] + p1->param[7];
	int DBnumber = 0x100 * p1->param[8] + p1->param[9];
	int area = p1->param[10];
	int start = 0x10000 * p1->param[11] + 0x100 * p1->param[12] + p1->param[13];
	LOG5("read %d bytes from %s %d beginning at %d.\n",
		bytes, daveAreaName(area), DBnumber, start);
	if (readCallBack)
		userBytes = readCallBack(area, DBnumber, start, bytes, &result);
	_daveConstructReadResponse(p2);
	_daveAddValue(p2, userBytes, bytes);
	_daveDumpPDU(p2);
};

void DECL2 _daveHandleWrite(PDU * p1, PDU * p2) {
	int result, bytes = 0x100 * p1->param[6] + p1->param[7];
	int DBnumber = 0x100 * p1->param[8] + p1->param[9];
	int area = p1->param[10];
	int start = 0x10000 * p1->param[11] + 0x100 * p1->param[12] + p1->param[13];
	LOG5("write %d bytes to %s %d beginning at %d.\n",
		bytes, daveAreaName(area), DBnumber, start);
	if (writeCallBack)
		writeCallBack(area, DBnumber, start, bytes, &result, p1->data + 4);
	LOG1("after callback\n");
	FLUSH;
	_daveConstructWriteResponse(p2);
	LOG1("after ConstructWriteResponse()\n");
	FLUSH;
	_daveDumpPDU(p2);
	LOG1("after DumpPDU()\n");
	FLUSH;
};


int DECL2 _davePackPDU(daveConnection * dc, PDU *p) {
	IBHpacket * ibhp;
	MPIheader * hm = (MPIheader*)(dc->msgOut + sizeof(IBHpacket)); // MPI headerPDU begins packet header
	hm->MPI = dc->MPIAdr;
	hm->localMPI = dc->iface->localMPI;
	hm->src_conn = dc->ibhSrcConn;
	hm->dst_conn = dc->ibhDstConn;
	hm->len = 2 + p->hlen + p->plen + p->dlen;		// set MPI length
	hm->func = 0xf1;				// set MPI "function code"
	hm->packetNumber = _daveIncMessageNumber(dc);
	ibhp = (IBHpacket*)dc->msgOut;
	ibhp->ch1 = 7;
	ibhp->ch2 = 0xff;
	ibhp->len = hm->len + 5;
	ibhp->packetNumber = dc->packetNumber;
	dc->packetNumber++;
	ibhp->rFlags = 0x82;
	ibhp->sFlags = 0;

	return 0;
}



uc _MPIconnectResponse[] = {
	0xff, 0x07, 0x13, 0x00, 0x00, 0x00, 0xc2, 0x02, 0x14, 0x14, 0x03, 0x00, 0x00, 0x22, 0x0c, 0xd0,
	0x04, 0x00, 0x80, 0x00, 0x02, 0x00, 0x02, 0x01, 0x00, 0x01, 0x00,
};

/*
	packet analysis. mixes all levels.
	*/
int DECL2 __daveAnalyze(daveConnection * dc) {
	int haveResp;
	IBHpacket * p, *p2;
	MPIheader * pm;
	MPIheader2 * m2;
	PDU p1;
#ifdef passiveMode    
	PDU pr;
#endif    
	uc resp[2000];

	//    if (dc->AnswLen==0) return _davePtEmpty;
	haveResp = 0;

	p = (IBHpacket*)dc->msgIn;
	dc->needAckNumber = -1;		// Assume no ack
	if (daveDebug & daveDebugPacket){
		LOG2("Channel: %d\n", p->ch1);
		LOG2("Channel: %d\n", p->ch2);
		LOG2("Length:  %d\n", p->len);
		LOG2("Number:  %d\n", p->packetNumber);
		LOG3("sFlags:  %04x rFlags:%04x\n", p->sFlags, p->rFlags);
	}
	if (p->rFlags == 0x82) {
		pm = (MPIheader*)(dc->msgIn + sizeof(IBHpacket));
		if (daveDebug & daveDebugMPI){
			LOG2("srcconn: %d\n", pm->src_conn);
			LOG2("dstconn: %d\n", pm->dst_conn);
			LOG2("MPI:     %d\n", pm->MPI);
			LOG2("MPI len: %d\n", pm->len);
			LOG2("MPI func:%d\n", pm->func);
		}
		if (pm->func == 0xf1) {
			if (daveDebug & daveDebugMPI)
				LOG2("MPI packet number: %d needs ackknowledge\n", pm->packetNumber);
			dc->needAckNumber = pm->packetNumber;
			_daveSetupReceivedPDU(dc, &p1);
#ifdef passiveMode
			// construct response:	    
			pr.header = resp + sizeof(IBHpacket) + sizeof(MPIheader2);
#endif	    
			p2 = (IBHpacket*)resp;
			p2->ch1 = p->ch2;
			p2->ch2 = p->ch1;
			p2->packetNumber = 0;
			p2->sFlags = 0;
			p2->rFlags = 0x2c2;

			m2 = (MPIheader2*)(resp + sizeof(IBHpacket));
			m2->src_conn = pm->src_conn;
			m2->dst_conn = pm->dst_conn;
			m2->MPI = pm->MPI;
			m2->xxx1 = 0;
			m2->xxx2 = 0;
			m2->xx22 = 0x22;
			if (p1.param[0] == daveFuncRead) {
#ifdef passiveMode	    
				_daveHandleRead(&p1, &pr);
				haveResp = 1;
				m2->len = pr.hlen + pr.plen + pr.dlen + 2;
#endif
				p2->len = m2->len + 7;
			}
			else if (p1.param[0] == daveFuncWrite) {
#ifdef passiveMode
				_daveHandleWrite(&p1, &pr);
				haveResp = 1;
				m2->len = pr.hlen + pr.plen + pr.dlen + 2;
#endif		
				p2->len = m2->len + 7;
			}
			else {
				if (daveDebug & daveDebugPDU)
					LOG2("Unsupported PDU function code: %d\n", p1.param[0]);
				return _davePtUnknownPDUFunc;
			}

		}
		if (pm->func == 0xb0) {
			LOG2("Ackknowledge for packet number: %d\n", *(dc->msgIn + 15));
			return _davePtMPIAck;
		}
		if (pm->func == 0xe0) {
			LOG2("Connect to MPI: %d\n", pm->MPI);
			memcpy(resp, _MPIconnectResponse, sizeof(_MPIconnectResponse));
			resp[8] = pm->src_conn;
			resp[9] = pm->src_conn;
			resp[10] = pm->MPI;
			haveResp = 1;
		}
	}

	if (p->rFlags == 0x2c2) {
		MPIheader2 * pm = (MPIheader2*)(dc->msgIn + sizeof(IBHpacket));
		if (daveDebug & daveDebugMPI) {
			LOG2("srcconn: %d\n", pm->src_conn);
			LOG2("dstconn: %d\n", pm->dst_conn);
			LOG2("MPI:     %d\n", pm->MPI);
			LOG2("MPI len: %d\n", pm->len);
			LOG2("MPI func:%d\n", pm->func);
		}
		if (pm->func == 0xf1) {
			if (daveDebug & daveDebugMPI)
				LOG1("analyze 1\n");
			dc->needAckNumber = pm->packetNumber;
			dc->PDUstartI = sizeof(IBHpacket) + sizeof(MPIheader2);
			_daveSendMPIAck_IBH(dc);

			return 55;

			/*
					if (daveDebug & daveDebugMPI)
					LOG2("MPI packet number: %d\n",pm->packetNumber);
					dc->needAckNumber=pm->packetNumber;
					//	    p1.header=((uc*)pm)+sizeof(MPIheader2);
					dc->PDUstartI= sizeof(IBHpacket)+sizeof(MPIheader2);
					_daveSetupReceivedPDU(dc, &p1);

					if (p1.param[0]==daveFuncRead) {
					LOG1("read Response\n");
					_daveSendMPIAck_IBH(dc);
					dc->resultPointer=p1.data+4;
					dc->_resultPointer=p1.data+4;
					dc->AnswLen=p1.dlen-4;
					return _davePtReadResponse;
					} else if (p1.param[0]==daveFuncWrite) {
					_daveSendMPIAck_IBH(dc);
					LOG1("write Response\n");
					return _davePtWriteResponse;
					} else {
					LOG2("Unsupported PDU function code: %d\n",p1.param[0]);
					}
					*/
		}

		if (pm->func == 0xb0) {
			if (daveDebug & daveDebugMPI)
				LOG2("Ackknowledge for packet number: %d\n", pm->packetNumber);
		}
		else {
			if (daveDebug & daveDebugMPI)
				LOG2("Unsupported MPI function code !!: %d\n", pm->func);
			_daveSendMPIAck_IBH(dc);
		}
	}
	/*
		Sending IBHNetAck also for packets with sFlags=082 nearly doubles the speed for LINUX and
		speeds up windows version to the level of LINUX.
		Thanks to Axel Kinting for this proposal and for his patience finding it out!
		*/
	if (((p->sFlags == 0x82) || (p->sFlags == 0x82)) && (p->packetNumber) && (p->len)) _daveSendIBHNetAck(dc);
	if (haveResp) {
		_daveWriteIBH(dc->iface, resp, resp[2] + 8);
		_daveDump("I send response:", resp, resp[2] + 8);
	}
	return 0;
};

/*
uc chal0[]={
0x00,0xff,0x03,0x00,0x00,0x00,0x04,0x00, 0x03,0x07,0x02,
};
*/
/*
us resp0[]={
0xff,0x00,0xfe,0x00, 0x04,0x00,0x00,0x00,
0x00,0x07,0x02,0x03,0x1f,
0x100,		// MPI address of NetLink
0x02,0x00,
0x103,		// 187,5, MPI = 3
// 19,5, PB = 1
0x00,0x00,0x00,0x102,
0x00,0x02,0x3e,
0x19f,		// 187,5, MPI = 9F
// 19,5, PB = 64
0x101,		// 187,5, MPI = 1
// 19,5, PB = 0

0x101,		// 187,5, MPI = 1
// 19,5, PB = 0
0x00,
0x102,		// 187,5, MPI = 1
// 19,5, PB = 2
0x00,
0x116,		// 187,5, MPI = 3c
// 19,5, PB = 16
0x00,
0x13c,		// 187,5, MPI = 90
// 19,5, PB = 3c

0x101,
0x110,
0x127,0x100,0x100,0x114,0x101, 0x100,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0e,
'H', 'i','l','s','c','h','e','r',' ','G','m','b','H',' ',

0x200, / * do not compare the rest * /
};
*/


/*
	Build a PDU with data from 2 data blocks.
	*/
int DECL2 BuildAndSendPDU(daveConnection * dc, PDU*p2, uc *pa, int psize, uc *ud, int usize,
	uc *ud2, int usize2) {
	int res;
	PDU p, *p3;
	uc * dn;
	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 7);
	_daveAddParam(&p, pa, psize);
	_daveAddUserData(&p, ud, usize);
	//    LOG2("*** here we are: %d\n",p.dlen);
	p3 = &p;
	dn = p3->data + p3->dlen;
	p3->dlen += usize2;
	memcpy(dn, ud2, usize2);

	((PDUHeader*)p3->header)->dlen = daveSwapIed_16(p3->dlen);

	LOG2("*** here we are: %d\n", p.dlen);
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(&p);
	}
	res = _daveExchange(dc, &p);
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveExchange(): %d\n", res);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, p2);
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p2);
	}
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveSetupReceivedPDU(): %d\n", res);
	if (res != daveResOK) return res;
	res = _daveTestPGReadResult(p2);
	if (daveDebug & daveDebugErrorReporting)
		LOG2("*** res of _daveTestPGReadResult(): %d\n", res);
	return res;
}

int DECL2 daveForce200(daveConnection * dc, int area, int start, int val) {
	int res;
	PDU p2;
	//    uc pa[]={0,1,18,4,17,67,2,0};
	//    uc da[]={'0','0'};

	//32,7,0,0,0,0,0,c,0,16,

	uc pa[] = { 0, 1, 18, 8, 18, 72, 14, 0, 0, 0, 0, 0 };
	uc da[] = { 0, 1, 0x10, 2,
		0, 1,
		0, 0,
		0,		// area
		0, 0, 0,		// start
	};
	uc da2[] = { 0, 4, 0, 8, 0, 0, };
	//    uc da2[]={0,4,0,8,7,0,};

	if ((area == daveAnaIn) || (area == daveAnaOut) /*|| (area==daveP)*/) {
		da[3] = 4;
		start *= 8;			/* bits */
	}
	else if ((area == daveTimer) || (area == daveCounter) || (area == daveTimer200) || (area == daveCounter200)) {
		da[3] = area;
	}
	else {
		start *= 8;
	}
	/*    else {
		if(isBit) {
		pa[3]=1;
		} else {
		start*=8;
		}
		}
		*/
	da[8] = area;
	da[9] = start / 0x10000;
	da[10] = (start / 0x100) & 0xff;
	da[11] = start & 0xff;


	da2[4] = val % 0x100;
	da2[5] = val / 0x100;
	res = BuildAndSendPDU(dc, &p2, pa, sizeof(pa), da, sizeof(da), da2, sizeof(da2));
	return res;
}

daveResultSet * DECL2 daveNewResultSet() {
	daveResultSet * p = (daveResultSet*)calloc(1, sizeof(daveResultSet));
#ifdef DEBUG_CALLS
	LOG2("daveNewResultSet() = %p\n", p);
	FLUSH;
#endif	        
	return p;
}

void DECL2 daveFree(void * dc) {
	//    if (dc!=NULL) {	// I'm not sure whether freeing a NULL pointer will do no harm on each and 
	// every system. So for safety, we check and set it to NULL afterwards.
	free(dc);
	//    }	
}

int DECL2 daveGetProgramBlock(daveConnection * dc, int blockType, int number, char* buffer, int * length) {
	int res, uploadID, len, more, totlen;
	uc *bb = (uc*)buffer;	//cs: is this right?
	len = 0;
	totlen = 0;
	if (dc->iface->protocol == daveProtoAS511) {
		return daveGetS5ProgramBlock(dc, blockType, number, buffer, length);
	}

	res = initUpload(dc, blockType, number, &uploadID);
	if (res != 0) return res;
	do {
		res = doUpload(dc, &more, &bb, &len, uploadID);
		totlen += len;
		if (res != 0) return res;
	} while (more);
	res = endUpload(dc, uploadID);
	*length = totlen;
	return res;
}

int DECL2 daveDeleteProgramBlock(daveConnection*dc, int blockType, int number) {
	int res;
	PDU p, p2;
	uc paDelete[] = {
		0x28, 0, 0, 0, 0, 0, 0, 0xFD, 0,
		0x0a, 0x01, 0x00,
		'0', 'C', //Block type in ASCII (0C = FC)
		'0', '0', '0', '0', '1', //Block Number in ASCII
		'B', //Direction?
		0x05, //Length of Command
		'_', 'D', 'E', 'L', 'E' //Command Delete	
	};

	paDelete[13] = blockType;
	sprintf((char*)(paDelete + 14), "%05d", number);
	paDelete[19] = 'B'; //This is overriden by sprintf via 0x00 as String seperator!

	p.header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(&p, 1);
	_daveAddParam(&p, paDelete, sizeof(paDelete));
	res = _daveExchange(dc, &p);
	if (res == daveResOK) {
		res = _daveSetupReceivedPDU(dc, &p2);
		if (daveDebug & daveDebugPDU) {
			_daveDumpPDU(&p2);
		}
	}

	//Retval of 0x28 in Recieved PDU Parameter Part means delete was sucessfull.
	//This needs to be implemneted and also error Codes Like Block does not exist or block is locked and so on...
	return res;
}



int DECL2 daveReadPLCTime(daveConnection * dc) {
	int res, len;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 'G', 1, 0 };
#ifdef DEBUG_CALLS
	LOG2("daveGetTime(dc:%p)\n", dc);
	FLUSH;
#endif	    	
	len = 0; res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), NULL, 1);
	if (res == daveResOK) {
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len = p2.udlen;
	}
	else {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("daveGetTime: %04X=%s\n", res, daveStrerror(res));
	}
	dc->AnswLen = len;
	return res;
}

int DECL2 daveSetPLCTime(daveConnection * dc, uc * ts) {
	int res, len;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 'G', 2, 0 };
#ifdef DEBUG_CALLS
	LOG2("daveSetTime(dc:%p)\n", dc);
	FLUSH;
#endif	    	
	len = 0;
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), ts, 10);
	if (res == daveResOK) {
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len = p2.udlen;
	}
	else {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("daveGetTime: %04X=%s\n", res, daveStrerror(res));
	}
	dc->AnswLen = len;
	return res;
}

uc DECL2 daveToBCD(uc i) {
	return 16 * (i / 10) + (i % 10);
}

uc DECL2 daveFromBCD(uc i) {
	return 10 * (i / 16) + (i % 16);
}

int DECL2 daveSetPLCTimeToSystime(daveConnection * dc) {
	int res, len;
	PDU p2;
	uc pa[] = { 0, 1, 18, 4, 17, 'G', 2, 0 };
	uc ts[] = {
		0x00, 0x19, 0x05, 0x08, 0x23, 0x04, 0x10, 0x23, 0x67, 0x83,
	};
#ifdef LINUX    
	struct tm systime;
	struct timeval t1;
	gettimeofday(&t1, NULL);
	localtime_r(&(t1.tv_sec), &systime);
	t1.tv_usec /= 100;		//tenth of miliseconds from microseconds
	//    ts[1]=daveToBCD(systime.tm_year/100+19);
	ts[1] = daveToBCD(systime.tm_year / 100); // fix 2010 bug is this line necessary? ok?
	ts[2] = daveToBCD(systime.tm_year % 100); // fix 2010 bug
	ts[3] = daveToBCD(systime.tm_mon + 1);
	ts[4] = daveToBCD(systime.tm_mday);
	ts[5] = daveToBCD(systime.tm_hour);
	ts[6] = daveToBCD(systime.tm_min);
	ts[7] = daveToBCD(systime.tm_sec);
	ts[8] = daveToBCD(t1.tv_usec / 100);
	ts[9] = daveToBCD(t1.tv_usec % 100);
	//    _daveDump("timestamp: ",ts,10);
	//    LOG2("tm.sec:  %d\n", systime.tm_sec);
	//    LOG2("tm.min:  %d\n", systime.tm_min);
	//    LOG2("tm.hour: %d\n", systime.tm_hour);
#endif    

#ifdef _WIN32
	SYSTEMTIME t1;
	//    gettimeofday(&t1, NULL);
	GetLocalTime(&t1);
	//    tm=localtime(&t1);
	//    t1.tv_usec/=100;		//tenth of miliseconds from microseconds
	//    WORD wYear;
	/*
		WORD wMonth;
		WORD wDayOfWeek;
		WORD wDay;
		WORD wHour;
		WORD wMinute;
		WORD wSecond;
		WORD wMilliseconds;
		*/
	ts[1] = daveToBCD(t1.wYear / 100); // fix 2010 bug
	ts[2] = daveToBCD(t1.wYear % 100); // fix 2010 bug
	ts[3] = daveToBCD(t1.wMonth);
	ts[4] = daveToBCD(t1.wDay);
	ts[5] = daveToBCD(t1.wHour);
	ts[6] = daveToBCD(t1.wMinute);
	ts[7] = daveToBCD(t1.wSecond);
	ts[8] = daveToBCD(t1.wMilliseconds / 10);
	ts[9] = daveToBCD((t1.wMilliseconds % 10) * 10);
	//    _daveDump("timestamp: ",ts,10);
	//    LOG2("tm.sec:  %d\n", t1.wSecond);
	//    LOG2("tm.min:  %d\n", t1.wMinute);
	//    LOG2("tm.hour: %d\n", t1.wHour);
#endif    

#ifdef DEBUG_CALLS
	LOG2("SetPLCTimeToSystime(dc:%p)\n", dc);
	FLUSH;
#endif	    	
	len = 0;
	res = daveBuildAndSendPDU(dc, &p2, pa, sizeof(pa), ts, sizeof(ts));
	if (res == daveResOK) {
		dc->resultPointer = p2.udata;
		dc->_resultPointer = p2.udata;
		len = p2.udlen;
	}
	else {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("daveGetTime: %04X=%s\n", res, daveStrerror(res));
	}
	dc->AnswLen = len;
	return res;
}


/***************
  Simatic S5:
  ****************/

uc __davet2[] = { STX };
uc __davet10[] = { DLE };
char __davet1006[] = { DLE, ACK };  // cs: this is only sent by system functions, so let it be char
us __daveT1006[] = { DLE, ACK };
uc __davet121003[] = { 0x12, DLE, ETX };
us __daveT121003[] = { 0x12, DLE, ETX };
uc __davet161003[] = { 0x16, DLE, ETX };
us __daveT161003[] = { 0x16, DLE, ETX };
/*
	Reads <count> bytes from area <BlockN> with offset <offset>,
	that can be readed with daveGetInteger etc. You can read bytes from
	PBs & FBs too, but use daveReadBlock for this:
	*/
int DECL2 daveReadS5Bytes(daveConnection * dc, uc area, uc BlockN, int offset, int count)
{
	int res, datastart, dataend;
	daveS5AreaInfo ai;
	uc b1[daveMaxRawLen];
	//    if (_daveIsS5BlockArea(area)==0) {
	if (area == daveDB) {
		res = _daveReadS5BlockAddress(dc, area, BlockN, &ai);//TODO make address cache
		if (res<0) {
			LOG2("%s *** Error in ReadS5Bytes.BlockAddr request.\n", dc->iface->name);
			return res - 50;
		}
		datastart = ai.address;
	}
	else {
		switch (area) {
		case daveRawMemoryS5: datastart = 0; break;
		case daveInputs: datastart = dc->cache->PAE; break;
		case daveOutputs: datastart = dc->cache->PAA; break;
		case daveFlags: datastart = dc->cache->flags; break;
		case daveTimer: datastart = dc->cache->timers; break;
		case daveCounter: datastart = dc->cache->counters; break;
		case daveSysDataS5: datastart = dc->cache->systemData; break;
		default:
			LOG2("%s *** Unknown area in ReadS5Bytes request.\n", dc->iface->name);
			return -1;
		}
	}
	//It's difficult to convert Intel-Motorola so I will use arithmetic:
	if ((count > daveMaxRawLen)
		//    ||(offset+count>ai.len)
		) {
		LOG2("%s *** readS5Bytes: Requested data is out-of-range.\n", dc->iface->name);
		return -1;
	}
	datastart += offset;
	dataend = datastart + count - 1;
	b1[0] = datastart / 256;
	b1[1] = datastart % 256;
	b1[2] = dataend / 256;
	b1[3] = dataend % 256;
	res = _daveExchangeAS511(dc, b1, 4, 2 * count + 7, 0x04);
	if (res < 0) {
		LOG2("%s *** Error in ReadS5Bytes.Exchange sequence.\n", dc->iface->name);
		return res - 10;
	}
	if (dc->AnswLen < count + 7) {
		LOG3("%s *** Too few chars (%d) in ReadS5Bytes data.\n", dc->iface->name, dc->AnswLen);
		return -5;
	}
	if ((dc->msgIn[0] != 0) || (dc->msgIn[1] != 0) || (dc->msgIn[2] != 0) || (dc->msgIn[3] != 0) || (dc->msgIn[4] != 0)) {
		LOG2("%s *** Wrong ReadS5Bytes data signature.\n", dc->iface->name);
		return -6;
	}
	dc->resultPointer = dc->msgIn + 5;
	dc->_resultPointer = dc->resultPointer;

	//    dc->dlen=dc->AnswLen-7;
	dc->AnswLen -= 7;
	return 0;
}

/*
	Write DLE,ACK to the serial interface:
	*/
void DECL2 _daveSendDLEACK(daveInterface * di)	// serial interface
{
	di->ifwrite(di, __davet1006, 2);
}

/*
	Sends a sequence of characters after doubling DLEs and adding DLE,EOT.
	*/
int DECL2 _daveSendWithDLEDup(daveInterface * di, // serial interface
	uc *b, 		    // a buffer containing the message
	int size		    // the size of the string
	)
{
	uc target[daveMaxRawLen];
	int res;
	int targetSize = 0, i; //preload 
	if (daveDebug & daveDebugExchange)
		LOG1("SendWithDLEDup: \n");
	if (daveDebug & daveDebugExchange)
		_daveDump("I send", b, size);
	for (i = 0; i < size; i++) {
		target[targetSize] = b[i]; targetSize++;
		if (b[i] == DLE) {
			target[targetSize] = DLE;
			targetSize++;
		}
	};
	target[targetSize] = DLE;
	target[targetSize + 1] = EOT;
	targetSize += 2;
	if (daveDebug & daveDebugExchange)
		_daveDump("I send", target, targetSize);
	res = di->ifwrite(di, (char*)target, targetSize);
	if (daveDebug & daveDebugExchange)
		LOG2("send: res:%d\n", res);
	return 0;
}

/*
	Executes part of the dialog that requests transaction with PLC:
	*/
int DECL2 _daveReqTrans(daveConnection * dc, uc trN)
{
	uc b1[3];
	int res;
	if (daveDebug & daveDebugExchange)
		LOG3("%s daveReqTrans %d\n", dc->iface->name, trN);
	_daveSendSingle(dc->iface, STX);
	res = dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT1006) / 2);
	if (daveDebug & daveDebugByte)
		_daveDump("2got", b1, res);
	if (_daveMemcmp(__daveT1006, b1, sizeof(__daveT1006) / 2)) {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("%s daveReqTrans %d *** no DLE,ACK before send.\n", dc->iface->name, trN);
		return -1;
	}
	_daveSendSingle(dc->iface, trN);
	if (_daveReadSingle(dc->iface) != STX) {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("%s daveReqTrans %d *** no STX before send.\n", dc->iface->name, trN);
		return -2;
	}
	_daveSendDLEACK(dc->iface);
	dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT161003) / 2);
	if (daveDebug & daveDebugByte)
		_daveDump("1got", b1, res);
	if (_daveMemcmp(__daveT161003, b1, sizeof(__daveT161003) / 2)) {
		if (daveDebug & daveDebugPrintErrors)
			LOG3("%s daveReqTrans %d *** no accept0 from plc.\n", dc->iface->name, trN);
		return -3;
	}
	_daveSendDLEACK(dc->iface);
	return 0;
}

/*
	Executes part of the dialog required to terminate transaction:
	*/
int DECL2 _daveEndTrans(daveConnection * dc)
{
	int res;
	uc b1[3];
	if (daveDebug & daveDebugExchange)
		LOG2("%s daveEndTrans\n", dc->iface->name);
	if (_daveReadSingle(dc->iface) != STX) {
		LOG2("%s daveEndTrans *** no STX at eot sequense.\n", dc->iface->name);
		//	return -1;
	}
	_daveSendDLEACK(dc->iface);
	res = dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT121003) / 2);
	if (daveDebug & daveDebugByte)
		_daveDump("3got", b1, res);
	if (_daveMemcmp(__daveT121003, b1, sizeof(__daveT121003) / 2)) {
		LOG2("%s daveEndTrans *** no accept of eot/ETX from plc.\n", dc->iface->name);
		return -2;
	}
	_daveSendDLEACK(dc->iface);
	return 0;
}

/*
	Remove the DLE doubling:
	*/
int DECL2 _daveDLEDeDup(daveConnection * dc, uc* rawBuf, int rawLen) {
	int j = 0, k;
	for (k = 0; k < rawLen - 2; k++){
		dc->msgIn[j] = rawBuf[k]; j++;
		if (DLE == rawBuf[k]){
			if (DLE != rawBuf[k + 1]) return -1;//Bad doubling found
			k++;
		}
	}
	dc->msgIn[j] = rawBuf[k];//Copy 2 last chars (DLE,ETX)
	j++; k++;
	dc->msgIn[j] = rawBuf[k];
	dc->AnswLen = j + 1;
	return 0;
}

int DECL2 _daveExchangeAS511(daveConnection * dc, uc * b, int len, int maxlen, int trN) {
	int res, i;
	uc b1[3];
	res = _daveReqTrans(dc, trN);
	if (res < 0) {
		LOG2("%s *** Error in Exchange.ReqTrans request.\n", dc->iface->name);
		return res - 10;
	}
	if (trN == 8) {		//Block write functions have advanced syntax
		LOG1("trN 8\n");
		_daveSendWithDLEDup(dc->iface, b, 4);
		LOG1("trN 8 done\n");
	}
	else {
		if (daveDebug & daveDebugExchange)
			LOG3("trN %d len %d\n", trN, len);
		_daveSendWithDLEDup(dc->iface, b, len);
		if (daveDebug & daveDebugExchange)
			LOG2("trN %d done\n", trN);
	}
	//    _daveSendDLEACK(dc->iface);
	//    res=dc->iface->ifread(dc->iface, b1, /*dc->iface->timeout,*/ 2000 /*sizeof(__daveT1006)/2*/);
	res = dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT1006) / 2);
	if (daveDebug & daveDebugByte)
		_daveDump("4 got:", b1, res);
	if (_daveMemcmp(__daveT1006, b1, sizeof(__daveT1006) / 2)) {
		LOG2("%s *** no DLE,ACK in Exchange request.\n", dc->iface->name);
		return -1;
	}
	if ((trN != 3) && (trN != 7) && (trN != 9)) {//write bytes, compress & delblk
		if (!_daveReadSingle(dc->iface) == STX) {
			LOG2("%s *** no STX in Exchange request.\n", dc->iface->name);
			return -2;
		}
		//	usleep(500000);
		_daveSendDLEACK(dc->iface);
		res = 0;
		do {
			//	    i=dc->iface->ifread(dc->iface, dc->msgIn+res, /*100*dc->iface->timeout,*/ daveMaxRawLen-res);
			i = dc->iface->ifread(dc->iface, (char *)(dc->msgIn + res), /*100*dc->iface->timeout,*/ 1);
			res += i;
			if (daveDebug & daveDebugByte)
				_daveDump("5 got:", dc->msgIn, res);
		} while ((i > 0) && ((dc->msgIn[res - 2] != DLE) || (dc->msgIn[res - 1] != ETX)));

		if (daveDebug & daveDebugByte)
			LOG3("%s *** got %d bytes.\n", dc->iface->name, res);
		if (res < 0) {
			LOG2("%s *** Error in Exchange.ReadChars request.\n", dc->iface->name);
			return res - 20;
		}
		if ((dc->msgIn[res - 2] != DLE) || (dc->msgIn[res - 1] != ETX)) {
			LOG2("%s *** No DLE,ETX in Exchange data.\n", dc->iface->name);
			return -4;
		}
		if (_daveDLEDeDup(dc, dc->msgIn, res) < 0) {
			LOG2("%s *** Error in Exchange rawdata.\n", dc->iface->name);
			return -3;
		}

		//	usleep(500000);
		_daveSendDLEACK(dc->iface);
	}
	if (trN == 8) { //Write requests have more differences from others ;(
		if (dc->msgIn[0] != 9) {
			LOG2("%s 8 *** No 0x09 in special Exchange request.\n", dc->iface->name);
			return -5;
		}
		_daveSendSingle(dc->iface, STX);
		res = dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT1006) / 2);
		_daveDump("got:", b1, res);
		if (_daveMemcmp(__daveT1006, b1, sizeof(__daveT1006) / 2)) {
			LOG2("%s 8 *** no DLE,ACK in special Exchange request.\n", dc->iface->name);
			return -6;
		}
		_daveSendWithDLEDup(dc->iface, b + 4, len);

		res = dc->iface->ifread(dc->iface, (char *)b1, /*dc->iface->timeout,*/ sizeof(__daveT1006) / 2);
		_daveDump("got:", b1, res);
		if (_daveMemcmp(__daveT1006, b1, sizeof(__daveT1006) / 2)) {
			//        if (!_daveTestChars(dc->iface, __davet1006, 2)) {
			LOG2("%s 8 *** no DLE,ACK after transfer in Exchange.\n", dc->iface->name);
			return -7;
		}
	}
	if (trN == 7) {
		//    	usleep(450000);
	}//TODO: check compression time
	res = _daveEndTrans(dc);
	if (res < 0) {
		LOG2("%s *** Error in Exchange.EndTrans request.\n", dc->iface->name);
		return res - 30;
	}
	return 0;
}

/*
	In S7, we need to tell the PLC what memory area we want to read from or write to. The PLC
	behaves as if each area were a different physical memory starting with an offset of 0.
	In S5, everything is in common 64k of memory. For different areas, we have to add start
	offsets of areas or objects.
	The following is needed to make memory access as S7-compatible as possible.
	*/
int areaFromBlockType(int area){
	switch (area) {
	case daveS5BlockType_DB:		// S5 block type
	case daveBlockType_DB:			// S7 block type
	case daveDB: 				// S7 area type
		return daveS5BlockType_DB;
	case daveS5BlockType_OB:
	case daveBlockType_OB:
		return daveS5BlockType_OB;
	case daveS5BlockType_FB:
	case daveBlockType_FB:
		return daveS5BlockType_FB;
		// s5 only:	    
	case daveS5BlockType_PB:
		return daveS5BlockType_PB;
	case daveS5BlockType_SB:
		return daveS5BlockType_SB;
	default: return area;
	}
}
/*
	Requests physical addresses and lengths of blocks in PLC memory and writes
	them to ai structure:
	*/
int DECL2 _daveReadS5BlockAddress(daveConnection * dc, uc area, uc BlockN, daveS5AreaInfo * ai)
{
	int res, dbaddr, dblen, s5Area;
	uc b1[24];			//15 + some Dups
	//    if (_daveIsS5BlockArea(area)<0) {
	//            printf("%s *** Not block area .\n", dc->iface->name);
	//	    return -1;
	//    }

	//    b1[0]=area;

	s5Area = areaFromBlockType(area);
	b1[0] = s5Area;
	b1[1] = BlockN;
	res = _daveExchangeAS511(dc, b1, 2, 24, 0x1A);
	if (res < 0) {
		printf("%s *** Error in BlockAddr.Exchange sequense.\n", dc->iface->name);
		return res - 10;
	}
	if (dc->AnswLen < 15) {
		printf("%s *** Too few chars (%d) in BlockAddr data.\n", dc->iface->name, dc->AnswLen);
		return -2;
	}
	if ((dc->msgIn[0] != 0)
		|| (dc->msgIn[3] != 0x70)
		|| (dc->msgIn[4] != 0x70)
		|| (dc->msgIn[5] != 0x40 + s5Area) || (dc->msgIn[6] != BlockN)) {
		printf("%s *** Wrong BlockAddr data signature.\n", dc->iface->name);
		return -3;
	}
	dbaddr = dc->msgIn[1];
	dbaddr = dbaddr * 256 + dc->msgIn[2];//Let make shift operations to compiler's optimizer
	dblen = dc->msgIn[11];
	dblen = (dblen * 256 + dc->msgIn[12] - 5) * 2; //PLC returns dblen in words including
	//5 word header (but returnes the
	//start address after the header) so 
	//dblen is length of block body
	ai->address = dbaddr;
	ai->len = dblen;
	return 0;
}

int DECL2 _daveIsS5BlockArea(uc area)
{
	if (
		//	(area!=daveBlockType_S5DB)&&
		(area != daveS5BlockType_SB) &&
		(area != daveS5BlockType_PB) &&
		(area != daveS5BlockType_FX) &&
		(area != daveS5BlockType_FB) &&
		(area != daveS5BlockType_DX) &&
		(area != daveS5BlockType_OB)) {
		return -1;
	}
	return 0;
}

int DECL2 _daveIsS5DBlockArea(uc area)
{
	if (area != daveDB) {
		//        (area!=daveBlockType_S5DX))    
		//        (area!=daveBlockType_S5DX)) {
		return -1;
	}
	return 0;
}

#define maxSysinfoLen 87
/*
	This is a trick which will intercept all functions not available for S5 AS511. It works
	this way:

	A function for S7 forms a packet for S7 communication and then calls daveExchange which
	will send the packet and return the answer.
	If a function is also vailable for S5, it must check whether the protocol is AS511. If so,
	the function calls it's S5 counterpart and returns the result of it.
	Hence, it will never reach daveExchange.

	Now, functions for which there is no S5 counterpart simply continue, (superfluously) form
	the S7 packet and call daveExchange, which will point hereto.
	This fake function allways returns a specific error code so the user knows the function
	is not available in the S5 protocol.

	The advantage of this mechanism is that additional functions for S7 can be added at any
	time without caring about S5: If no special handling is provided, they end up here.
	*/
int DECL2 _daveFakeExchangeAS511(daveConnection * dc, PDU *p){
	return daveNotAvailableInS5;
}

/*
	This is a deviation from normal use of connect functions: There are no connections in AS511.
	The reason why we provide a daveConnect() is this:
	From an S5 CPU, you don't read inputs, outputs,flags or any other memory area but simply
	bytes from global memory.
	There are addresses of input image area, output image area, flags, timers etc. These depend
	on CPU model. Next, there are start addresses of the data blocks. These addresses change
	whenever a data block is created or changed in size or modified by programming device.

	In both cases, we could read the adresses from the PLC before reading the data. To save
	time and gain efficiency, we read them once in connectPLC. We rely on users following the
	S7 scheme: connect to a PLC before reading from it !!

	If we would read addresses each time, you could do something you cannot with S7: pull the
	plug from one PLC, connect to another PLC and the program still works.

	Here, you CANNOT do that. You have to call connectPLC again after changing to the new PLC.

	Another thing are data block addresses. We could fetch all 256 possible addresses in connectPLC,
	too. But that would use 256 entries that must exist while the program might not use data
	blocks at all. So we don't. We add data block addresses to the PLC address cache when they
	are used for the first time.
	There are S5 programs that create data blocks dynamically. Hence cached addresses get invalid.
	If you have a PLC with such a program use
	daveSetNonCacheable(dc, DBnumber);
	If you suspect somebody could pull the plug, connect a programming device, modify data blocks
	and reconnect your application program, use
	daveSetNonCacheable(dc, allDBs);
	In this case, the actual address will be fetched before each read or write from/to the related
	data blocks (which will slow down your application).
	*/

int DECL2 _daveConnectPLCAS511(daveConnection * dc){
	int res;
	uc b1[maxSysinfoLen]; //20 words + some Dups
	//    dc->maxPDUlength=1000;
	dc->maxPDUlength = 240;
	dc->cache = (daveS5cache*)calloc(1, sizeof(daveS5cache));

	res = _daveExchangeAS511(dc, b1, 0, maxSysinfoLen, 0x18);
	if (res < 0) {
		LOG2("%s *** Error in ImageAddr.Exchange sequence.\n", dc->iface->name);
		return res - 10;
	}
	if (dc->AnswLen < 47) {
		LOG3("%s *** Too few chars (%d) in ImageAddr data.\n", dc->iface->name, dc->AnswLen);
		return -2;
	}
	_daveDump("connect:", dc->msgIn, 47);
	dc->cache->PAE = daveGetU16from(dc->msgIn + 5);	// start of inputs;
	dc->cache->PAA = daveGetU16from(dc->msgIn + 7);	// start of outputs;
	dc->cache->flags = daveGetU16from(dc->msgIn + 9);	// start of flag (marker) memory;
	dc->cache->timers = daveGetU16from(dc->msgIn + 11);	// start of timer memory;
	dc->cache->counters = daveGetU16from(dc->msgIn + 13);	// start of counter memory
	dc->cache->systemData = daveGetU16from(dc->msgIn + 15);	// start of system data
	dc->cache->first = NULL;
	LOG2("start of inputs in memory %04x\n", dc->cache->PAE);
	LOG2("start of outputs in memory %04x\n", dc->cache->PAA);
	LOG2("start of flags in memory %04x\n", dc->cache->flags);
	LOG2("start of timers in memory %04x\n", dc->cache->timers);
	LOG2("start of counters in memory %04x\n", dc->cache->counters);
	LOG2("start of system data in memory %04x\n", dc->cache->systemData);
	return 0;
}

int DECL2 _daveDisconnectPLCAS511(daveConnection * dc){
	free(dc->cache);
	dc->cache = 0;
	return 0;
}

/*
	Writes <count> bytes from area <BlockN> with offset <offset> from buf.
	You can't write data to the program blocks because you can't syncronize
	with PLC cycle. For this purposes use daveWriteBlock:
	*/
int DECL2 daveWriteS5Bytes(daveConnection * dc, uc area, uc BlockN, int offset, int count, void * buf)
{
	int res, datastart;
	daveS5AreaInfo ai;
	uc b1[daveMaxRawLen];
	//    if (_daveIsS5DBlockArea(area)==0) {
	if (area == daveDB) {
		//	LOG1("_daveIsS5DBlockArea\n");
		res = _daveReadS5BlockAddress(dc, area, BlockN, &ai);
		if (res<0) {
			LOG2("%s *** Error in WriteS5Bytes.BlockAddr request.\n", dc->iface->name);
			return res - 50;
		}
		datastart = ai.address;
	}
	else {
		switch (area) {
		case daveRawMemoryS5: datastart = 0; break;
		case daveInputs: datastart = dc->cache->PAE; break;
		case daveOutputs: datastart = dc->cache->PAA; break;
		case daveFlags: datastart = dc->cache->flags; break;
		case daveTimer: datastart = dc->cache->timers; break;
		case daveCounter: datastart = dc->cache->counters; break;
		case daveSysDataS5: datastart = dc->cache->systemData; break;
		default:
			LOG2("%s *** Unknown area in WriteS5Bytes request.\n", dc->iface->name);
			return -1;
		}
	}
	if ((count>daveMaxRawLen) || (offset + count > ai.len)) {
		LOG2("%s writeS5Bytes *** Requested data is out-of-range.\n", dc->iface->name);
		return -1;
	}
	//    datastart=ai.address+offset;
	LOG2("area start is %04x, ", datastart);
	datastart += offset;
	LOG2("data start is %04x\n", datastart);
	b1[0] = datastart / 256;
	b1[1] = datastart % 256;
	memcpy(&b1[2], buf, count);
	res = _daveExchangeAS511(dc, b1, 2 + count, 0, 0x03);
	if (res < 0) {
		LOG2("%s *** Error in WriteS5Bytes.Exchange sequense.\n", dc->iface->name);
		return res - 10;
	}
	return 0;
}

int DECL2 daveStopS5(daveConnection * dc) {
	uc b1[] = { 0x88, 0x04 };	// I don't know what this mean
	return daveWriteBytes(dc, daveSysDataS5, 0, 0x0c, 2, b1);
}

int DECL2 daveStartS5(daveConnection * dc) {
	uc b1[] = { 0x68, 0x00 };	// I don't know what this mean
	return daveWriteBytes(dc, daveSysDataS5, 0, 0x0c, 2, b1);
}

int DECL2 daveGetS5ProgramBlock(daveConnection * dc, int blockType, int number, char* buffer, int * length) {
	//    int totlen,res;
	//    *length=totlen;
	return daveResNotYetImplemented;
}


/*
	Changes:
	09/09/04  applied patch for variable Profibus speed from Andrew Rostovtsew.
	12/09/04  removed debug printf from daveConnectPLC.
	12/09/04  found and fixed a bug in daveFreeResults(): The result set is provided by the
	application and not necessarily dynamic. So we shall not free() it.
	12/10/04  added single bit read/write functions.
	12/12/04  added Timer/Counter read functions.
	12/13/04  changed dumpPDU to dump multiple results from daveFuncRead
	12/15/04  changed comments to pure C style
	12/15/04  replaced calls to write() with makro daveWriteFile.
	12/15/04  removed daveSendDialog. Was only used in 1 place.
	12/16/04  removed daveReadCharsPPI. It is replaced by daveReadChars.
	12/30/04  Read Timers and Counters from 200 family. These are different internal types!
	01/02/05  Hopefully fixed local MPI<>0.
	01/10/05  Fixed some debug levels in connectPLCMPI
	01/10/05  Splitted daveExchangeMPI into the send and receive parts. They are separately
	useable when communication is initiated by PLC.
	01/10/05  Code cleanup. Some more things in connectPLC can be done using genaral
	MPI communication subroutines.
	01/10/05  Partially applied changes from Andrew Rostovtsew for multiple MPI connections
	over the same adapter.
	01/11/05  Lasts steps in connect PLC can be done with exchangeMPI.
	01/26/05  replaced _daveConstructReadRequest by the sequence prepareReadRequest, addVarToReadRequest
	01/26/05  added multiple write
	02/02/05  added readIBHpacket
	02/05/05  merged in fixes for (some?) ARM processors.
	02/06/05  Code cleanup.
	03/06/05  Fixed disconnectPLC_IBH for MPI adresses other than 2.
	03/12/05  clear answLen before read
	03/12/05  reset templ.packetNumber in connectPLC_IBH. This is necessary to reconnect if the
	connection has been interrupted.
	03/23/05  fixes for target PPI addresses other than 2.

	04/05/05  reworked error reporting.
	04/06/05  renamed swap functions. When I began libnodave on little endian i386 and Linux, I used
	used Linux bswap functions. Then users (and later me) tried other systems without
	a bswap. I also cannot use inline functions in Pascal. So I made my own bswaps. Then
	I, made the core of my own swaps dependent of DAVE_LITTLE_ENDIAN conditional to support also
	bigendien systems. Now I want to rename them from bswap to something else to avoid
	confusion for LINUX/UNIX users. The new names are daveSwapIed_16 and daveSwapIed_32. This
	shall say swap "if endianness differs". While I could have used similar functions
	from the network API (htons etc.) on Unix and Win32, they may not be present on
	e.g. microcontrollers.
	I highly recommend to use these functions even when writing software for big endian
	systems, where they effectively do nothing, as it will make your software portable.
	04/09/05  removed template IBH_MPI header from daveConnection. Much of the information is
	also available from other fields and the structure is simpler to define in other
	languages.
	04/09/05  removed CYGWIN defines. As there were no more differences against LINUX, it should
	work with LINUX defines.
	04/21/05  renamed LITTLEENDIAN to DAVE_LITTLE_ENDIAN because it seems to conflict with
	another #define in winsock2.h.
	05/09/05  renamed more functions to daveXXX.
	05/11/05  added some functions for the convenience of usage with .net or mono. The goal is
	that the application doesn't have to use members of data structures defined herein
	directly. This avoids "unsafe" pointer expressions in .net/MONO. It should also ease
	porting to VB or other languages for which it could be difficult to define byte by
	byte equivalents of these structures.
	05/12/05  applied some bug fixes from Axel Kinting.
	05/12/05  applied bug fix from Lutz Nitzsche in daveSendISOpacket.
	07/31/05  added message string copying for Visual Basic.
	09/09/05  added code to ignore 7 byte packets from soft PLC 6ES7-4PY00-0YB7 in ISO_TCP.
	09/10/05  added explicit type casts for pointers optained from malloc and calloc.
	09/11/05  added read/write functions for long blocks of data.
	09/24/05  Code clean up:
	- Pointers to basic read/functions allow to redirect these functions, e.g. to libusb.
	- More common code. Only the very fundamental read/functions differ between Linux and Win32.
	09/24/05  added MPI protocol version 3. This is what Step7 talks to MPI adapters and seems
	to be the only thing the Siemens USB-MPI adapter understands. This adapter is
	currently only useable under Linux via libusb.
	09/27/05  added bug fix from Renato Gartmann: freeResults didn't free() the memory used for
	the result pointer array.
	09/29/05  hopefully fixed superfluos STX in daveConnectPLCMPI2.
	10/04/05  No there are adapters which want it...
	10/05/05  Added first helper functions to use s7onlinx.dll for transport.
	10/06/05  Added standard protocol specific functions to use s7onlinx.dll for transport.
	10/06/05  renamed LITTLE_ENDIAN to DAVE_LITTLE_ENDIAN because it conflicts with
	another #define in some headers on some ARM systems.
	10/10/05  change some pointer increments for gcc-4.0.2 compatibility.
	10/18/05  Indroduced a (temporary?) work around to allow applications to use normal sequence
	disconnectAdapter/closePort also with s7online.
	02/20/06  Added code to support NetLink Pro.
	05/15/06  Applied changes from Ken Wenzel for NetLink Pro.
	07/28/06  Added CRC calculation code from Peter Etheridge.

	11/21/06  Hope to have fixed PDU length problem with IBHLink reported by Axel Kinting.

	01/04/07  Set last byte of resp09 to don't care as reported by Axel Kinting.
	02/07/08  Removed patch from Keith Harris for RTS line.
	Version 0.8.4.5
	07/10/09  Changed readISOpacket for Win32 to select() before recv().
	07/10/09  Added daveCopyRAMtoROM
	07/11/09  Changed calculation of netLen in doUpload()
	Version 0.8.5
	05/17/13  return bad results from daveBuildAndSendPDU() in several places
	05/17/13  removed old code for table based CRC calculation
	05/17/13  added a disconnect function for S7online
	05/17/13  added support for transport of long PDUs split into multiple IBH packets
	05/18/13  added support for transport of long PDUs split into multiple ISO packets
	05/18/13  added routing support
	05/19/13  added communication type (PG, OP, S7-Basic)
	10/19/13  changed general code to be ARM compatible without ARM_FIX
	*/

/*
build the PDU for a PDU length negotiation
*/

int DECL2 _daveNegPDUlengthRequest(daveConnection * dc, PDU *p) {
	uc pa[] = { 0xF0, 0, 0, 1, 0, 1,
		dc->maxPDUlength / 0x100, //3, 		
		dc->maxPDUlength % 0x100, //0xC0,
	};
	int res;
	int CpuPduLimit;
	PDU p2;
	p->header = dc->msgOut + dc->PDUstartO;
	_daveInitPDUheader(p, 1);
	_daveAddParam(p, pa, sizeof(pa));
	if (daveDebug & daveDebugPDU) {
		_daveDumpPDU(p);
	}
	res = _daveExchange(dc, p);
	if (res != daveResOK) return res;
	res = _daveSetupReceivedPDU(dc, &p2);
	if (res != daveResOK) return res;
	CpuPduLimit = daveGetU16from(p2.param + 6);
	if (dc->maxPDUlength > CpuPduLimit) dc->maxPDUlength = CpuPduLimit; // use lower number as limit
	if (daveDebug & daveDebugConnect) {
		LOG3("\n*** Partner offered PDU length: %d used limit %d\n\n", CpuPduLimit, dc->maxPDUlength);
	}
	return res;
}
