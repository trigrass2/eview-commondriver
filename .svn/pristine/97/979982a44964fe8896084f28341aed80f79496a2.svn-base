
#include "openS7online.h"
#include "log2.h"

/*
    Access S7onlinx.dll and load pointers to the functions. 
    We load them using GetProcAddress on their names because:
    1. We have no .lib file for them.
    2. We don't want to link with a particular version.
    3. Libnodave shall remain useable without Siemens .dlls. So it shall not try to access them
       unless the user chooses the daveProtoS7online.
*/

_openFunc SCP_open;
_closeFunc SCP_close;
_sendFunc SCP_send;
_receiveFunc SCP_receive;
_get_errnoFunc SCP_get_errno;

typedef int (DECL2 * _setHWnd) (int, HWND);

EXPORTSPEC HANDLE DECL2 openS7online(const char * accessPoint, HWND handle) {
    HMODULE hmod;
    int h,en;
	_setHWnd SetSinecHWnd; 

    hmod=LoadLibrary("S7onlinx.dll");
    if (daveDebug & daveDebugOpen)
	LOG2("LoadLibrary(S7onlinx.dll) returned %p)\n",hmod);

    SCP_open=(_openFunc)GetProcAddress(hmod,"SCP_open");
    if (daveDebug & daveDebugOpen)
    	LOG2("GetProcAddress returned %p)\n",SCP_open);

    SCP_close=(_closeFunc)GetProcAddress(hmod,"SCP_close");
    if (daveDebug & daveDebugOpen)
	LOG2("GetProcAddress returned %p)\n",SCP_close);

    SCP_get_errno=(_get_errnoFunc)GetProcAddress(hmod,"SCP_get_errno");
    if (daveDebug & daveDebugOpen)
	LOG2("GetProcAddress returned %p)\n",SCP_get_errno);

    SCP_send=(_sendFunc)GetProcAddress(hmod,"SCP_send");
    if (daveDebug & daveDebugOpen)
	LOG2("GetProcAddress returned %p)\n",SCP_send);

    SCP_receive=(_receiveFunc)GetProcAddress(hmod,"SCP_receive");
    if (daveDebug & daveDebugOpen)
	LOG2("GetProcAddress returned %p)\n",SCP_receive);
    
    SetSinecHWnd=(_setHWnd)GetProcAddress(hmod,"SetSinecHWnd");
    if (daveDebug & daveDebugOpen)
	LOG2("GetProcAddress returned %p)\n",SetSinecHWnd);

    en=SCP_get_errno();
    h=SCP_open(accessPoint);
    en=SCP_get_errno();
    LOG3("handle: %d  error:%d\n", h, en);
	SetSinecHWnd(h, handle);
    return (HANDLE)h;
};
    
EXPORTSPEC HANDLE DECL2 closeS7online(int h) {
    SCP_close(h);
	return NULL;
}


/********************************************
  Use Siemens DLLs and drivers for transport:
*********************************************/

/*
    While the following code is useless under operating systems others than win32,
    I leave it here, independent of conditionals. This ensures it is and will continue
    to be at least compileable now and over version changes. Who knows what it might
    be good for in the future...
*/
/*
    fill some standard fields and pass it to SCP-send:
*/
int DECL2 _daveSCP_send(int fd, uc * reqBlock) {
    S7OexchangeBlock* fdr;
    fdr=(S7OexchangeBlock*)reqBlock;
    fdr->headerlength=80;
    fdr->allways2 = 2;
    fdr->payloadStart= 80;
    return SCP_send(fd, fdr->payloadLength+80, reqBlock);
}

int daveSCP_receive(int h, uc * buffer) {
    int res, datalen;
    S7OexchangeBlock * fdr;
    fdr=(S7OexchangeBlock*) buffer;
    res=SCP_receive(h, 0xFFFF, &datalen, sizeof(S7OexchangeBlock), buffer);
    if (daveDebug & daveDebugByte) {
	_daveDump("header:",buffer, 80);
	_daveDump("data:",buffer+80, fdr->payloadLength);
	_daveDump("data:",buffer+80, fdr->payloadLength);
    }	
    return res;	
}    

