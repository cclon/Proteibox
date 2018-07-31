// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A7F323DE_6BD2_40B6_81EE_25390E82641E__INCLUDED_)
#define AFX_STDAFX_H__A7F323DE_6BD2_40B6_81EE_25390E82641E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#define STRICT

#define WINVER 0x501
#define _WIN32_WINNT 0x0501
#define NTDDI_VERSION NTDDI_WINXP

// #ifndef _WIN32_WINNT
// #define _WIN32_WINNT 0x600
// #endif
#define _ATL_APARTMENT_THREADED

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将为显式的

// 关闭 ATL 对某些常被安全忽略的常见警告消息的隐藏
#define _ATL_ALL_WARNINGS

// USES_CONVERSION 将不使用线程级的地域标志
#define _CONVERSION_DONT_USE_THREAD_LOCALE

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

// SDK
#include <time.h>

// WTL
#include <atlapp.h>
#include <atlmisc.h>
#include <atlwin.h>
#include <atlcrack.h>
#include <atlcomcli.h>

// STL
#include<vector> 
using namespace std;

#include <shlwapi.h>
#include <Shlobj.h>
#include <WinIoCtl.h>
//#include <winternl.h>
#include <Sddl.h>

// TODO: reference additional headers your program requires here
// StringSafe
#include <strsafe.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <mountmgr.h>
#include <ObjIdl.h>
#include <ShellAPI.h>

#include <ntdll.h>
#pragma comment(lib, "ntdll.lib")

#include <WinCrypt.h>
#pragma comment(lib, "Crypt32.lib") 

// TODO: reference additional headers your program requires here
#include "Common.h"
#include "Outputdebug.h"
#include "..\Base\Ioctl.h"
#include "..\Base\mhoolib\mhook.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A7F323DE_6BD2_40B6_81EE_25390E82641E__INCLUDED_)
