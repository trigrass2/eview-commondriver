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
#include "protoIBH.h"
#include "log2.h"

uc _MPIack[]={
	0x07,0xff,0x08,0x05,0x00,0x00,0x82,0x00, 0x15,0x14,0x02,0x00,0x03,0xb0,0x01,0x00,
};

void DECL2 _daveSendMPIAck_IBH(daveConnection*dc) {
    _MPIack[15]=dc->msgIn[16];
    _MPIack[8]=dc->ibhSrcConn;
    _MPIack[9]=dc->ibhDstConn;
    _MPIack[10]=dc->MPIAdr;
    _daveWriteIBH(dc->iface,_MPIack,sizeof(_MPIack));
}

/*
    send a network level ackknowledge
*/
void DECL2 _daveSendIBHNetAck(daveConnection * dc) {
    IBHpacket * p;
    uc ack[13];
    memcpy(ack, dc->msgIn, sizeof(ack));
    p= (IBHpacket*) ack;
    p->len=sizeof(ack)-sizeof(IBHpacket);
    ack[11]=1;
    ack[12]=9;
//    LOG2("Sending net level ack for number: %d\n",p->packetNumber);
    _daveWriteIBH(dc->iface, ack,sizeof(ack));
}


int DECL2 _daveExchangeIBH(daveConnection * dc, PDU * p) {
    
    _daveSendMessageMPI_IBH(dc, p);
    dc->AnswLen=0;
    return _daveGetResponseMPI_IBH(dc);
}

int DECL2 _daveSendMessageMPI_IBH(daveConnection * dc, PDU * p) {
    int res;
    _davePackPDU(dc, p);
    res=_daveWriteIBH(dc->iface, dc->msgOut, dc->msgOut[2]+8);
    if (daveDebug & daveDebugPDU)    
	_daveDump("I send request: ",dc->msgOut, dc->msgOut[2]+8);
    return res;	
}	

int DECL2 _daveGetResponseMPI_IBH(daveConnection * dc) {
    int res,count,pt;
    count=0;
    pt=0;
    do {
	res=_daveReadIBHPacket(dc->iface, dc->msgIn);
	count++;
	if(res>4)
	    pt=__daveAnalyze(dc);
	if (daveDebug & daveDebugExchange)    
	    LOG2("ExchangeIBH packet type:%d\n",pt);
    } while ((pt!=55)&&(count<7));	// 05/21/2013
    if(pt!=55) return daveResTimeout;
    return 0;
}


/*
    This performs initialization steps with sampled byte sequences. If chal is <>NULL
    it will send this byte sequence.
    It will then wait for a packet and compare it to the sample.
*/
int DECL2 _daveInitStepIBH(daveInterface * iface, uc * chal, int cl, us* resp, int rl, uc*b) {
    int res, res2, a=0;
    if (daveDebug & daveDebugConnect) 
	LOG1("_daveInitStepIBH before write.\n");
    if (chal) res=_daveWriteIBH(iface, chal, cl);else res=daveResInvalidParam;
    if (daveDebug & daveDebugConnect) 
	LOG2("_daveInitStepIBH write returned %d.\n",res);
    if (res<0) return 100;
    res=_daveReadIBHPacket(iface, b);
/*
    We may get a network layer ackknowledge and an MPI layer ackknowledge, which we discard.
    So, normally at least the 3rd packet should have the desired response. 
    Waiting for more does:
	-discard extra packets resulting from last step.
	-extend effective timeout.
*/    
    while (a<5) {
	if (a) {
	    res=_daveReadIBHPacket(iface, b);
//	    _daveDump("got:",b,res);
	}
	if (res>0) {
	    res2=_daveMemcmp(resp,b,rl/2);
	    if (0==res2) {
		if (daveDebug & daveDebugInitAdapter) 
		    LOG3("*** Got response %d %d\n",res,rl);
		return a;
	    }  else {
		if (daveDebug & daveDebugInitAdapter)  
		    LOG2("wrong! %d\n",res2);
	    }
	}
	a++;
    }
    return a;
}


int DECL2 _daveReadIBHPacket2(daveInterface * di,uc *b) {
    int res, len;
    res=_daveTimedRecv(di, b, 3);
    if (res<3) {
        if (daveDebug & daveDebugByte) {
	    LOG2("res %d ",res);
	    _daveDump("readIBHpacket2: short packet", b, res);
        }
        return (0); /* short packet */
    }
    len=b[2]+8;
    res+=_daveTimedRecv(di, b+3, len-3);
    if (daveDebug & daveDebugByte) {
        LOG3("readIBHpacket2: %d bytes read, %d needed\n",res, len);
        _daveDump("readIBHpacket2: packet", b, res);
    }
//    _daveDump("readIBHpacket2: packet", b, 8);
    return res;
}



