#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "targetver.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <ShellAPI.h>
#include <shlwapi.h>
#include <ShlObj.h>
#pragma comment(lib, "shlwapi.lib")

#include <ntdll.h>
#pragma comment(lib, "ntdll.lib")

#include "../../Base/Ioctl.h"
#include "../../ProteinBoxDLL/Exportfunc.h"
#include "Common.h"
#include "Outputdebug.h"


//////////////////////////////////////////////////////////////////////////

#define STATUS_ALREADY_COMMITTED ((NTSTATUS)0xC0000021L)


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
