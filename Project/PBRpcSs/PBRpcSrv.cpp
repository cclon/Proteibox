/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/01/11 [11:1:2012 - 12:23]
* MODULE : \PBRpcSs\PBRpcSrv.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include <Sddl.h>
#include "HookHelper.h"
#include "PBRpcSrv.h"

//////////////////////////////////////////////////////////////////////////


DWORD g_dwCurrentState	= 0;
DWORD g_dwWin32ExitCode = 0;
DWORD g_dwCheckPoint	= 0;

HANDLE g_Handle_Mapped = NULL;


// Services
static HOOKINFOLittleRpcss g_HookInfoServices_Array [] = 
{
	{ ADVAPI32_TAG, SetServiceStatus_TAG, "SetServiceStatus", "", 0, NULL, fake_SetServiceStatus },
	{ ADVAPI32_TAG, StartServiceCtrlDispatcherW_TAG, "StartServiceCtrlDispatcherW", "", 0, NULL, fake_StartServiceCtrlDispatcherW },
	{ ADVAPI32_TAG, OpenServiceW_TAG, "OpenServiceW", "", 0, NULL, fake_OpenServiceW },
	{ ADVAPI32_TAG, CloseServiceHandle_TAG, "CloseServiceHandle", "", 0, NULL, fake_CloseServiceHandle },
	{ ADVAPI32_TAG, QueryServiceStatus_TAG, "QueryServiceStatus", "", 0, NULL, fake_QueryServiceStatus },
	{ ADVAPI32_TAG, QueryServiceStatusEx_TAG, "QueryServiceStatusEx", "", 0, NULL, fake_QueryServiceStatusEx },
	{ ADVAPI32_TAG, StartServiceW_TAG, "StartServiceW", "StartService", 0, NULL, fake_StartService },
	{ ADVAPI32_TAG, ControlService_TAG, "ControlService", "", 0, NULL, fake_ControlService },
	{ Kernel32_TAG, CreateFileMappingW_TAG, "CreateFileMappingW", "", 0, NULL, fake_CreateFileMappingW },
};