uc IBHfollow[]={
	0,0,7,0xb,
	0,0,0x82,0,
	0,0,0,0,
	2,5,1
    };


int DECL2 _daveReadIBHPacket(daveInterface * di,uc *b) {

    int res,res2,len2;
    uc b2[300];
    res= _daveReadIBHPacket2(di, b);

    if ((res>15) && (b[15]==0xf0)) {
again:	    
//	    LOG1("FOLLOW UP\n");
	    
	    IBHfollow[0]=b[1];
	    IBHfollow[1]=b[0];
	    IBHfollow[8]=b[8];
	    IBHfollow[9]=b[9];
	    IBHfollow[10]=b[10];
	    IBHfollow[11]=b[11];
	
//	    _daveDump("IBHfollow", IBHfollow, 15);

	    res2=send((unsigned int)(di->fd.wfd), IBHfollow, 15, 0);

//	    LOG2("send: res2:%d\n",res2);

	    res2= _daveReadIBHPacket2(di, b2);
//	    LOG2("read: res2:%d\n",res2);
	    res2= _daveReadIBHPacket2(di, b2);
//	    LOG2("read: res2:%d\n",res2);

//	    if ((res>15) && (b[15]==0xf0)) 
	    memcpy(b+res,b2+17,res2-17);
	    b[16]=b2[16];
	    res+=res2-17;
	    b[15]=0xf1;
//	    LOG2("new b2[15]: %d\n",b2[15]);
	    if ((res>15) && (b2[15]==0xf0)) goto again;
	}

	if (daveDebug & daveDebugByte) {
	    LOG2("readIBHpacket: %d bytes read\n",res);
//	    _daveDump("readIBHpacket: packet", b, res);
	}

    return (res);
    //return 0;
}


/*
    10/04/2003 	PPI has an address. Implemented now.
    06/03/2004 	Fixed a bug in _davePPIexchange, which caused timeouts
		when the first call to readChars returned less then 4 characters.
*/

int DECL2 _daveWriteIBH(daveInterface * di, uc * buffer, int len) { //cs: let it be uc 
    int res;
    if (daveDebug & daveDebugByte) {
	_daveDump("writeIBH: ", buffer, len);
    }	
#ifdef HAVE_SELECT    
    res=write(di->fd.wfd, buffer, len);
#endif    
#ifdef BCCWIN
//    res=send((SOCKET)(di->fd.wfd), buffer, ((len+1)/2)*2, 0);
    res=send((SOCKET)(di->fd.wfd), buffer, len, 0);
#endif
    return res;
    //return 0;
}    


int DECL2 _daveExchangePPI_IBH(daveConnection * dc, PDU * p) {
	int res, count, pt;
	_davePackPDU_PPI(dc, p);

	res=_daveWriteIBH(dc->iface, dc->msgOut, dc->msgOut[2]+8);
	if (daveDebug & daveDebugExchange)    
		_daveDump("I send request: ",dc->msgOut, dc->msgOut[2]+8);
	//    _daveSendIBHNetAckPPI(dc);	
	count=0;
	do {
		res=_daveReadIBHPacket(dc->iface, dc->msgIn);
		count++;
		if (res>0)
			pt=__daveAnalyzePPI(dc,1);
		else 
			pt=0;    
		if (daveDebug & daveDebugExchange)    
			LOG2("ExchangeIBH packet type:%d\n",pt);
	} while ((pt!=55)&&(count<7));
	if(pt!=55) return daveResTimeout;
	return 0;
}

int DECL2 _daveGetResponsePPI_IBH(daveConnection * dc) {
	int res, count, pt;
	count=0;
	do {
		_daveSendIBHNetAckPPI(dc);
		res=_daveReadIBHPacket(dc->iface, dc->msgIn);
		LOG2("_daveReadIBHPacket():%d\n",res);
		count++;
		if (res>0)
			pt=__daveAnalyzePPI(dc,0);
		else 
			pt=0;    
		if (daveDebug & daveDebugExchange)    
			LOG2("ExchangeIBH packet type:%d\n",pt);
	} while ((pt!=55)&&(count<7));
	if(pt!=55) return daveResTimeout;
	return 0;
}


