#pragma once


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


extern ULONG g_Service_TickCount ;

typedef struct _QUERY_SERVICE_CONFIGW_X64_ 
{
	DWORD dwServiceType;
	DWORD dwStartType;
	DWORD dwErrorControl;
	ULONG Reserved1 ;
	LPWSTR lpBinaryPathName;
	DWORD BinaryPathNameLength ;
	LPWSTR lpLoadOrderGroup ;
	DWORD LoadOrderGroupLength ;
	DWORD dwTagId ;
	ULONG Reserved2 ;
	LPWSTR  lpDependencies ;
	ULONG Reserved3 ;
	LPWSTR  lpServiceStartName ;
	ULONG Reserved4 ;
	LPWSTR  lpDisplayName;

} QUERY_SERVICE_CONFIGW_X64, *LPQUERY_SERVICE_CONFIGW_X64 ;


typedef struct _RPC_OUT_QueryServiceConfigW_
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ ULONG Reserved[ 9 ];
/*0x02C*/ DWORD cbBytesNeeded ;
/*0x030*/
	union
	{
		QUERY_SERVICE_CONFIGW_X64 ServiceConfigX64 ;
		QUERY_SERVICE_CONFIGW ServiceConfigX86 ;
	} u ;

} RPC_OUT_QueryServiceConfigW, *LPRPC_OUT_QueryServiceConfigW ;


// fake_StartServiceW()
typedef struct _RPC_IN_StartService_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG	NameLength ;
/*0x00C */  LPWSTR   szName ;

} RPC_IN_StartService, *LPRPC_IN_StartService ;



