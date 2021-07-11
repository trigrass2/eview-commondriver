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
#include "log2.h"

/***
    NetLink Pro
***/
#define NET

#ifndef NET
int DECL2 _daveReadMPINLpro(daveInterface * di, uc *b) {
	int res=0,state=0,nr_read;
	uc bcc=0;
        nr_read= di->ifread(di, b, 2);
	if (nr_read>=2) {
	    res=256*b[0]+b[1]+2;
	    if (res>nr_read)
		nr_read+=di->ifread(di, b+nr_read, res-nr_read);
	    if (daveDebug & daveDebugInitAdapter) 
		LOG4("%s nr_read:%d res:%d.\n", di->name, nr_read, res);
	    return res-2;
	}
}
#endif

#ifdef NET
/*
    Read one complete packet. The bytes 0 and 1 contain length information.
    This version needs a socket filedescriptor that is set to O_NONBLOCK or
    it will hang, if there are not enough bytes to read.
    The advantage may be that the timeout is not used repeatedly.
*/

int DECL2 _daveReadMPINLpro(daveInterface * di,uc *b) {
	int res,i,length;
	i=_daveTimedRecv(di, b, 2);
	res=i;
	if (res <= 0) return daveResTimeout;
	if (res<2) {
	    if (daveDebug & daveDebugByte) {
		LOG2("res %d ",res);
		_daveDump("readISOpacket: short packet", b, res);
	    }
	    return daveResShortPacket; /* short packet */
	}
	length=b[1]+0x100*b[0];
	i=_daveTimedRecv(di, b+2, length);
	res+=i;
	if (daveDebug & daveDebugByte) {
	    LOG3("readMPINLpro: %d bytes read, %d needed\n",res, length);
	    _daveDump("readMPIpro: packet", b, res);
	}
	return (res);
}
#endif


/*
    This initializes the MPI adapter. Andrew's version.
*/

int DECL2 _daveInitAdapterNLpro(daveInterface * di)  /* serial interface */
{
    uc b3[]={
	0x01,0x03,0x02,0x27, 0x00,0x9F,0x01,0x14,
 	0x00,0x90,0x01,0xc, 0x00,	/* ^^^ MaxTsdr */
	0x00,0x5,
 	0x02,/* Bus speed */

	0x00,0x0F,0x05,0x01,0x01,0x03,0x81,/* from topserverdemo */
	/*^^ - Local mpi */
    };		
    
    int res;
    b3[16]=di->localMPI;
    if (di->speed==daveSpeed500k)
	b3[7]=0x64;
    if (di->speed==daveSpeed1500k)
	b3[7]=0x96;
    b3[15]=di->speed;

//    res=_daveInitStep(di, 1, b3, sizeof(b3),"initAdapter()");
    res=_daveInitStepNLpro(di, 1, b3, sizeof(b3),"initAdapter()", NULL);

//    res= _daveReadMPINLpro(di, b1);
    if (daveDebug & daveDebugInitAdapter) 
	LOG2("%s initAdapter() success.\n", di->name);
//   _daveSendSingle(di,DLE);
    di->users=0;	/* there cannot be any connections now */
    return 0;
}


void DECL2 _daveSendSingleNLpro(daveInterface * di,	/* serial interface */
		     uc c  			/* chracter to be send */
		     ) 
{

    unsigned long i;
    uc c3[3];
    c3[0]=0;
    c3[1]=1;
    c3[2]=c;
//    di->ifwrite(di, c3, 3);
#ifdef HAVE_SELECT
    daveWriteFile(di->fd.wfd, c3, 3, i);
#endif    
#ifdef BCCWIN
    send((unsigned int)(di->fd.wfd), c3, 3, 0);
#endif
}


