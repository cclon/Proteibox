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


typedef enum _TOKENTAG_ 
{
	RtlAdjustPrivilege_TAG = 0,
	NtSetInformationProcess_TAG ,
	AccessCheckByType_TAG ,
	SetThreadToken_TAG ,
	GetTokenInformation_TAG ,

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


BOOL InitHook_pharase1_Token();

ULONG WINAPI
fake_RtlAdjustPrivilege(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    );

typedef ULONG (WINAPI* _RtlAdjustPrivilege_) (
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    );

ULONG WINAPI
fake_NtSetInformationProcess (
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
    );

typedef ULONG (WINAPI* _NtSetInformationProcess_) (
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength
	);

ULONG WINAPI
fake_AccessCheckByType(
	PSECURITY_DESCRIPTOR pSecurityDescriptor,
	PSID PrincipalSelfSid,
	HANDLE ClientToken,
	DWORD DesiredAccess,
	POBJECT_TYPE_LIST ObjectTypeList,
	DWORD ObjectTypeListLength,
	PGENERIC_MAPPING GenericMapping,
	PPRIVILEGE_SET PrivilegeSet,
	LPDWORD PrivilegeSetLength,
	LPDWORD GrantedAccess,
	LPBOOL AccessStatus
	);

ULONG WINAPI
fake_SetThreadToken (
	PHANDLE ThreadHandle, 
	LPVOID TokenHandle
	);

typedef ULONG (WINAPI* _SetThreadToken_) (
	PHANDLE ThreadHandle, 
	LPVOID TokenHandle
	);

ULONG WINAPI
fake_GetTokenInformation (
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	LPVOID TokenInformation,
	DWORD TokenInformationLength,
	PDWORD ReturnLength
	);

typedef ULONG (WINAPI* _GetTokenInformation_) (
	HANDLE TokenHandle,
	TOKEN_INFORMATION_CLASS TokenInformationClass,
	LPVOID TokenInformation,
	DWORD TokenInformationLength,
	PDWORD ReturnLength
	);

extern _SetThreadToken_ g_SetThreadToken_addr;


///////////////////////////////   END OF FILE   ///////////////////////////////
