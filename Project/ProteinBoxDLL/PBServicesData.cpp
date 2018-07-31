/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/01/19 [19:1:2011 - 15:16]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\PBServicesData.cpp
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
#include "MemoryManager.h"
#include "PBFilesData.h"
#include "PBLoadData.h"
#include "PBToken.h"
#include "PBRegsData.h"
#include "PBDynamicData.h"
#include "PBServicesData.h"
#include "PBServices.h"

#pragma warning(disable : 4995 )


//////////////////////////////////////////////////////////////////////////

DWORD g_pRegValueData_SbieSvc_SandboxedServices = 0 ;

BOOL g_bFlag_SBIE_CurrentState_is_1 = FALSE ;
BOOL g_bFlag_SBIE_CurrentState_is_4 = FALSE ;

BOOL g_bFlag_is_MSIServer = FALSE ;

HANDLE g_CurrentService_KeyHandle = NULL ;
HANDLE g_hEvent_PB_WindowsInstallerInUse = NULL ;

LPHANDLER_FUNCTION g_RegisterServiceCtrlHandler_lpHandlerProc_addr = NULL ;
LPHANDLER_FUNCTION_EX g_RegisterServiceCtrlHandlerEx_lpHandlerProc = NULL ;
LPVOID g_RegisterServiceCtrlHandlerEx_lpContext = NULL ;

static const SerArrayInfo g_Service_Arrays[ ] = 
{ 
	{ L"bits", L"PBBITS.exe" },
	{ L"cryptsvc", L"PBCrypto.exe" },
	{ L"wuauserv", L"PBWUAU.exe" },
};


//
// Services
//

