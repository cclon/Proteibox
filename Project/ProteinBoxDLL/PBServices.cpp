/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/01/19 [19:1:2011 - 15:15]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBServices.cpp
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
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBFilesData.h"
#include "PBLoadData.h"
#include "PBToken.h"
#include "PBDynamicData.h"
#include "PBServicesData.h"
#include "PBServices.h"

#pragma warning(disable : 4995 4018 )

//////////////////////////////////////////////////////////////////////////

ULONG g_Service_TickCount = 0 ;

DWORD g_DACL_data[ 24 ] = 
{
	0x600002, 0x4, 0x140000, 0x201FD,
	0x101, 0x5000000, 0x12, 0x180000,
	0x0F01FF, 0x201, 0x5000000, 0x20,
	0x220, 0x140000, 0x2018D,0x101,
	0x5000000, 0x0B, 0x180000, 0x201FD,
	0x201, 0x5000000, 0x20, 0x223
};



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_OpenSCManager (
    PVOID lpMachineName,
    PVOID lpDatabaseName,
    DWORD   dwDesiredAccess
    )
{
	return 0x12340001 ;
}



ULONG WINAPI
fake_OpenServiceA (
    SC_HANDLE hSCManager,
    LPSTR lpServiceName,
    DWORD dwDesiredAccess
    )
{
	DWORD dwErrorCode ;
	SC_HANDLE hService = NULL ;
	ANSI_STRING ansiBuffer ; 
	UNICODE_STRING uniBuffer ;

	uniBuffer.Buffer = 0;
	if ( lpServiceName )
	{
		RtlInitString( &ansiBuffer, lpServiceName );
		RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
	}

	hService = (SC_HANDLE) fake_OpenServiceW( hSCManager, uniBuffer.Buffer, dwDesiredAccess );

	if ( uniBuffer.Buffer ) { RtlFreeUnicodeString( &uniBuffer ); }

	dwErrorCode = GetLastError();
	SetLastError( dwErrorCode );
	return (ULONG)hService;
}



ULONG WINAPI
fake_OpenServiceW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    DWORD dwDesiredAccess
    )
{
	BOOL bRet = TRUE ;
	int Buffer = 0 ;
	PVOID pOutBuffer = NULL ;

	// 1. 只考虑 hSCManager 为 0x12340001 的情况
	if ( hSCManager != (HANDLE)0x12340001 )
	{
		SetLastError( ERROR_INVALID_HANDLE );
		return 0;
	}

	// 2. 确保服务名的合法性
	if ( NULL == lpServiceName || NULL == *lpServiceName )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	// 3.1 删除当前服务对应的结点
	RemoveRegNode__service( (LPWSTR)lpServiceName );
	g_Service_TickCount = GetTickCount();

	// 3.2 服务名的黑白名单判断; 白名单放行,黑名单继续处理
	bRet = Is_white_service( (LPWSTR)lpServiceName );
	if ( FALSE == bRet )
	{
		// 黑名单走RPC流程,然后返回之
		pOutBuffer = (PVOID) Get_serviceStatus_rpc( (LPWSTR)lpServiceName, 0, 0 );
		if ( NULL == pOutBuffer )
		{
			SetLastError( ERROR_SERVICE_DOES_NOT_EXIST );
			return 0;
		}

		kfree( pOutBuffer );
	}

	// 3.3 填充数据,返回该数据
	Buffer = (int) kmalloc( 2 * wcslen(lpServiceName) + 6 );

	*(DWORD *)Buffer = _FuckedTag_ ;
	wcscpy( (LPWSTR)(Buffer + 4), lpServiceName );
	wcslwr( (LPWSTR)Buffer );

	SetLastError( NO_ERROR );
	return (ULONG) Buffer ;
}



ULONG WINAPI
fake_CloseServiceHandle (
    SC_HANDLE hService
    )
{
	if ( hService == (SC_HANDLE)0x12340001 )
	{
		SetLastError( 0 );
		return 1 ;
	}

	if ( NULL == Get_service_name_from_handle( hService ) )
	{
		SetLastError( CR_INVALID_RES_DES );
		return 0;
	}

	kfree( hService );
	SetLastError( 0 );
	return 1 ;
}



ULONG WINAPI
fake_QueryServiceStatus (
    SC_HANDLE           hService,
    LPSERVICE_STATUS    lpServiceStatus
    )
{
	int pOutBuffer = 0 ;
	LPWSTR szServiceName = NULL ;
	BYTE TempBuffer[ MAX_PATH ] ;

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	pOutBuffer = (int)Get_serviceStatus_rpc( szServiceName, 0, 1 );
	if ( 0 == pOutBuffer ) { return 0; }
	
	memcpy( TempBuffer, (PVOID)(pOutBuffer + 8), 0x24);
	kfree((PVOID)pOutBuffer);
	SetLastError( NO_ERROR );

	memcpy( lpServiceStatus, TempBuffer, 0x1C );
	return 1;
}