#define GetServiceFunc( X )	 g_HookInfoServices_Array[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL InitHook_pharase0_Service()
{
	int i = 0;
	BOOL bRet = FALSE;
	char* lpFuncName = NULL;
	LPHOOKINFOLittleRpcss pArray = g_HookInfoServices_Array ;
	int ArrayCounts = ARRAYSIZEOF( g_HookInfoServices_Array );
	HMODULE* phModuleArray = __ProcModulesInfo->hModuleArrays;

	for( i=0; i<ArrayCounts; i++ )
	{
		if ( phModuleArray[KernelBase_TAG] && pArray[i].FunctionNameEx )
			lpFuncName	= pArray[i].FunctionNameEx;
		else
			lpFuncName	= pArray[i].FunctionName;

		pArray[i].OrignalAddress = (PVOID) GetProcAddress( phModuleArray[pArray[i].DllTag], lpFuncName );
	}

	for( i=0; i<ArrayCounts; i++ )
	{
		bRet = HookOne( &pArray[i] );
		if ( FALSE == bRet )
		{
			MYTRACE( "err! | InitHook_pharase0_Service() - HookOne(); | \"%s\" \n", pArray[i].FunctionName );
			return FALSE ;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

ULONG WINAPI
fake_SetServiceStatus (
	SC_HANDLE hService,
	LPSERVICE_STATUS lpServiceStatus
	)
{
	g_dwCurrentState = lpServiceStatus->dwCurrentState;
	if ( lpServiceStatus->dwCurrentState & SERVICE_STOPPED )
		g_dwWin32ExitCode = lpServiceStatus->dwWin32ExitCode;
	else
		g_dwCheckPoint = lpServiceStatus->dwCheckPoint;
	
	return 1;
}


ULONG WINAPI
fake_StartServiceCtrlDispatcherW (
	IN LPSERVICE_TABLE_ENTRYW lpServiceStartTable
	)
{
	ShowTickcount("enter fake_StartServiceCtrlDispatcherW");

	LPSERVICE_TABLE_ENTRYW pNodeCur = lpServiceStartTable;
	if ( NULL == pNodeCur ) { return 1; }

	do 
	{
		if ( NULL == pNodeCur->lpServiceProc )
			break;

		pNodeCur->lpServiceProc( 1, (LPWSTR *)&pNodeCur->lpServiceName );
		++ pNodeCur;

	} while (pNodeCur);

	ShowTickcount("leave fake_StartServiceCtrlDispatcherW");
	return 1;
}


ULONG WINAPI
fake_OpenServiceW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    DWORD dwDesiredAccess
    )
{
	SetLastError(NO_ERROR);
	if ( 0 == wcsicmp(lpServiceName, L"RpcSs") )
		return (ULONG)0x12345672;
	else if ( 0 == wcsicmp(lpServiceName, L"MSIServer") )
		return (ULONG)0x12345673;
	else if ( 0 == wcsicmp(lpServiceName, L"EventSystem") )
		return (ULONG)0x12345674;

	_OpenServiceW_ OrignalAddr = (_OpenServiceW_) GetServiceFunc(OpenServiceW);
	SC_HANDLE hService = (SC_HANDLE) OrignalAddr( hSCManager, lpServiceName, dwDesiredAccess );
	DWORD dwErrorCode = GetLastError();

	if ( PB_IsBoxedService(hService) )
	{
		SetLastError(dwErrorCode);
		return (ULONG)hService;
	}
	else
	{
		SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);
	}

	return 0;	
}


ULONG WINAPI
fake_CloseServiceHandle (
    SC_HANDLE hService
    )
{
	ULONG ret = 0;
	if ( (ULONG)hService <= 0x12345670 || (ULONG)hService >= 0x12345679 )
	{
		_CloseServiceHandle_ OrignalAddr = (_CloseServiceHandle_) GetServiceFunc(CloseServiceHandle);
		ret = OrignalAddr(hService);
	}
	else
	{
		SetLastError(NO_ERROR);
		ret = 1;
	}

	return ret;
}


ULONG WINAPI
fake_QueryServiceStatus (
    SC_HANDLE           hService,
    LPSERVICE_STATUS    lpServiceStatus
    )
{
	SERVICE_STATUS_PROCESS Buffer = {};

	DWORD cbBytesNeeded = 0;
	ULONG ret = QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&Buffer, 0x24, &cbBytesNeeded );
	if ( ret )
	{
		lpServiceStatus->dwServiceType = Buffer.dwServiceType;
		lpServiceStatus->dwCurrentState = Buffer.dwCurrentState;
		lpServiceStatus->dwControlsAccepted = Buffer.dwControlsAccepted;
		lpServiceStatus->dwWin32ExitCode = Buffer.dwWin32ExitCode;
		lpServiceStatus->dwServiceSpecificExitCode = Buffer.dwServiceSpecificExitCode;
		lpServiceStatus->dwCheckPoint = Buffer.dwCheckPoint;
		lpServiceStatus->dwWaitHint = Buffer.dwWaitHint;
	}

	return ret;
}


ULONG WINAPI
fake_QueryServiceStatusEx (
	HANDLE hService, 
	int InfoLevel,
	LPSERVICE_STATUS_PROCESS lpBuffer, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	)
{
	if ( InfoLevel )
	{
		SetLastError(ERROR_INVALID_LEVEL);
		return 0;
	}

	if ( cbBufSize < 0x24 )
	{
		*pcbBytesNeeded = 0x24;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}

	lpBuffer->dwServiceType = 0x10;
	lpBuffer->dwCurrentState = 4;
	lpBuffer->dwControlsAccepted = 5;
	lpBuffer->dwWin32ExitCode = 0;
	lpBuffer->dwServiceSpecificExitCode = 0;
	lpBuffer->dwCheckPoint = 0;
	lpBuffer->dwWaitHint = 0;
	lpBuffer->dwServiceFlags = 1;
	lpBuffer->dwProcessId = 0;

	if ( hService == (HANDLE)0x12345672 )
	{
		lpBuffer->dwServiceType = 0x20;
		lpBuffer->dwProcessId = GetSpecPID(  L"PBRpcSs.exe", FALSE );
		SetLastError(0);
		return 1;
	}
	else if ( hService == (HANDLE)0x12345673 )
	{
		lpBuffer->dwProcessId = GetSpecPID(  L"msiexec.exe", TRUE );
		SetLastError(0);
		return 1;
	}
	else if ( hService == (HANDLE)0x12345674 )
	{
		lpBuffer->dwProcessId = 0;
		SetLastError(0);
		return 1;
	}
	else
	{
		_QueryServiceStatusEx_ OrignalAddr = (_QueryServiceStatusEx_) GetServiceFunc(QueryServiceStatusEx);
		return OrignalAddr( hService, 0, lpBuffer, cbBufSize, pcbBytesNeeded );
	}

	return 0;
}


