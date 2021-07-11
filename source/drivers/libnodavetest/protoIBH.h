
#ifndef __simens_IBHGateway
#define __simens_IBHGateway


#ifdef __cplusplus
extern "C" {		// All here is C, *** NOT *** C++
#endif

#include "nodave.h"

/*
    Header for MPI packets on IBH-NetLink:
*/
/* 
    This is the packet header used by IBH ethernet NetLink. 
*/
typedef struct {
    uc ch1;	// logical connection or channel ?
    uc ch2;	// logical connection or channel ?
    uc len;	// number of bytes counted from the ninth one.
    uc packetNumber;	// a counter, response packets refer to request packets
    us sFlags;		// my guess
    us rFlags;		// my interpretation
} IBHpacket;

/*
    make internal IBH functions available for experimental use:
*/
EXPORTSPEC int DECL2 _daveReadIBHPacket(daveInterface * di,uc *b);
EXPORTSPEC int DECL2 _daveWriteIBH(daveInterface * di, uc * buffer, int len);
EXPORTSPEC int DECL2 _davePackPDU(daveConnection * dc,PDU *p);
EXPORTSPEC void DECL2 _daveSendMPIAck_IBH(daveConnection*dc);
EXPORTSPEC void DECL2 _daveSendIBHNetAck(daveConnection * dc);
EXPORTSPEC int DECL2 __daveAnalyze(daveConnection * dc);
EXPORTSPEC int DECL2 _daveExchangeIBH(daveConnection * dc, PDU * p);
EXPORTSPEC int DECL2 _daveSendMessageMPI_IBH(daveConnection * dc, PDU * p);
EXPORTSPEC int DECL2 _daveGetResponseMPI_IBH(daveConnection *dc);
EXPORTSPEC int DECL2 _daveInitStepIBH(daveInterface * iface, uc * chal, int cl, us* resp,int rl, uc*b);

EXPORTSPEC int DECL2 _daveConnectPLC_IBH(daveConnection*dc);
EXPORTSPEC int DECL2 _daveDisconnectPLC_IBH(daveConnection*dc);
EXPORTSPEC void DECL2 _daveSendMPIAck2(daveConnection *dc);
EXPORTSPEC int DECL2 _davePackPDU_PPI(daveConnection * dc,PDU *p);
EXPORTSPEC void DECL2 _daveSendIBHNetAckPPI(daveConnection * dc);
EXPORTSPEC int DECL2 _daveListReachablePartnersMPI_IBH(daveInterface *di, char * buf);
EXPORTSPEC int DECL2 __daveAnalyzePPI(daveConnection * dc, uc sa);
EXPORTSPEC int DECL2 _daveExchangePPI_IBH(daveConnection * dc, PDU * p);
EXPORTSPEC int DECL2 _daveGetResponsePPI_IBH(daveConnection *dc);

/*
    Special function do disconnect arbitrary connections on IBH-Link:
*/
EXPORTSPEC int DECL2 daveForceDisconnectIBH(daveInterface * di, int src, int dest, int mpi);
/*
    Special function do reset an IBH-Link:
*/
EXPORTSPEC int DECL2 daveResetIBH(daveInterface * di);

#ifdef __cplusplus
}
#endif

#endif

/*
    01/09/07  Used Axel Kinting's version.
*/