typedef enum _CMxx_INFO_ 
{
	CR_SUCCESS                        = 0x00000000,
	CR_DEFAULT                        = 0x00000001,
	CR_OUT_OF_MEMORY                  = 0x00000002,
	CR_INVALID_POINTER                = 0x00000003,
	CR_INVALID_FLAG                   = 0x00000004,
	CR_INVALID_DEVNODE                = 0x00000005,
	CR_INVALID_DEVINST                = CR_INVALID_DEVNODE,
	CR_INVALID_RES_DES                = 0x00000006,
	CR_INVALID_LOG_CONF               = 0x00000007,
	CR_INVALID_ARBITRATOR             = 0x00000008,
	CR_INVALID_NODELIST               = 0x00000009,
	CR_DEVNODE_HAS_REQS               = 0x0000000A,
	CR_DEVINST_HAS_REQS               = CR_DEVNODE_HAS_REQS,
	CR_INVALID_RESOURCEID             = 0x0000000B,
	CR_DLVXD_NOT_FOUND                = 0x0000000C,
	CR_NO_SUCH_DEVNODE                = 0x0000000D,
	CR_NO_SUCH_DEVINST                = CR_NO_SUCH_DEVNODE,
	CR_NO_MORE_LOG_CONF               = 0x0000000E,
	CR_NO_MORE_RES_DES                = 0x0000000F,
	CR_ALREADY_SUCH_DEVNODE           = 0x00000010,
	CR_ALREADY_SUCH_DEVINST           = CR_ALREADY_SUCH_DEVNODE,
	CR_INVALID_RANGE_LIST             = 0x00000011,
	CR_INVALID_RANGE                  = 0x00000012,
	CR_FAILURE                        = 0x00000013,
	CR_NO_SUCH_LOGICAL_DEV            = 0x00000014,
	CR_CREATE_BLOCKED                 = 0x00000015,
	CR_NOT_SYSTEM_VM                  = 0x00000016,
	CR_REMOVE_VETOED                  = 0x00000017,
	CR_APM_VETOED                     = 0x00000018,
	CR_INVALID_LOAD_TYPE              = 0x00000019,
	CR_BUFFER_SMALL                   = 0x0000001A,
	CR_NO_ARBITRATOR                  = 0x0000001B,
	CR_NO_REGISTRY_HANDLE             = 0x0000001C,
	CR_REGISTRY_ERROR                 = 0x0000001D,
	CR_INVALID_DEVICE_ID              = 0x0000001E,
	CR_INVALID_DATA                   = 0x0000001F,
	CR_INVALID_API                    = 0x00000020,
	CR_DEVLOADER_NOT_READY            = 0x00000021,
	CR_NEED_RESTART                   = 0x00000022,
	CR_NO_MORE_HW_PROFILES            = 0x00000023,
	CR_DEVICE_NOT_THERE               = 0x00000024,
	CR_NO_SUCH_VALUE                  = 0x00000025,
	CR_WRONG_TYPE                     = 0x00000026,
	CR_INVALID_PRIORITY               = 0x00000027,
	CR_NOT_DISABLEABLE                = 0x00000028,
	CR_FREE_RESOURCES                 = 0x00000029,
	CR_QUERY_VETOED                   = 0x0000002A,
	CR_CANT_SHARE_IRQ                 = 0x0000002B,
	CR_NO_DEPENDENT                   = 0x0000002C,
	CR_SAME_RESOURCES                 = 0x0000002D,
	CR_NO_SUCH_REGISTRY_KEY           = 0x0000002E,
	CR_INVALID_MACHINENAME            = 0x0000002F,
	CR_REMOTE_COMM_FAILURE            = 0x00000030,
	CR_MACHINE_UNAVAILABLE            = 0x00000031,
	CR_NO_CM_SERVICES                 = 0x00000032,
	CR_ACCESS_DENIED                  = 0x00000033,
	CR_CALL_NOT_IMPLEMENTED           = 0x00000034,
	CR_INVALID_PROPERTY               = 0x00000035,
	CR_DEVICE_INTERFACE_ACTIVE        = 0x00000036,
	CR_NO_SUCH_DEVICE_INTERFACE       = 0x00000037,
	CR_INVALID_REFERENCE_STRING       = 0x00000038,
	CR_INVALID_CONFLICT_LIST          = 0x00000039,
	CR_INVALID_INDEX                  = 0x0000003A,
	CR_INVALID_STRUCTURE_SIZE         = 0x0000003B
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_OpenSCManager (
    PVOID lpMachineName,
    PVOID lpDatabaseName,
    DWORD dwDesiredAccess
    );

ULONG WINAPI
fake_OpenServiceA (
    SC_HANDLE hSCManager,
    LPSTR lpServiceName,
    DWORD dwDesiredAccess
    );

ULONG WINAPI
fake_OpenServiceW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    DWORD dwDesiredAccess
    );

ULONG WINAPI
fake_CloseServiceHandle (
    SC_HANDLE hService
    );

ULONG WINAPI
fake_QueryServiceStatus (
    SC_HANDLE           hService,
    LPSERVICE_STATUS    lpServiceStatus
    );

ULONG WINAPI
fake_QueryServiceStatusEx (
	HANDLE hService, 
	int InfoLevel,
	LPBYTE lpBuffer, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	);

ULONG WINAPI
fake_QueryServiceConfigA (
	SC_HANDLE hService,
	LPQUERY_SERVICE_CONFIGA lpServiceConfig, 
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded
	);

ULONG WINAPI
fake_QueryServiceConfigW (
	SC_HANDLE hService,
	LPQUERY_SERVICE_CONFIGW lpServiceConfig, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	);

ULONG WINAPI
fake_QueryServiceConfig2A (
	SC_HANDLE hService, 
	DWORD dwInfoLevel, 
	LPSERVICE_FAILURE_ACTIONSA lpBuffer, 
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded
	);

ULONG WINAPI
fake_QueryServiceConfig2W (
	SC_HANDLE hService,
	int dwInfoLevel, 
	LPSERVICE_FAILURE_ACTIONSW lpBuffer,
	int cbBufSize,
	LPDWORD pcbBytesNeeded
	);

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
    );

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
    );

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
	);

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
	);

ULONG WINAPI
fake_QueryServiceLockStatus (
	SC_HANDLE hSCManager,
    int lpLockStatus,
    DWORD cbBufSize,
    LPDWORD pcbBytesNeeded
    );

