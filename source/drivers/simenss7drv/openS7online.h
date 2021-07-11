/*
 Part of Libnodave, a free communication libray for Siemens S7 300/400.
 
 (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2002, 2003.2004

 Libnodave is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 Libnodave is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Libnodave; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  
*/
/* use s7onlinx.dll for transport */
#ifndef __openS7online
#define __openS7online


#ifdef __cplusplus
extern "C" {		// All here is C, *** NOT *** C++
#endif

#include "nodave.h"

//#ifdef WIN32
//#define us unsigned short

////EXPORTSPEC HANDLE DECL2 openS7online(const char * accessPoint);
//EXPORTSPEC HANDLE DECL2 openS7online(const char * accessPoint, HWND handle);
//EXPORTSPEC HANDLE DECL2 closeS7online(int h);
//#endif


/*
    Prototypes for the functions in S7onlinx.dll:
    They are guessed.
*/
typedef int (DECL2 * _openFunc) (const uc *);
typedef int (DECL2 * _closeFunc) (int);
typedef int (DECL2 * _sendFunc) (int, us, uc *);
typedef int (DECL2 * _receiveFunc) (int, us, int *, us, uc *);
//typedef int (DECL2 * _SetHWndMsgFunc) (int, int, ULONG);
//typedef int (DECL2 * _SetHWndFunc) (int, HANDLE);
typedef int (DECL2 * _get_errnoFunc) (void);

/*
    And pointers to the functions. We load them using GetProcAddress() on their names because:
    1. We have no .lib file for s7onlinx.
    2. We don't want to link with a particular version.
    3. Libnodave shall remain useable without Siemens .dlls. So it shall not try to access them
       unless the user chooses the daveProtoS7online transport.
*/
extern _openFunc SCP_open;
extern _closeFunc SCP_close;
extern _sendFunc SCP_send;
extern _receiveFunc SCP_receive;
//_SetHWndMsgFunc SetSinecHWndMsg;
//_SetHWndFunc SetSinecHWnd;
extern _get_errnoFunc SCP_get_errno;
/*
    A block of data exchanged between S7onlinx.dll and a program using it. Most fields seem to 
    be allways 0. Meaningful names are guessed.
*/

typedef struct {
    us		unknown [2];
    uc		headerlength;
    us		user;
    uc		allways2;
    uc		priority;
    uc		unknown3 [3];
    uc		field6;
    uc		field7;
    us		field8;
    us		validDataLength;
    uc          unknown11;
    us          payloadLength;
    us		payloadStart;
    uc		unknown2[12];
    uc		field10;
    us		id3;
    short 	application_block_service;
    uc		unknown4[2];
    uc          field13;
    uc          field11;
    uc          field12;
    uc		unknown6[35];
    uc          payload[520];
} S7OexchangeBlock;

#ifdef __cplusplus
}
#endif

#endif

/*
    01/09/07  Used Axel Kinting's version.
*/
