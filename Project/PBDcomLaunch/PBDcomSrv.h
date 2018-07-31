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


typedef enum _SERVICESTAG_ 
{
	SetServiceStatus_TAG = 0,
	StartServiceCtrlDispatcherW_TAG ,
	OpenServiceW_TAG ,
	CloseServiceHandle_TAG ,
	QueryServiceStatus_TAG ,
	QueryServiceStatusEx_TAG ,
	StartServiceW_TAG ,
	ControlService_TAG ,
	CreateFileMappingW_TAG ,

};

extern DWORD g_dwCurrentState;
extern DWORD g_dwWin32ExitCode;
extern DWORD g_dwCheckPoint;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL InitHook_pharase0_Service();

ULONG WINAPI
fake_SetServiceStatus (
	SC_HANDLE hService,
	LPSERVICE_STATUS lpServiceStatus
	);

ULONG WINAPI
fake_StartServiceCtrlDispatcherW (
	IN LPSERVICE_TABLE_ENTRYW lpServiceStartTable
	);

ULONG WINAPI
fake_OpenServiceW (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    DWORD dwDesiredAccess
    );

typedef ULONG (WINAPI* _OpenServiceW_) (
    SC_HANDLE hSCManager,
    LPCWSTR lpServiceName,
    DWORD dwDesiredAccess
    );

ULONG WINAPI
fake_CloseServiceHandle (
    SC_HANDLE hService
    );

typedef ULONG (WINAPI* _CloseServiceHandle_) (
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
	LPSERVICE_STATUS_PROCESS lpBuffer, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	);

typedef ULONG (WINAPI* _QueryServiceStatusEx_) (
	HANDLE hService, 
	int InfoLevel,
	LPSERVICE_STATUS_PROCESS lpBuffer, 
	DWORD cbBufSize,
	LPDWORD pcbBytesNeeded
	);

ULONG WINAPI
fake_StartService (
	SC_HANDLE hService, 
	DWORD dwNumServiceArgs, 
	LPCWSTR *lpServiceArgVectors
	);

typedef ULONG (WINAPI* _StartService_) (
	SC_HANDLE hService, 
	DWORD dwNumServiceArgs, 
	LPCWSTR *lpServiceArgVectors
	);

ULONG WINAPI
fake_ControlService (
	SC_HANDLE hService,
	DWORD dwControl,
	LPSERVICE_STATUS lpServiceStatus
	);

typedef ULONG (WINAPI* _ControlService_) (
	HANDLE hService,
	DWORD dwControl,
	LPSERVICE_STATUS lpServiceStatus
	);

ULONG WINAPI
fake_CreateFileMappingW (
	HANDLE hFile, 
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes, 
	int flProtect, 
	int dwMaximumSizeHigh, 
	int dwMaximumSizeLow, 
	LPCWSTR lpName
	);

typedef ULONG (WINAPI* _CreateFileMappingW_) (
	HANDLE hFile, 
	LPSECURITY_ATTRIBUTES lpFileMappingAttributes, 
	int flProtect, 
	int dwMaximumSizeHigh, 
	int dwMaximumSizeLow, 
	LPCWSTR lpName
	);

DWORD GetSpecPID(LPWSTR szPorcessName, BOOL bFlag);

///////////////////////////////   END OF FILE   ///////////////////////////////