ULONG WINAPI
fake_QueryServiceStatusEx (
	HANDLE hService, 
	int InfoLevel,
	LPBYTE lpBuffer, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	)
{
	LPWSTR szServiceName = NULL ;
	int pOutBuffer = 0 ; 

	if ( InfoLevel )
	{
		SetLastError(ERROR_INVALID_LEVEL);
		return 0;
	}

	if ( cbBufSize < 0x24 )
	{
		*pcbBytesNeeded = 0x24 ;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	pOutBuffer = (int)Get_serviceStatus_rpc( szServiceName, 0, 1 );
	if ( 0 == pOutBuffer ) { return 0; }

	memcpy( lpBuffer, (PVOID)(pOutBuffer + 8), 0x24 );

	kfree( (PVOID)pOutBuffer );
	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_QueryServiceConfigA (
	SC_HANDLE hService,
	LPQUERY_SERVICE_CONFIGA lpServiceConfig, 
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded
	)
{
	BOOL bResult ;
	ULONG bRet ;
	DWORD dwErrorCode ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ; 
	LPQUERY_SERVICE_CONFIGW lpServiceConfig_tmp = NULL ;
	
	lpServiceConfig_tmp = (LPQUERY_SERVICE_CONFIGW) kmalloc( 0x2800 );
	bResult = fake_QueryServiceConfigW( hService, (LPQUERY_SERVICE_CONFIGW)lpServiceConfig_tmp, 0x2710, pcbBytesNeeded );
	dwErrorCode = GetLastError();

	if ( *pcbBytesNeeded > cbBufSize )
	{

		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		bRet = 0;
		goto _over_ ;
	}
	
	if ( FALSE == bResult )
	{
		SetLastError( dwErrorCode );
		bRet = 0;
		goto _over_ ;
	}
	
	memcpy( lpServiceConfig, lpServiceConfig_tmp, *pcbBytesNeeded );
	
	if ( lpServiceConfig->lpBinaryPathName )
	{
		RtlInitUnicodeString( &uniBuffer, lpServiceConfig_tmp->lpBinaryPathName );

		ansiBuffer.Length = 0 ;
		ansiBuffer.MaximumLength = 0x2000 ;
		ansiBuffer.Buffer = lpServiceConfig->lpBinaryPathName ;

		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}

	if ( lpServiceConfig->lpLoadOrderGroup )
	{
		RtlInitUnicodeString( &uniBuffer, lpServiceConfig_tmp->lpLoadOrderGroup );
		
		ansiBuffer.Length = 0 ;
		ansiBuffer.MaximumLength = 0x2000 ;
		ansiBuffer.Buffer = lpServiceConfig->lpLoadOrderGroup ;
		
		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}

	if ( lpServiceConfig->lpDependencies )
	{
		RtlInitUnicodeString( &uniBuffer, lpServiceConfig_tmp->lpDependencies );
		
		ansiBuffer.Length = 0 ;
		ansiBuffer.MaximumLength = 0x2000;
		ansiBuffer.Buffer = lpServiceConfig->lpDependencies ;
		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}

	if ( lpServiceConfig->lpServiceStartName )
	{
		RtlInitUnicodeString( &uniBuffer, lpServiceConfig_tmp->lpServiceStartName );
		
		ansiBuffer.Length = 0 ;
		ansiBuffer.MaximumLength = 0x2000 ;
		ansiBuffer.Buffer = lpServiceConfig->lpServiceStartName ;
		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}

	if ( lpServiceConfig->lpDisplayName )
	{
		RtlInitUnicodeString( &uniBuffer, lpServiceConfig_tmp->lpDisplayName );
		
		ansiBuffer.Length = 0 ;
		ansiBuffer.MaximumLength = 0x2000 ;
		ansiBuffer.Buffer = lpServiceConfig->lpDisplayName ;
		RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
	}

	SetLastError( NO_ERROR );
	bRet = 1;

_over_ :
	kfree( lpServiceConfig_tmp );
	return bRet ;
}



ULONG WINAPI
fake_QueryServiceConfigW (
	SC_HANDLE hService,
	LPQUERY_SERVICE_CONFIGW lpServiceConfig, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	)
{
	LPWSTR szServiceName = NULL ;
	LPRPC_OUT_QueryServiceConfigW pOutBuffer = NULL ;
	LPWSTR lpBinaryPathName; // eax@11
	LPWSTR lpLoadOrderGroup; // eax@13
	LPWSTR lpDependencies; // eax@15
	LPWSTR lpServiceStartName; // eax@17
	LPWSTR lpDisplayName; // eax@19

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	pOutBuffer = (LPRPC_OUT_QueryServiceConfigW) Get_serviceStatus_rpc( szServiceName, 0xFFFFFFFF, 0 );
	if ( NULL == pOutBuffer ) { return 0; }

	if ( pcbBytesNeeded ) { *pcbBytesNeeded = pOutBuffer->cbBytesNeeded; }

	if ( pOutBuffer->cbBytesNeeded > cbBufSize )
	{
		kfree( pOutBuffer );
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}

	if ( PB_IsWow64() )
	{
		lpServiceConfig->dwServiceType = pOutBuffer->u.ServiceConfigX64.dwServiceType;
		lpServiceConfig->dwStartType = pOutBuffer->u.ServiceConfigX64.dwStartType;
		lpServiceConfig->dwErrorControl = pOutBuffer->u.ServiceConfigX64.dwErrorControl;
		lpServiceConfig->lpBinaryPathName = pOutBuffer->u.ServiceConfigX64.lpBinaryPathName;
		lpServiceConfig->lpLoadOrderGroup = pOutBuffer->u.ServiceConfigX64.lpLoadOrderGroup;
		lpServiceConfig->dwTagId = pOutBuffer->u.ServiceConfigX64.dwTagId;
		lpServiceConfig->lpDependencies = pOutBuffer->u.ServiceConfigX64.lpDependencies;
		lpServiceConfig->lpServiceStartName = pOutBuffer->u.ServiceConfigX64.lpServiceStartName;
		lpServiceConfig->lpDisplayName = pOutBuffer->u.ServiceConfigX64.lpDisplayName;
	}
	else
	{
		memcpy(lpServiceConfig, &pOutBuffer->u.ServiceConfigX86 , pOutBuffer->cbBytesNeeded);
	}


	kfree( pOutBuffer );

	lpBinaryPathName = lpServiceConfig->lpBinaryPathName;
	if ( lpBinaryPathName )
		lpServiceConfig->lpBinaryPathName = (LPWSTR)((DWORD)lpServiceConfig + (DWORD)lpBinaryPathName);

	lpLoadOrderGroup = lpServiceConfig->lpLoadOrderGroup;
	if ( lpLoadOrderGroup )
		lpServiceConfig->lpLoadOrderGroup = (LPWSTR)((DWORD)lpServiceConfig + (DWORD)lpLoadOrderGroup);

	lpDependencies = lpServiceConfig->lpDependencies;
	if ( lpDependencies )
		lpServiceConfig->lpDependencies = (LPWSTR)((DWORD)lpServiceConfig + (DWORD)lpDependencies);

	lpServiceStartName = lpServiceConfig->lpServiceStartName;
	if ( lpServiceStartName )
		lpServiceConfig->lpServiceStartName = (LPWSTR)((DWORD)lpServiceConfig + (DWORD)lpServiceStartName);

	lpDisplayName = lpServiceConfig->lpDisplayName;
	if ( lpDisplayName )
		lpServiceConfig->lpDisplayName = (LPWSTR)((DWORD)lpServiceConfig + (DWORD)lpDisplayName);

	SetLastError( NO_ERROR );
	return 1 ;
}



ULONG WINAPI
fake_QueryServiceConfig2A (
	SC_HANDLE hService, 
	DWORD dwInfoLevel, 
	LPSERVICE_FAILURE_ACTIONSA lpBuffer, 
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded
	)
{
	ULONG bRet = 0 ;
	DWORD dwErrorCode ;
	UNICODE_STRING uniBuffer;
	ANSI_STRING ansiBuffer ; 
	BOOL bResult = FALSE ; 
	LPSERVICE_FAILURE_ACTIONSW lpBuffer_tmp = NULL ;

	lpBuffer_tmp = (LPSERVICE_FAILURE_ACTIONSW) kmalloc( 0x2800 );
	bResult = fake_QueryServiceConfig2W( hService, dwInfoLevel, lpBuffer_tmp, 10000, pcbBytesNeeded );

	dwErrorCode = GetLastError();
	if ( *pcbBytesNeeded > cbBufSize )
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		goto _over_ ;
	}

	if ( FALSE == bResult )
	{
		SetLastError(dwErrorCode);
		goto _over_ ;
	}

	memcpy( lpBuffer, lpBuffer_tmp, *pcbBytesNeeded );

	ansiBuffer.Length = 0;
	ansiBuffer.MaximumLength = 0x2000 ;

	if ( SERVICE_CONFIG_DESCRIPTION == dwInfoLevel )
	{
		if ( lpBuffer->dwResetPeriod )
		{
			LPSERVICE_DESCRIPTIONA lpPtr = (LPSERVICE_DESCRIPTIONA) lpBuffer_tmp ;

			RtlInitUnicodeString( &uniBuffer, (LPWSTR)lpPtr->lpDescription );
			ansiBuffer.Buffer = lpPtr->lpDescription ;
			RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
		}
	}
	else if ( SERVICE_CONFIG_FAILURE_ACTIONS == dwInfoLevel )
	{	
		if ( lpBuffer->lpRebootMsg )
		{
			RtlInitUnicodeString( &uniBuffer, lpBuffer_tmp->lpRebootMsg );
			ansiBuffer.Buffer = lpBuffer->lpRebootMsg ;
			RtlUnicodeStringToAnsiString(&ansiBuffer, &uniBuffer, FALSE);
		}

		if ( lpBuffer->lpCommand )
		{
			RtlInitUnicodeString( &uniBuffer, lpBuffer_tmp->lpCommand );
			ansiBuffer.Buffer = lpBuffer->lpCommand ;
			RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
		}
	}

	SetLastError( NO_ERROR );
	bRet = 1 ;

_over_ :
	kfree( lpBuffer_tmp );
	return bRet;
}



ULONG WINAPI
fake_QueryServiceConfig2W (
	SC_HANDLE hService,
	int dwInfoLevel, 
	LPSERVICE_FAILURE_ACTIONSW lpBuffer,
	int cbBufSize,
	LPDWORD pcbBytesNeeded
	)
{
	LPWSTR szServiceName = NULL ;
	LPRPC_OUT_QueryServiceConfigW pOutBuffer = NULL ;

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	pOutBuffer = (LPRPC_OUT_QueryServiceConfigW) Get_serviceStatus_rpc( szServiceName, dwInfoLevel, 0 );
	if ( NULL == pOutBuffer ) { return 0; }

	if ( pcbBytesNeeded ) { *pcbBytesNeeded = pOutBuffer->cbBytesNeeded ; }

	if ( pOutBuffer->cbBytesNeeded > cbBufSize )
	{
		kfree( pOutBuffer );
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}

	memcpy( lpBuffer, &pOutBuffer->u.ServiceConfigX86 , pOutBuffer->cbBytesNeeded );
	kfree( pOutBuffer );

	if ( SERVICE_CONFIG_DESCRIPTION == dwInfoLevel )
	{
		LPSERVICE_DESCRIPTIONA lpPtr = (LPSERVICE_DESCRIPTIONA) lpBuffer ;
		if ( lpPtr->lpDescription )
			lpPtr->lpDescription = (LPSTR)( (UINT_PTR)lpPtr + (UINT_PTR)lpPtr->lpDescription );
	}
	else if ( SERVICE_CONFIG_FAILURE_ACTIONS == dwInfoLevel )
	{
		LPSERVICE_FAILURE_ACTIONSA lpPtr = (LPSERVICE_FAILURE_ACTIONSA) lpBuffer ;

		if (lpPtr->lpRebootMsg != NULL)
			lpPtr->lpRebootMsg = (LPSTR)((UINT_PTR)lpPtr + (UINT_PTR)lpPtr->lpRebootMsg);

		if (lpPtr->lpCommand != NULL)
			lpPtr->lpCommand = (LPSTR)((UINT_PTR)lpPtr + (UINT_PTR)lpPtr->lpCommand);

		if (lpPtr->lpsaActions != NULL)
			lpPtr->lpsaActions = (SC_ACTION*)((UINT_PTR)lpPtr + (UINT_PTR)lpPtr->lpsaActions);
	}

	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_EnumServicesStatusA (
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSA  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle
    )
{
	return EnumServiceStatus_filter(
		hSCManager,
		dwServiceType,
		dwServiceState,
		(int)lpServices,
		cbBufSize,
		pcbBytesNeeded,
		lpServicesReturned,
		lpResumeHandle,
		FALSE,
		FALSE
		);
}



ULONG WINAPI
fake_EnumServicesStatusW(
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSW  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle
    )
{
	return EnumServiceStatus_filter(
		hSCManager,
		dwServiceType,
		dwServiceState,
		(int)lpServices,
		cbBufSize,
		pcbBytesNeeded,
		lpServicesReturned,
		lpResumeHandle,
		TRUE,
		FALSE
		);
}



ULONG WINAPI
fake_EnumServicesStatusExA (
	SC_HANDLE hSCManager,
	int a2, 
	DWORD dwServiceType, 
	DWORD dwServiceState,
	int lpServices, 
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded,
	LPDWORD lpServicesReturned,
	LPDWORD lpResumeHandle, 
	int a10
	)
{
	ULONG ret = 0 ;

	if ( a2 )
	{
		SetLastError( ERROR_INVALID_LEVEL );
	}
	else
	{
		ret = EnumServiceStatus_filter (
			hSCManager,
			dwServiceType,
			dwServiceState,
			lpServices,
			cbBufSize,
			pcbBytesNeeded,
			lpServicesReturned,
			lpResumeHandle,
			FALSE,
			TRUE
			);
	}

	return ret ;
}



ULONG WINAPI
fake_EnumServicesStatusExW (
	SC_HANDLE hSCManager, 
	int a2, 
	DWORD dwServiceType,
	DWORD dwServiceState, 
	int lpServices,
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded,
	LPDWORD lpServicesReturned,
	LPDWORD lpResumeHandle,
	int a10
	)
{
	ULONG ret = 0 ;

	if ( a2 )
	{
		SetLastError( ERROR_INVALID_LEVEL );
	}
	else
	{
		ret = EnumServiceStatus_filter (
			hSCManager,
			dwServiceType,
			dwServiceState,
			lpServices,
			cbBufSize,
			pcbBytesNeeded,
			lpServicesReturned,
			lpResumeHandle,
			TRUE,
			TRUE
			);
	}

	return ret ;
}



ULONG WINAPI
fake_QueryServiceLockStatus (
	SC_HANDLE hSCManager,
    int lpLockStatus,
    DWORD cbBufSize,
    LPDWORD pcbBytesNeeded
    )
{
	ULONG ret = 0 ;

	*pcbBytesNeeded = 0xE ;

	if ( cbBufSize < 0xE )
	{
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0 ;
	}
	
	*(DWORD *)lpLockStatus = 0 ;
	*(DWORD *)(lpLockStatus + 4) = 0 ;
	*(DWORD *)(lpLockStatus + 8) = 0 ;
	*(WORD *)(lpLockStatus + 0xC) = 0 ;
	*(DWORD *)(lpLockStatus + 4) = lpLockStatus + 0xC ;

	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_GetServiceDisplayNameA (
    SC_HANDLE hSCManager,
    LPCSTR lpServiceName,
    LPSTR lpDisplayName,
    LPDWORD lpcchBuffer
    )
{
	PVOID ptr = NULL ;
	int pOutBuffer = 0 ;
	ULONG n = 0, Length = 0 ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	if ( lpDisplayName ) { *lpDisplayName = 0 ; }

	// 1. 调用RPC得到服务对应的Buffer
	RtlInitString( &ansiBuffer, lpServiceName );
	RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );

	pOutBuffer = Get_serviceStatus_rpc( uniBuffer.Buffer, 0xFFFFFFFF, 0 );
	RtlFreeUnicodeString( &uniBuffer );
	if ( 0 == pOutBuffer ) { return 0; }

	// 2. 若服务名长度大于参数三 @lpcchBuffer,返回内存不足
	ptr = (PVOID)( *(DWORD *)(pOutBuffer + 0x50) + pOutBuffer + 0x30 );
	Length = wcslen( (LPWSTR)ptr );
	if ( Length > *lpcchBuffer )
	{
		*lpcchBuffer = Length ;
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}

	// 3. 拷贝查询到的字符串内容至参数二 @lpDisplayName
	if ( Length )
	{
		do
		{
			lpDisplayName[ n ] = *( (BYTE *)ptr + 2 * n );
			++ n ;
		}
		while ( n < Length );
	}

	if ( *lpcchBuffer >= Length + 1 ) { lpDisplayName[ Length ] = 0; }

	*lpcchBuffer = Length ;
	SetLastError( NO_ERROR );
	return 1 ;
}



ULONG WINAPI
fake_GetServiceDisplayNameW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    LPWSTR lpDisplayName,
    LPDWORD lpcchBuffer
    )
{
	DWORD Length = 0, pOutBuffer = 0 ;
	LPWSTR ptr = NULL ; 

	if ( lpDisplayName ) { *lpDisplayName = 0; }

	pOutBuffer = Get_serviceStatus_rpc( (LPWSTR)lpServiceName, 0xFFFFFFFF, 0 );
	if ( 0 == pOutBuffer ) { return 0; }
	
	ptr = (LPWSTR)( *(DWORD *)(pOutBuffer + 0x50) + pOutBuffer + 0x30 );
	Length = wcslen( ptr );

	if ( Length > *lpcchBuffer )
	{
		*lpcchBuffer = Length ;
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}
	
	memcpy( lpDisplayName, ptr, 2 * Length );
	if ( *lpcchBuffer >= Length + 2 ) { lpDisplayName[ Length ] = 0; }

	*lpcchBuffer = Length ;
	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_GetServiceKeyNameA (
    SC_HANDLE hSCManager,
    LPCSTR lpDisplayName,
    LPSTR lpServiceName,
    LPDWORD lpcchBuffer
    )
{
	LPWSTR pServiceNameArray = NULL ;
	DWORD Length = 0, pOutBuffer = 0, n = 0 ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING uniBuffer ;

	if ( lpServiceName ) { *lpServiceName = 0; }

	RtlInitString( &ansiBuffer, lpDisplayName );
	RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );

	pServiceNameArray = (LPWSTR) GetSandboxedServices();
	if ( *pServiceNameArray )
	{
		do
		{
			pOutBuffer = Get_serviceStatus_rpc( pServiceNameArray, 0xFFFFFFFF, 0 );
			if ( !pOutBuffer ) { break; }

			if ( 0 == wcsicmp((LPWSTR)(*(DWORD *)(pOutBuffer + 0x50) + pOutBuffer + 0x30), uniBuffer.Buffer) ) { break; }
	
			kfree( (PVOID)pOutBuffer );
			pOutBuffer = 0 ;
			pServiceNameArray += wcslen( pServiceNameArray ) + 1 ;
		}
		while ( *pServiceNameArray );
	}
	else
	{
		pOutBuffer = (int)lpServiceName;
	}

	RtlFreeUnicodeString( &uniBuffer );
	if ( 0 == pOutBuffer )
	{
		if ( !*pServiceNameArray ) { SetLastError( ERROR_SERVICE_NOT_FOUND ); }
		return 0 ;
	}
	
	kfree( (PVOID)pOutBuffer );
	Length = wcslen( pServiceNameArray );
	if ( Length > *lpcchBuffer )
	{
		*lpcchBuffer = Length ;
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}
	
	if ( Length )
	{
		do
		{
			lpServiceName[ n ] = LOBYTE( pServiceNameArray[n] );
			++ n ;
		}
		while ( n < Length );
	}

	if ( *lpcchBuffer >= Length + 1 ) { lpServiceName[ Length ] = 0; }

	*lpcchBuffer = Length ;
	SetLastError( NO_ERROR );
	return 1 ;
}



ULONG WINAPI
fake_GetServiceKeyNameW (
    SC_HANDLE hSCManager,
    LPCWSTR lpDisplayName,
    LPWSTR lpServiceName,
    LPDWORD lpcchBuffer
    )
{
	DWORD pOutBuffer = 0, Length = 0 ;
	LPWSTR pServiceNameArray = NULL ;

	if ( lpServiceName ) { *lpServiceName = 0 ; }

	pServiceNameArray = (LPWSTR) GetSandboxedServices();
	if ( ! *pServiceNameArray )
	{
		SetLastError( ERROR_SERVICE_NOT_FOUND );
		return 0;
	}

	while ( TRUE )
	{
		pOutBuffer = Get_serviceStatus_rpc( pServiceNameArray, 0xFFFFFFFF, 0 );
		if ( 0 == pOutBuffer ) { break; }

		if ( 0 == wcsicmp( (LPWSTR)(*(DWORD *)(pOutBuffer + 0x50) + pOutBuffer + 0x30), lpDisplayName ) )
		{
			kfree( (PVOID)pOutBuffer );
			Length = wcslen( pServiceNameArray );
			if ( Length > *lpcchBuffer )
			{
				*lpcchBuffer = Length ;
				SetLastError( ERROR_INSUFFICIENT_BUFFER );
				return 0 ;
			}

			memcpy( lpServiceName, pServiceNameArray, 2 * Length );
			if ( *lpcchBuffer >= Length + 2 ) { lpServiceName[ Length ] = 0; }

			*lpcchBuffer = Length;
			SetLastError( NO_ERROR );
			return 1 ;
		}

		kfree( (PVOID)pOutBuffer );
		pServiceNameArray += wcslen( pServiceNameArray ) + 1 ;
		if ( !*pServiceNameArray )
		{
			SetLastError( ERROR_SERVICE_NOT_FOUND );
			return 0 ;
		}
	}

	if ( *pServiceNameArray ) { return 0 ; }

	SetLastError( ERROR_SERVICE_NOT_FOUND );
	return 0;
}



ULONG WINAPI
fake_EnumDependentServices (
	SC_HANDLE hService,
	DWORD dwServiceState, 
	int lpServices,
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded,
	LPDWORD lpServicesReturned
	)
{
	*lpServicesReturned = 0;
	SetLastError( NO_ERROR );
	return 1 ;
}



ULONG WINAPI
fake_QueryServiceObjectSecurity (
    SC_HANDLE               hService,
    SECURITY_INFORMATION    dwSecurityInformation,
    /*PSECURITY_DESCRIPTOR*/int    lpSecurityDescriptor,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded
    )
{
	ULONG offset = 0x14 ;
	ULONG StructLength = 0x14 ; // 默认值

	if ( NULL == Get_service_name_from_handle(hService) ) { return 0 ; }

	if ( !dwSecurityInformation || dwSecurityInformation & 0xFFFFFFF0 )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0 ;
	}

	if ( dwSecurityInformation & DACL_SECURITY_INFORMATION ) { StructLength = 0x74 ; }

	if ( dwSecurityInformation & OWNER_SECURITY_INFORMATION ) { StructLength += 0xC ; }

	if ( dwSecurityInformation & GROUP_SECURITY_INFORMATION ) { StructLength += 0xC ; }

	*pcbBytesNeeded = StructLength ;
	if ( StructLength > cbBufSize )
	{
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return 0;
	}

	*(BYTE *)lpSecurityDescriptor = 1;
	*(BYTE *)(lpSecurityDescriptor + 1) = 0;
	*(WORD *)(lpSecurityDescriptor + 2) = 0x8000 ;
	*(DWORD *)(lpSecurityDescriptor + 4) = 0;
	*(DWORD *)(lpSecurityDescriptor + 8) = 0;
	*(DWORD *)(lpSecurityDescriptor + 0xC) = 0;
	*(DWORD *)(lpSecurityDescriptor + 0x10) = 0;

	if ( dwSecurityInformation & DACL_SECURITY_INFORMATION )
	{
		*(DWORD *)(lpSecurityDescriptor + 0x10) = 0x14 ;
		*(WORD *)(lpSecurityDescriptor + 2) = 0x8004 ;
		memcpy( (PVOID)(lpSecurityDescriptor + 0x14), g_DACL_data, 0x60 );
	
		offset = 0x74 ;
	}

	if ( dwSecurityInformation & OWNER_SECURITY_INFORMATION )
	{
		*(DWORD *)(lpSecurityDescriptor + 4) = offset ;
		*(DWORD *)(offset + lpSecurityDescriptor) = 0x101 ;
		*(DWORD *)(offset + lpSecurityDescriptor + 4) = 0x5000000 ;
		*(DWORD *)(offset + lpSecurityDescriptor + 8) = 0x12 ;
		offset += 0xC ;
	}

	if ( dwSecurityInformation & GROUP_SECURITY_INFORMATION )
	{
		*(DWORD *)(lpSecurityDescriptor + 8) = offset ;
		*(DWORD *)(offset + lpSecurityDescriptor) = 0x101 ;
		*(DWORD *)(offset + lpSecurityDescriptor + 4) = 0x5000000 ;
		*(DWORD *)(offset + lpSecurityDescriptor + 8) = 0x12 ;
	}

	SetLastError( NO_ERROR );
	return 1 ;
}



