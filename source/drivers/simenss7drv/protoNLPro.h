
#ifndef __simens_NLPRO
#define __simens_NLPRO


#ifdef __cplusplus
extern "C" {		// All here is C, *** NOT *** C++
#endif

#include "nodave.h"

/*
	IBH NetLink Pro Gateway
*/
EXPORTSPEC int DECL2 _daveInitAdapterNLpro(daveInterface * di);
EXPORTSPEC int DECL2 _daveInitStepNLpro(daveInterface * iface, int nr, uc* fix, int len, char*caller, uc * buffer);
EXPORTSPEC int DECL2 _daveConnectPLCNLpro (daveConnection * dc);
EXPORTSPEC int DECL2 _daveSendMessageNLpro(daveConnection *dc, PDU *p);
EXPORTSPEC int DECL2 _daveGetResponseNLpro(daveConnection *dc);
EXPORTSPEC int DECL2 _daveExchangeNLpro(daveConnection * dc, PDU * p);
EXPORTSPEC int DECL2 _daveSendDialogNLpro(daveConnection * dc, int size);
EXPORTSPEC int DECL2 _daveSendWithPrefixNLpro(daveConnection * dc, uc * b,int size);
EXPORTSPEC int DECL2 _daveSendWithPrefix2NLpro(daveConnection * dc, int size);
EXPORTSPEC int DECL2 _daveDisconnectPLCNLpro(daveConnection * dc);
EXPORTSPEC int DECL2 _daveDisconnectAdapterNLpro(daveInterface * di);
EXPORTSPEC int DECL2 _daveListReachablePartnersNLpro(daveInterface * di, char * buf);


#ifdef __cplusplus
}
#endif

#endif

/*
    01/09/07  Used Axel Kinting's version.
*/