ULONG WINAPI
fake_StartService (
	SC_HANDLE hService, 
	DWORD dwNumServiceArgs, 
	LPCWSTR *lpServiceArgVectors
	)
{
	ULONG ret = 0;

	if ( hService == (SC_HANDLE)0x12345673 )
	{
		ret = PB_StartBoxedService( L"MSIServer" );
	}
	else
	{
		if ( PB_IsBoxedService(hService) )
		{
			_StartService_ OrignalAddr = (_StartService_) GetServiceFunc(StartServiceW);
			ret = OrignalAddr(hService, dwNumServiceArgs, lpServiceArgVectors);
		}
		else
		{
			SetLastError(ERROR_ACCESS_DENIED);
			ret = 0;
		}
	}

	return ret;
}


ULONG WINAPI
fake_ControlService (
	SC_HANDLE hService,
	DWORD dwControl,
	LPSERVICE_STATUS lpServiceStatus
	)
{
	ULONG ret = 0;
	_ControlService_ OrignalAddr = (_ControlService_) GetServiceFunc(ControlService);

	if ( PB_IsBoxedService((SC_HANDLE)hService) )
		ret = OrignalAddr( hService, dwControl, lpServiceStatus );
	else
		ret = fake_QueryServiceStatus( hService, lpServiceStatus );

	return ret;
}


ULONG WINAPI
fake_CreateFileMappingW (
	HANDLE hFile, 
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes, 
	int flProtect, 
	int dwMaximumSizeHigh, 
	int dwMaximumSizeLow, 
	LPCWSTR lpName
	)
{
	ULONG ret = 0;

	if ( NULL == lpName || wcsicmp(lpName, L"Global\\ComPlusCOMRegTable") || (ret = (ULONG)g_Handle_Mapped, !g_Handle_Mapped) )
	{
		_CreateFileMappingW_ OrignalAddr = (_CreateFileMappingW_) GetServiceFunc(CreateFileMappingW);
		ret = OrignalAddr( hFile,lpFileMappingAttributes,flProtect,dwMaximumSizeHigh,dwMaximumSizeLow,lpName );
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////

DWORD GetSpecPID(LPWSTR szPorcessName, BOOL bFlag)
{
	NTSTATUS status = STATUS_SUCCESS ;
	PB_BOX_INFO BoxInfo = {} ;
	BOOL bResult = FALSE;
	LPWSTR StringSid = NULL;
	PTOKEN_USER TokenInfo = NULL ;
	CHAR TokenInfoBuffer[ 0x40 ] = {};
	HANDLE hProcess = NULL, hToken = NULL ;
	DWORD PIDRet = 0, PID = 0, TotalProcCounts = 0, nIndex = 0, ReturnLength = 0 ;

	ULONG pBuffer = (ULONG) kmalloc( 0x800 );
	status = PB_EnumProcessEx( L"DefaultBox", (int*)pBuffer );
	if ( ! NT_SUCCESS(status) ) { goto _end_ ; }

	TotalProcCounts = *(DWORD *)pBuffer ;
	if ( TotalProcCounts < 1 ) { goto _end_ ; }

	while ( TRUE )
	{
		PID = *(ULONG *)( pBuffer + 4 * nIndex );
		if (  PB_QueryProcess(PID, &BoxInfo) && 0 == wcsicmp(BoxInfo.BoxName, szPorcessName) )
		{
			bResult = FALSE;
			if ( FALSE == bFlag )
			{
				PIDRet = PID;
				break;
			}

			hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, PID );
			if ( hProcess ) 
			{
				if ( OpenProcessToken(hProcess, 8, &hToken) )
				{
					ReturnLength = 0x40;
					if ( GetTokenInformation(hToken, TokenUser, (LPVOID)TokenInfoBuffer, 0x40, &ReturnLength) )
					{
						TokenInfo = (PTOKEN_USER)TokenInfoBuffer;
						ConvertSidToStringSidW( TokenInfo->User.Sid, &StringSid );
						if ( 0 == wcscmp(StringSid, L"S-1-5-18") )
							bResult = TRUE;
						
						LocalFree(StringSid);
					}

					CloseHandle(hToken);
				}

				CloseHandle(hProcess);
				if ( bResult )
				{
					PIDRet = PID;
					break;
				}
			}
			else
			{
				if ( GetLastError() == ERROR_ACCESS_DENIED )
				{
					PIDRet = PID;
					break;
				}
			}
		} 

		++ nIndex ;
		if ( nIndex > TotalProcCounts ) { break; }
	}

_end_ :
	kfree( (PVOID)pBuffer );
	return PIDRet;
}


///////////////////////////////   END OF FILE   ///////////////////////////////