ULONG WINAPI
fake_SetServiceObjectSecurity (
	SC_HANDLE hService, 
	SECURITY_INFORMATION dwSecurityInformation,
	PSECURITY_DESCRIPTOR lpSecurityDescriptor
	)
{
	if ( Get_service_name_from_handle( hService ) )
	{
		SetLastError( NO_ERROR );
		return 1;
	}

	return 0 ;
}



ULONG WINAPI
fake_NotifyServiceStatusChange (
	SC_HANDLE hService, 
	int a2,
	int a3
	)
{
	if ( Get_service_name_from_handle(hService) )
		SetLastError( NO_ERROR );

	return 0 ;
}



ULONG WINAPI
fake_LockServiceDatabase (
	SC_HANDLE hSCManager
	)
{
	ULONG ret ;

	if ( hSCManager == (SC_HANDLE)0x12340001 )
	{
		ret = 0x12340003 ;
	}
	else
	{
		SetLastError( ERROR_INVALID_HANDLE );
		ret = 0;
	}

	return ret;
}



ULONG WINAPI
fake_UnlockServiceDatabase (
	SC_LOCK ScLock
	)
{
	ULONG bRet ;

	if ( ScLock == (SC_LOCK)0x12340003 )
	{
		bRet = 1;
	}
	else
	{
		SetLastError( ERROR_INVALID_HANDLE );
		bRet = 0;
	}

	return bRet ;
}