static HOOKINFOLittle g_HookInfoServices_Array [] = 
{
	{ Nothing_TAG, OpenSCManagerA_TAG, "OpenSCManagerA", 0, NULL, fake_OpenSCManager },
	{ Nothing_TAG, OpenSCManagerW_TAG, "OpenSCManagerW", 0, NULL, fake_OpenSCManager },
	{ Nothing_TAG, OpenServiceA_TAG, "OpenServiceA", 0, NULL, fake_OpenServiceA },
	{ Nothing_TAG, OpenServiceW_TAG, "OpenServiceW", 0, NULL, fake_OpenServiceW },
	{ Nothing_TAG, CloseServiceHandle_TAG, "CloseServiceHandle", 0, NULL, fake_CloseServiceHandle },
	{ Nothing_TAG, QueryServiceStatus_TAG, "QueryServiceStatus", 0, NULL, fake_QueryServiceStatus },
	{ Nothing_TAG, QueryServiceStatusEx_TAG, "QueryServiceStatusEx", 0, NULL, fake_QueryServiceStatusEx },
	{ Nothing_TAG, QueryServiceConfigA_TAG, "QueryServiceConfigA", 0, NULL, fake_QueryServiceConfigA },
	{ Nothing_TAG, QueryServiceConfigW_TAG, "QueryServiceConfigW", 0, NULL, fake_QueryServiceConfigW },
	{ Nothing_TAG, QueryServiceConfig2A_TAG, "QueryServiceConfig2A", 0, NULL, fake_QueryServiceConfig2A },
	{ Nothing_TAG, QueryServiceConfig2W_TAG, "QueryServiceConfig2W", 0, NULL, fake_QueryServiceConfig2W },
	{ Nothing_TAG, EnumServicesStatusA_TAG, "EnumServicesStatusA", 0, NULL, fake_EnumServicesStatusA },
	{ Nothing_TAG, EnumServicesStatusW_TAG, "EnumServicesStatusW", 0, NULL, fake_EnumServicesStatusW },
	{ Nothing_TAG, EnumServicesStatusExA_TAG, "EnumServicesStatusExA", 0, NULL, fake_EnumServicesStatusExA },
	{ Nothing_TAG, EnumServicesStatusExW_TAG, "EnumServicesStatusExW", 0, NULL, fake_EnumServicesStatusExW },
	{ Nothing_TAG, QueryServiceLockStatusA_TAG, "QueryServiceLockStatusA", 0, NULL, fake_QueryServiceLockStatus },
	{ Nothing_TAG, QueryServiceLockStatusW_TAG, "QueryServiceLockStatusW", 0, NULL, fake_QueryServiceLockStatus },
	{ Nothing_TAG, GetServiceDisplayNameA_TAG, "GetServiceDisplayNameA", 0, NULL, fake_GetServiceDisplayNameA },
	{ Nothing_TAG, GetServiceDisplayNameW_TAG, "GetServiceDisplayNameW", 0, NULL, fake_GetServiceDisplayNameW },
	{ Nothing_TAG, GetServiceKeyNameA_TAG, "GetServiceKeyNameA", 0, NULL, fake_GetServiceKeyNameA },
	{ Nothing_TAG, GetServiceKeyNameW_TAG, "GetServiceKeyNameW", 0, NULL, fake_GetServiceKeyNameW },
	{ Nothing_TAG, EnumDependentServicesA_TAG, "EnumDependentServicesA", 0, NULL, fake_EnumDependentServices },
	{ Nothing_TAG, EnumDependentServicesW_TAG, "EnumDependentServicesW", 0, NULL, fake_EnumDependentServices },
	{ Nothing_TAG, QueryServiceObjectSecurity_TAG, "QueryServiceObjectSecurity", 0, NULL, fake_QueryServiceObjectSecurity },
	{ Nothing_TAG, SetServiceObjectSecurity_TAG, "SetServiceObjectSecurity", 0, NULL, fake_SetServiceObjectSecurity },
	{ Nothing_TAG, LockServiceDatabase_TAG, "LockServiceDatabase", 0, NULL, fake_LockServiceDatabase },
	{ Nothing_TAG, UnlockServiceDatabase_TAG, "UnlockServiceDatabase", 0, NULL, fake_UnlockServiceDatabase },
	{ Nothing_TAG, CreateServiceA_TAG, "CreateServiceA", 0, NULL, fake_CreateServiceA },
	{ Nothing_TAG, CreateServiceW_TAG, "CreateServiceW", 0, NULL, fake_CreateServiceW },
	{ Nothing_TAG, ChangeServiceConfigA_TAG, "ChangeServiceConfigA", 0, NULL, fake_ChangeServiceConfigA },
	{ Nothing_TAG, ChangeServiceConfigW_TAG, "ChangeServiceConfigW", 0, NULL, fake_ChangeServiceConfigW },
	{ Nothing_TAG, ChangeServiceConfig2A_TAG, "ChangeServiceConfig2A", 0, NULL, fake_ChangeServiceConfig2 },
	{ Nothing_TAG, ChangeServiceConfig2W_TAG, "ChangeServiceConfig2W", 0, NULL, fake_ChangeServiceConfig2 },
	{ Nothing_TAG, DeleteService_TAG, "DeleteService", 0, NULL, fake_DeleteService },
	{ Nothing_TAG, StartServiceA_TAG, "StartServiceA", 0, NULL, fake_StartService },
	{ Nothing_TAG, StartServiceW_TAG, "StartServiceW", 0, NULL, fake_StartService },
	{ Nothing_TAG, StartServiceCtrlDispatcherA_TAG, "StartServiceCtrlDispatcherA", 0, NULL, fake_StartServiceCtrlDispatcherA },
	{ Nothing_TAG, StartServiceCtrlDispatcherW_TAG, "StartServiceCtrlDispatcherW", 0, NULL, fake_StartServiceCtrlDispatcherW },
	{ Nothing_TAG, RegisterServiceCtrlHandlerA_TAG, "RegisterServiceCtrlHandlerA", 0, NULL, fake_RegisterServiceCtrlHandler },
	{ Nothing_TAG, RegisterServiceCtrlHandlerW_TAG, "RegisterServiceCtrlHandlerW", 0, NULL, fake_RegisterServiceCtrlHandler },
	{ Nothing_TAG, RegisterServiceCtrlHandlerExA_TAG, "RegisterServiceCtrlHandlerExA", 0, NULL, fake_RegisterServiceCtrlHandlerEx },
	{ Nothing_TAG, RegisterServiceCtrlHandlerExW_TAG, "RegisterServiceCtrlHandlerExW", 0, NULL, fake_RegisterServiceCtrlHandlerEx },
	{ Nothing_TAG, SetServiceStatus_TAG, "SetServiceStatus", 0, NULL, fake_SetServiceStatus },
	{ Nothing_TAG, ControlService_TAG, "ControlService", 0, NULL, fake_ControlService },
//	{ Nothing_TAG, ControlServiceExW_TAG, "ControlServiceExW", 0, NULL, fake_ControlServiceExW },
//	{ Nothing_TAG, ControlServiceExA_TAG, "ControlServiceExA", 0, NULL, fake_ControlServiceExA },
	{ Nothing_TAG, RegisterEventSourceA_TAG, "RegisterEventSourceA", 0, NULL, fake_RegisterEventSourceA },
	{ Nothing_TAG, RegisterEventSourceW_TAG, "RegisterEventSourceW", 0, NULL, fake_RegisterEventSourceW },
	{ Nothing_TAG, DeregisterEventSource_TAG, "DeregisterEventSource", 0, NULL, fake_DeregisterEventSource },
	{ Nothing_TAG, ReportEventA_TAG, "ReportEventA", 0, NULL, fake_ReportEvent },
	{ Nothing_TAG, ReportEventW_TAG, "ReportEventW", 0, NULL, fake_ReportEvent },
//	{ Nothing_TAG, NotifyServiceStatusChange_TAG, "NotifyServiceStatusChange", 0, NULL, fake_NotifyServiceStatusChange }, // 该API可能有,可能没有,有的话才Hook

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


BOOL HandlerServices( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;
	int i = 0, TotalCounts = 0, NotifyServiceStatusChange_addr = 0 ;
	LPHOOKINFOLittle pArray = NULL ;

	// 1. 获取函数原始地址,保存至数组中
	pArray		= g_HookInfo.SERVICES.pArray      = g_HookInfoServices_Array ;
	TotalCounts = g_HookInfo.SERVICES.ArrayCounts = ARRAYSIZEOF( g_HookInfoServices_Array );

	for( i=0; i<TotalCounts; i++ )
	{
		g_HookInfoServices_Array[ i ].OrignalAddress = (PVOID) GetProcAddress( hModule, g_HookInfoServices_Array[ i ].FunctionName );
		if ( NULL == g_HookInfoServices_Array[ i ].OrignalAddress ) { return FALSE; }
	}

	// 2. 开始Hook Services相关API
	for( i=0; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | HandlerServices() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	NotifyServiceStatusChange_addr = (int) GetProcAddress( __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG], "NotifyServiceStatusChange" );
	if ( NotifyServiceStatusChange_addr ) 
	{
		bRet = Mhook_SetHook( (PVOID*)&NotifyServiceStatusChange_addr, fake_NotifyServiceStatusChange );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | HandlerServices() - Mhook_SetHook(); | \"NotifyServiceStatusChange\" \n" );
			return FALSE ;
		}
	}

	return TRUE;
}


