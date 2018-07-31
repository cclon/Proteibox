#pragma once

//////////////////////////////////////////////////////////////////////////

#define GetFileFunc( X )		g_HookInfo.FILES.pArray[ X##_TAG ].OrignalAddress


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
fake_NtCreateFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
	);

ULONG WINAPI
fake_NtOpenFile (
	OUT PHANDLE FileHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG ShareAccess,
	IN ULONG OpenOptions
	);

ULONG WINAPI
fake_NtQueryAttributesFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
	);

ULONG WINAPI
fake_NtQueryFullAttributesFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
	);

ULONG WINAPI
fake_NtQueryInformationFile (
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
	);

ULONG WINAPI
fake_NtQueryDirectoryFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
	);

ULONG WINAPI
fake_NtSetInformationFile (
    IN HANDLE FileHandle,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
	);

ULONG WINAPI
fake_NtDeleteFile (
    IN POBJECT_ATTRIBUTES ObjectAttributes
	);

ULONG WINAPI
fake_NtClose (
	IN HANDLE  Handle
	);

ULONG WINAPI
fake_NtCreateNamedPipeFile (
    OUT PHANDLE NamedPipeFileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG WriteModeMessage,
    IN ULONG ReadModeMessage,
    IN ULONG NonBlocking,
    IN ULONG MaxInstances,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PLARGE_INTEGER DefaultTimeOut
	);

ULONG WINAPI
fake_NtCreateMailslotFile (
    OUT PHANDLE MailSlotFileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG MaxMessageSize,
    IN PLARGE_INTEGER TimeOut
	);

ULONG WINAPI
fake_NtReadFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL,
    IN PVOID UserApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
	);

ULONG WINAPI
fake_NtWriteFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset,
    IN PULONG Key OPTIONAL
	);

ULONG WINAPI
fake_NtFsControlFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer,
    IN ULONG InputBufferSize,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferSize
	);

ULONG WINAPI
fake_NtQueryVolumeInformationFile (
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
	);

ULONG WINAPI
fake_WaitNamedPipeW (
	IN LPCWSTR lpNamedPipeName,
	IN DWORD nTimeOut
	);

ULONG WINAPI
fake_RtlGetCurrentDirectory_U (
    ULONG MaximumLength,
    PWSTR Buffer
	);

ULONG WINAPI
fake_RtlSetCurrentDirectory_U (
    IN PUNICODE_STRING name
	);

ULONG WINAPI
fake_RtlGetFullPathName_U (
    IN PCWSTR FileName,
    IN ULONG Size,
    IN PWSTR Buffer,
    OUT PWSTR *ShortName
	);

ULONG WINAPI
fake_MoveFileWithProgressW (
	LPCWSTR			lpExistingFileName,
	LPCWSTR			lpNewFileName,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	DWORD			dwFlags
	);


///////////////////////////////   END OF FILE   ///////////////////////////////