ULONG WINAPI
fake_CreateServiceA (
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
    )
{
	NTSTATUS Status ;
	DWORD dwErrorCode; 
	SC_HANDLE hService; 
	ANSI_STRING ansiBuffer;
	UNICODE_STRING lpPassword_uni; 
	UNICODE_STRING lpServiceStartName_uni;
	UNICODE_STRING lpDependencies_uni;
	UNICODE_STRING lpdwTagId_uni; 
	UNICODE_STRING lpLoadOrderGroup_uni; 
	UNICODE_STRING lpBinaryPathName_uni;
	UNICODE_STRING lpDisplayName_uni; 
	UNICODE_STRING lpServiceName_uni;

	lpServiceName_uni.Buffer = 0;
	if ( lpServiceName )
	{
		RtlInitString( &ansiBuffer, lpServiceName );
		Status = RtlAnsiStringToUnicodeString( &lpServiceName_uni, &ansiBuffer, TRUE );
	}

	lpDisplayName_uni.Buffer = 0;
	if ( lpDisplayName )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpDisplayName);
			Status = RtlAnsiStringToUnicodeString(&lpDisplayName_uni, &ansiBuffer, TRUE);
		}
	}

	lpBinaryPathName_uni.Buffer = 0;
	if ( lpBinaryPathName )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpBinaryPathName);
			Status = RtlAnsiStringToUnicodeString(&lpBinaryPathName_uni, &ansiBuffer, TRUE);
		}
	}

	lpLoadOrderGroup_uni.Buffer = 0;
	if ( lpLoadOrderGroup )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpLoadOrderGroup);
			Status = RtlAnsiStringToUnicodeString(&lpLoadOrderGroup_uni, &ansiBuffer, TRUE);
		}
	}

	lpdwTagId_uni.Buffer = 0;
	if ( lpdwTagId )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, (PCSZ)lpdwTagId);
			Status = RtlAnsiStringToUnicodeString(&lpdwTagId_uni, &ansiBuffer, TRUE);
		}
	}

	lpDependencies_uni.Buffer = 0;
	if ( lpDependencies )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpDependencies);
			Status = RtlAnsiStringToUnicodeString(&lpDependencies_uni, &ansiBuffer, TRUE);
		}
	}

	lpServiceStartName_uni.Buffer = 0;
	if ( lpServiceStartName )
	{
		if ( Status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpServiceStartName);
			Status = RtlAnsiStringToUnicodeString(&lpServiceStartName_uni, &ansiBuffer, TRUE);
		}
	}

	lpPassword_uni.Buffer = 0;
	if ( lpPassword )
	{
		if ( Status < 0 )
		{
_ERROR_NOT_ENOUGH_MEMORY_:
			hService = 0;
			dwErrorCode = ERROR_NOT_ENOUGH_MEMORY;
			goto _clear_up_;
		}

		RtlInitString(&ansiBuffer, lpPassword);
		Status = RtlAnsiStringToUnicodeString(&lpPassword_uni, &ansiBuffer, TRUE);
	}

	if ( Status < 0 )
		goto _ERROR_NOT_ENOUGH_MEMORY_;

	hService = (SC_HANDLE) fake_CreateServiceW (
		hSCManager,
		lpServiceName_uni.Buffer,
		lpDisplayName_uni.Buffer,
		dwDesiredAccess,
		dwServiceType,
		dwStartType,
		dwErrorControl,
		lpBinaryPathName_uni.Buffer,
		lpLoadOrderGroup_uni.Buffer,
		(LPDWORD)lpdwTagId_uni.Buffer,
		lpDependencies_uni.Buffer,
		lpServiceStartName_uni.Buffer,
		lpPassword_uni.Buffer);

	dwErrorCode = GetLastError();
