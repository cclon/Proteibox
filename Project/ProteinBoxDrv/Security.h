#pragma once

//////////////////////////////////////////////////////////////////////////

extern PACL g_DefaultDacl_new ;
extern PSECURITY_DESCRIPTOR g_SecurityDescriptor_new ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef struct _PRIVILEGE_SET_EX 
{
	ULONG PrivilegeCount ;
	LUID_AND_ATTRIBUTES Privilege[ ANYSIZE_ARRAY ] ;

} PRIVILEGE_SET_EX, *LPPRIVILEGE_SET_EX ;


//
// 黑名单权限对应的结构体
//

typedef struct _PRIVILEGE_BLACKLIST_ 
{
/*0x000 */ ULONG BlackListPrivilegeSetSize ;
/*0x004 */ LPPRIVILEGE_SET_EX BlackListPrivilege ;

} PRIVILEGE_BLACKLIST, *LPPRIVILEGE_BLACKLIST ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
DropAdminRights (
	IN PVOID _pNode
	);

HANDLE
DropAdminRights_phrase1 (
	IN HANDLE TokenHandle, 
	IN BOOL bDropAdminRights
	);

BOOL
DropAdminRights_phrase2 (
	IN PVOID LocalPrivileges, 
	IN PVOID LocalGroups, 
	IN BOOL bDropAdminRights
	);

BOOL
QueryInformationToken(
	IN HANDLE TokenHandle,
	IN int TokenInformationClass,
	OUT PVOID* out_TokenInformation,
	OUT PULONG out_TokenInformationLength OPTIONAL
	);

NTSTATUS
SetSecurityObject (
	IN HANDLE Handle,
	IN PACL Dacl
	);

BOOL
SepVariableInitialization(
	);

//////////////////////////////////////////////////////////////////////////

BOOL
CreateAcl(
	);

BOOL
InitializeAcl (
	IN PACL		pAcl,
	IN DWORD	nAclLength,
	IN DWORD	dwAclRevision
	);

BOOL
InitializeSid (
	);

BOOL
__InitializeSid (
	IN PSID Sid,
	IN PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
	IN BYTE SubAuthorityCount,
	IN ULONG SubAuthority
	);

NTSTATUS
Call_RtlAddAccessAllowedAce (
	IN OUT PACL Acl,
	IN PSID Sid
	);

BOOL
AdjustPrivilege (
	);

NTSTATUS 
RtlGetUserSid(
	IN HANDLE ProcessHandle,
	OUT LPWSTR RegisterUserID
	);

BOOL 
ProcessIdToSessionId(
	IN DWORD dwProcessId OPTIONAL,
	OUT PULONG pSessionId
	);

NTSTATUS
Reduce_TokenPrivilegeGroups (
	IN PVOID _pNode,
	IN HANDLE Token,
	IN ACCESS_MASK GrantedAccess
	);

///////////////////////////////   END OF FILE   ///////////////////////////////