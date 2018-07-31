/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2012/02/01 [1:2:2012 - 17:11]
* MODULE : \Code\Project\ProteinBoxDLL\PBWtsapi32dll\PBWtsapi32dll.cpp
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

#include "../StdAfx.h"
#include "../common.h"
#include "../MemoryManager.h"
#include "../HookHelper.h"
#include "../PBDynamicData.h"
#include "../ProteinBoxDLL.h"
#include "../Exportfunc.h"
#include "../PBServices.h"
#include "../PBServicesData.h"
#include "../PBCreateProcess.h"
#include "../PBUser32dll/PBUser32dll.h"
#include "PBWtsapi32dll.h"

//////////////////////////////////////////////////////////////////////////

typedef HWND (WINAPI* _WTSFreeMemory_)(PVOID);
_WTSFreeMemory_ g_WTSFreeMemory_addr = NULL ;

typedef HWND (WINAPI* _WinStationFreeMemory_)(PVOID);
_WinStationFreeMemory_ g_WinStationFreeMemory_addr = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

PVOID Call_GetUserObjectInformation()
{
	ULONG Length = 0x100 ;
	PVOID buffer = NULL ;
	HMODULE hModule = NULL ;
	HANDLE WindowStation = NULL ;
	HANDLE (*GetProcessWindowStation)(void) ;
	int (__stdcall *GetUserObjectInformationW)(HANDLE, signed int, PVOID, DWORD, LPDWORD) ; 

	hModule = Get_hModule_user32DLL();
	if ( NULL == hModule ) { return NULL; }

	GetProcessWindowStation = (HANDLE (*)(void))GetProcAddress( hModule, "GetProcessWindowStation" );
	GetUserObjectInformationW = (int (__stdcall *)(HANDLE, signed int, PVOID, DWORD, LPDWORD))GetProcAddress( hModule, "GetUserObjectInformationW" );

	if (   NULL == GetProcessWindowStation
		|| (WindowStation = GetProcessWindowStation(), !WindowStation)
		|| NULL == GetUserObjectInformationW 
		)
	{
		return NULL ;
	}

	buffer = kmalloc( Length );
	if ( !GetUserObjectInformationW( WindowStation, 2, buffer, 0x100, &Length ) )
	{
		kfree( buffer );
		return NULL ;
	}

	return buffer ;
}


ULONG WTSFreeMemory_filter( int pMemory, _WTSFreeMemory_ pfn )
{
	PWTSEnumerateSessionsW_INFO pInfo = (PWTSEnumerateSessionsW_INFO) (pMemory - 8) ;

	if ( _FuckedTag_ == pInfo->Tag && 0 == pInfo->Reserved2 )
	{
		kfree( (PVOID)pMemory );
		return 1;
	}

	return (ULONG) pfn( (PVOID)pMemory ) ;
}


ULONG WINAPI fake_WTSQueryUserToken( int a1, PHANDLE phToken )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/05/06 [6:5:2011 - 14:46]

Routine Description:
先调用PB_EnumProcessEx()统计沙箱中运行的进程个数 & PID,对每个PID,调用PB_QueryProcess()得到对应的进程名,若是"PBRpcSs.exe"
则调用OpenProcess / NtOpenProcessToken 将得到的hToken保存至参数二@phToken

Arguments:
phToken - [OUT] 保存得到的句柄

--*/
{
	PB_BOX_INFO BoxInfo ;
	BOOL bRet = FALSE ;
	HANDLE hProcess = NULL ;
	ULONG PID = 0, TotalProcCounts = 0 ;
	ULONG pBuffer = 0, nIndex = 1, dwErrCode = ERROR_NO_TOKEN ;
	NTSTATUS status = STATUS_SUCCESS ;

	if ( NULL == phToken ) { return 0; }
	*phToken = NULL ;

	pBuffer = (ULONG) kmalloc( 0x800 );
	status = PB_EnumProcessEx( g_BoxInfo.BoxName, (int*)pBuffer );
	if ( ! NT_SUCCESS(status) ) { goto _end_ ; }

	TotalProcCounts = *(DWORD *)pBuffer ;
	if ( TotalProcCounts < 1 ) { goto _end_ ; }

	while ( TRUE )
	{
		PID = *(ULONG *)( pBuffer + 4 * nIndex );
		bRet = PB_QueryProcess( PID, &BoxInfo );
		if ( bRet && 0 == wcsicmp( BoxInfo.BoxName, g_PBRpcSs_exe ) )
		{
			hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, PID );
			if ( hProcess ) { break; }
		} 

		++ nIndex ;
		if ( nIndex > TotalProcCounts ) { goto _end_ ; }
	}

	status = ZwOpenProcessToken( hProcess, TOKEN_ALL_ACCESS, phToken );
	if ( NT_SUCCESS(status) ) { dwErrCode = NO_ERROR; }
	CloseHandle( hProcess );

