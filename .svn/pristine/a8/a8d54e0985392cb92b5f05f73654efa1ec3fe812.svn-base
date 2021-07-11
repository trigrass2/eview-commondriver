
#ifndef _MODBUS_DRIVER_H_
#define _MODBUS_DRIVER_H_

#include "pkdriver/pkdrvcmn.h"
#include <vector>
using namespace std;

/**
 *  是否 little_endian序.
 *
 *
 */
inline bool is_little_endian()
{
	const int i = 1;
	char *ch = (char*)&i;
	return ch[0] && 0x01;
}

/**
 *  字节顺序转换.
 *
 *  @param  -[in, out]  char*  pOrig: [comment]
 *  @param  -[in]  int  nLength: [comment]
 *
 */
inline long SwapByteOrder(char* pOrig, int nLength, int nWordBytes)
{
	int i = 0;
	int nSwapCount = 0;
	char chTemp;
	
	if (nWordBytes == 2)
	{
		nSwapCount = nLength / 2;
		for(i = 0; i < nSwapCount; i++)
		{
			chTemp = *pOrig;
			*pOrig = *(pOrig + 1);
			*(pOrig + 1) = chTemp;
			pOrig += 2;
		}
	}
	else if (nWordBytes == 4)
	{
		nSwapCount = nLength / 4;
		for(i = 0; i < nSwapCount; i++)
		{
			// 第0和第3个字节交换
			chTemp = *(pOrig + 3);
			*(pOrig + 3) = *pOrig; 
			*pOrig = chTemp;

			// 第1和第2个字节交换
			chTemp = *(pOrig + 2);
			*(pOrig + 2) = *(pOrig + 1);
			*(pOrig + 1) = chTemp;

			pOrig += 4;
		}
		// 不足4个字节的部分
		if (nLength - nSwapCount * 4 == 2)
		{
			// 剩余两个字节
			chTemp = *pOrig;
			*pOrig = *(pOrig + 1);
			*(pOrig + 1) = chTemp;
		}
		else if (nLength - nSwapCount * 4 == 3)
		{
			// 剩余三个字节
			chTemp = *pOrig;
			*pOrig = *(pOrig + 2);
			*(pOrig + 2) = chTemp;
		}
	}
	return PK_SUCCESS;
}

#endif //_MODBUS_DRIVER_H_ 