void RemoveRegNode__service( IN LPWSTR ServiceName )
{
	LPWSTR ServicePath = (LPWSTR) kmalloc( 0x200 );

	wcscpy( ServicePath, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\" );
	wcscat( ServicePath, ServiceName );
	
	RemoveRegNodeExp( ServicePath, TRUE );
	
	kfree( ServicePath );
	return;
}


static const LPWSTR g_WhiteList_ServiceName[] =	// 对应函数 Is_white_service()
{ 
	L"MSIServer",
	L"TrustedInstaller",
	L"bits",
	L"wuauserv",
	L"cryptsvc"
} ;


BOOL Is_white_service( IN LPWSTR szServiceName )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/01/19 [19:1:2011 - 15:48]

Routine Description:
  得到"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PBSvc"项中SandboxedServices的内容;还有一组白名单: g_WhiteList_ServiceName
  若@szServiceName 在以上白名单中,返回TRUE;否则返回FALSE.
    
Arguments:
  szServiceName - [IN] 要检查匹配的服务名

Return Value:
  BOOL
    
--*/
{
	ULONG index = 0 ;
	LPWSTR szPath = NULL ; 

	// 1. 从注册表中取出动态的白名单
	szPath = (LPWSTR) Get_RegValueData_SbieSvc_SandboxedServices();
	if ( *szPath )
	{
		while ( TRUE )
		{
			if ( 0 == wcsicmp(szPath, szServiceName) ) { return TRUE; }

			szPath += wcslen(szPath) + 1 ;
			if ( NULL == *szPath ) { break; }
		}
	}

	// 2. 过第2道固定白名单
	for ( index = 0; index < ARRAYSIZEOF(g_WhiteList_ServiceName); index++ )
	{
		if ( 0 == wcsicmp( szServiceName, g_WhiteList_ServiceName[index] )  ) 
		{ 
			return TRUE ;
		}
	}

	return FALSE ;
}


int 
Get_serviceStatus_rpc (
	IN LPWSTR szServiceName,
	IN int dwInfoLevel,
	IN int Flag1
	)
{
	DWORD err = 0 ;
	LPRPC_OUT_HEADER pOutBuffer = NULL ;
	RPC_IN_EnumServiceStatus pInBuffer ; 

	if ( Is_white_service(szServiceName) ) { return FuckService( szServiceName, Flag1, dwInfoLevel ); }

	pInBuffer.NameLength = wcslen(szServiceName);
	if ( (pInBuffer.NameLength+1) * sizeof(WCHAR) > 0x200 ) { return 0; }
//	pInBuffer.lpServiceName = (LPWSTR) kmalloc( (pInBuffer.NameLength+1) * sizeof(WCHAR) );
	wcscpy( pInBuffer.wszServiceName, szServiceName );

	pInBuffer.Flag1 = (short)Flag1 ;
	pInBuffer.dwInfoLevel = (short)dwInfoLevel ;
	pInBuffer.RpcHeader.DataLength = 2 * pInBuffer.NameLength + 0x16;
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_EnumServiceStatus_ ;

	pOutBuffer = (LPRPC_OUT_HEADER) PB_CallServer( &pInBuffer );
//	kfree( pInBuffer.lpServiceName );

	if ( pOutBuffer )
	{
		err = pOutBuffer->u.ErrorCode;
		if ( err )
		{
			PB_FreeReply( pOutBuffer );
			SetLastError(err);
			return 0;
		}
	}
	else
	{
		err = RPC_S_SERVER_UNAVAILABLE ;
	}

	SetLastError( err );
	return (int)pOutBuffer ;
}


LPWSTR Get_service_name_from_handle ( IN HANDLE hService )
{
	LPWSTR ptr = NULL ;

	if ( hService && _FuckedTag_ == *(DWORD *)hService ) 
	{ 
		ptr = (LPWSTR)( (char *)hService + 4 );
	}

	if ( NULL == ptr ) { SetLastError( ERROR_INVALID_HANDLE ); }
	return ptr;
}


ULONG 
EnumServiceStatus_filter (
	SC_HANDLE hSCManager,
	DWORD dwServiceType, 
	DWORD dwServiceState,
	int lpServices, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded, 
	LPDWORD lpServicesReturned, 
	LPDWORD lpResumeHandle, 
	BOOL bIsUnicode, 
	BOOL bIsEx
	)
{
	BOOL bFlag1 = FALSE, bFlag2 = FALSE ;
	SC_HANDLE hService = NULL, hServiceDummy = NULL ;
	ULONG ret = 0, StructLength = 0 ;
	DWORD n = 0, lpServicesPtr = 0, ResumeHandleCounts = 0, ServiceNameLength = 0, ServicesReturned = 0 ;
	DWORD ResumeHandleCurCounts = 0, offset = 0, dwErrorCode = 0, ServiceNameCurLength = 0, DisplayNameLength = 0 ;
	DWORD BufSize = 0, cbBytesNeeded = 0 ;
	LPWSTR lpServiceName = NULL, lpServiceNamePtrCur = NULL, Name = NULL, lpServiceNamePtrBegin = NULL ;
	LPWSTR lpDisplayName = NULL, lpServicesPtrCur1 = NULL, lpServicesPtrCur2 = NULL ;
	LPQUERY_SERVICE_CONFIGW lpServiceConfig = NULL ;
	LPENUM_SERVICE_STATUS_PROCESSW pOutBuffer = NULL ;
	SERVICE_STATUS_PROCESS ServiceStatusProcess ;
	UNICODE_STRING uniBuffer ;
	ANSI_STRING ansiBuffer ;

	// 1. 检测参数合法性
	if ( hSCManager != (SC_HANDLE)0x12340001 )
	{
		SetLastError( ERROR_INVALID_HANDLE );
		return 0;
	}

	if ( !(dwServiceType & SERVICE_TYPE_ALL) || !(dwServiceState & SERVICE_STATE_ALL) )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return 0;
	}

	// 2. 初始化各个局部变量
	StructLength = 0x2C ;
	if ( FALSE == bIsEx ) { StructLength = 0x24 ; }

	lpServiceConfig = (LPQUERY_SERVICE_CONFIGW) kmalloc( 0x2000 );

	lpServicesPtr = lpServices + cbBufSize - 1 ;
	lpServiceNamePtrCur = lpServiceName = (LPWSTR) GetSandboxedServices();

	if ( lpResumeHandle )
	{
		if ( *lpResumeHandle )
		{
			do
			{
				if ( ! *lpServiceName ) { break; }

				ServiceNameLength = wcslen( lpServiceName );
				lpServiceNamePtrCur = &lpServiceName[ ServiceNameLength + 1 ];
				lpServiceName += ServiceNameLength + 1 ;
				++ n ;
			}
			while ( n < *lpResumeHandle );
		}

		ResumeHandleCounts = *lpResumeHandle ;
		hServiceDummy = hService ;
	}

	if ( ! *lpServiceName ) { goto _over_ ; }

	ResumeHandleCurCounts = ResumeHandleCounts - 1 ;
	offset = lpServices + 0x10 ;

	// 3. 超级大循环
	while ( TRUE )
	{
		if ( hServiceDummy ) 
		{
			dwErrorCode = NO_ERROR ;
			if ( hServiceDummy != (SC_HANDLE)0x12340001 )
			{
				if ( Get_service_name_from_handle(hServiceDummy) )
				{
					kfree( hServiceDummy );
				}
				else
				{	
					dwErrorCode = ERROR_INVALID_HANDLE ;
				}
			}

			SetLastError( dwErrorCode );
		}

		hService = (SC_HANDLE)fake_OpenServiceW( (SC_HANDLE)0x12340001, lpServiceName, SERVICE_QUERY_STATUS );
		if ( NULL == hService ) { goto _over_ ; }

		Name = Get_service_name_from_handle( hService );
		if ( NULL == Name || (pOutBuffer = (LPENUM_SERVICE_STATUS_PROCESSW)Get_serviceStatus_rpc(Name, 0, 1), NULL == pOutBuffer) )
		{
			goto _end_ ;
		}

		memcpy( &ServiceStatusProcess, &pOutBuffer->ServiceStatusProcess, sizeof(ServiceStatusProcess) );
		kfree( pOutBuffer );
		SetLastError( NO_ERROR );

		lpServiceNamePtrBegin = lpServiceNamePtrCur ;
		ServiceNameCurLength = wcslen( lpServiceNamePtrCur ) + 1 ;
		lpServiceNamePtrCur += ServiceNameCurLength ;
		++ ResumeHandleCurCounts ;

		if ( !(ServiceStatusProcess.dwServiceType & dwServiceType) ) { goto _while_next_ ; }

		bFlag1 = ServiceStatusProcess.dwCurrentState == SERVICE_ACTIVE ? (dwServiceState & SERVICE_INACTIVE) == 0 : (dwServiceState & SERVICE_ACTIVE) == 0;
		if ( bFlag1 ) { goto _while_next_ ; }

		if ( !fake_QueryServiceConfigW(hService, lpServiceConfig, 0x2000, 0) ) { goto _end_ ; }

		lpDisplayName = lpServiceConfig->lpDisplayName;
		if ( lpDisplayName )
			DisplayNameLength = wcslen(lpDisplayName) + 1;
		else
			DisplayNameLength = 0;

		if ( bIsUnicode )
		{
			ServiceNameCurLength *= 2 ;
			DisplayNameLength *= 2 ;
		}

		BufSize = StructLength + DisplayNameLength + ServiceNameCurLength ;

		if ( cbBytesNeeded )
		{
			cbBytesNeeded += BufSize ;
			goto _while_next_ ;
		}
	
		if ( cbBufSize < BufSize )
		{
			if ( 0 == cbBytesNeeded && lpResumeHandle )
			{
				*lpResumeHandle = ResumeHandleCurCounts ;
			}

			cbBytesNeeded += BufSize ;
			goto _while_next_ ;
		}

		cbBufSize -= BufSize ;
		++ ServicesReturned ;

		if ( bIsUnicode )
		{
			*(DWORD *)(offset - 0xC) = 0;

			lpServicesPtrCur1 = (LPWSTR)(lpServicesPtr - ServiceNameCurLength) ;
			memcpy( (PVOID)lpServicesPtrCur1, lpServiceNamePtrBegin, ServiceNameCurLength );
			*(DWORD *)(offset - 0x10) = (DWORD)lpServicesPtrCur1 ;

			if ( lpDisplayName )
			{
				lpServicesPtrCur2 = lpServicesPtrCur1 - DisplayNameLength; 
				memcpy( (PVOID)lpServicesPtrCur2, lpDisplayName, DisplayNameLength );
				*(DWORD *)(offset - 0xC) = (DWORD)lpServicesPtrCur2 ;
			}
		}
		else
		{
			lpServicesPtr -= ServiceNameCurLength ;

			RtlInitUnicodeString( &uniBuffer, lpServiceNamePtrBegin );

			ansiBuffer.Length = 0 ;
			ansiBuffer.MaximumLength = (USHORT)ServiceNameCurLength ;
			ansiBuffer.Buffer = (PCHAR)lpServicesPtr ;

			RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );

			*(DWORD *)(offset - 0xC) = 0 ;
			*(DWORD *)(offset - 0x10) = lpServicesPtr ;

			if ( lpDisplayName )
			{
				lpServicesPtr = lpServicesPtr - DisplayNameLength ;

				RtlInitUnicodeString( &uniBuffer, lpDisplayName );

				ansiBuffer.Length = 0 ;
				ansiBuffer.MaximumLength = (USHORT)DisplayNameLength ;
				ansiBuffer.Buffer = (PCHAR) lpServicesPtr ;

				RtlUnicodeStringToAnsiString( &ansiBuffer, &uniBuffer, FALSE );
				
				*(DWORD *)(offset - 0xC) = lpServicesPtr ;
			}
		}

		*(DWORD *)(offset - 8)		= ServiceStatusProcess.dwServiceType ;
		*(DWORD *)(offset - 4)		= ServiceStatusProcess.dwCurrentState ;
		*(DWORD *) offset			= ServiceStatusProcess.dwControlsAccepted ;
		*(DWORD *)(offset + 4)		= 0 ;
		*(DWORD *)(offset + 8)		= 0 ;
		*(DWORD *)(offset + 0xC)	= 0 ;
		*(DWORD *)(offset + 0x10)	= 0 ;

		if ( bIsEx )
		{
			*(DWORD *)(offset + 0x14) = ServiceStatusProcess.dwProcessId ;
			*(DWORD *)(offset + 0x18) = ServiceStatusProcess.dwServiceFlags ;
		}

		offset += StructLength ;

_while_next_ :
		if ( !*lpServiceNamePtrCur )
		{
			fake_CloseServiceHandle( hService );
			break ;
		}

		hServiceDummy = hService ;
		lpServiceName = lpServiceNamePtrCur;

	} // end-of-while
	
