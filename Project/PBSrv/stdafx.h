// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将为显式的

// 关闭 ATL 对某些常被安全忽略的常见警告消息的隐藏
#define _ATL_ALL_WARNINGS

// USES_CONVERSION 将不使用线程级的地域标志
#define _CONVERSION_DONT_USE_THREAD_LOCALE

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

// Windows Header Files:
#include <process.h>
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <winbase.h>
#include <winsvc.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windef.h>

#include <strsafe.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <mountmgr.h>
#include <ObjIdl.h>
#include <ShellAPI.h>

#include <ntdll.h>
#pragma comment(lib, "ntdll.lib")

#include "../Base/Ioctl.h"
#include "../ProteinBoxDLL/Exportfunc.h"
#include "Common.h"
#include "MemoryManager.h"
#include "Global.h"
#include "Outputdebug.h"


//////////////////////////////////////////////////////////////////////////

#define STATUS_CANT_OPEN_ANONYMOUS		((NTSTATUS) 0xc00000a6)
#define STATUS_INVALID_SYSTEM_SERVICE	((NTSTATUS) 0xc000001c)


