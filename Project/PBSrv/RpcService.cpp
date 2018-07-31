/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/08/12 [12:8:2011 - 11:04]
* MODULE : \SandBox\Code\Project\PBSrv\RpcApiNum0x1300.cpp
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
#include "RpcService.h"


//////////////////////////////////////////////////////////////////////////

CRpcService g_CRpcService ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


static PVOID __ProcService( PVOID pInfo, PRPC_IN_HEADER pRpcBuffer, int Msg )
{
	return g_CRpcService.ProcService( pInfo, pRpcBuffer, Msg );
}



CRpcService::CRpcService(void)
{
	
}

CRpcService::~CRpcService(void)
{
}



VOID CRpcService::HandlerService( PVOID pBuffer, LPTOTOAL_DATA pNode )
{
	_InsertList( pNode, 0x1300, pBuffer, (_PROCAPINUMBERFUNC_)__ProcService, TRUE );

	return;
}



PVOID 
CRpcService::ProcService(
	IN PVOID pInfo,
	IN PRPC_IN_HEADER pRpcBuffer, 
	IN int Msg
	)
{
	LPPOSTPROCRPCINFO ret = NULL ;

	switch ( pRpcBuffer->Flag )
	{
	case _PBSRV_APINUM_StartService_ :
		ret = RpcStartService( (LPRPC_IN_StartService)pRpcBuffer );
		break;

	case _PBSRV_APINUM_EnumServiceStatus_ :
		ret = RpcEnumServiceStatus( (LPRPC_IN_EnumServiceStatus)pRpcBuffer );
		break;

	case _PBSRV_APINUM_GetSandboxedServices_ :
		ret = RpcGetSandboxedServices( (LPRPC_IN_GetSandboxedServices)pRpcBuffer );
		break;

	case _PBSRV_APINUM_StartBoxedService_:
		ret = RpcStartBoxedService( (LPRPC_IN_StartBoxedService)pRpcBuffer, Msg );
		break;

	default:
		break;
	}

	 return (PVOID)ret;
}