_over_ :
	*pcbBytesNeeded = cbBytesNeeded ;
	*lpServicesReturned = ServicesReturned ;
	
	if ( 0 == cbBytesNeeded )
	{
		SetLastError( NO_ERROR );
		ret = 1;
	}
	else
	{
		SetLastError( ERROR_MORE_DATA );
		ret = 1;
	}

	return ret;

_end_ :
	fake_CloseServiceHandle( hService );
	kfree( lpServiceConfig );
	return 0 ;
}


HANDLE 
Open_Services_RegKey (
	IN LPWSTR ServiceName,
	IN BOOL bCreateNewKey
	)
{
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR ServicePath[ MAX_PATH ] ;
	UNICODE_STRING uniName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	ULONG Disposition ;

	wcscpy( ServicePath, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\" );
	wcscat( ServicePath, ServiceName );
	RtlInitUnicodeString( &uniName, ServicePath );

	InitializeObjectAttributes( &ObjAtr, &uniName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	if ( bCreateNewKey )
		status = ZwCreateKey( &hKey, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, &Disposition );
	else
		status = ZwOpenKey( &hKey, KEY_QUERY_VALUE, &ObjAtr );

	if ( status >= 0 ) { return hKey ; }
	return NULL ;
}


PVOID Get_RegValueData_SbieSvc_SandboxedServices()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/01/26 [26:1:2011 - 14:18]

Routine Description:
  想方设法得到"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PBSvc"项中SandboxedServices的内容;
  1. 先判定总结构体+0x0xEC处保存的内容是否存在,不存在则查询注册表获取之
  2. 计算TickCount,即每隔一时段就重新获取一次,保证得到的内容是最新的

Return Value:
  SandboxedServices的内容
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bGetData = FALSE ;
	DWORD CurrentTickCount = 0, OldTickCount = 0, ResultLength = 0 ;
	PVOID ServiceDatas = NULL ;
	HANDLE KeyHandle = NULL ;
	UNICODE_STRING uniBuffer ;
	KEY_VALUE_PARTIAL_INFORMATION KeyInfo ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	CurrentTickCount = GetTickCount();
	ServiceDatas = pNode->sSandboxServices.ServiceDatas;

	if ( ServiceDatas )
	{
		OldTickCount = pNode->sSandboxServices.OldTickCount ;
		if ( OldTickCount <= g_Service_TickCount || CurrentTickCount - OldTickCount > 0x7D0 )
		{
			if ( ServiceDatas != &g_pRegValueData_SbieSvc_SandboxedServices )
			{
				kfree( pNode->sSandboxServices.ServiceDatas );
			}

			bGetData = TRUE ;
		}
	}
	else
	{
		bGetData = TRUE ;
	}

	if ( FALSE == bGetData ) { return ServiceDatas; }

	pNode->sSandboxServices.ServiceDatas = &g_pRegValueData_SbieSvc_SandboxedServices ;
	pNode->sSandboxServices.OldTickCount = CurrentTickCount ;
	g_Service_TickCount = CurrentTickCount ;

	KeyHandle = Open_Services_RegKey( _PBServiceNameW_, FALSE );
	if ( NULL == KeyHandle ) { goto _over_; }

	RtlInitUnicodeString( &uniBuffer, L"SandboxedServices" );
	status = ZwQueryValueKey( KeyHandle, &uniBuffer, KeyValuePartialInformation, &KeyInfo, 0x200, &ResultLength );

	ZwClose( KeyHandle );
	if ( ! NT_SUCCESS(status) ) { goto _over_; }

	if ( KeyInfo.Type != REG_MULTI_SZ || 0 == KeyInfo.DataLength ) { goto _over_; }

	pNode->sSandboxServices.ServiceDatas = ServiceDatas = kmalloc( KeyInfo.DataLength + 8 );
	memcpy( ServiceDatas, KeyInfo.Data, KeyInfo.DataLength );
	return ServiceDatas ;

_over_ :
	ServiceDatas = &g_pRegValueData_SbieSvc_SandboxedServices ;
	return ServiceDatas ;
}


int 
FuckService (
	IN LPWSTR ServiceName,
	IN int pid,
	IN int Flag
	)
{
	BOOL bRet = FALSE, bFlag = FALSE ;
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPFuckService_Info pBuffer = NULL ;
	ULONG NeedSize = 0, ResultLength = 0, BufferSize = 0, length = 0 ;
	LPWSTR szDisplayName = NULL, szImagePath = NULL, szObjectName = NULL ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL, ptr5 = NULL ;
	UNICODE_STRING ValueName ;
	PB_BOX_INFO BoxInfo ;
	PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = NULL ;

	RemoveRegNode__service( ServiceName );

	hKey = Open_Services_RegKey( ServiceName, FALSE );
	if ( NULL == hKey )
	{
		SetLastError( ERROR_INVALID_HANDLE );
		return 0;
	}

	// 计算需要申请的内存总大小
	if ( Flag )
	{
		if ( 1 == Flag || 3 == Flag || 4 == Flag || 7 == Flag )
		{
			NeedSize = 4 ;
		}
		else if ( 2 == Flag )
		{
			NeedSize = 0x14 ;
		}
		else if ( 0xFFFFFFFF == Flag )
		{
			BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR) ;
			KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) kmalloc( BufferSize );

			//
			RtlInitUnicodeString( &ValueName, L"DisplayName" );
			status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
			if ( NT_SUCCESS(status) && KeyInfo->Type == REG_SZ && KeyInfo->DataLength )
			{
				ResultLength = 2 * wcslen((LPWSTR)KeyInfo->Data) + 2;
				szDisplayName = (LPWSTR) kmalloc( ResultLength );

				memcpy( szDisplayName, KeyInfo->Data, ResultLength );
				szDisplayName[ ResultLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				NeedSize = ResultLength + 0x26 ;
			}

			//
			RtlInitUnicodeString( &ValueName, L"ImagePath" );
			status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
			if ( NT_SUCCESS(status) && (KeyInfo->Type == REG_SZ || KeyInfo->Type == REG_EXPAND_SZ) && KeyInfo->DataLength )
			{
				ResultLength = 2 * wcslen((LPWSTR)KeyInfo->Data) + 2;
				szImagePath = (LPWSTR) kmalloc( ResultLength );

				memcpy( szImagePath, KeyInfo->Data, ResultLength );
				szImagePath[ ResultLength / sizeof(WCHAR) ] = UNICODE_NULL ;
				
				NeedSize += ResultLength ;
			}

			//
			RtlInitUnicodeString( &ValueName, L"ObjectName" );
			status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
			if ( NT_SUCCESS(status) && KeyInfo->Type == REG_SZ && KeyInfo->DataLength )
			{
				ResultLength = 2 * wcslen((LPWSTR)KeyInfo->Data) + 2;
				szObjectName = (LPWSTR) kmalloc( ResultLength );

				memcpy( szObjectName, KeyInfo->Data, ResultLength );
				szObjectName[ ResultLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				NeedSize += ResultLength ;
			}
		}
		else
		{
			ZwClose( hKey );
			SetLastError( ERROR_INVALID_LEVEL );
			return 0;
		}
	}

	// 根据计算的总大小申请内存.填充之
	pBuffer = (LPFuckService_Info) kmalloc( NeedSize + 0x54 );

	pBuffer->StructLength = NeedSize + 0x54 ;
	pBuffer->NeedSize = NeedSize ;
	pBuffer->SBIE_CurrentState = 1 ;
	pBuffer->SBIE_ProcessId = 0 ;

	//
	RtlInitUnicodeString( &ValueName, L"Type" );
	status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
	if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
	{
		pBuffer->ServiceType = *(DWORD *) &KeyInfo->Data[0] ;
	}

	if ( !(pBuffer->ServiceType & 0x30) || !pid ) { goto _goon4_ ; }

	//
	RtlInitUnicodeString( &ValueName, L"SBIE_CurrentState" );
	status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
	if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
	{
		pBuffer->SBIE_CurrentState = *(DWORD *) &KeyInfo->Data[0] ;
	}

	//
	RtlInitUnicodeString( &ValueName, L"SBIE_ProcessId" );
	status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
	if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
	{
		bRet = PB_QueryProcess( *(PULONG) &KeyInfo->Data[0], &BoxInfo );
		if ( FALSE == bRet ) { goto _goon2_ ; }

		if ( (g_BoxInfo.SessionId != BoxInfo.SessionId) || wcsicmp(BoxInfo.BoxName, g_BoxInfo.BoxName) ) { goto _goon2_ ; }

		ptr1 = wcsrchr( BoxInfo.ProcessName, '.' ); // eg:"PBCrypto.exe"
		if ( ptr1 ) { *ptr1 = 0 ; }

		RtlInitUnicodeString( &ValueName, L"ImagePath" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && (KeyInfo->Type == REG_SZ || KeyInfo->Type == REG_EXPAND_SZ) && KeyInfo->DataLength )
		{
			ptr2 = wcsrchr( (LPWSTR)KeyInfo->Data, '\\' ); // eg:"%SystemRoot%\system32\svchost.exe -k netsvcs"
			
			if ( ptr2 || (ptr2 = wcsrchr( (LPWSTR)KeyInfo->Data, '\"' )) != 0 )
				ptr3 = ptr2 + 1 ;
			else
				ptr3 = (LPWSTR)KeyInfo->Data ;

			ptr4 = wcsrchr(ptr3, '\"');   // eg:"svchost.exe -k netsvcs"
			if ( ptr4 || (ptr4 = wcsrchr(ptr3, ' ')) != 0 ) { *ptr4 = 0; }

			ptr5 = wcsrchr(ptr3, '.');    // 举例:"svchost.exe -k"
			if ( ptr5 ) { *ptr5 = 0; }

			if ( wcsicmp( ptr3, BoxInfo.ProcessName ) ) // eg: *ptr3 == "svchost"
			{
				for (int Index = 0; Index < ARRAYSIZEOF(g_Service_Arrays); Index++ )
				{
					if ( wcsicmp( ServiceName, g_Service_Arrays[Index].ServiceName ) ) { continue; }

					length = (wcsrchr( g_Service_Arrays[Index].ProcessName, '.' ) - g_Service_Arrays[Index].ProcessName) / sizeof(WCHAR) ;
					if ( 0 == wcsnicmp( BoxInfo.ProcessName, g_Service_Arrays[Index].ProcessName, length ) )
					{
						bFlag = TRUE;
						break ;
					}
				}

				if ( FALSE == bFlag ) { goto _goon2_ ; }
			}

			pBuffer->SBIE_ProcessId = *(DWORD *) &KeyInfo->Data[0] ;
		}
	}

_goon2_ :
	if ( pBuffer->SBIE_CurrentState == 4 && 0 == pBuffer->SBIE_ProcessId ) { pBuffer->SBIE_CurrentState = 1 ; }

	if ( pBuffer->SBIE_ProcessId )
	{
		//
		RtlInitUnicodeString( &ValueName, L"SBIE_ControlsAccepted" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			pBuffer->SBIE_ControlsAccepted = *(ULONG *) &KeyInfo->Data[0] ;
		}

		//
		RtlInitUnicodeString( &ValueName, L"SBIE_Win32ExitCode" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			pBuffer->SBIE_Win32ExitCode = *(ULONG *) &KeyInfo->Data[0] ;
		}

		//
		RtlInitUnicodeString( &ValueName, L"SBIE_ServiceSpecificExitCode" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			pBuffer->SBIE_ServiceSpecificExitCode = *(ULONG *) &KeyInfo->Data[0] ;
		}

		//
		RtlInitUnicodeString( &ValueName, L"SBIE_CheckPoint" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			pBuffer->SBIE_CheckPoint = *(ULONG *) &KeyInfo->Data[0] ;
		}

		//
		RtlInitUnicodeString( &ValueName, L"SBIE_WaitHint" );
		status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			pBuffer->SBIE_WaitHint = *(ULONG *) &KeyInfo->Data[0] ;
		}
	}

_goon4_ :
	if ( Flag != 0xFFFFFFFF ) { goto _over_ ; }

	pBuffer->Reserved7 = 0 ;
	pBuffer->Reserved6 = 0x24 ;
	pBuffer->Reserved4 = 0x24 ;
	ptr1 = pBuffer->Buffer ;

	if ( szDisplayName )
	{
		wcscpy( pBuffer->Buffer, szDisplayName );
		pBuffer->szDisplayNameOffset = 0x26 ;

		ptr1 += wcslen(ptr1) + 1 ;
		kfree( szDisplayName );
	}

	if ( szImagePath )
	{
		wcscpy( ptr1, szImagePath );
		pBuffer->szImagePathOffset = (ULONG)((char *)ptr1 - (char *)&pBuffer->Separatrix);
		
		ptr1 += wcslen(ptr1) + 1 ;
		kfree( szImagePath );
	}

	if ( szObjectName )
	{
		wcscpy( ptr1, szObjectName );
		pBuffer->szObjectNameOffset = (ULONG)((char *)ptr1 - (char *)&pBuffer->Separatrix);
		kfree( szObjectName );
	}

	pBuffer->Separatrix = pBuffer->ServiceType ;

	//
	RtlInitUnicodeString( &ValueName, L"Start" );
	status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
	if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
	{
		pBuffer->ServiceStart = *(ULONG *) &KeyInfo->Data[0] ;
	}

	//
	RtlInitUnicodeString( &ValueName, L"ErrorControl" );
	status = ZwQueryValueKey( hKey, &ValueName, KeyValuePartialInformation, KeyInfo, 0x200, &ResultLength );
	if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
	{
		pBuffer->ServiceErrorControl = *(ULONG *) &KeyInfo->Data[0] ;
	}

_over_ :
	ZwClose( hKey );
	if ( KeyInfo ) { kfree( KeyInfo ); }
	return (int)pBuffer ;
}


