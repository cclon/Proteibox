// ControlService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

//////////////////////////////////////////////////////////////////////////

static PCHAR g_ServiceName = \
//	"helpsvc"
	"cryptsvc"
	;


//////////////////////////////////////////////////////////////////////////

HRESULT 
HandlerService (
	PCHAR szName,
	BOOL bStart,
	BOOL bPersist
	)
{
    SC_HANDLE hScm = NULL, hService = NULL;
    DWORD dwAccess = SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | (bStart ? SERVICE_START : SERVICE_STOP) ;
	DWORD dwErr = ERROR_SUCCESS;
    SERVICE_STATUS  status;
	
    if( NULL == (hScm = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT )) )
	{
		dwErr = GetLastError();
		goto _end_ ;
	}
    
	if( NULL == (hService = OpenService( hScm, szName, dwAccess )) )
	{
		dwErr = GetLastError();
		goto _clearup_ ;
	}
	
	if( FALSE == QueryServiceStatus( hService, &status ) )
	{
		dwErr = GetLastError();
		goto _clearup_ ;
	}

	if( bPersist )
	{
		dwErr = ChangeServiceConfig ( 
			hService, 
			SERVICE_NO_CHANGE,
			bStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
			SERVICE_NO_CHANGE, 
			NULL, NULL, NULL, NULL, NULL, NULL, NULL 
			);
	}
	
	if( bStart )
	{
		if( SERVICE_PAUSED == status.dwCurrentState || SERVICE_PAUSE_PENDING == status.dwCurrentState )
		{
			dwErr = ControlService( hService, SERVICE_CONTROL_CONTINUE, &status ) ? ERROR_SUCCESS : GetLastError();
		}
		else
		{
			dwErr = StartService( hService, 0, NULL ) ? ERROR_SUCCESS : GetLastError();
			if( ERROR_SERVICE_ALREADY_RUNNING == dwErr )
				dwErr = ERROR_SUCCESS;
		}
	}
	else
	{
		dwErr = ControlService( hService, SERVICE_CONTROL_STOP, &status ) ? ERROR_SUCCESS : GetLastError();
	}

_clearup_ :	
	CloseServiceHandle( hScm ); 

_end_ :
    return HRESULT_FROM_WIN32( dwErr );
}



VOID
Handler_service_total (
	BOOL bStart
	)
{
	char* szTmp = bStart ? "Start" : "Stop" ;
	DWORD ErrorCode = HandlerService( g_ServiceName, bStart, FALSE );

	if ( ERROR_SUCCESS == ErrorCode )
	{
		printf( "[+] %s服务操作成功 \n\n", szTmp );
	}
	else
	{
		printf( "[+] %s服务操作失败 \n\n", szTmp );
	}

	return ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+											                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#define CI_SERVICEA "cisvc"
#define CI_SERVICEW L"cisvc"

#ifdef UNICODE
#define CI_SERVICE  CI_SERVICEW
#else
#define CI_SERVICE  CI_SERVICEA
#endif 

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	//
	// 先在服务管理器中启用 Help and Support 服务,然后调用下面的函数终止该服务.
	// 在Sandboxie中的运行该程序,跟踪fake函数处理流程 
	// sudami 2010-4-1 10:47:31
	//

	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	system( "title=Test for Sandboxie:ControlService()" );

	while ( bOK )
	{
		printf (
			"请选择操作: \n"
			"0. ESC \n"
			"1. 停止Help and Support 服务						\n"
			"2. 开启Help and Support 服务						\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			ExitProcess( -1 );
			break;
			
		case 1:
			Handler_service_total ( FALSE );
			break;

		case 2:
			Handler_service_total ( TRUE );
			break;
			
		default:
			break;
		}
	}
	
//	getchar() ;
//	system( "pause" );
	return 0;
}