/*
    This sends a string after doubling DLEs in the String
    and adding DLE,ETX and bcc.
*/
int DECL2 _daveSendWithCRCNLpro(daveInterface * di, /* serial interface */
		    uc *b, 		 /* a buffer containing the message */
		    int size		 /* the size of the string */
		)
{		
    uc target[daveMaxRawLen];
    int i,targetSize=2;
    target[0]=size / 256;
    target[1]=size % 256;
    
//    int bcc=DLE^ETX; 
    for (i=0; i<size; i++) {
	target[targetSize]=b[i];targetSize++;
    };
//    targetSize+=0;
//    di->ifwrite(di, target, targetSize);
#ifdef HAVE_SELECT
    daveWriteFile(di->fd.wfd, target, targetSize, i);
#endif    
#ifdef BCCWIN
    send((unsigned int)(di->fd.wfd), target, targetSize, 0);
#endif
    
    if (daveDebug & daveDebugPacket)
        _daveDump("_daveSendWithCRCNLpro",target, targetSize);
    return 0;
}


/* 
    Send a string of init data to the MPI adapter.
*/
int DECL2 _daveInitStepNLpro(daveInterface * di, int nr, uc *fix, int len, char * caller, uc * buffer ) {
    uc res[500];
    int i;
    if (daveDebug & daveDebugInitAdapter)
	LOG4("%s %s step %d.\n", di->name, caller, nr);

    _daveSendWithCRCNLpro(di, fix, len);
    i=_daveReadMPINLpro(di, (buffer != NULL) ? buffer : res );
    return 0;
}    

/* 
    Open connection to a PLC. This assumes that dc is initialized by
    daveNewConnection and is not yet used.
    (or reused for the same PLC ?)
*/
int DECL2 _daveConnectPLCNLpro(daveConnection * dc) {
    int res, len;
    PDU p1;
    uc * pcha;

    uc b4[]={
		0x04, //00
		0x80, //01 (0x80 | MPI)
		0x80, //02
		0x0D, //03
		0x00, //04
		0x14, //05
		
		0xE0, //06
		0x04, //07
		0x00, //08
		0x80, //09
		0x00, //10
		0x02, //11
		0x00, //12 //01 ??? Routing???
		0x02, //13 //02 = no routing / 0c = Routing to MPI / 0f Routing to IP (Bytecount to End-2)
		1, 	//
		0, 	//
		1,	//
		0, 	//17 //End of Telegram when no Routing (00) / 01 Routing to MPI / 04 Routing to IP
		0x00, //18
		0x00, //19 subnet1
		0x00, //20 subnet1
		0x00, //21
		0x00, //22
		0x00, //23 subnet2
		0x00, //24 subnet2
		
		0x00, //25 PLC address
		0x00, //26 
		0x00, //27 
		0x00, //28 
		0x02, //29 Communication type
		0x04  //30 Rack, Slot
    };

    us t4[]={
	0x04,0x80,0x180,0x0C,0x114,0x103,0xD0,0x04,	// 1/10/05 trying Andrew's patch
	0x00,0x80,
	0x00,0x02,0x00,0x02,0x01,
	0x00,0x01,0x00,
    };
    uc b5[]={	
	0x05,0x07,
    };
    us t5[]={    
	0x04,
	0x80,
	0x180,0x0C,0x114,0x103,0x05,0x01,
    };
    b4[1]|=dc->MPIAdr;	
    b4[5]=dc->connectionNumber; // 1/10/05 trying Andrew's patch
    
    len=18;
    t4[1]|=dc->MPIAdr;	
    t5[1]|=dc->MPIAdr;	

    pcha=b4+16;
    if(dc->routing) {
    
	pcha=b4+12;
	*pcha=1; pcha++;
	*pcha=11+dc->routingData.PLCadrsize; 
	pcha=b4+16;
	*pcha=6; pcha++;
	*pcha=dc->routingData.PLCadrsize; pcha++;
	*pcha=2; pcha++;

	*pcha=(dc->routingData.subnetID1) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID1) % 0x100; pcha++;
	*pcha=(dc->routingData.subnetID2) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID2) % 0x100; pcha++;
	*pcha=(dc->routingData.subnetID3) / 0x100; pcha++;
	*pcha=(dc->routingData.subnetID3) % 0x100; pcha++;
	memcpy(pcha, dc->routingData.PLCadr, dc->routingData.PLCadrsize);
	pcha+=dc->routingData.PLCadrsize;
	len=27+dc->routingData.PLCadrsize; 
	
    }
    *pcha=dc->communicationType; pcha++;
	*pcha=dc->rack; pcha++;
    _daveInitStepNLpro(dc->iface, 1, b4, len, "connectPLC(1)", dc->msgIn);
    
    // first 2 bytes of msgIn[] contain packet length
    dc->connectionNumber2=dc->msgIn[2+5]; // 1/10/05 trying Andrew's patch
    if (daveDebug & daveDebugConnect) 
	LOG2("%s daveConnectPLC(1) step 4.\n", dc->iface->name);	
    
    if (daveDebug & daveDebugConnect) 
	LOG2("%s daveConnectPLC() step 5.\n", dc->iface->name);	

    _daveSendWithPrefixNLpro(dc, b5, sizeof(b5));		
        
    if (daveDebug & daveDebugConnect) 
	LOG2("%s daveConnectPLC() step 6.\n", dc->iface->name);	
    res= _daveReadMPINLpro(dc->iface,dc->msgIn);
    if (daveDebug & daveDebugConnect) 
	LOG2("%s daveConnectPLC() step 7.\n", dc->iface->name);	
    res= _daveNegPDUlengthRequest(dc, &p1);
    return 0;
}


