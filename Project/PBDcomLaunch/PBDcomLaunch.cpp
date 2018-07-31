//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HookHelper.h"
#include "PBDcomSrv.h"
#include "PBDcomToken.h"

//////////////////////////////////////////////////////////////////////////

#define __lpServiceName		L"DCOMLAUNCH" 
#define _CompletedEvent_	L"PB_ServiceInitComplete_%s"

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

VOID DoWork();
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

	if ( InitHook_pharase0_Service() )
	{
		if ( InitHook_pharase1_Token() )
		{
			if( MyStartServiceCtrlDispatcher(__lpServiceName, L"rpcss.dll", "ServiceMain", TRUE) )
				CreateXXThread();
		}
	}
	
	return;
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

		MessageBoxW( NULL, Buffer, L"PBDcomLaunch.exe", MB_ICONWARNING );
	}

	RtlZeroMemory( Buffer, MAX_PATH );
	swprintf( Buffer, _CompletedEvent_, lpServiceName );
	HANDLE hEvent = OpenEventW( EVENT_ALL_ACCESS, 0, Buffer );
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