LPPOSTPROCRPCINFO 
CRpcService::RpcStartService( 
	IN LPRPC_IN_StartService pRpcInBuffer 
	)
{
	BOOL bRet = FALSE ;
	int err = NO_ERROR ;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	ULONG NameLength = 2 * pRpcInBuffer->NameLength;

	MYTRACE( "RpcStartService \n" );

	do 
	{
		// 1. 校验参数合法性
		if (   pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_StartService) 
			|| NameLength > 0xFFFFFF
			|| NameLength + 0xC > pRpcInBuffer->RpcHeader.DataLength
			)
		{
			err = ERROR_INVALID_PARAMETER ;
			break;
		}

		// 2. 启动服务
		SC_HANDLE hSCManager = OpenSCManagerW( NULL, NULL, GENERIC_READ );
		if ( NULL == hSCManager ) 
		{ 
			err = GetLastError();
			break;
		}

		SC_HANDLE hService = OpenServiceW( hSCManager, pRpcInBuffer->szName, SERVICE_START );
		if ( NULL == hService ) 
		{ 
			CloseServiceHandle( hSCManager );
			err = GetLastError();
			break;
		}

		if ( !StartServiceW(hService, 0, NULL) )
			err = GetLastError();
	
		CloseServiceHandle( hService );
		CloseServiceHandle( hSCManager );

	} while (FALSE);	

	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcService::RpcEnumServiceStatus( 
	IN LPRPC_IN_EnumServiceStatus pRpcInBuffer 
	)
{
	BOOL bRet = FALSE ;
	int err = NO_ERROR ;
	DWORD dwInfoLevel = 0;
	LPRPCENUMSERVICESTATUS_INFO pBuffer = NULL;
	SC_HANDLE hSCManager = NULL, hService = NULL;
	ULONG NameLength = 2 * pRpcInBuffer->NameLength;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcEnumServiceStatus \n" );

	do 
	{
		// 1. 校验参数合法性
		if (   pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_EnumServiceStatus) 
			|| NameLength > 0xFFFFFF
			|| NameLength + 0x10 > pRpcInBuffer->RpcHeader.DataLength
			)
		{
			err = ERROR_INVALID_PARAMETER ;
			break;
		}

		hSCManager = OpenSCManagerW( NULL, NULL, GENERIC_READ );
		if ( NULL == hSCManager ) 
		{ 
			err = GetLastError();
			break;
		}

		hService = OpenServiceW( hSCManager, pRpcInBuffer->pServiceName, SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS );
		if ( NULL == hService ) 
		{ 
			err = GetLastError();
			break;
		}

		DWORD length = 0x54, cbBytesNeeded = 0;
		dwInfoLevel = pRpcInBuffer->dwInfoLevel;
		if ( dwInfoLevel )
		{
			if ( dwInfoLevel == 0xFFFF )
				QueryServiceConfigW( hService, NULL, 0, &cbBytesNeeded );
			else
				QueryServiceConfig2W( hService, dwInfoLevel, NULL, 0, &cbBytesNeeded );

			err = GetLastError();
			if ( ERROR_INSUFFICIENT_BUFFER != err )
			{
				if ( NO_ERROR == err ) { err = ERROR_INVALID_DATA; }
				break;
			}

			length = cbBytesNeeded + 0x5c;
		} 
		
		pBuffer = (LPRPCENUMSERVICESTATUS_INFO) AllocRpcBuffer( pTotalData, length );	
		if ( NULL == pBuffer ) { break; }

		DWORD BytesNeeded = 0x24;
		if (   !pRpcInBuffer->Flag1
			|| QueryServiceStatusEx( hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&pBuffer->ServiceStatus, 0x24, &BytesNeeded )
			|| (err = GetLastError(), !err)
			)
		{
			bRet = TRUE;
		}

		if ( FALSE == bRet ) { break; }
		if ( 0 == dwInfoLevel ) { break; }

		if ( 0xFFFF == dwInfoLevel )
			bRet = QueryServiceConfigW( hService, &pBuffer->ServiceConfig, cbBytesNeeded, &pBuffer->cbBytesNeeded );
		else
			bRet = QueryServiceConfig2W( hService, dwInfoLevel, (LPBYTE)&pBuffer->ServiceConfig, cbBytesNeeded, &pBuffer->cbBytesNeeded );

		if ( FALSE == bRet ) { break; }

		ULONG_PTR* lpServiceConfigPtr = (ULONG_PTR*)&pBuffer->ServiceConfig;
		switch ( dwInfoLevel )
		{
		case 65535 :
			if ( pBuffer->ServiceConfig.lpBinaryPathName )
			{
				pBuffer->ServiceConfig.lpBinaryPathName = (LPWSTR)((ULONG_PTR)pBuffer->ServiceConfig.lpBinaryPathName - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.lpLoadOrderGroup )
			{
				pBuffer->ServiceConfig.lpLoadOrderGroup = (LPWSTR)((ULONG_PTR)pBuffer->ServiceConfig.lpLoadOrderGroup - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.lpDependencies )
			{
				pBuffer->ServiceConfig.lpDependencies = (LPWSTR)((ULONG_PTR)pBuffer->ServiceConfig.lpDependencies - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.lpServiceStartName )
			{
				pBuffer->ServiceConfig.lpServiceStartName = (LPWSTR)((ULONG_PTR)pBuffer->ServiceConfig.lpServiceStartName - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.lpDisplayName )
			{
				pBuffer->ServiceConfig.lpDisplayName = (LPWSTR)((ULONG_PTR)pBuffer->ServiceConfig.lpDisplayName - (ULONG_PTR)lpServiceConfigPtr);
			}
			break;

		case SERVICE_CONFIG_DESCRIPTION :
			if ( *lpServiceConfigPtr )
				*lpServiceConfigPtr = *lpServiceConfigPtr - (ULONG_PTR)lpServiceConfigPtr;
			break;

		case SERVICE_CONFIG_FAILURE_ACTIONS :
			if ( pBuffer->ServiceConfig.dwStartType )
			{
				pBuffer->ServiceConfig.dwStartType = (DWORD)((ULONG_PTR)pBuffer->ServiceConfig.dwStartType - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.dwErrorControl )
			{
				pBuffer->ServiceConfig.dwErrorControl = (DWORD)((ULONG_PTR)pBuffer->ServiceConfig.dwErrorControl - (ULONG_PTR)lpServiceConfigPtr);
			}
			if ( pBuffer->ServiceConfig.lpLoadOrderGroup )
			{
				PVOID addr = &pBuffer->ServiceConfig;
				LPWSTR lpLoadOrderGroup = pBuffer->ServiceConfig.lpLoadOrderGroup;
				__asm
				{
					lea     esi, addr
					lea     edx, ds:0[esi*8]
					lea		eax, lpLoadOrderGroup
					sub     eax, edx
					mov     [esi+10h], eax
				}
			
			//	pBuffer->ServiceConfig.lpLoadOrderGroup = &pBuffer->ServiceConfig.lpLoadOrderGroup[-4 * lpServiceConfigPtr] ;
			}
			break;
		}
	} while (FALSE);	

	err = GetLastError();

	if ( hService ) { CloseServiceHandle( hService ); }
	if ( hSCManager ) { CloseServiceHandle( hSCManager ); }
	if ( pBuffer )
	{
		pBuffer->err = err;
		return (LPPOSTPROCRPCINFO)pBuffer;
	}
	else
	{
		return PostProcRPC( pTotalData, ERROR_NOT_ENOUGH_MEMORY );
	}
}



LPPOSTPROCRPCINFO 
CRpcService::RpcGetSandboxedServices( 
	IN LPRPC_IN_GetSandboxedServices pRpcInBuffer 
	)
{
	BOOL bRet = FALSE ;
	int err = NO_ERROR ;
	ULONG_PTR Size = 0;
	DWORD ResumeHandle = 0, ServicesReturned = 0, cbBytesNeeded = 0;
	SC_HANDLE hSCManager = NULL, hService = NULL;
	LPENUM_SERVICE_STATUSW lpServices = NULL ;
	LPRPCGETSANDBOXEDSERVICES_INFO pBuffer = NULL;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcGetSandboxedServices \n" );

	do
	{
		if ( pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_GetSandboxedServices) )
		{
			err = ERROR_INVALID_PARAMETER ;
			break;
		}

		hSCManager = OpenSCManagerW( NULL, NULL, GENERIC_READ );
		if ( NULL == hSCManager ) 
		{ 
			err = GetLastError();
			break;
		}

		while (TRUE)
		{
			if ( lpServices ) { HeapFree( GetProcessHeap(), 4, lpServices ); }

			if ( err )
				break;

			Size += 0x4000 ;
			lpServices = (LPENUM_SERVICE_STATUSW)HeapAlloc( GetProcessHeap(), 0, Size );
			if ( NULL == lpServices )
			{
				err = ERROR_NOT_ENOUGH_MEMORY;
				break;
			}

			ResumeHandle = 0;
			cbBytesNeeded = 0;
			bRet = EnumServicesStatusW (
					hSCManager,
					pRpcInBuffer->dwServiceType,
					pRpcInBuffer->dwServiceState,
					lpServices,
					Size,
					&cbBytesNeeded,
					&ServicesReturned,
					&ResumeHandle
					);

			if ( bRet )
				break;
	
			err = GetLastError();
			if ( ERROR_MORE_DATA == err ) 
			{
				err = NO_ERROR;
				continue; 
			}
		}

		CloseServiceHandle(hSCManager);
		if ( err ) { break; }

		DWORD n = 0;
		ULONG_PTR length = 0xE;
		if ( ServicesReturned )
		{
			do 
			{
				length += (wcslen(lpServices[n].lpServiceName) + 1) * sizeof(WCHAR) ;	
				++ n;
			} while ( n < ServicesReturned );
		}

		pBuffer = (LPRPCGETSANDBOXEDSERVICES_INFO)AllocRpcBuffer( pTotalData, length );
		if ( pBuffer )
		{
			n = 0;
			pBuffer->err = 0;
			LPWSTR lpData = pBuffer->ServiceNamesBuffer;
			if ( ServicesReturned > 0 )
			{
				do 
				{
					wcscpy( lpData, lpServices[n].lpServiceName );
					lpData += wcslen(lpData) + 1 ;
					++ n;
				} while ( n < ServicesReturned );
			}

			*lpData = 0;
		}

		HeapFree( GetProcessHeap(), 4, lpServices );
		if ( pBuffer ) { return (LPPOSTPROCRPCINFO)pBuffer; }

		err = ERROR_NOT_ENOUGH_MEMORY;

	} while (FALSE);

	return PostProcRPC( pTotalData, err );
}



LPPOSTPROCRPCINFO 
CRpcService::RpcStartBoxedService( 
	IN LPRPC_IN_StartBoxedService pRpcInBuffer,
	IN int PID
	)
{
	BOOL bRet = FALSE ;
	int err = NO_ERROR ;
	ULONG_PTR Size = 0;
	LPWSTR szCommandLine = NULL;
	SC_HANDLE hSCManager = NULL, hService = NULL;
	LPTOTOAL_DATA pTotalData = g_CGlobal.GetTotalData();

	MYTRACE( "RpcStartBoxedService \n" );

	do 
	{
		// 1. 校验参数合法性
		if (   pRpcInBuffer->RpcHeader.DataLength < sizeof(RPC_IN_StartBoxedService) 
			|| pRpcInBuffer->NameLength + 0xC > pRpcInBuffer->RpcHeader.DataLength
			)
		{
			err = ERROR_INVALID_PARAMETER ;
			break;
		}

		// 2. 如果配置文件中指定DropAdminRights=y，表明当前沙箱的进程需要降权，所以没有权限启动沙箱中的服务
		if ( IsSepcailAccount(PID) ) 
		{
			err = ERROR_SPECIAL_ACCOUNT ;
			break;
		}

		// 3.1 查询PID对应的沙箱名，拼接出完整的命令行,eg:/nosbiectrl /box:DefaultBox /sid:
		szCommandLine = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, pRpcInBuffer->NameLength + 0x180 );
		wcscpy( szCommandLine, L"/nosbiectrl /box:" );

		PB_BOX_INFO BoxInfo ;
		bRet = PB_QueryProcess( (ULONG)PID, &BoxInfo );
		if ( FALSE == bRet )
		{
			err = ERROR_INVALID_HANDLE ;
			break;
		}

		wcscat( szCommandLine, BoxInfo.BoxName );
		wcscat( szCommandLine, L" " );
		wcscat( szCommandLine, pRpcInBuffer->szName );

		// 3.2 启动沙箱中的服务
		STARTUPINFOW StartupInfo;
		memset( &StartupInfo, 0, sizeof(StartupInfo) );
		StartupInfo.lpReserved = 0;

		PB_RunFromHome( L"Start.exe", szCommandLine, &StartupInfo, NULL );

		// 3.3 设置后续的权限
		if ( ! StartupInfo.lpReserved )
		{
			err = ERROR_PATH_NOT_FOUND ;
			break;
		}

		HANDLE hToken = NULL, DuplicateTokenHandle = NULL ;
		SetThreadToken( NULL, NULL );

		if ( OpenProcessToken(GetCurrentProcess(), 0x1CB, &hToken) )
		{
			if ( DuplicateTokenEx(hToken, 0x1CB, NULL, SecurityAnonymous, TokenPrimary, &DuplicateTokenHandle) )
			{
				if ( SetTokenInformation(DuplicateTokenHandle, TokenSessionId, &BoxInfo.SessionId, 4) )
				{
					CopyProcessToken( DuplicateTokenHandle, PID );

					memset( &StartupInfo, 0, sizeof(StartupInfo) );
					StartupInfo.cb = 0x44 ;
					StartupInfo.dwFlags = 0x80 ;

					PROCESS_INFORMATION ProcessInformation ;
					if ( CreateProcessAsUserW(DuplicateTokenHandle, NULL, StartupInfo.lpReserved, 0, 0, 0, 0, 0, 0, &StartupInfo, &ProcessInformation) )
					{
						CloseHandle( ProcessInformation.hProcess );
						CloseHandle( ProcessInformation.hThread );
						SetLastError( NO_ERROR );
					}
				}
			}
		}

		if ( hToken ) { CloseHandle(hToken); }
		if ( DuplicateTokenHandle ) { CloseHandle(DuplicateTokenHandle); }

		err = GetLastError();

	} while (FALSE);

	if ( szCommandLine ) { HeapFree(GetProcessHeap(), 4, szCommandLine); }
	return PostProcRPC( pTotalData, err );
}


//////////////////////////////////////////////////////////////////////////

VOID CRpcService::CopyProcessToken( IN HANDLE TokenHandle, IN DWORD dwProcessId )
{
	PVOID pBuffer = HeapAlloc( GetProcessHeap(), 0, 0x400 );
	if ( NULL == pBuffer ) { return; }

	do 
	{
		HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, dwProcessId );
		if ( NULL == hProc ) { break; }

		HANDLE hToken = NULL;
		BOOL bRet = OpenProcessToken( hProc, TOKEN_QUERY, &hToken );
		CloseHandle( hProc );
		if ( FALSE == bRet ) { break; }

		DWORD ReturnLength = 0;
		PTOKEN_USER pUserInfo = (PTOKEN_USER) pBuffer ;

		bRet = GetTokenInformation( hToken, TokenUser, pUserInfo, 0x80, &ReturnLength );
		CloseHandle( hToken );
		if ( FALSE == bRet ) { break; }

		PTOKEN_DEFAULT_DACL pTokenDefaultDacl = (PTOKEN_DEFAULT_DACL) ((int)pBuffer + 0x80);
		bRet = GetTokenInformation( hToken, TokenDefaultDacl, pTokenDefaultDacl, 0x380, &ReturnLength );
		if ( FALSE == bRet ) { break; }

		pTokenDefaultDacl->DefaultDacl->AclSize += (WORD)(GetLengthSid(pUserInfo->User.Sid) + 8);

		AddAccessAllowedAce( pTokenDefaultDacl->DefaultDacl, ACL_REVISION, GENERIC_ALL, pUserInfo->User.Sid );
		SetTokenInformation( TokenHandle, TokenDefaultDacl, pTokenDefaultDacl, 0x380 );

	} while (FALSE);

	HeapFree( GetProcessHeap(), 4, pBuffer );
	return;
}



BOOL CRpcService::IsSepcailAccount( IN DWORD PID )
{
	// 查询配置文件中的DropAdminRights，如果为y,1,返回TRUE


	return FALSE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////