/*
    Executes part of the dialog necessary to send a message:
*/
int DECL2 _daveSendDialogNLpro(daveConnection * dc, int size)
{
    if (size>5){
	dc->needAckNumber=dc->messageNumber;
	dc->msgOut[dc->iface->ackPos+1]=_daveIncMessageNumber(dc);
    }	
    _daveSendWithPrefix2NLpro(dc, size);
    return 0;
}


/*
    Sends a message and gets ackknowledge:
*/
int DECL2 _daveSendMessageNLpro(daveConnection * dc, PDU * p) {
    if (daveDebug & daveDebugExchange) {
        LOG2("%s enter _daveSendMessageNLpro\n", dc->iface->name);
    }    
    if (_daveSendDialogNLpro(dc, /*2+*/p->hlen+p->plen+p->dlen)) {
	LOG2("%s *** _daveSendMessageMPI error in _daveSendDialog.\n",dc->iface->name);	    		
//	return -1;	
    }	
    if (daveDebug & daveDebugExchange) {
        LOG3("%s _daveSendMessageMPI send done. needAck %x\n", dc->iface->name,dc->needAckNumber);	    
    }	
    return 0;
}


int DECL2 _daveExchangeNLpro(daveConnection * dc, PDU * p) {
    _daveSendMessageNLpro(dc, p);
    dc->AnswLen=0;
    return _daveGetResponseNLpro(dc);
}

int DECL2 _daveGetResponseNLpro(daveConnection *dc) {
    int res;
    if (daveDebug & daveDebugExchange) {
        LOG2("%s _daveGetResponseNLpro receive message.\n", dc->iface->name);	    
    }	
    res = _daveReadMPINLpro(dc->iface,dc->msgIn);
    if (res<0) {
	return res;
    }	
    if (res==0) {
	if (daveDebug & daveDebugPrintErrors) {
	    LOG2("%s *** _daveGetResponseNLpro no answer data.\n", dc->iface->name);
	}        
	return -3;
    }	
    return 0;
}


int DECL2 _daveSendWithPrefixNLpro(daveConnection * dc, uc *b, int size)
{
    uc target[daveMaxRawLen];
//    uc fix[]= {04,0x80,0x80,0x0C,0x03,0x14};
    uc fix[]= {0x4,0x80,0x80,0x0C,0x14,0x14};    
	fix[4]=dc->connectionNumber2; 		// 1/10/05 trying Andrew's patch
	fix[5]=dc->connectionNumber; 		// 1/10/05 trying Andrew's patch
    	memcpy(target,fix,sizeof(fix));
	memcpy(target+sizeof(fix),b,size);
	target[1]|=dc->MPIAdr;
//	target[2]|=dc->iface->localMPI;
	memcpy(target+sizeof(fix),b,size);
	return _daveSendWithCRCNLpro(dc->iface,target,size+sizeof(fix));
}