_end_ :
	kfree( (PVOID)pBuffer );
	SetLastError( dwErrCode );
	return dwErrCode == 0 ;
}


ULONG WINAPI fake_WTSEnumerateSessionsW( HANDLE hServer, DWORD Reserved, DWORD Version, PWTS_SESSION_INFOW *ppSessionInfo, LPDWORD pCount )
{
	LPWSTR pWinStationName = NULL ;
	PWTSEnumerateSessionsW_INFO pInfo = NULL ;

	if ( hServer && hServer != (HANDLE)0x12345678 )
	{
		*pCount = 0;
		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	if ( Reserved || Version != 1 )
	{
		*pCount = 0;
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	pWinStationName = (LPWSTR) Call_GetUserObjectInformation();
	if ( NULL == pWinStationName )
	{
		*pCount = 0;
		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	pInfo = (PWTSEnumerateSessionsW_INFO) kmalloc( wcslen(pWinStationName) * sizeof(WCHAR) + 0x24);

	pInfo->Tag = (ULONG)_FuckedTag_ ;
	pInfo->Reserved2 = 0;
	pInfo->ppSessionInfo.SessionId = g_BoxInfo.SessionId ;
	pInfo->ppSessionInfo.State = 0;
	pInfo->ppSessionInfo.pWinStationName = (LPWSTR) pInfo->pWinStationName ;

	wcscpy( pInfo->pWinStationName, pWinStationName );

	*ppSessionInfo = &pInfo->ppSessionInfo ;
	*pCount = 1 ;
	kfree( pWinStationName );
	SetLastError( NO_ERROR );

	return 1 ;
}


ULONG WINAPI fake_WTSFreeMemory( PVOID pMemory )
{
	return WTSFreeMemory_filter( (int)pMemory, g_WTSFreeMemory_addr );
}


ULONG WINAPI fake_WTSRegisterSessionNotification(int a1, int a2)
{
	SetLastError(NO_ERROR);
	return 1;
}


ULONG WINAPI fake_WTSRegisterSessionNotificationEx(int a1, int a2, int a3)
{
	SetLastError(NO_ERROR);
	return 1;
}


ULONG WINAPI fake_WinStationOpenServerW(int a1)
{
	ULONG ret = 0 ;

	if ( a1 )
	{
		SetLastError(ERROR_ACCESS_DENIED);
	}
	else
	{
		SetLastError(NO_ERROR);
		ret = 0x12345678 ;
	}

	return ret;
}


ULONG WINAPI fake_WinStationCloseServer(int a1)
{
	SetLastError(NO_ERROR);
	return 1;
}


ULONG WINAPI fake_WinStationEnumerateW( int a1, PWTSEnumerateSessionsW_INFO *ppSessionInfo, LPDWORD Counts )
{
	int pInfo = 0 ;
	LPWSTR buffer = NULL ;

	if ( a1 && a1 != 0x12345678 )
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return 0;
	}
	
	buffer = (LPWSTR)Call_GetUserObjectInformation();
	if ( NULL == buffer )
	{
		*Counts = 0;
		SetLastError(ERROR_ACCESS_DENIED);
		return 0;
	}
	
	pInfo = (int)kmalloc( 0x54 );
	*(DWORD *)pInfo = _FuckedTag_ ;
	*(DWORD *)(pInfo + 4) = 0;
	*(DWORD *)(pInfo + 8) = g_BoxInfo.SessionId ;
	*(DWORD *)(pInfo + 0x50) = 0;
	wcsncpy( (LPWSTR)(pInfo + 0xC), buffer, 0x22 );

	*ppSessionInfo = (PWTSEnumerateSessionsW_INFO)(pInfo + 8);
	*Counts = 1;
	kfree( buffer );
	SetLastError( NO_ERROR );
	return 1;	
}


ULONG WINAPI 
fake_WinStaQueryInformationW (
	HANDLE hServer,
	ULONG ulLogonId,
	int WinStationInformationClass,
	PVOID pWinStationInformation,
	ULONG ulWinStationInformationLength,
	PULONG pulReturnLength
	)
{
	RPC_IN_WINSTAQUERYINFORMATIONW pInBuffer ;
	LPRPC_OUT_WINSTAQUERYINFORMATIONW pOutBuffer = NULL ;
	int ulLogonIdDummy = 0, dwErrorCode = 0 ;

	if ( hServer && hServer != (HANDLE)0x12345678 )
	{
		SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	ulLogonIdDummy = ulLogonId ;
	if ( 0xFFFFFFFF == ulLogonId ) { ulLogonIdDummy = g_BoxInfo.SessionId ; }

	pInBuffer.ulLogonId						= ulLogonIdDummy ;
	pInBuffer.RpcHeader.DataLength			= sizeof(pInBuffer) ;
	pInBuffer.RpcHeader.Flag				= _PBSRV_APINUM_WinStaQueryInformationW_ ;
	pInBuffer.WinStationInformationClass	= WinStationInformationClass ;
	pInBuffer.ulWinStationInformationLength = ulWinStationInformationLength ;

	pOutBuffer = (LPRPC_OUT_WINSTAQUERYINFORMATIONW) PB_CallServer( &pInBuffer );
	if ( NULL == pOutBuffer )
	{
		SetLastError( ERROR_GEN_FAILURE );
		return 0;
	}

	dwErrorCode = pOutBuffer->RpcHeader.u.ErrorCode ;
	if ( ERROR_SUCCESS == dwErrorCode )
	{
		memcpy( pWinStationInformation, pOutBuffer->WinStationInformation, pOutBuffer->ulReturnLength );
		*pulReturnLength = pOutBuffer->ulReturnLength ;
	}

	PB_FreeReply( pOutBuffer );
	SetLastError( dwErrorCode );

	return (ULONG)(dwErrorCode == 0);
}


ULONG WINAPI fake_WinStationFreeMemory(PVOID pMemory)
{
	return WTSFreeMemory_filter( (int)pMemory, g_WinStationFreeMemory_addr );
}


BOOL WINAPI Hook_pharase14_wtsapi32dll( IN HMODULE hModule )
{
	BOOL bRet = FALSE, bIsWhite = FALSE, bIsBlack = FALSE ;
	DWORD WTSQueryUserToken_addr = 0, WTSEnumerateSessionsW_addr = 0 ;
	DWORD WTSRegisterSessionNotification_addr = 0, WTSRegisterSessionNotificationEx_addr = 0 ;
	DWORD WTSUnRegisterSessionNotification_addr = 0, WTSUnRegisterSessionNotificationEx_addr = 0 ;

	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, L"\\RPC Control\\IcaApi", &bIsWhite, &bIsBlack );
	if ( bIsWhite ) { return TRUE; }

	WTSQueryUserToken_addr = (DWORD) GetProcAddress( hModule, "WTSQueryUserToken" );
	WTSEnumerateSessionsW_addr = (DWORD) GetProcAddress( hModule, "WTSEnumerateSessionsW" );
	g_WTSFreeMemory_addr = (_WTSFreeMemory_) GetProcAddress( hModule, "WTSFreeMemory" );
	WTSRegisterSessionNotification_addr = (DWORD) GetProcAddress( hModule, "WTSRegisterSessionNotification" );
	WTSRegisterSessionNotificationEx_addr = (DWORD) GetProcAddress( hModule, "WTSRegisterSessionNotificationEx" );
	WTSUnRegisterSessionNotification_addr = (DWORD) GetProcAddress( hModule, "WTSUnRegisterSessionNotification" );
	WTSUnRegisterSessionNotificationEx_addr = (DWORD) GetProcAddress( hModule, "WTSUnRegisterSessionNotificationEx" );

	if ( WTSQueryUserToken_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSQueryUserToken_addr, fake_WTSQueryUserToken );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSQueryUserToken\" \n" );
			return FALSE ;
		}
	}

	if ( WTSEnumerateSessionsW_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSEnumerateSessionsW_addr, fake_WTSEnumerateSessionsW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSEnumerateSessionsW\" \n" );
			return FALSE ;
		}
	}

	if ( g_WTSFreeMemory_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_WTSFreeMemory_addr, fake_WTSFreeMemory );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSFreeMemory\" \n" );
			return FALSE ;
		}
	}

	if ( WTSRegisterSessionNotification_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSRegisterSessionNotification_addr, fake_WTSRegisterSessionNotification );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSRegisterSessionNotification\" \n" );
			return FALSE ;
		}
	}

	if ( WTSRegisterSessionNotificationEx_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSRegisterSessionNotificationEx_addr, fake_WTSRegisterSessionNotificationEx );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSRegisterSessionNotificationEx\" \n" );
			return FALSE ;
		}
	}

	if ( WTSUnRegisterSessionNotification_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSUnRegisterSessionNotification_addr, fake_DeregisterEventSource );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSUnRegisterSessionNotification\" \n" );
			return FALSE ;
		}
	}

	if ( WTSUnRegisterSessionNotificationEx_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&WTSUnRegisterSessionNotificationEx_addr, fake_WTSRegisterSessionNotification );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase14_wtsapi32dll() - Mhook_SetHook(); | \"WTSUnRegisterSessionNotificationEx\" \n" );
			return FALSE ;
		}
	}

	return TRUE;
}