ULONG
Set_service_data (
	IN SC_HANDLE hService,
	IN PSERVICE_DATA lpServiceData,
	IN HANDLE KeyHandle
	)
{
	DWORD SBIE_CurrentState ; 
	UNICODE_STRING ValueName;
	DWORD Buffer ;

	if ( hService != (SC_HANDLE)0x12340002 )
	{
		SetLastError( ERROR_INVALID_HANDLE );
		return 0;
	}
	
	RtlInitUnicodeString( &ValueName, L"SBIE_ProcessId" );
	Buffer = GetCurrentProcessId();
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_CurrentState" );
	Buffer = lpServiceData->SBIE_CurrentState;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_ControlsAccepted" );
	Buffer = lpServiceData->SBIE_ControlsAccepted;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_Win32ExitCode" );
	Buffer = lpServiceData->SBIE_Win32ExitCode;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_ServiceSpecificExitCode" );
	Buffer = lpServiceData->SBIE_ServiceSpecificExitCode;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_CheckPoint");
	Buffer = lpServiceData->SBIE_CheckPoint;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	RtlInitUnicodeString( &ValueName, L"SBIE_WaitHint");
	Buffer = lpServiceData->SBIE_WaitHint;
	ZwSetValueKey( KeyHandle, &ValueName, 0, REG_DWORD, &Buffer, 4 );

	SBIE_CurrentState = lpServiceData->SBIE_CurrentState ;
	if ( SBIE_CurrentState == 4 )
	{
		g_bFlag_SBIE_CurrentState_is_4 = TRUE ;
	}
	else if ( SBIE_CurrentState == 1 )
	{
		g_bFlag_SBIE_CurrentState_is_1 = TRUE ;
	}

	return 1 ;
}


