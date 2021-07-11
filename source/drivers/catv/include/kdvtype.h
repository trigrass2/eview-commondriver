/*****************************************************************************
   模块名      : KDV type 
   文件名      : kdvtype.h
   相关文件    : 
   文件实现功能: KDV宏定义
   作者        : 魏治兵
   版本        : V3.0  Copyright(C) 2001-2002 KDC, All rights reserved.
-----------------------------------------------------------------------------
   修改记录:
   日  期      版本        修改人      修改内容
   2004/02/17  3.0         魏治兵        创建
******************************************************************************/
#ifndef _KDV_TYPE_H_
#define _KDV_TYPE_H_
#ifdef _MAC_IOS_
#include <string.h>
#else
#include <iostream>
#endif

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	  /* Type definition */
/*-----------------------------------------------------------------------------
系统公用文件，开发人员严禁修改
------------------------------------------------------------------------------*/

typedef int		s32,BOOL32;
typedef unsigned long   u32;
typedef unsigned char	u8;
typedef unsigned short  u16;
typedef short           s16;
typedef char            s8;

typedef int         l32;
    
#ifndef WIN32
typedef unsigned long DWORD;
#define VOID void
#endif
    
#ifdef _MAC_IOS_
#define PACKED
#endif


#if defined(ANDROID)||defined(__ANDROID__)
typedef int  BOOL;
#endif

#ifdef _MSC_VER
typedef __int64			s64;
#else 
typedef long long		s64;
#endif 

#ifdef _MSC_VER
typedef unsigned __int64 u64;
#else 
typedef unsigned long long u64;
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif
    
#ifndef NULL
#define NULL 0
#endif
    
#ifndef _MSC_VER
    #ifndef LPSTR
    #define LPSTR   char *
    #endif
    #ifndef LPCSTR
    #define LPCSTR  const char *
    #endif
#endif
    
#if defined(UNICODE) || defined( _UNICODE )
    /*
     * NOTE: This tests UNICODE, which is different from the _UNICODE define
     *       used to differentiate standard C runtime calls.
     */
    typedef wchar_t TCHAR;
    typedef wchar_t _TCHAR;
    
    
#define _T( str ) L( str )
    
    
#else
    typedef char TCHAR;
    typedef char _TCHAR;
#define _T( str ) str
#endif
    
#ifdef WIN32
	typedef TCHAR *PTCH;
#else
	typedef TCHAR TBYTE,*PTCH,*PTBYTE;
#endif
    
    typedef TCHAR *LPTCH,*PTSTR,*LPTSTR,*LP,*PTCHAR;
    typedef const TCHAR *LPCTSTR;
    
    
#ifdef UNICODE
    
#define tstring wstring
#define  tcin	wcin
#define  tcout	wcout
#define tstringstream wstringstream
#define tfstram		wfstream
#define tofstream	wofstream
#define tifstream	wifstream
    
#else
    
#define  tstring string
#define tcin cin
#define tcout cout
#define tstringstream stringstream
#define tfstram		fstream
#define tofstream	ofstream
#define tifstream	ifstream
    
#endif


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _KDV_def_H_ */

/* end of file kdvdef.h */

