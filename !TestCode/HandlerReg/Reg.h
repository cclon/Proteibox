/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/01/05 [5:1:2010 - 14:38]
* MODULE : D:\Program\R0\Coding\…≥œ‰\SandBox\Code\HandlerReg\Reg.h
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include<string.h>

#include "ntdll.h"
#pragma comment(lib, "ntdll.lib")

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

NTSTATUS
HRCreateKey(
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	) ;

NTSTATUS
HRCreateKeyEx(
	IN ACCESS_MASK DesiredAccess,
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	) ;


NTSTATUS
HROpenKey(
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	) ;

NTSTATUS
HROpenKeyEx(
	IN ACCESS_MASK DesiredAccess,
	IN PCWSTR wszRegPath,
	OUT HANDLE *hKey
	) ;

NTSTATUS
HRSetValueKey(
	IN PCWSTR wszRegPath,
	IN PCWSTR wszRegValue,
	IN PCWSTR Buffer,
	IN ULONG Type
	) ;



///////////////////////////////   END OF FILE   ///////////////////////////////