/*
    send a network level ackknowledge
*/
void DECL2 _daveSendIBHNetAckPPI(daveConnection * dc) {
    uc ack[]={7,0xff,5,3,0,0,0x82,0,0xff,0xff,2,0,0};
    ack[10]=dc->MPIAdr;
    ack[3]=_daveIncMessageNumber(dc);
    _daveWriteIBH(dc->iface, ack, sizeof(ack));
}

uc chal1[]={ 
	0x07,0xff,0x08,0x01,0x00,0x00,0x96,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x0a,0x01,
};

us resp1[]={ 
	0xff,0x07,0x87,0x01,0x96,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x7f,0x0a,0x02,
};
/*
uc chal2[]={
	0x00,0x10,0x01,0x00,0x00,0x00,0x01,0x00, 0x0f,
};
*/
/*
us resp2[]={
	0x10,0x00,
	    0x20,
	    0x00,
	    0x01,0x00,0x00,0x00, 
	    0x01,0x106,0x120,0x103,0x17,0x00,0x43,0x00,
	    0x00,0x00,
	    0x122,0x121,
	    0x00,0x00,0x00,0x00, 0x49,0x42,0x48,0x53,0x00,0x00,0x00,0x00,
	    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
};
*/
uc chal3[]={
	0x07,0xff,0x06,0x01,0x00,0x00,0x97,0x00, 0x15,0xff,0xf0,0xf0,0xf0,0xf0,
};

us resp3[]={
	0xff,0x107,0x02,0x01,0x97,0x00,0x00,0x00, 0x114,0x100,
};

uc chal8[]={
	0x07,
	0xff,
	0x11,	// 17 bytes
	0x02,
	0x00,
	0x00,
	0x82,
	0x00,

	0x14,	// source SAP
	0x00,
	0x02,	// MPI address
	0x01,
	0x0c,	// 12 bytes follow
	0xe0,0x04,0x00,0x80,
	0x00,0x02,
	0x00,0x02,
	0x01,	// communication type?
	0x00,
	0x01,	// 6 when routed
	0x00,
};

	uc chal8R[]={   //Routing
	0x07,
	0xff,
	0x1b,	// 27 bytes, 10 more
	0x02,
	0x00,
	0x00,
	0x82,
	0x00,
	0x14,	// source SAP
	0x00,	
	0x02,	// MPI address
	0x01,
	0x16,	// 22 bytes follow (when routing to MPI, 25 when routing to IP)
	0xe0,0x04,0x00,0x80,
	0x00,0x02,
	0x01,0x0c,
	0x01,	// dc->CommunicationType, // communication type? I do NOT think so.
	0x00,
	0x06,	// 6 byte subnet ID
	0x01,	// 1 byte PLC address	my guess
	0x02,	// 2 byte something else
	0x01,0x52,0x00,0x00,0x00,0x13,	// subnet ID
	0x01,				// PLC address, MPI or Profibus or IP
	0x01,	// comunication type?
	0x02,	// rack,slot ?
	0,0,0
	};

/* This is the correct response. I just inserted a "don't care" to make it work with latest 
   IBH simulator. Better fix the simulator!
us resp7[]={
	0xff,0x07,0x13,0x00,0x00,0x00,0xc2,0x02, 0x115,0x114,0x102,0x100,0x00,0x22,0x0c,0xd0,
	0x04,0x00,0x80,0x00,0x02,0x00,0x02,0x01, 0x00,0x01,0x00,
};
*/

// from routing, target MPI
us resp7[]={
	0xFF,0x07,0x11D,0x00,0x00,0x00,0xC2,0x02, 0x114,0x12E,0x102,0x100,0x00,0x22,0x116,0xD0,
	0x04,0x00,0x80,
//	0x01,0x0C,0x00,0x02,0x06, 0x01,0x02,0x01,0x52,0x00,0x00,0x00,0x13,
//	0x102,0x101,0x102,0x101,0x100,
};

uc chal011[]={
	0x07,0xff,0x07,0x03,0x00,0x00,0x82,0x00, 0x15,0x14,0x02,0x00,0x02,0x05,0x01,
};

us resp09[]={
	0xff,0x07,0x09,0x00,0x00,0x00,0xc2,0x02, 0x115,0x114,0x102,0x100,0x00,0x22,0x02,0x05,
	0x101,	// A.K.
};


int DECL2 _daveListReachablePartnersMPI_IBH(daveInterface * di, char * buf) {
    int a, i;
    uc b[2*daveMaxRawLen];
    a=_daveInitStepIBH(di, chal1,sizeof(chal1),resp1,16,b);
    if (daveDebug & daveDebugListReachables) 
	LOG2("_daveListReachablePartnersMPI_IBH:%d\n",a); 
    for (i=0;i<126;i++) {
	if (b[i+16]==0xFF) buf[i]=0x10; else buf[i]=0x30;
//	LOG3(" %d %d\n",i, b[i+16]); 
    }	
    return 126;
}    