BOOL WINAPI Hook_pharase15_winstadll( IN HMODULE hModule )
{
	BOOL bRet = FALSE, bIsWhite = FALSE, bIsBlack = FALSE ;
	DWORD WinStationOpenServerW_addr = 0, WinStationCloseServer_addr = 0 ;
	DWORD WinStationEnumerateW_addr = 0, WinStationQueryInformationW_addr = 0 ;

	WhiteOrBlack( WhiteOrBlack_Flag_XIpcPath, L"\\RPC Control\\IcaApi", &bIsWhite, &bIsBlack );
	if ( bIsWhite ) { return TRUE; }

	WinStationOpenServerW_addr = (DWORD) GetProcAddress( hModule, "WinStationOpenServerW" );
	WinStationCloseServer_addr = (DWORD) GetProcAddress( hModule, "WinStationCloseServer" );
	WinStationEnumerateW_addr  = (DWORD) GetProcAddress( hModule, "WinStationEnumerateW" );
	WinStationQueryInformationW_addr = (DWORD) GetProcAddress( hModule, "WinStationQueryInformationW" );
	g_WinStationFreeMemory_addr = (_WinStationFreeMemory_) GetProcAddress( hModule, "WinStationFreeMemory" );

	bRet = Mhook_SetHook( (PVOID*)&WinStationOpenServerW_addr, fake_WinStationOpenServerW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase15_winstadll() - Mhook_SetHook(); | \"WinStationOpenServerW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&WinStationCloseServer_addr, fake_WinStationCloseServer );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase15_winstadll() - Mhook_SetHook(); | \"WinStationCloseServer\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&WinStationEnumerateW_addr, fake_WinStationEnumerateW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase15_winstadll() - Mhook_SetHook(); | \"WinStationEnumerateW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&WinStationQueryInformationW_addr, fake_WinStaQueryInformationW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase15_winstadll() - Mhook_SetHook(); | \"WinStaQueryInformationW\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_WinStationFreeMemory_addr, fake_WinStationFreeMemory );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase15_winstadll() - Mhook_SetHook(); | \"WinStationFreeMemory\" \n" );
		return FALSE ;
	}

	return TRUE;
}

///////////////////////////////   END OF FILE   ///////////////////////////////