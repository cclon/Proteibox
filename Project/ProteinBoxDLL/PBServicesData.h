#pragma once

//////////////////////////////////////////////////////////////////////////

extern HANDLE g_CurrentService_KeyHandle ;
extern HANDLE g_hEvent_PB_WindowsInstallerInUse;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



typedef enum _SERVICESTAG_ 
{
	OpenSCManagerA_TAG = 0,
	OpenSCManagerW_TAG ,
	OpenServiceA_TAG ,
	OpenServiceW_TAG ,
	CloseServiceHandle_TAG ,
	QueryServiceStatus_TAG ,
	QueryServiceStatusEx_TAG ,
	QueryServiceConfigA_TAG ,
	QueryServiceConfigW_TAG ,
	QueryServiceConfig2A_TAG ,
	QueryServiceConfig2W_TAG ,
	EnumServicesStatusA_TAG ,
	EnumServicesStatusW_TAG ,
	EnumServicesStatusExA_TAG ,
	EnumServicesStatusExW_TAG ,
	QueryServiceLockStatusA_TAG ,
	QueryServiceLockStatusW_TAG ,
	GetServiceDisplayNameA_TAG ,
	GetServiceDisplayNameW_TAG ,
	GetServiceKeyNameA_TAG ,
	GetServiceKeyNameW_TAG ,
	EnumDependentServicesA_TAG ,
	EnumDependentServicesW_TAG ,
	QueryServiceObjectSecurity_TAG ,
	SetServiceObjectSecurity_TAG ,
	LockServiceDatabase_TAG ,
	UnlockServiceDatabase_TAG ,
	CreateServiceA_TAG ,
	CreateServiceW_TAG ,
	ChangeServiceConfigA_TAG ,
	ChangeServiceConfigW_TAG ,
	ChangeServiceConfig2A_TAG ,
	ChangeServiceConfig2W_TAG ,
	DeleteService_TAG ,
	StartServiceA_TAG ,
	StartServiceW_TAG ,
	StartServiceCtrlDispatcherA_TAG ,
	StartServiceCtrlDispatcherW_TAG ,
	RegisterServiceCtrlHandlerA_TAG ,
	RegisterServiceCtrlHandlerW_TAG ,
	RegisterServiceCtrlHandlerExA_TAG ,
	RegisterServiceCtrlHandlerExW_TAG ,
	SetServiceStatus_TAG ,
	ControlService_TAG ,
	ControlServiceExW_TAG ,
	ControlServiceExA_TAG ,
	RegisterEventSourceA_TAG ,
	RegisterEventSourceW_TAG ,
	DeregisterEventSource_TAG ,
	ReportEventA_TAG ,
	ReportEventW_TAG ,
	NotifyServiceStatusChange_TAG ,

};


typedef struct _RPC_IN_EnumServiceStatus_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  short Flag1 ;
/*0x00A*/   short dwInfoLevel ; 
/*0x00C*/	ULONG NameLength ; 
/*0x010*/   WCHAR wszServiceName[ 0x200 ] ; 

} RPC_IN_EnumServiceStatus, *LPRPC_IN_EnumServiceStatus ;


// FuckService()
typedef struct _FuckService_Info_ 
{
/*0x000*/ ULONG StructLength ;
/*0x004*/ ULONG Reserved1 ;
/*0x008*/ ULONG ServiceType ;
/*0x00C*/ ULONG SBIE_CurrentState ;
/*0x010*/ ULONG SBIE_ControlsAccepted ;
/*0x014*/ ULONG SBIE_Win32ExitCode ;
/*0x018*/ ULONG SBIE_ServiceSpecificExitCode ;
/*0x01C*/ ULONG SBIE_CheckPoint ;
/*0x020*/ ULONG SBIE_WaitHint ;
/*0x024*/ ULONG SBIE_ProcessId ;

/*0x028*/ ULONG Reserved2 ;
/*0x02C*/ ULONG NeedSize ;
/*0x030*/ ULONG Separatrix ; // 分界线
/*0x034*/ ULONG ServiceStart ;
/*0x038*/ ULONG ServiceErrorControl ;
/*0x03C*/ ULONG szImagePathOffset ;
/*0x040*/ ULONG Reserved4 ;
/*0x044*/ ULONG Reserved5 ;
/*0x048*/ ULONG Reserved6 ;
/*0x04C*/ ULONG szObjectNameOffset ;
/*0x050*/ ULONG szDisplayNameOffset ;
/*0x054*/ WORD Reserved7 ;
/*0x056*/ WCHAR Buffer[1] ;

} FuckService_Info, *LPFuckService_Info ;


