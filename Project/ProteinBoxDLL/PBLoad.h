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


#define GetLoadFunc( X )		g_HookInfo.LOAD.pArray[ X##_TAG ].OrignalAddress


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef ULONG (WINAPI* _LdrLoadDll_) (
	IN PWSTR DllPath OPTIONAL,
	IN PULONG DllCharacteristics OPTIONAL,
	IN PUNICODE_STRING DllName,
	OUT PVOID *DllHandle
	);

ULONG WINAPI
fake_LdrLoadDll (
    IN PWSTR SearchPath OPTIONAL,
    IN PULONG LoadFlags OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *BaseAddress
    );


typedef ULONG (WINAPI* _LdrUnloadDll_) (
	IN PVOID BaseAddress
	);

ULONG WINAPI
fake_LdrUnloadDll (
	IN PVOID BaseAddress
	);


typedef ULONG (WINAPI* _NtLoadDriver_) (
    IN PUNICODE_STRING DriverServiceName
    );

ULONG WINAPI
fake_NtLoadDriver (
    IN PUNICODE_STRING DriverServiceName
    );


typedef ULONG (WINAPI* _GetModuleFileNameW_) (
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    );

ULONG WINAPI
fake_GetModuleFileNameW (
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    );

VOID fake_AddressOfEntryPoint();


//////////////////////////////////////////////////////////////////////////

typedef ULONG (WINAPI* _SetConsoleTitleA_) (
	LPCSTR lpConsoleTitle
	);

ULONG WINAPI
fake_SetConsoleTitleA (
    LPCSTR lpConsoleTitle
    );


typedef ULONG (WINAPI* _SetConsoleTitleW_) (
	LPCWSTR lpConsoleTitle
	);

ULONG WINAPI
fake_SetConsoleTitleW (
    LPCWSTR lpConsoleTitle
    );


typedef ULONG (WINAPI* _GetConsoleTitleA_) (
	LPSTR lpConsoleTitle,
	DWORD nSize
	);

ULONG WINAPI
fake_GetConsoleTitleA (
    LPSTR lpConsoleTitle,
    DWORD nSize
    );


typedef ULONG (WINAPI* _GetConsoleTitleW_) (
	LPWSTR lpConsoleTitle,
	DWORD nSize
	);

ULONG WINAPI
fake_GetConsoleTitleW (
    LPWSTR lpConsoleTitle,
    DWORD nSize
    );

//////////////////////////////////////////////////////////////////////////

typedef ULONG (WINAPI* _QueryActCtxSettingsW_) (
	IN DWORD dwFlags,
	IN HANDLE hActCtx,
	IN PCWSTR settingsNameSpace,
	IN PCWSTR settingName,
	OUT PWSTR pvBuffer,
	IN SIZE_T dwBuffer,
	OUT SIZE_T *pdwWrittenOrRequired
	);

extern _QueryActCtxSettingsW_	g_QueryActCtxSettingsW_addr ;

ULONG WINAPI
fake_QueryActCtxSettingsW (
	IN DWORD dwFlags,
	IN HANDLE hActCtx,
	IN PCWSTR settingsNameSpace,
	IN PCWSTR settingName,
	OUT PWSTR pvBuffer,
	IN SIZE_T dwBuffer,
	OUT SIZE_T *pdwWrittenOrRequired
	);



///////////////////////////////   END OF FILE   ///////////////////////////////