int
StartServiceCtrlDispatcher_filter (
	IN BOOL bUniCode,
	IN int lpServiceStartTable
	)
{
	BOOL bFlag = FALSE ;
	ULONG ThreadId = 0, BufferSize = 0, ResultLength = 0 ;
	HANDLE KeyHandle = NULL, hEvent = NULL ;
	LPWSTR ServiceName = NULL ;
	PVOID ChangeBuffer = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	ANSI_STRING    ansiBuffer ;
	UNICODE_STRING uniBuffer  ;
	SERVICE_DATA ServiceDatas ;
	IO_STATUS_BLOCK IoStatusBlock ;
	StartServiceCtrlDispatcherThreadInfo ThreadInfo ;
	PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = NULL ;

	if ( bUniCode )
	{
		ServiceName = *(LPWSTR *) lpServiceStartTable ;
	}
	else
	{
		RtlInitString( &ansiBuffer, *(PCSZ *)lpServiceStartTable );
		RtlAnsiStringToUnicodeString( &uniBuffer, &ansiBuffer, TRUE );
		ServiceName = uniBuffer.Buffer ;
	}

	KeyHandle = Open_Services_RegKey( ServiceName, TRUE );
	g_CurrentService_KeyHandle = KeyHandle ;
	if ( NULL == KeyHandle )
	{
		SetLastError( ERROR_SERVICE_DOES_NOT_EXIST );
		return 0;
	}

	ServiceDatas.SBIE_ProcessId = 0;
	ServiceDatas.SBIE_Win32ExitCode = 0;
	ServiceDatas.SBIE_ServiceSpecificExitCode = 0;
	ServiceDatas.SBIE_CheckPoint = 0;
	ServiceDatas.SBIE_CurrentState = 2;
	ServiceDatas.SBIE_ControlsAccepted = 1;
	ServiceDatas.SBIE_WaitHint = 5000;

	Set_service_data( (SC_HANDLE)0x12340002, &ServiceDatas, KeyHandle );
	
	ThreadInfo.Reserved = 0 ;
	ThreadInfo.lpServiceName = *(PVOID *) lpServiceStartTable ;
	ThreadInfo.lpServiceProc = *(PVOID *)(lpServiceStartTable + 4);

	if ( 0 == wcsicmp(ServiceName, L"MSIServer") ) { g_bFlag_is_MSIServer = TRUE ; }

	if ( FALSE == CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartServiceCtrlDispatcherThread, &ThreadInfo, 0, &ThreadId) )
	{
		g_bFlag_SBIE_CurrentState_is_1 = TRUE ;
	}

	hEvent = CreateEventW( 0, 0, 0, 0 );
	if ( NULL == hEvent ) { return 0; }

	ChangeBuffer = kmalloc( 0x400 );
	RtlInitUnicodeString( &uniBuffer, L"SBIE_ControlCode" );

	BufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR) ;
	KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION) kmalloc( BufferSize );

	while ( FALSE == g_bFlag_SBIE_CurrentState_is_1 )
	{
		if ( FALSE == bFlag && g_bFlag_SBIE_CurrentState_is_4 ) { bFlag = TRUE; }

		status = ZwNotifyChangeKey (
			g_CurrentService_KeyHandle,
			hEvent,
			NULL,
			NULL,
			&IoStatusBlock,
			REG_NOTIFY_CHANGE_LAST_SET,
			FALSE,
			ChangeBuffer,
			0x400,
			TRUE
			);

		if ( ! NT_SUCCESS(status) ) { continue; }

		if ( WaitForSingleObject(hEvent, 1000) < 0 ) { continue; }

		RemoveRegNode__service( ServiceName );

		status = ZwQueryValueKey (
			g_CurrentService_KeyHandle,
			&uniBuffer,
			KeyValuePartialInformation,
			KeyInfo,
			0x80,
			&ResultLength
			);

		if ( NT_SUCCESS(status) && KeyInfo->Type == REG_DWORD && 4 == KeyInfo->DataLength )
		{
			if ( g_RegisterServiceCtrlHandler_lpHandlerProc_addr )
			{
				g_RegisterServiceCtrlHandler_lpHandlerProc_addr( *(DWORD *)&KeyInfo->Data[0] );
			}
			else if ( g_RegisterServiceCtrlHandlerEx_lpHandlerProc )
			{
				g_RegisterServiceCtrlHandlerEx_lpHandlerProc( *(DWORD *)&KeyInfo->Data[0],0,0,g_RegisterServiceCtrlHandlerEx_lpContext );
			}
		}

		ZwDeleteValueKey( g_CurrentService_KeyHandle, &uniBuffer );
	} // end-of-while

	if ( g_bFlag_SBIE_CurrentState_is_4 ) { return 1; }
	return 0;
}