/*
    packet analysis. mixes all levels.
*/
int DECL2 __daveAnalyzePPI(daveConnection * dc, uc sa) {
    IBHpacket* p;
    
    p= (IBHpacket*) dc->msgIn;
    if (daveDebug & daveDebugPacket){
	LOG2("Channel: %d\n",p->ch1);
	LOG2("Channel: %d\n",p->ch2);
        LOG2("Length:  %d\n",p->len);
	LOG2("Number:  %d\n",p->packetNumber);
        LOG3("sFlags:  %04x rFlags:%04x\n",p->sFlags,p->rFlags);
    }    
    if (p->sFlags==0x82) {
	if(p->len<=5) {
	    if(sa) _daveSendIBHNetAckPPI(dc);
	} else    
	    if ((p->len>=7) && (dc->msgIn[14]==0x32))
	    return 55;
    }	
    return 0;
};



/*
    Connect to a PLC via IBH-NetLink. Returns 0 for success and a negative number on errors.
*/
int DECL2 _daveConnectPLC_IBH(daveConnection*dc) {
    int a, retries;
    PDU p1;
    uc b[daveMaxRawLen];
    uc * pcha;
    dc->iface->timeout=500000;
    dc->iface->localMPI=0;
    dc->ibhSrcConn=20-1;
    dc->ibhDstConn=20-1;
    retries=0;
    do {
	if (daveDebug & daveDebugConnect)  // show only if in debug mode
	    LOG1("trying next ID:\n");
	dc->ibhSrcConn++;
	chal3[8]=dc->ibhSrcConn;
	a=_daveInitStepIBH(dc->iface, chal3,sizeof(chal3),resp3,sizeof(resp3),b);
	retries++;
    } while ((b[9]!=0) && (retries<10));
    if (daveDebug & daveDebugConnect) 
	LOG2("_daveInitStepIBH 4:%d\n",a); if (a>3) /* !!! */ return -4;;
//    if (!dc->routing) {
	chal8[8]=dc->ibhSrcConn;
	chal8[10]=dc->MPIAdr;
//    } else {
	chal8R[8]=dc->ibhSrcConn;
	chal8R[10]=dc->MPIAdr;
//    }

//    LOG2("setting MPI %d\n",dc->MPIAdr);
    if (!dc->routing) {
	a=_daveInitStepIBH(dc->iface, chal8,sizeof(chal8),resp7,sizeof(resp7),b);
    } else {
	pcha=chal8R+2;
	*pcha=26+dc->routingData.PLCadrsize;

	pcha=chal8R+12;
	*pcha=21+dc->routingData.PLCadrsize;
	
	pcha=chal8R+20;
	*pcha=11+dc->routingData.PLCadrsize;

	pcha=chal8R+24;
	*pcha=dc->routingData.PLCadrsize;

	pcha=chal8R+26;
//	_daveAddSubnet(dc, &pcha);
	
	*pcha=(dc->routingData.subnetID1) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID1) % 0x100; pcha++;
	*pcha=(dc->routingData.subnetID2) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID2) % 0x100; pcha++;
	*pcha=(dc->routingData.subnetID3) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID3) % 0x100; pcha++;

	memcpy(pcha, dc->routingData.PLCadr, dc->routingData.PLCadrsize);
	pcha+=dc->routingData.PLCadrsize;
	*pcha=dc->communicationType;
        *pcha=(dc->slot | dc->rack<<5);
	
//	a=_daveInitStepIBH(dc->iface, chal8R, sizeof(chal8R), resp7R, sizeof(resp7R), b);
//	a=_daveInitStepIBH(dc->iface, chal8R, sizeof(chal8R)-4+dc->routingData.PLCadrsize, resp7R, sizeof(resp7R), b);
//	a=_daveInitStepIBH(dc->iface, chal8R, sizeof(chal8R)-4+dc->routingData.PLCadrsize, resp7R, 19, b);
	a=_daveInitStepIBH(dc->iface, chal8R, sizeof(chal8R)-4+dc->routingData.PLCadrsize, resp7, sizeof(resp7), b);
    }
    dc->ibhDstConn=b[9];
    if (daveDebug & daveDebugConnect) 
	LOG3("_daveInitStepIBH 5:%d connID: %d\n",a, dc->ibhDstConn); if (a>3) return -5;

    chal011[8]=dc->ibhSrcConn;
    chal011[9]=dc->ibhDstConn;
    chal011[10]=dc->MPIAdr;	//??????
    a=_daveInitStepIBH(dc->iface, chal011,sizeof(chal011),resp09,sizeof(resp09),b);

    dc->ibhDstConn=b[9];
    if (daveDebug & daveDebugConnect) 
	LOG3("_daveInitStepIBH 5a:%d connID: %d\n",a, dc->ibhDstConn); if (a>3) return -5;
    
    dc->packetNumber=4;
    return _daveNegPDUlengthRequest(dc, &p1);
}    