_clear_up_:
	if ( lpServiceName_uni.Buffer )
		RtlFreeUnicodeString(&lpServiceName_uni);
	if ( lpDisplayName_uni.Buffer )
		RtlFreeUnicodeString(&lpDisplayName_uni);
	if ( lpBinaryPathName_uni.Buffer )
		RtlFreeUnicodeString(&lpBinaryPathName_uni);
	if ( lpLoadOrderGroup_uni.Buffer )
		RtlFreeUnicodeString(&lpLoadOrderGroup_uni);
	if ( lpdwTagId_uni.Buffer )
		RtlFreeUnicodeString(&lpdwTagId_uni);
	if ( lpDependencies_uni.Buffer )
		RtlFreeUnicodeString(&lpDependencies_uni);
	if ( lpServiceStartName_uni.Buffer )
		RtlFreeUnicodeString(&lpServiceStartName_uni);
	if ( lpPassword_uni.Buffer )
		RtlFreeUnicodeString(&lpPassword_uni);

	SetLastError(dwErrorCode);
	return (ULONG)hService;
}



ULONG WINAPI
fake_CreateServiceW (
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
    )
{
	HANDLE KeyHandle = NULL, hReg = NULL ;
	UNICODE_STRING uniBuffer ;
	ULONG TotalLength = 0, TotalLengthCur = 0, NewLength = 0 ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, Buffer = NULL, pData = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	SC_HANDLE hService = NULL ;

	// 1. 查询服务是否存在,若存在返回相应状态
	hService = (SC_HANDLE) fake_OpenServiceW( hSCManager, lpServiceName, SERVICE_QUERY_STATUS );
	if ( hService )
	{
		fake_CloseServiceHandle( hService );
		SetLastError( ERROR_SERVICE_EXISTS );
		return 0;
	}

	// 只有返回的状态时ERROR_SERVICE_DOES_NOT_EXIST,表明服务不存在,才会继续处理
	if ( GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST )
		return 0;

	// 2. 打开(不存在则创建)对应的服务键值,得到的KeyHandle已被重定向
	KeyHandle = Open_Services_RegKey( (LPWSTR)lpServiceName, TRUE );
	if ( NULL == KeyHandle )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	// 3. 设置服务项的各个键值
	RtlInitUnicodeString( &uniBuffer, L"Type" );
	status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwServiceType, 4 );
	if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }

	RtlInitUnicodeString( &uniBuffer, L"Start" );
	status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwStartType, 4 );
	if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }

	RtlInitUnicodeString( &uniBuffer, L"ErrorControl" );
	status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwErrorControl, 4 );
	if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }

	if ( lpDisplayName )
	{
		RtlInitUnicodeString( &uniBuffer, L"DisplayName" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpDisplayName, (wcslen(lpDisplayName)+1) * 2 );
		if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }
	}

	if ( lpBinaryPathName )
	{
		RtlInitUnicodeString( &uniBuffer, L"ImagePath" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpBinaryPathName, (wcslen(lpBinaryPathName)+1) * 2 );
		if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }
	}

	if ( lpServiceStartName )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObjectName" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpServiceStartName, (wcslen(lpServiceStartName)+1) * 2 );
		if ( ! NT_SUCCESS(status) ) { goto _ERROR_INVALID_PARAMETER_; }
	}

	// 4. 获取当前在沙箱中的服务名(可以包含多个服务),分配内存拷贝出来,在此Buffer末尾追加新增的服务名,再写回注册表
	Buffer = (LPWSTR) Get_RegValueData_SbieSvc_SandboxedServices();
	for ( ptr1 = Buffer; *ptr1; ptr1 += wcslen(ptr1) + 1 )
	{
		TotalLengthCur = TotalLength + wcslen(ptr1) + 1 ;
		TotalLength = TotalLengthCur ;
	}

	// 4.1 分配内存,加入新增的服务名
	pData = (LPWSTR) kmalloc( 2 * (TotalLength + wcslen(lpServiceName)) + 0x10 );
	memcpy( pData, Buffer, 2 * TotalLength );
	wcscpy( &pData[TotalLength], lpServiceName );
	NewLength = TotalLength + wcslen(lpServiceName) + 1 ;
	pData[ NewLength ] = 0 ;

	// 4.2 打开"SandboxedServices"服务键值,将新增的内容写入之
	hReg = Open_Services_RegKey( _PBServiceNameW_, TRUE );
	if ( hReg )
	{
		RtlInitUnicodeString( &uniBuffer, L"SandboxedServices" );
		status = ZwSetValueKey( hReg, &uniBuffer, 0, 7, pData, 2 * NewLength + 2 );
	}
	else
	{
		status = STATUS_UNSUCCESSFUL ;
	}

	// 4.3 收尾工作:释放内存,关闭句柄
	kfree( pData );
	ZwClose( hReg );

	// 5. 若以上操作成功,则返回对应的服务句柄
	if ( NT_SUCCESS(status) )
	{
		ZwClose( KeyHandle );
		DWORD pBuffer = (DWORD) kmalloc( 2 * wcslen(lpServiceName) + 6 );
		
		*(DWORD *)pBuffer = _FuckedTag_;
		wcscpy( (LPWSTR)(pBuffer + 4), lpServiceName );
		wcslwr( (LPWSTR)pBuffer );

		g_Service_TickCount = GetTickCount();
		SetLastError( NO_ERROR );
		return pBuffer ;
	}

	// 6. 操作失败,则返回相应错误