LPWSTR GetSandboxedServices()
{
	ULONG length1 = 1000, length2 = 0, length3 = 0, Size = 0, SizeDummy = 0 ;
	LPWSTR pBuffer1 = NULL, pBuffer2 = NULL ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, ptr1Cur = NULL, ptr1CurDummy = NULL, ptr2Cur = NULL ;
	LPRPC_OUT_GetSandboxedServices pOutBuffer = NULL ;
	RPC_IN_GetSandboxedServices pInBuffer ;

	ptr1 = (LPWSTR)&g_pRegValueData_SbieSvc_SandboxedServices ;

	pInBuffer.RpcHeader.DataLength = sizeof(RPC_IN_GetSandboxedServices);
	pInBuffer.RpcHeader.Flag = _PBSRV_APINUM_GetSandboxedServices_ ;
	pInBuffer.dwServiceType = 0x13F ;
	pInBuffer.dwServiceState = SERVICE_STATE_ALL;

	pOutBuffer = (LPRPC_OUT_GetSandboxedServices) PB_CallServer( &pInBuffer );
	if ( pOutBuffer && STATUS_SUCCESS == pOutBuffer->RpcHeader.u.Status )
	{
		ptr1 = pOutBuffer->szPath;
	}

	ptr2 = (LPWSTR) Get_RegValueData_SbieSvc_SandboxedServices();
	pBuffer1 = (LPWSTR) kmalloc( 2000 );

	while ( TRUE )
	{
		ptr1Cur = ptr1;
		for ( ptr1CurDummy = 0; *ptr1Cur; ptr1Cur += wcslen(ptr1Cur) + 1 )
		{
			if ( *ptr1Cur != '*' )
			{
				if ( !ptr1CurDummy || wcsicmp(ptr1Cur, ptr1CurDummy) < 0 )
					ptr1CurDummy = ptr1Cur;
			}
		}

		for ( ptr2Cur = ptr2; *ptr2Cur; ptr2Cur += wcslen(ptr2Cur) + 1 )
		{
			if ( *ptr2Cur != '*' )
			{
				if ( !ptr1CurDummy || wcsicmp(ptr2Cur, ptr1CurDummy) < 0 )
					ptr1CurDummy = ptr2Cur;
			}
		}

		if ( NULL == ptr1CurDummy ) { break; }

		if ( wcslen(ptr1CurDummy) + Size + 1 >= length1 )
		{
			length1 += 1000 ;
			pBuffer2 = (LPWSTR) kmalloc( 2 * length1 );

			memcpy( pBuffer2, pBuffer1, 2 * Size );
			kfree( pBuffer1 );
			Size = SizeDummy ; 
			pBuffer1 = pBuffer2 ;
		}

		wcscpy( &pBuffer1[Size], ptr1CurDummy );

		length3 = wcslen( ptr1CurDummy );
		SizeDummy = Size + length3 + 1 ;
		*ptr1CurDummy = '*' ;
		Size += length3 + 1 ;

	}  // end-of-while

	g_Service_TickCount = GetTickCount();
	pBuffer1[ Size ] = 0;

	if ( pOutBuffer ) { PB_FreeReply( pOutBuffer ); }
	return pBuffer1 ;
}


int 
StartServiceCtrlDispatcherThread (
	IN LPStartServiceCtrlDispatcherThreadInfo lpServiceStartTable
	)
{
	HANDLE hEvent = NULL ;

	((void (__cdecl *)(signed int, PVOID *))lpServiceStartTable->lpServiceProc)( 1, &lpServiceStartTable->lpServiceName );
	
	if ( FALSE == g_bFlag_is_MSIServer ) { return 0; }
	
	if ( g_hEvent_PB_WindowsInstallerInUse )
	{
		CloseHandle( g_hEvent_PB_WindowsInstallerInUse );
		g_hEvent_PB_WindowsInstallerInUse = NULL ;
	}

	for ( hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"SBIE_WindowsInstallerInUse");
		hEvent;
		hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"SBIE_WindowsInstallerInUse") )
	{
		CloseHandle( hEvent );
		Sleep( 2000 );
	}

	ExitProcess( 0 );
	return 0;
}

///////////////////////////////   END OF FILE   ///////////////////////////////