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




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



typedef ULONG (WINAPI* _CreateProcessAsUserA_) (
	HANDLE  hToken,
	LPCSTR lpApplicationName,
	LPSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCSTR lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);

extern _CreateProcessAsUserA_ g_CreateProcessAsUserA_addr ;

ULONG WINAPI
fake_CreateProcessAsUserA (
	  HANDLE  hToken,
	  LPCSTR lpApplicationName,
	  LPSTR lpCommandLine,
	  LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  BOOL bInheritHandles,
	  DWORD dwCreationFlags,
	  LPVOID lpEnvironment,
	  LPCSTR lpCurrentDirectory,
	  LPSTARTUPINFOA lpStartupInfo,
	  LPPROCESS_INFORMATION lpProcessInformation
	  );


typedef ULONG (WINAPI* _CreateProcessAsUserW_) (
	HANDLE  hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);

extern _CreateProcessAsUserW_ g_CreateProcessAsUserW_addr ;

ULONG WINAPI
fake_CreateProcessAsUserW (
	HANDLE  hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	);


typedef BOOL (WINAPI* _CreateProcessW_) (
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

extern _CreateProcessW_ g_CreateProcessW_addr ;

ULONG WINAPI
fake_CreateProcessW(
	LPWSTR lpApplicationName, 
	LPWSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles, 
	DWORD dwCreationFlags, 
	LPVOID lpEnvironment, 
	LPCWSTR lpCurrentDirectory, 
	LPSTARTUPINFOW lpStartupInfo, 
	LPPROCESS_INFORMATION lpProcessInformation
	);


typedef BOOL (WINAPI* _CreateProcessA_) (LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,PVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION);
extern _CreateProcessA_ g_CreateProcessA_addr ;

ULONG WINAPI 
fake_CreateProcessA (
	LPCSTR lpApplicationName, 
	LPSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles, 
	DWORD dwCreationFlags, 
	LPVOID lpEnvironment, 
	LPCSTR lpCurrentDirectory, 
	LPSTARTUPINFOA lpStartupInfo, 
	LPPROCESS_INFORMATION lpProcessInformation
	);


typedef NTSTATUS (WINAPI* _RtlCreateProcessParameters_)(
	OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
	IN PUNICODE_STRING ImagePathName,
	IN PUNICODE_STRING DllPath OPTIONAL,
	IN PUNICODE_STRING CurrentDirectory OPTIONAL,
	IN PUNICODE_STRING CommandLine OPTIONAL,
	IN PVOID Environment OPTIONAL,
	IN PUNICODE_STRING WindowTitle OPTIONAL,
	IN PUNICODE_STRING DesktopInfo OPTIONAL,
	IN PUNICODE_STRING ShellInfo OPTIONAL,
	IN PUNICODE_STRING RuntimeData OPTIONAL
	);

extern _RtlCreateProcessParameters_	g_RtlCreateProcessParameters_addr ;

ULONG WINAPI 
fake_RtlCreateProcessParameters (
    OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL
    );


typedef NTSTATUS (WINAPI* _RtlCreateProcessParametersEx_)(
	OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
	IN PUNICODE_STRING ImagePathName,
	IN PUNICODE_STRING DllPath OPTIONAL,
	IN PUNICODE_STRING CurrentDirectory OPTIONAL,
	IN PUNICODE_STRING CommandLine OPTIONAL,
	IN PVOID Environment OPTIONAL,
	IN PUNICODE_STRING WindowTitle OPTIONAL,
	IN PUNICODE_STRING DesktopInfo OPTIONAL,
	IN PUNICODE_STRING ShellInfo OPTIONAL,
	IN PUNICODE_STRING RuntimeData OPTIONAL,
	IN ULONG Reserved
	);

extern _RtlCreateProcessParametersEx_	g_RtlCreateProcessParametersEx_addr ;

ULONG WINAPI 
fake_RtlCreateProcessParametersEx (
    OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL,
	IN ULONG Reserved
    );

typedef VOID (WINAPI* _ExitProcess_)( UINT );
extern _ExitProcess_ g_ExitProcess_addr ;

ULONG WINAPI 
fake_ExitProcess(
    UINT uExitCode
    );


typedef UINT (WINAPI* _WinExec_)( LPCSTR, UINT );
extern _WinExec_ g_WinExec_addr ;

ULONG WINAPI 
fake_WinExec(
    LPSTR lpCmdLine,
    UINT uCmdShow
    );


NTSTATUS 
RtlCreateProcessParameters_Filter (
	PVOID *ProcessParameters,
	PUNICODE_STRING ImagePathName, 
	PUNICODE_STRING DllPath, 
	PUNICODE_STRING CurrentDirectory, 
	PUNICODE_STRING CommandLine, 
	PVOID Environment, 
	PUNICODE_STRING WindowTitle, 
	PUNICODE_STRING DesktopInfo, 
	PUNICODE_STRING ShellInfo, 
	PUNICODE_STRING RuntimeData, 
	int a11, 
	BOOL bIsCallEx
	);

VOID 
FixupProcessNodeL (
	OUT LPUNICODE_STRING_EX* _pNode,
	IN PUNICODE_STRING puniBuffer 
	);

VOID
FixupProcessNodeEnv (
	OUT LPUNICODE_STRING_EX* _pNode, 
	IN PVOID lpBuffer
	);

BOOL Hook_CreateProcessAsUser( IN HMODULE hModule );

VOID FixupProcessNode( PVOID _pNode );

LPWSTR 
GetProcFullPathL( 
	IN LPCWSTR lpFileName, 
	IN BOOLEAN bFlag_TranslateNtToDosPath 
	);

LPWSTR GetProcFullPathK( LPWSTR lpFileName );

LPWSTR
GetRedirectedSearchPath (
	PVOID _pNode, 
	LPWSTR lpBuffer,
	BOOL bFlag
	);

ULONG 
GetRedirectedSearchPathEx (
	int* pArray, 
	LPWSTR lpFileName
	);

LPWSTR 
SetSBEnv (
	PVOID _pNode, 
	LPWSTR lpEnvironment 
	);

LPWSTR 
SetSBEnvEx (
	int Length, 
	LPWSTR *pOutBuffer
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