_ERROR_INVALID_PARAMETER_ :
	ZwDeleteKey( KeyHandle );
	ZwClose( KeyHandle );
	SetLastError( ERROR_INVALID_PARAMETER );
	return 0;
}



ULONG WINAPI
fake_ChangeServiceConfigA (
    SC_HANDLE    hService,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword,
    LPCSTR     lpDisplayName
    )
{
	NTSTATUS status = STATUS_SUCCESS ;
	DWORD ErrorCode ;
	BOOL ret ;
	ANSI_STRING ansiBuffer ;
	UNICODE_STRING unilpDisplayName ;
	UNICODE_STRING unilpPassword ; 
	UNICODE_STRING unilpServiceStartName ; 
	UNICODE_STRING unilpDependencies ; 
	UNICODE_STRING unilpdwTagId ; 
	UNICODE_STRING unilpLoadOrderGroup ; 
	UNICODE_STRING unilpBinaryPathName ; 

	unilpBinaryPathName.Buffer = 0;
	if ( lpBinaryPathName )
	{
		RtlInitString(&ansiBuffer, lpBinaryPathName);
		status = RtlAnsiStringToUnicodeString(&unilpBinaryPathName, &ansiBuffer, TRUE);
	}

	unilpLoadOrderGroup.Buffer = 0;
	if ( lpLoadOrderGroup )
	{
		if ( status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpLoadOrderGroup);
			status = RtlAnsiStringToUnicodeString(&unilpLoadOrderGroup, &ansiBuffer, TRUE);
		}
	}

	unilpdwTagId.Buffer = 0;
	if ( lpdwTagId )
	{
		if ( status >= 0 )
		{
			RtlInitString(&ansiBuffer, (PCSZ)lpdwTagId);
			status = RtlAnsiStringToUnicodeString(&unilpdwTagId, &ansiBuffer, TRUE);
		}
	}

	unilpDependencies.Buffer = 0;
	if ( lpDependencies )
	{
		if ( status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpDependencies);
			status = RtlAnsiStringToUnicodeString(&unilpDependencies, &ansiBuffer, TRUE);
		}
	}

	unilpServiceStartName.Buffer = 0;
	if ( lpServiceStartName )
	{
		if ( status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpServiceStartName);
			status = RtlAnsiStringToUnicodeString(&unilpServiceStartName, &ansiBuffer, TRUE);
		}
	}

	unilpPassword.Buffer = 0;
	if ( lpPassword )
	{
		if ( status >= 0 )
		{
			RtlInitString(&ansiBuffer, lpPassword);
			status = RtlAnsiStringToUnicodeString(&unilpPassword, &ansiBuffer, TRUE);
		}
	}

	unilpDisplayName.Buffer = 0;
	if ( lpDisplayName )
	{
		if ( status < 0 )
		{
_ERROR_NOT_ENOUGH_MEMORY_:
			ret = 0;
			ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
			goto _clearup_;
		}

		RtlInitString(&ansiBuffer, lpDisplayName);
		status = RtlAnsiStringToUnicodeString(&unilpDisplayName, &ansiBuffer, TRUE);
	}

	if ( status < 0 )
		goto _ERROR_NOT_ENOUGH_MEMORY_;

	ret = fake_ChangeServiceConfigW (
		hService,
		dwServiceType,
		dwStartType,
		dwErrorControl,
		unilpBinaryPathName.Buffer,
		unilpLoadOrderGroup.Buffer,
		(LPDWORD)unilpdwTagId.Buffer,
		unilpDependencies.Buffer,
		unilpServiceStartName.Buffer,
		unilpPassword.Buffer,
		unilpDisplayName.Buffer);

	ErrorCode = GetLastError();                   

