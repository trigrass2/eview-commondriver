#include "pkdriver/pkdrvcmn.h"
#include "S7Device.h"
#include "pkdriver/pkdrvcmn.h"
#include <vector>
using namespace std;

#include "nodave.h"
#include "log2.h"
#include "memory.h"

bool FindSpecialChar(unsigned char *szMsg, int nMsgLen, unsigned char ucChar)
{
	for (int i = 0; i < nMsgLen; i++)
	{
		if (szMsg[i] == ucChar)
			return true;
	}
	return false;
}

int DECL2 _daveGetResponsePPI(daveConnection *dc) {
	int res, expectedLen, expectingLength, i, sum, alt;
	uc * b;
	res = 0;
	expectedLen = 6;
	expectingLength = 1;
	b = dc->msgIn;
	alt = 1;
	
	while ((expectingLength) || (res < expectedLen)) {
		int nLeftBuffLen = sizeof(dc->msgIn) - res;
		if (nLeftBuffLen < 0)
		{
			res = 0;
			nLeftBuffLen = sizeof(dc->msgIn) - res;
		}

		i = dc->iface->ifread(dc->iface, (char *)(dc->msgIn + res), nLeftBuffLen);
		res += i;
		if ((daveDebug & daveDebugByte) != 0)
		{
			LOG3("i:%d res:%d\n", i, res);
			FLUSH;
		}

		if (i == 0) {
			return daveResTimeout;
		}

		if ((expectingLength) && (res >= 1) && FindSpecialChar(dc->msgIn, res, 0xE5))
		{
			if (alt)
			{
				_daveSendRequestData(dc, alt);
				res = 0;
				alt = 0;
			}
			else {
				_daveSendRequestData(dc, alt);
				res = 0;
				alt = 1;
			}
		}

		if(!expectingLength)
			continue;

		unsigned char *pMsg = dc->msgIn;
		unsigned char *pMsgEnd = dc->msgIn + res;
		bool bFoundPackReponseHeader = false;
		while (pMsg <= pMsgEnd-4) // 因为有很多乱七八糟的数据，因此需要一直找到满足b[0] == b[3]) && (b[1] == b[2]的地方才能作为数据的开始
		{
			if (pMsg[0] == pMsg[3] && pMsg[1] == pMsg[2])
			{
				bFoundPackReponseHeader = true;
				if (pMsg != dc->msgIn) // 如果头部不是希望的数据，那么去掉前面的数据，直到符合头部数据为止;
				{
					res = pMsgEnd - pMsg;
					memcpy(dc->msgIn, pMsg, res);
				}
				expectedLen = pMsg[1] + 6;
				expectingLength = 0;
				b = dc->msgIn;
				break;
			}
			pMsg++;
		}
	} // while

	if (res > expectedLen) // 抛弃包尾收到的其他人请求的脏数据;
		res = expectedLen;

	if ((daveDebug & daveDebugByte) != 0) {
		LOG2("res %d testing lastChar\n", res);
	}
	if (b[res - 1] != SYN) {
		LOG1("block format error\n");
		return 1024;
	}
	if ((daveDebug & daveDebugByte) != 0) {
		LOG1("testing check sum\n");
	}
	sum = 0;
	for (i = 4; i < res - 2; i++){
		sum += b[i];
	}
	sum = sum & 0xff;
	if ((daveDebug & daveDebugByte) != 0) {
		LOG3("I calc: %x sent: %x\n", sum, b[res - 2]);
	}
	if (b[res - 2] != sum) {
		if ((daveDebug & daveDebugByte) != 0) {
			LOG1("checksum error\n");
		}
		return 2048;
	}
	return 0;
}

int seconds, thirds;
int DECL2 _daveExchangePPI(daveConnection * dc, PDU * p1) {
	int i, res = 0, len;
	dc->msgOut[0] = dc->MPIAdr;	/* address ? */
	dc->msgOut[1] = dc->iface->localMPI;
	dc->msgOut[2] = 108;
	len = 3 + p1->hlen + p1->plen + p1->dlen;	/* The 3 fix bytes + all parts of PDU */
	_daveSendLengthAndIt(dc->iface, len, dc->msgOut);
 	//_daveSendLength(dc->iface, len);
	//_daveSendIt(dc->iface, dc->msgOut, len);	
	i = dc->iface->ifread(dc->iface, (char *)(dc->msgIn + res), 1);
	if ((daveDebug & daveDebugByte) != 0) {
		LOG3("i:%d res:%d\n", i, res);
		_daveDump("got", dc->msgIn, i); // 5.1.2004
	}
	if (i == 0) {
		seconds++;
		_daveSendLengthAndIt(dc->iface, len, dc->msgOut);
		//_daveSendLength(dc->iface, len);
		//_daveSendIt(dc->iface, dc->msgOut, len);
		i = dc->iface->ifread(dc->iface,(char *)(dc->msgIn + res), 1);
		if (i == 0) {
			thirds++;
			_daveSendLengthAndIt(dc->iface, len, dc->msgOut);
			//_daveSendLength(dc->iface, len);
			//_daveSendIt(dc->iface, dc->msgOut, len);
			i = dc->iface->ifread(dc->iface, (char *)(dc->msgIn + res), 1);
			if (i == 0) {
				LOG1("timeout in _daveExchangePPI!\n");
				FLUSH;
				return daveResTimeout;
			}
		}
	}
	_daveSendRequestData(dc, 0);
	return _daveGetResponsePPI(dc);
}

int DECL2 _daveConnectPLCPPI(daveConnection * dc) {
	PDU p;
	return _daveNegPDUlengthRequest(dc, &p);
}