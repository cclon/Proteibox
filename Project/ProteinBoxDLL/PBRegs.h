#pragma once

//////////////////////////////////////////////////////////////////////////

#define GetRegFunc( X )		g_HookInfo.REGS.pArray[ X##_TAG ].OrignalAddress

// fake_NtLoadKey()
typedef struct _RPC_IN_NtLoadKey_	// size - 0x208
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  WCHAR szOrignalPath[0x80] ;
/*0x108 */  WCHAR szRedirectedPath[0x80] ;

} RPC_IN_NtLoadKey, *LPRPC_IN_NtLoadKey ;

extern BOOL g_Is_NtSetValueKey_Ready;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         º¯ÊýÔ¤¶¨Òå                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG WINAPI
fake_NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    IN PULONG Disposition OPTIONAL
	);

ULONG WINAPI
fake_NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
	);

typedef ULONG (WINAPI* _NtOpenKeyEx_)(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG Reserved
	);

ULONG WINAPI
fake_NtOpenKeyEx(
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG Reserved
	);

ULONG WINAPI
fake_NtDeleteKey(
    IN HANDLE KeyHandle
	);

ULONG WINAPI
fake_NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
	);

ULONG WINAPI
fake_NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
	);

ULONG WINAPI
fake_NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	);

ULONG WINAPI
fake_NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	);

ULONG WINAPI
fake_NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN OUT PKEY_VALUE_ENTRY ValueList,
    IN ULONG NumberOfValues,
    OUT PVOID Buffer,
    IN OUT PULONG Length,
    OUT PULONG ReturnLength
	);

ULONG WINAPI
fake_NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	);

ULONG WINAPI
fake_NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	);

ULONG WINAPI
fake_NtNotifyChangeKey(
    IN HANDLE KeyHandle,
    IN HANDLE Event,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN Asynchroneous,
    OUT PVOID ChangeBuffer,
    IN ULONG Length,
    IN BOOLEAN WatchSubtree
	);

ULONG WINAPI
fake_NtNotifyChangeMultipleKeys (
    IN HANDLE MasterKeyHandle,
    IN ULONG Count,
    IN POBJECT_ATTRIBUTES SlaveObjects,
    IN HANDLE Event,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN Asynchronous
	);

ULONG WINAPI
fake_NtSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
	);

ULONG WINAPI
fake_NtLoadKey(
    IN POBJECT_ATTRIBUTES KeyObjectAttributes,
    IN POBJECT_ATTRIBUTES FileObjectAttributes
	);

ULONG WINAPI
fake_NtRenameKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ReplacementName
	);

///////////////////////////////   END OF FILE   ///////////////////////////////