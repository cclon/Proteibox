//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HookHelper.h"
#include "PBRpcSrv.h"
#include "PBRpcToken.h"
#include "PBRpcWinsock.h"

//////////////////////////////////////////////////////////////////////////

#define __lpServiceName		L"RPCSS" 
#define __lpServiceName_RpcEptMapper	L"RpcEptMapper"
#define _CompletedEvent_	L"PB_ServiceInitComplete_%s"

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

VOID DoWork();
BOOL StartServiceCtrlDispatcher_RpcEptMapper();
BOOL MyStartServiceCtrlDispatcher(LPWSTR lpServiceName,LPWSTR lpDllName,LPSTR lpFuncName,BOOL bFlag);
DWORD SetEventServiceInitComplete(LPWSTR lpServiceName);
VOID CreateXXThread();
VOID ThreadProc(LPVOID lpParameter);


//////////////////////////////////////////////////////////////////////////
// int _tmain(int argc, _TCHAR* argv[])
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	DoWork();
	return 0;
}


VOID DoWork()
{
	GethModule();

	if (   InitHook_pharase0_Service() 
		&& InitHook_pharase1_Token()
		&& InitHook_pharase2_WinSock()
		)
	{	
		g_Handle_Mapped = CreateFileMappingW (
			(HANDLE)0xFFFFFFFF,
			NULL,
			PAGE_READWRITE | SEC_COMMIT/*0x8000004*/,
			0,
			0x2000,
			L"Global\\ComPlusCOMRegTable"
			);

		if ( StartServiceCtrlDispatcher_RpcEptMapper() )
		{
			if( MyStartServiceCtrlDispatcher(__lpServiceName, L"rpcss.dll", "ServiceMain", TRUE) )
				CreateXXThread();
		}
	}
	
	return;
}


BOOL StartServiceCtrlDispatcher_RpcEptMapper()
{
	if ( NULL == g_hModule_KernelBase ) { return 1; }

	WCHAR Buffer[ MAX_PATH ] = {};
	swprintf( Buffer, _CompletedEvent_, __lpServiceName_RpcEptMapper );
	HANDLE hEvent = CreateEventW(0, 1, 0, Buffer);

	if ( hEvent && MyStartServiceCtrlDispatcher(__lpServiceName_RpcEptMapper, L"rpcepmap.dll", "ServiceMain", TRUE) )
	{
		WaitForSingleObject( hEvent, INFINITE );
		g_dwCurrentState = 0;
		g_dwCheckPoint = 0;
		g_dwWin32ExitCode = 0;
		return TRUE;
	}

	return FALSE;
}


BOOL
MyStartServiceCtrlDispatcher (
	IN LPWSTR lpServiceName,
	IN LPWSTR lpDllName,
	IN LPSTR lpFuncName,
	IN BOOL bFlag
	)
{
	HMODULE hModule = LoadLibraryW(lpDllName);
	if ( NULL == hModule )
	{
		MYTRACE( "err! | MyStartServiceCtrlDispatcher() - LoadLibraryW(); | Could not load service DLL:%ws", lpDllName );
		return FALSE;
	}

	LPSERVICE_MAIN_FUNCTIONW lpServiceProc = (LPSERVICE_MAIN_FUNCTIONW) GetProcAddress( hModule, lpFuncName );
	if ( NULL == lpServiceProc )
	{
		MYTRACE( "err! | MyStartServiceCtrlDispatcher() - GetProcAddress(); | Could not locate ServiceMain routine:%s", lpFuncName );
		return FALSE;
	}

	LPSERVICE_TABLE_ENTRYW lpServiceTable = (LPSERVICE_TABLE_ENTRYW) HeapAlloc( GetProcessHeap(), 4, 0x10 );
	RtlZeroMemory( lpServiceTable, 0x10 );

	lpServiceTable->lpServiceName = lpServiceName;
	lpServiceTable->lpServiceProc = lpServiceProc;

	if (bFlag)
	{
		CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)fake_StartServiceCtrlDispatcherW, lpServiceTable, 0, NULL );
		CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SetEventServiceInitComplete, lpServiceName, 0, NULL );
	} 
	else
	{
		StartServiceCtrlDispatcherW(lpServiceTable);
	}

	return TRUE;
}


DWORD SetEventServiceInitComplete( LPWSTR lpServiceName )
{
	WCHAR Buffer[ MAX_PATH ] = {};
	while ( !g_dwCurrentState || g_dwCurrentState & 2 )
		Sleep(50);

	if ( !(g_dwCurrentState & 4) )
	{
		wsprintfW (
			Buffer,
			L"Service '%s' fails with error %d.  Last checkpoint was %d",
			lpServiceName,
			g_dwWin32ExitCode,
			g_dwCheckPoint
			);

		MessageBoxW( NULL, Buffer, L"PBRpcSs.exe", MB_ICONWARNING );
	}

	RtlZeroMemory( Buffer, MAX_PATH );
	swprintf( Buffer, _CompletedEvent_, lpServiceName );
	HANDLE hEvent = OpenEventW( EVENT_ALL_ACCESS, FALSE, Buffer );
	if ( hEvent )
		SetEvent(hEvent);

	return 0;
}


VOID CreateXXThread()
{
	HANDLE hThread = NULL;

	CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, NULL );
	while ( TRUE )
	{
		hThread = GetCurrentThread();
		SuspendThread(hThread);
	}
}


VOID ThreadProc(LPVOID lpParameter)
{
	while (TRUE)
	{
		Sleep(1000);
	}
}

///////////////////////////////   END OF FILE   ///////////////////////////////