uc chal31[]={
	0x07,0xff,0x06,0x08,0x00,0x00,0x82,0x00, 0x14,0x14,0x02,0x00,0x01,0x80,
};

/*
    Disconnect from a PLC via IBH-NetLink. 
    Returns 0 for success and a negative number on errors.
*/
int DECL2 _daveDisconnectPLC_IBH(daveConnection*dc) {
    uc b[daveMaxRawLen];
    chal31[8]=dc->ibhSrcConn;
    chal31[9]=dc->ibhDstConn;
    chal31[10]=dc->MPIAdr;
    _daveWriteIBH(dc->iface, chal31, sizeof(chal31));
    _daveReadIBHPacket(dc->iface, b);
#ifdef BCCWIN    
#else    
    _daveReadIBHPacket(dc->iface, b);
#endif    
    return 0;
}

/*
    Disconnect from a PLC via IBH-NetLink. This can be used to free other than your own 
    connections. Be careful, this may disturb third party programs/devices.
*/
int DECL2 daveForceDisconnectIBH(daveInterface * di, int src, int dest, int mpi) {
    uc b[daveMaxRawLen];
    chal31[8]=src;
    chal31[9]=dest;
    chal31[10]=mpi;
    _daveWriteIBH(di, chal31, sizeof(chal31));
    _daveReadIBHPacket(di, b);
#ifdef BCCWIN    
#else    
    _daveReadIBHPacket(di, b);
#endif    
    return 0;
}   

/*
    Resets the IBH-NetLink.
    Returns 0 for success and a negative number on errors.
*/
int DECL2 daveResetIBH(daveInterface * di) {
    uc chalReset[]={
	0x00,0xff,0x01,0x00,0x00,0x00,0x01,0x00,0x01
    };
    uc b[daveMaxRawLen];
    _daveWriteIBH(di, chalReset, sizeof(chalReset));
    _daveReadIBHPacket(di, b);
#ifdef BCCWIN
#else
    _daveReadIBHPacket(di, b);
#endif   
    return 0;
}

void DECL2 _daveSendMPIAck2(daveConnection *dc) {
    IBHpacket * p;
    uc c;
    uc ack[18];
    memcpy(ack,dc->msgIn,sizeof(ack));
    p= (IBHpacket*) ack;
    p->rFlags|=0x240;	//Why that?
    c=p->ch1; p->ch1=p->ch2; p->ch2=c;
    p->len=sizeof(ack)-sizeof(IBHpacket);
    p->packetNumber=0;	// this may mean: no net work level acknowledge
    ack[13]=0x22;
    ack[14]=3;
    ack[15]=176;
    ack[16]=1;
    ack[17]=dc->needAckNumber;
    _daveDump("send MPI-Ack2",ack,sizeof(ack));
    _daveWriteIBH(dc->iface,ack,sizeof(ack));
};

int DECL2 _davePackPDU_PPI(daveConnection * dc,PDU *p) {
    IBHpacket * ibhp;
    uc IBHPPIHeader[]={0xff,0xff,2,0,8};
    IBHPPIHeader[2]=dc->MPIAdr;
    memcpy(dc->msgOut+sizeof(IBHpacket), &IBHPPIHeader, sizeof(IBHPPIHeader));
    
    ibhp = (IBHpacket*) dc->msgOut;
    ibhp->ch1=7;
    ibhp->ch2=0xff;
    ibhp->len=p->hlen+p->plen+p->dlen+5;		// set MPI length
//    dc->msgOut[3]=2;
    *(dc->msgOut+sizeof(IBHpacket)+4)=p->hlen+p->plen+p->dlen+0;
    ibhp->packetNumber=dc->packetNumber;
    ibhp->rFlags=0x80;
    ibhp->sFlags=0;
//    dc->msgOut[3]=dc->packetNumber;
    dc->packetNumber++;
    dc->msgOut[6]=0x82;
    dc->msgOut[3]=2;
    return 0;
}    