typedef struct _SerArrayInfo_ 
{
/*0x000*/ LPWSTR ServiceName ;
/*0x004*/ LPWSTR ProcessName ;

} SerArrayInfo, *LPSerArrayInfo ;


// PB_StartBoxedService()
typedef struct _RPC_IN_StartBoxedService_ 
{
/*0x000*/  RPC_IN_HEADER RpcHeader ;
/*0x008*/  ULONG	NameLength ;
/*0x00C*/  LPWSTR   szName ;

} RPC_IN_StartBoxedService, *LPRPC_IN_StartBoxedService ;


typedef struct _StartServiceCtrlDispatcherThreadInfo_ 
{
/*0x000*/ PVOID lpServiceProc;
/*0x004*/ PVOID lpServiceName;
/*0x008*/ ULONG Reserved ;

} StartServiceCtrlDispatcherThreadInfo, *LPStartServiceCtrlDispatcherThreadInfo ;


typedef struct _SERVICE_DATA_ // size - 0x01C
{  
/*0x000 */  DWORD SBIE_ProcessId ;
/*0x004 */  DWORD SBIE_CurrentState;
/*0x008 */  DWORD SBIE_ControlsAccepted ;
/*0x00C */  DWORD SBIE_Win32ExitCode ;
/*0x010 */  DWORD SBIE_ServiceSpecificExitCode ;
/*0x014 */  DWORD SBIE_CheckPoint ;
/*0x018 */  DWORD SBIE_WaitHint ; 

} SERVICE_DATA, *PSERVICE_DATA ;


// GetSandboxedServices()
typedef struct _RPC_IN_GetSandboxedServices_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD dwServiceType;
/*0x00C*/ DWORD dwServiceState;

} RPC_IN_GetSandboxedServices, *LPRPC_IN_GetSandboxedServices ;


typedef struct _RPC_OUT_GetSandboxedServices_ 
{
/*0x000 */  RPC_OUT_HEADER RpcHeader ;
/*0x008 */  WCHAR	szPath[1] ;

} RPC_OUT_GetSandboxedServices, *LPRPC_OUT_GetSandboxedServices ;


extern LPHANDLER_FUNCTION g_RegisterServiceCtrlHandler_lpHandlerProc_addr ;
extern LPHANDLER_FUNCTION_EX g_RegisterServiceCtrlHandlerEx_lpHandlerProc ;
extern LPVOID g_RegisterServiceCtrlHandlerEx_lpContext ;




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL HandlerServices( IN HMODULE hModule );

void RemoveRegNode__service( IN LPWSTR ServiceName );

BOOL Is_white_service( IN LPWSTR szServiceName );

int 
Get_serviceStatus_rpc (
	IN LPWSTR szServiceName,
	IN int dwInfoLevel,
	IN int Flag1
	);

LPWSTR Get_service_name_from_handle ( IN HANDLE hService );

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
	);

HANDLE 
Open_Services_RegKey (
	IN LPWSTR ServiceName,
	IN BOOL bCreateNewKey
	);

PVOID Get_RegValueData_SbieSvc_SandboxedServices();

int 
FuckService (
	IN LPWSTR ServiceName,
	IN int pid,
	IN int Flag
	);

ULONG
Set_service_data (
	IN SC_HANDLE hService,
	IN PSERVICE_DATA lpServiceData,
	IN HANDLE KeyHandle
	);

int
StartServiceCtrlDispatcher_filter (
	IN BOOL bUniCode,
	IN int lpServiceStartTable
	);

LPWSTR GetSandboxedServices();

int 
StartServiceCtrlDispatcherThread (
	IN LPStartServiceCtrlDispatcherThreadInfo lpServiceStartTable
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
