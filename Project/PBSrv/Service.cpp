//  Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RPCHandler.h"

//////////////////////////////////////////////////////////////////////////

#define  _lpServiceName	"PBSvc" 


VOID ServiceMainProc();
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceHandler(DWORD fdwControl);
VOID ComProxy( LPWSTR szData );
VOID ServiceHandlerDep( PVOID pBuffer );

CRITICAL_SECTION	myCS;
SERVICE_TABLE_ENTRY	lpServiceStartTable[] = 
{
	{ _lpServiceName, ServiceMain },
	{ NULL, NULL }
};

SERVICE_STATUS_HANDLE   hServiceStatusHandle; 
SERVICE_STATUS          ServiceStatus; 

int _tmain(int argc, _TCHAR* argv[])
{
	ServiceMainProc();
	return 0;
}



VOID ServiceMainProc()
{
	LPWSTR ptr = NULL, CommandLine = NULL ;

	::InitializeCriticalSection(&myCS);

	CommandLine = GetCommandLineW();
	if ( CommandLine )
	{
		ptr = wcsstr(CommandLine, L"Sandboxie_ComProxy");
		if ( ptr )
		{
			ComProxy(ptr);
			return;
		}
	}
	
	if(!StartServiceCtrlDispatcher(lpServiceStartTable))
	{
		MYTRACE( "StartServiceCtrlDispatcher failed, error code = %d\n", GetLastError() );
	}

	::DeleteCriticalSection(&myCS);
}


VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD   status = 0; 
    DWORD   specificError = 0xfffffff; 
 
    ServiceStatus.dwServiceType        = SERVICE_WIN32; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hServiceStatusHandle = RegisterServiceCtrlHandler( _lpServiceName, ServiceHandler ); 
    if (hServiceStatusHandle==0) 
    {
		MYTRACE( "RegisterServiceCtrlHandler failed, error code = %d\n", GetLastError() );
        return; 
    } 
 
    // Initialization complete - report running status 
    ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0;  
    if(!SetServiceStatus(hServiceStatusHandle, &ServiceStatus)) 
    { 
		MYTRACE( "SetServiceStatus failed, error code = %d\n", GetLastError() );
    } 

	// do work
	CRPCHandler::GetInstance().HandlerRPC() ;

	return;
}



VOID WINAPI ServiceHandler(DWORD fdwControl)
{
	LPTOTOAL_DATA pTotalNode = NULL ;

	switch(fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			ServiceStatus.dwWin32ExitCode = 0; 
			ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
			ServiceStatus.dwCheckPoint    = 0; 
			ServiceStatus.dwWaitHint      = 0;
			
			pTotalNode = g_CGlobal.GetTotalData();
			if ( pTotalNode )
			{
				g_CGlobal.DestroyTotalNode( pTotalNode );
				operator delete( pTotalNode );
			}

			if ( g_Buffer_0x1B00 )
				ServiceHandlerDep(g_Buffer_0x1B00);

			break; 
		case SERVICE_CONTROL_PAUSE:
			ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
			break;
		case SERVICE_CONTROL_CONTINUE:
			ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default:
			break;
	};

    if (!SetServiceStatus(hServiceStatusHandle,  &ServiceStatus)) 
	{ 
		MYTRACE( "SetServiceStatus failed, error code = %d\n", GetLastError());
    } 

	return;
}



VOID ComProxy( LPWSTR szData )
{
	return;
}



VOID ServiceHandlerDep( PVOID pBuffer )
{
	return;
}