ULONG WINAPI
fake_GetServiceDisplayNameA (
    SC_HANDLE hSCManager,
    LPCSTR lpServiceName,
    LPSTR lpDisplayName,
    LPDWORD lpcchBuffer
    );

ULONG WINAPI
fake_GetServiceDisplayNameW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    LPWSTR lpDisplayName,
    LPDWORD lpcchBuffer
    );

ULONG WINAPI
fake_GetServiceKeyNameA (
    SC_HANDLE hSCManager,
    LPCSTR lpDisplayName,
    LPSTR lpServiceName,
    LPDWORD lpcchBuffer
    );

ULONG WINAPI
fake_GetServiceKeyNameW (
    SC_HANDLE hSCManager,
    LPCWSTR lpDisplayName,
    LPWSTR lpServiceName,
    LPDWORD lpcchBuffer
    );

ULONG WINAPI
fake_EnumDependentServices (
	SC_HANDLE hService,
	DWORD dwServiceState, 
	int lpServices,
	DWORD cbBufSize, 
	LPDWORD pcbBytesNeeded,
	LPDWORD lpServicesReturned
	);

ULONG WINAPI
fake_QueryServiceObjectSecurity (
    SC_HANDLE               hService,
    SECURITY_INFORMATION    dwSecurityInformation,
    /*PSECURITY_DESCRIPTOR*/int    lpSecurityDescriptor,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded
    );

ULONG WINAPI
fake_SetServiceObjectSecurity (
	SC_HANDLE hService, 
	SECURITY_INFORMATION dwSecurityInformation,
	PSECURITY_DESCRIPTOR lpSecurityDescriptor
	);

ULONG WINAPI
fake_NotifyServiceStatusChange (
	SC_HANDLE hService, 
	int a2,
	int a3
	);

ULONG WINAPI
fake_LockServiceDatabase (
	SC_HANDLE hSCManager
	);

ULONG WINAPI
fake_UnlockServiceDatabase (
	SC_LOCK ScLock
	);

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
    );

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
    );

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
    );

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
    );

ULONG WINAPI
fake_ChangeServiceConfig2 (
	HANDLE hService,
	DWORD dwInfoLevel,
	LPVOID lpInfo
	);

ULONG WINAPI
fake_DeleteService (
	HANDLE hService
	);

ULONG WINAPI
fake_StartService (
	SC_HANDLE hService, 
	DWORD dwNumServiceArgs, 
	LPCWSTR *lpServiceArgVectors
	);

ULONG WINAPI
fake_StartServiceCtrlDispatcherA (
	IN LPSERVICE_TABLE_ENTRYA lpServiceStartTable
	);

ULONG WINAPI
fake_StartServiceCtrlDispatcherW (
	IN LPSERVICE_TABLE_ENTRYW lpServiceStartTable
	);

ULONG WINAPI
fake_RegisterServiceCtrlHandler (
	int lpServiceName, 
	LPHANDLER_FUNCTION lpHandlerProc
	);

ULONG WINAPI
fake_RegisterServiceCtrlHandlerEx (
	LPCWSTR lpServiceName,
	LPHANDLER_FUNCTION_EX lpHandlerProc,
	LPVOID lpContext
	);

ULONG WINAPI
fake_SetServiceStatus (
	SC_HANDLE hService,
	LPVOID lpServiceData
	);

ULONG WINAPI
fake_ControlService (
	HANDLE hService,
	DWORD dwControl,
	LPSERVICE_STATUS lpServiceStatus
	);

ULONG WINAPI
fake_RegisterEventSourceA (
	LPCSTR lpUNCServerName,
	LPCSTR lpSourceName
	);

ULONG WINAPI
fake_RegisterEventSourceW (
	LPCWSTR lpUNCServerName,
	LPCWSTR lpSourceName
	);

ULONG WINAPI
fake_DeregisterEventSource (
	int a1
	);

ULONG WINAPI fake_ReportEvent( int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9 );


///////////////////////////////   END OF FILE   ///////////////////////////////