int DECL2 _daveConnectPLCS7online (daveConnection * dc) {

    int res=0;
    uc p2[]={
	0x00,0x02,0x01,0x00,
	0x0C,0x01,0x00,0x00,
	0x00,0x06,
	0x00,0x00,0x00,0x00,0x00,0x01,
	0x00,0x02,0x01,
    };

    uc pa[]=	{0xF0, 0 ,0, 1, 0, 1, 3, 0xc0,};

    PDU pu2,pu1, *p;

    int a;
    
    S7OexchangeBlock * fdr;
    fdr=(S7OexchangeBlock*)(dc->msgOut);
    p2[9]=dc->MPIAdr;	///
    dc->PDUstartI=80;

    memset(fdr,0,80);
    fdr->user= 110;
    fdr->field6= 64;
    fdr->field8= 16642;

    a= _daveSCP_send(((int)dc->iface->fd.wfd), dc->msgOut);
    daveSCP_receive((int)(dc->iface->fd.rfd), dc->msgIn);

    memset(fdr,0,206);
    fdr->user= 111;
    fdr->field6= 64;
    fdr->field7= 1;
    fdr->field8= 16642;
    fdr->validDataLength= 126;
    fdr->payloadLength= 126;
    fdr->field10= 1;
    fdr->field13= 2;
    fdr->field12= 114;
    memcpy(&(fdr->payload),p2,sizeof(p2));

    a= _daveSCP_send((int)(dc->iface->fd.wfd), dc->msgOut);
    daveSCP_receive((int)(dc->iface->fd.rfd), dc->msgIn);
    
    memset(fdr,0,98);
//    fdr->user= 112;
    fdr->field6= 64;
    fdr->field7= 6;
    fdr->field8= 16642;
    fdr->validDataLength= 18;
    fdr->payloadLength= 18;
    fdr->field10= 1;
    
    p=&pu1;
//    p->header=dc->msgOut+dc->PDUstartO;
    p->header=fdr->payload;
    _daveInitPDUheader(p,1);
    _daveAddParam(p, pa, sizeof(pa));
    if (daveGetDebug() & daveDebugPDU)
	_daveDumpPDU(p);
    
    a= _daveSCP_send((int)(dc->iface->fd.wfd), dc->msgOut);
    daveSCP_receive((int)(dc->iface->fd.rfd), dc->msgIn);
    pu2.header=dc->msgIn+80;
    _daveSetupReceivedPDU(dc, &pu2);
    if (daveGetDebug() & daveDebugPDU)
	_daveDumpPDU(&pu2);

    memset(fdr,0,560);
    fdr->user= 0;
    fdr->field6= 64;
    fdr->field7= 7;
    fdr->field8= 16642;
    fdr->payloadLength= 480;
    fdr->field10= 1;

    a= _daveSCP_send((int)(dc->iface->fd.wfd), dc->msgOut);
    daveSCP_receive((int)(dc->iface->fd.rfd), dc->msgIn);
    _daveSetupReceivedPDU(dc, &pu2);
    if (daveGetDebug() & daveDebugPDU)
	_daveDumpPDU(&pu2);
    dc->maxPDUlength=daveGetU16from(pu2.param+6);
//    if (daveDebug & daveDebugConnect) 
	LOG2("\n*** Partner offered PDU length: %d\n\n",dc->maxPDUlength);
    return res;

/*
    memset((uc*)(&giveBack)+80,0,480);
    giveBack.payloadLength= 480;
    return _daveNegPDUlengthRequest(dc, &pu1);
*/
}

int DECL2 _daveSendMessageS7online(daveConnection *dc, PDU *p) {
    int a;
    int datalen;
    int len=p->hlen+p->plen+p->dlen;
    uc buffer[sizeof(S7OexchangeBlock)];
    S7OexchangeBlock* fdr;
    fdr=(S7OexchangeBlock*)dc->msgOut;
    memset(dc->msgOut,0,80);
//    fdr->user= 114;
    fdr->field6= 64;
    fdr->field7= 6;
    fdr->field8= 16642;
    fdr->validDataLength= len;
    fdr->payloadLength= len;
    fdr->field10= 1;
    
//    memcpy(&(fdr->payload),buffer,len);
    a= _daveSCP_send((int)(dc->iface->fd.wfd), dc->msgOut);
    SCP_receive((int)(dc->iface->fd.rfd), 0xFFFF, &datalen, sizeof(S7OexchangeBlock), buffer);
//    daveSCP_receive(dc->iface->fd.rfd, dc->msgIn);
    return 0;
}

int DECL2 _daveDisconnectPLCS7online (daveConnection * dc) {
    int co,er,a;
    S7OexchangeBlock reqBlock;
    S7OexchangeBlock* fdr;
    uc b1[sizeof(S7OexchangeBlock)];


    fdr=&reqBlock;

    memset(fdr,0,140);
    fdr->user= 102;
    fdr->priority= 0;
    
    fdr->field6= 64;
    fdr->field7= 12;
    fdr->field8= 255;
    fdr->payloadLength= 0;
    fdr->field10= 0x1;
//    fdr->functionCode= 0x1;
//    fdr->id3= 0x1;
    co=0;
    do{
	a= _daveSCP_send((int)(dc->iface->fd.wfd), (char *) &reqBlock);
	er=SCP_get_errno();
	printf("res 7:%d %d\n",a,er);
#ifdef BCCWIN
	Sleep(100);
#endif
#ifdef LINUX
	usleep(100000);
#endif
	co++;
    }	while ((a!=0)&&(co<10));
//    daveSCP_receive((int)(dc->iface->fd.rfd), b1);
    
    co=0;
    do{
	a=daveSCP_receive((int)(dc->iface->fd.rfd), b1);
	er=SCP_get_errno();
	printf("result 7:%d %d\n",a,er);
	co++;
    } while ((a!=0)&&(co<10));
	
	return 0;
}


