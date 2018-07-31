#pragma once

//////////////////////////////////////////////////////////////////////////

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PROTEINBOXDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PROTEINBOXDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef PROTEINBOXDLL_EXPORTS
#define PROTEINBOXDLL_API __declspec(dllexport)
#else
#define PROTEINBOXDLL_API __declspec(dllimport)
#endif


//////////////////////////////////////////////////////////////////////////
// Functions

#ifdef __cplusplus 
extern "C" { 
#endif

extern PROTEINBOXDLL_API int nProteinBoxDLL;

PROTEINBOXDLL_API int fnProteinBoxDLL(void);
PROTEINBOXDLL_API void Test(void);
PROTEINBOXDLL_API BOOL PB_IsWow64(void);
PROTEINBOXDLL_API BOOL PB_QueryProcess ( IN ULONG PID, OUT PVOID Data );
PROTEINBOXDLL_API BOOL PB_QueryProcessPath ( IN ULONG PID, OUT LPWSTR szPath );
PROTEINBOXDLL_API NTSTATUS PB_GetHandlePath( IN HANDLE FileHandle, IN LPWSTR outSzPath, OUT BOOL *bIsHandlerSelfFilePath );
PROTEINBOXDLL_API BOOL PB_TranslateNtToDosPath( IN LPWSTR szName );
PROTEINBOXDLL_API PVOID PB_CallServer( IN PVOID pBuffer );
PROTEINBOXDLL_API NTSTATUS PB_RenameFile( IN HANDLE hFile,IN LPWSTR szPathFather,IN LPWSTR szPathSun,IN BOOLEAN Replace );
PROTEINBOXDLL_API NTSTATUS PB_CreateDirOrLink( IN LPWSTR lpcFullPath, IN LPWSTR lpcFatherPath );
PROTEINBOXDLL_API BOOL PB_StartCOM();
PROTEINBOXDLL_API BOOL PB_IsOpenCOM();
PROTEINBOXDLL_API BOOL PB_RunFromHome(LPWSTR szProcessName, LPWSTR szCommandLine, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInfo);
PROTEINBOXDLL_API NTSTATUS PB_DuplicateObject( PHANDLE TargetHandle, HANDLE FuckedHandle, HANDLE SourceHandle, ACCESS_MASK DesiredAccess, ULONG_PTR Options );
PROTEINBOXDLL_API void PB_DeviceChange ( int wParam, int lParam );	
PROTEINBOXDLL_API int PB_StartBoxedService( LPWSTR ServiceName );
PROTEINBOXDLL_API BOOL PB_CanElevateOnVista();
PROTEINBOXDLL_API BOOL PB_IsOpenClsid ( LPCLSID,DWORD,BOOL );
PROTEINBOXDLL_API BOOL PB_IsDirectory ( LPCWSTR );
PROTEINBOXDLL_API NTSTATUS PB_MonitorPut( IN ULONG Flag, IN LPWSTR szInfo );
PROTEINBOXDLL_API void PB_Log( IN LPCWSTR szInfo );
PROTEINBOXDLL_API NTSTATUS PB_GetFileName( IN HANDLE HandleValue, IN ULONG BufferLength, IN LPWSTR lpData );
PROTEINBOXDLL_API NTSTATUS PB_EnumProcessEx( IN LPWSTR szBoxName, OUT int* pArrays );
PROTEINBOXDLL_API LPWSTR PB_AssocQueryProgram( IN LPWSTR lpValueName );
PROTEINBOXDLL_API BOOL PB_QueryBoxPath( IN LPWSTR szBoxName, OUT LPWSTR szBoxPath );
PROTEINBOXDLL_API BOOL PB_EnumBoxes( IN PVOID pData );
PROTEINBOXDLL_API BOOL PB_DisableForceProcess( IN int Flag, IN LPWSTR szForcePath );
PROTEINBOXDLL_API BOOL PB_KillAll( IN int PID, IN LPWSTR szBoxName );
PROTEINBOXDLL_API BOOL PB_KillOne( IN int PID );
PROTEINBOXDLL_API BOOL PB_GetVersion();
PROTEINBOXDLL_API BOOL PB_StartPBSvc( IN BOOL bRetry );
PROTEINBOXDLL_API BOOL PB_StartPBDrv( IN BOOL bWait );
PROTEINBOXDLL_API BOOL PB_StopPBDrv();
PROTEINBOXDLL_API BOOL PB_QueryConf( IN LPWSTR section, IN LPWSTR key, IN ULONG MaxLength, OUT PVOID pBuffer );
PROTEINBOXDLL_API BOOL PB_ReloadConf();
PROTEINBOXDLL_API VOID PB_FreeReply( IN PVOID pBuffer );
PROTEINBOXDLL_API NTSTATUS PB_StartProcess( IN HANDLE new_hToken, IN HANDLE new_ImpersonationToken, OUT BOOL* pbDriverNotLoad );
PROTEINBOXDLL_API BOOL PB_InitProcess();
PROTEINBOXDLL_API BOOL PB_HookShadow( IN BOOL bHook );
PROTEINBOXDLL_API BOOL PB_HookObject( IN BOOL bHook );
PROTEINBOXDLL_API BOOL PB_IsBoxedService( IN SC_HANDLE hService );


#ifdef __cplusplus 
} 
#endif

///////////////////////////////   END OF FILE   ///////////////////////////////