_clearup_:
	if ( unilpBinaryPathName.Buffer )
		RtlFreeUnicodeString(&unilpBinaryPathName);
	if ( unilpLoadOrderGroup.Buffer )
		RtlFreeUnicodeString(&unilpLoadOrderGroup);
	if ( unilpdwTagId.Buffer )
		RtlFreeUnicodeString(&unilpdwTagId);
	if ( unilpDependencies.Buffer )
		RtlFreeUnicodeString(&unilpDependencies);
	if ( unilpServiceStartName.Buffer )
		RtlFreeUnicodeString(&unilpServiceStartName);
	if ( unilpPassword.Buffer )
		RtlFreeUnicodeString(&unilpPassword);
	if ( unilpDisplayName.Buffer )
		RtlFreeUnicodeString(&unilpDisplayName);

	SetLastError(ErrorCode);
	return ret;
}



ULONG WINAPI
fake_ChangeServiceConfigW (
    SC_HANDLE    hService,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword,
    LPCWSTR     lpDisplayName
    )
{
	NTSTATUS status = STATUS_SUCCESS ;
	LPWSTR szServiceName = NULL ;
	HANDLE KeyHandle = NULL ;
	UNICODE_STRING uniBuffer;

	// 1. 通过hService得到服务名,检查是否在白名单中.不是则不让其做事,返回之
	szServiceName = Get_service_name_from_handle( (HANDLE)hService );
	if ( NULL == szServiceName ) { return 0; }

	if ( FALSE == Is_white_service(szServiceName) )
	{
		if ( 0 == wcsnicmp(szServiceName, L"clr_optimization_", 0x11) )
		{
			SetLastError( NO_ERROR );
			return 1;
		}

		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	// 2. 是白名单服务名,做事吧
	KeyHandle = Open_Services_RegKey( szServiceName, TRUE );
	if ( NULL == KeyHandle )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	if ( dwServiceType != -1 )
	{
		RtlInitUnicodeString( &uniBuffer, L"Type" );
		 status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwServiceType, 4 );
		 if ( ! NT_SUCCESS(status) ) { goto _error_ ; }

		 RtlInitUnicodeString( &uniBuffer, L"Start" );
		 status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwStartType, 4 );
		 if ( ! NT_SUCCESS(status) ) { goto _error_ ; }

		 RtlInitUnicodeString( &uniBuffer, L"ErrorControl" );
		 status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwErrorControl, 4 );
		 if ( ! NT_SUCCESS(status) ) { goto _error_ ; }
	}

	if ( lpDisplayName )
	{
		RtlInitUnicodeString( &uniBuffer, L"DisplayName" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpDisplayName, 2 * wcslen(lpDisplayName) + 2 );
		if ( ! NT_SUCCESS(status) ) { goto _error_ ; }
	}

	if ( lpBinaryPathName )
	{
		RtlInitUnicodeString( &uniBuffer, L"ImagePath" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpBinaryPathName, 2 * wcslen(lpBinaryPathName) + 2 );
		if ( ! NT_SUCCESS(status) ) { goto _error_ ; }
	}

	if ( lpServiceStartName )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObjectName" );
		status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_SZ, (PVOID)lpServiceStartName, 2 * wcslen(lpServiceStartName) + 2 );
		if ( ! NT_SUCCESS(status) ) { goto _error_ ; }
	}
	
	ZwClose( KeyHandle );
	SetLastError( NO_ERROR );
	return 1;

_error_ :
	ZwClose( KeyHandle );
	SetLastError( ERROR_INVALID_PARAMETER );
	return 0;
}