int DECL2 _daveGetResponseS7online(daveConnection *dc) {
    int a;
    a= _daveSCP_send((int)(dc->iface->fd.rfd), dc->msgIn);
    daveSCP_receive((int)(dc->iface->fd.rfd), dc->msgIn);
    return 0;
}

int DECL2 _daveExchangeS7online(daveConnection * dc, PDU * p) {
    int res;
    res=_daveSendMessageS7online(dc, p);
    dc->AnswLen=0;
    res=_daveGetResponseS7online(dc);
    return res;
}

int DECL2 _daveListReachablePartnersS7online (daveInterface * di, char * buf) {
    int a;
    S7OexchangeBlock reqBlock;
    uc b1[sizeof(S7OexchangeBlock)];
    S7OexchangeBlock* fdr;
    fdr=&reqBlock;

    memset(fdr,0,140);
    fdr->user= 102;
    fdr->priority= 1;
    fdr->field6= 34; // 0x22 is FDL
    fdr->field8= 16642;
    fdr->payloadLength= 60;
    fdr->application_block_service= 0x28;

    a= _daveSCP_send((int)(di->fd.wfd), (uc *) &reqBlock);
    daveSCP_receive((int)(di->fd.rfd), b1);

    fdr->user= 103;
    fdr->priority= 1;
    fdr->field6= 34;
    fdr->field8= 16642;
    fdr->application_block_service= 0x17;

    a= _daveSCP_send((int)(di->fd.wfd), (uc *) &reqBlock);
    daveSCP_receive((int)(di->fd.rfd), b1);
    memset(fdr,0,140);

    fdr->user= 104;
    fdr->priority= 1;
    fdr->field6= 34;
    fdr->field8= 16642;
    fdr->payloadLength= 60;
    fdr->application_block_service= 0x28;
    a= _daveSCP_send((int)(di->fd.wfd), (uc *) &reqBlock);
    daveSCP_receive((int)(di->fd.rfd), b1);

    memset(fdr,0,208);
    fdr->user= 105;
    fdr->priority= 1;
    fdr->field6= 34;
    fdr->field8 =16642;
    fdr->payloadLength= 128;
    fdr->application_block_service= 0x1a;
    a= _daveSCP_send((int)(di->fd.wfd), (uc *) &reqBlock);
    daveSCP_receive((int)(di->fd.rfd), b1);
    
    memcpy(buf,b1+80,126);
    return 126;
}

/*
    This is not quite the same as in other protocols: Normally, we have a file descriptor or
    file handle in di->fd.rfd, di->fd.wfd. disconnectAdapter does something like making the
    MPI adapter leaving the Profibus token ring. File descriptor remains valid until it is 
    closed with closePort().
    In principle, instead of closing it, we could redo the sequence 
	daveNewInterface, initAdapter and then continue to use it.
    We cannot use closePort() on a "handle" retrieved from SCP_open(). It isn't a file handle.
    We cannot make closePort() treat it differently as there is no information in di->fd.rfd
    WHAT it is.
    - We could make di->fd.rfd a structure with extra information.
    - We could pass struct di->fd (daveOSserialtype) instead of di->fd.rfd / di->fd.wfd to all
      all functions dealing with di->fd.rfd. Then we could add extra information to 
      daveOSserialtype
    - We could better pas a pointer to an extended daveOSserialtype as it makes less problems
      when passing it to different programming languages.
    These would be major changes. They would give up the (theroetical?) possibility to use file 
    handles obtained in quite a different way and to put them into daveOSserialtype.
    I chose to change as little as possible for s7online and just SCP_close the handle here,
    expecting no one will try to reuse after this.
    
	Up to here is what version 0.8 does. The probleme is now that an application cannot do
	daveDisconnectAdapter(), closePort() as it does for other protocols.
    
    Now comes a second kludge for 0.8.1: We replace the "file handles" value by -1. Now we can 
    tell closePort() to do nothing for a value of -1.
    
    Befor releasing that, I think it is better to use different close functions, closeS7oline 
    for s7online and closePort() for everything else.
*/

/*
int DECL2 _daveDisconnectAdapterS7online(daveInterface * di) {
    int res;
    res=SCP_close((int)(di->fd.rfd));
    di->fd.rfd=-1;
    di->fd.wfd=-1;
    return res;
}
*/
/*
    01/09/07  Used Axel Kinting's version.
*/