int DECL2 _daveSendWithPrefix2NLpro(daveConnection * dc, int size)
{
//    uc fix[]= {04,0x80,0x80,0x0C,0x03,0x14};
    uc fix[]= {0x14,0x80,0x80,0x0C,0x14,0x14};
	fix[4]=dc->connectionNumber2;		// 1/10/05 trying Andrew's patch
	fix[5]=dc->connectionNumber;		// 1/10/05 trying Andrew's patch
	memcpy(dc->msgOut, fix, sizeof(fix));
	dc->msgOut[1]|=dc->MPIAdr;
//	dc->msgOut[2]|=dc->iface->localMPI; //???
///	dc->msgOut[sizeof(fix)]=0xF1;
/*	if (daveDebug & daveDebugPacket)
	    _daveDump("_daveSendWithPrefix2",dc->msgOut,size+sizeof(fix)); */
	return _daveSendWithCRCNLpro(dc->iface, dc->msgOut, size+sizeof(fix));
//    }
    return -1; /* shouldn't happen. */
}

int DECL2 _daveDisconnectPLCNLpro(daveConnection * dc)
{
    int res;
    uc m[]={
        0x80
    };
    uc b1[daveMaxRawLen];
    
    _daveSendSingleNLpro(dc->iface, STX);
    
    res=_daveReadMPINLpro(dc->iface,b1);
    _daveSendWithPrefixNLpro(dc, m, 1);	
    
    res=_daveReadMPINLpro(dc->iface,b1);
/*
    res=_daveReadMPI(dc->iface,b1);
    if (daveDebug & daveDebugConnect) 
	_daveDump("got",b1,10);
    _daveSendSingle(dc->iface, DLE);
*/
    return 0;
}    

/*
    It seems to be better to complete this subroutine even if answers
    from adapter are not as expected.
*/
int DECL2 _daveDisconnectAdapterNLpro(daveInterface * di) {
    int res;
    uc m2[]={
        1,4,2
    };
    
    uc b1[daveMaxRawLen];
    if (daveDebug & daveDebugInitAdapter) 
	LOG2("%s enter DisconnectAdapter()\n", di->name);	
//    _daveSendSingleNLpro(di, STX);
//    res=_daveReadMPINLpro(di,b1);
/*    if ((res!=1)||(b1[0]!=DLE)) return -1; */
    _daveSendWithCRCNLpro(di, m2, sizeof(m2));		
    if (daveDebug & daveDebugInitAdapter) 
	LOG2("%s daveDisconnectAdapter() step 1.\n", di->name);	
    res=_daveReadMPINLpro(di, b1);
/*    if ((res!=1)||(b1[0]!=DLE)) return -2; */
/*
    res=_daveReadMPI(di, b1);
*/    
/*    if ((res!=1)||(b1[0]!=STX)) return -3; */
/*
    if (daveDebug & daveDebugInitAdapter) 
	LOG2("%s daveDisconnectAdapter() step 2.\n", di->name);	
    _daveSendSingle(di, DLE);
    dc->iface->ifread(di, b1, daveMaxRawLen);
//    _daveReadChars(di, b1, tmo_normal, daveMaxRawLen);
    _daveSendSingle(di, DLE);
    if (daveDebug & daveDebugInitAdapter) 
	_daveDump("got",b1,10);
*/	
    return 0;	
}

/*
*/
int DECL2 _daveListReachablePartnersNLpro(daveInterface * di,char * buf) {
    uc b1[daveMaxRawLen];
    uc m1[]={1,7,2};
    int res;
    _daveSendWithCRCNLpro(di, m1, sizeof(m1));
    res=_daveReadMPINLpro(di,b1);
//    LOG2("res: %d\n", res);	
    if(135==res){
        memcpy(buf,b1+8,126);
	return 126;
    } else
	return 0;	
}   