ULONG WINAPI
fake_ChangeServiceConfig2 (
	HANDLE hService,
	DWORD dwInfoLevel,
	LPVOID lpInfo
	)
{
	LPWSTR szServiceName = NULL ;

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	if ( FALSE == Is_white_service(szServiceName) )
	{
		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_DeleteService (
	HANDLE hService
	)
{
	int Size = 0 ;
	NTSTATUS Status = STATUS_SUCCESS ;
	HANDLE hKey = NULL, hKey_SbieSvc = NULL ;
	LPWSTR ptr = NULL, PtrOld = NULL, szServiceName = NULL ;
	UNICODE_STRING uniBuffer ;

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	if ( FALSE == Is_white_service(szServiceName) )
	{
		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	hKey = Open_Services_RegKey( szServiceName, TRUE );
	hKey_SbieSvc = Open_Services_RegKey( _PBServiceNameW_, TRUE );

	if ( NULL == hKey || NULL == hKey_SbieSvc )
	{
		if ( hKey ) { ZwClose( hKey ); }
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	Status = ZwDeleteKey( hKey );
	ZwClose( hKey ); 

	if ( ! NT_SUCCESS(Status) )
	{
		ZwClose( hKey_SbieSvc );
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	PtrOld = ptr = (LPWSTR) Get_RegValueData_SbieSvc_SandboxedServices();
	if ( *ptr )
	{
		do
		{
			if ( 0 == wcsicmp(ptr, szServiceName) ) { *ptr = '*'; }

			Size += wcslen(ptr) + 1;
			ptr += wcslen(ptr) + 1;
		}
		while ( *ptr );
	}

	RtlInitUnicodeString( &uniBuffer, L"SandboxedServices" );
	ZwSetValueKey( hKey_SbieSvc, &uniBuffer, 0, REG_MULTI_SZ, PtrOld, 2 * Size + 2 );
	ZwClose( hKey_SbieSvc );
	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI
fake_StartService (
	SC_HANDLE hService, 
	DWORD dwNumServiceArgs, 
	LPCWSTR *lpServiceArgVectors
	)
{
	LPWSTR szServiceName = NULL ; 
	LPRPC_OUT_HEADER pOutBuffer = NULL ; 
	RPC_IN_StartService pInBuffer ; 

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	if ( Is_white_service(szServiceName) )
	{
		return PB_StartBoxedService( szServiceName );
	}

	pInBuffer.NameLength = wcslen( szServiceName );
	pInBuffer.RpcHeader.DataLength = 2 * pInBuffer.NameLength + 0x12 ;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_StartService_ ;

	pInBuffer.szName = (LPWSTR) kmalloc( pInBuffer.NameLength + 2 );
	wcscpy( pInBuffer.szName, szServiceName );

	pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pInBuffer );
	kfree( pInBuffer.szName );
	if ( NULL == pOutBuffer ) { return 0; }

	PB_FreeReply( pOutBuffer );
	SetLastError( pOutBuffer->u.ErrorCode );

	if ( ERROR_SUCCESS == pOutBuffer->u.ErrorCode )
	{
		return 1;
	}

	return 0;
}



ULONG WINAPI
fake_StartServiceCtrlDispatcherA (
	IN LPSERVICE_TABLE_ENTRYA lpServiceStartTable
	)
{
	return StartServiceCtrlDispatcher_filter( FALSE, (int)lpServiceStartTable );
}



ULONG WINAPI
fake_StartServiceCtrlDispatcherW (
	IN LPSERVICE_TABLE_ENTRYW lpServiceStartTable
	)
{
	return StartServiceCtrlDispatcher_filter( TRUE, (int)lpServiceStartTable );
}



ULONG WINAPI
fake_RegisterServiceCtrlHandler (
	int lpServiceName, 
	LPHANDLER_FUNCTION lpHandlerProc
	)
{
	g_RegisterServiceCtrlHandler_lpHandlerProc_addr = lpHandlerProc ;
	return 0x12340002 ;
}



ULONG WINAPI
fake_RegisterServiceCtrlHandlerEx (
	LPCWSTR lpServiceName,
	LPHANDLER_FUNCTION_EX lpHandlerProc,
	LPVOID lpContext
	)
{
	g_RegisterServiceCtrlHandlerEx_lpHandlerProc = lpHandlerProc ;
	g_RegisterServiceCtrlHandlerEx_lpContext = lpContext ;
	return 0x12340002 ;
}



ULONG WINAPI
fake_SetServiceStatus (
	SC_HANDLE hService,
	LPVOID lpServiceData
	)
{
	return Set_service_data( hService, (PSERVICE_DATA)lpServiceData, g_CurrentService_KeyHandle );
}



ULONG WINAPI
fake_ControlService (
	HANDLE hService,
	DWORD dwControl,
	LPSERVICE_STATUS lpServiceStatus
	)
{
	HANDLE KeyHandle = NULL ;
	LPWSTR szServiceName = NULL ;
	LPFuckService_Info pBuffer = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = NULL ;
	ULONG SBIE_CurrentState = 0, ResultLength = 0, BufferSize = 0, nCounts = 0 ;
	UNICODE_STRING uniBuffer ;

	szServiceName = Get_service_name_from_handle( hService );
	if ( NULL == szServiceName ) { return 0; }

	if ( FALSE == Is_white_service(szServiceName) )
	{
		if ( dwControl == SERVICE_CONTROL_CONTINUE || dwControl == SERVICE_CONTROL_INTERROGATE )
			return fake_QueryServiceStatus( (SC_HANDLE)hService, lpServiceStatus );

		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	pBuffer = (LPFuckService_Info) FuckService( szServiceName, 1, 0 );
	if ( NULL == pBuffer ) { return 0; }

	SBIE_CurrentState = pBuffer->SBIE_CurrentState;
	kfree( pBuffer );

	if ( SBIE_CurrentState == SERVICE_CONTROL_STOP )
	{
		SetLastError( ERROR_SERVICE_NOT_ACTIVE );
		return 0;
	}

	if ( SBIE_CurrentState != SERVICE_CONTROL_INTERROGATE && SBIE_CurrentState != SERVICE_CONTROL_NETBINDADD )
	{
		SetLastError( ERROR_SERVICE_CANNOT_ACCEPT_CTRL );
		return 0;
	}

	KeyHandle = Open_Services_RegKey( szServiceName, TRUE );
	if ( NULL == KeyHandle )
	{
		SetLastError( ERROR_SERVICE_REQUEST_TIMEOUT );
		return 0;
	}

	BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR) ;
	KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) kmalloc( BufferSize );

	RtlInitUnicodeString( &uniBuffer, L"SBIE_ControlCode" );
	status = ZwSetValueKey( KeyHandle, &uniBuffer, 0, REG_DWORD, &dwControl, 4 );
	
	if ( NT_SUCCESS(status) )
	{
		do
		{
			RemoveRegNode__service( szServiceName );

			status = ZwQueryValueKey ( KeyHandle, &uniBuffer, KeyValuePartialInformation, KeyInfo, 0x80, &ResultLength );
			if ( status < 0 ) { break; }
		}
		while ( (nCounts++ + 1) < 0xE );

		ZwClose( KeyHandle );
		if ( status == STATUS_OBJECT_NAME_NOT_FOUND )
		{
			return fake_QueryServiceStatus( (SC_HANDLE)hService, lpServiceStatus );
		}
	}
	else
	{
		ZwClose( KeyHandle );
	}

	if ( KeyInfo ) { kfree( KeyInfo ); }

	SetLastError( ERROR_SERVICE_REQUEST_TIMEOUT );
	return 0;
}



ULONG WINAPI
fake_RegisterEventSourceA (
	LPCSTR lpUNCServerName,
	LPCSTR lpSourceName
	)
{
	DWORD dwErrorCode ;
	UNICODE_STRING uniBuffer ;
	ANSI_STRING ansiBuffer   ;

	RtlInitString( &ansiBuffer, lpSourceName );
	RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );

	SetLastError( NO_ERROR );
	dwErrorCode = GetLastError();
	RtlFreeUnicodeString( &uniBuffer );
	SetLastError(dwErrorCode);
	return 0x12340004 ;
}



ULONG WINAPI
fake_RegisterEventSourceW (
	LPCWSTR lpUNCServerName,
	LPCWSTR lpSourceName
	)
{
	SetLastError( NO_ERROR );
	return 0x12340004 ;
}



ULONG WINAPI
fake_DeregisterEventSource (
	int a1
	)
{
	SetLastError( NO_ERROR );
	return 1;
}



ULONG WINAPI fake_ReportEvent(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
	SetLastError( NO_ERROR );
	return 1;
}



///////////////////////////////   END OF FILE   ///////////////////////////////