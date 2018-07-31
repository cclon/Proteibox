#pragma once

//////////////////////////////////////////////////////////////////////////

extern CRITICAL_SECTION g_Lock_DirHandle ;
extern CRITICAL_SECTION g_Lock_RtlGetCurrentDirectory ;
extern CRITICAL_SECTION g_HandlesLock ;
extern BOOL g_bFlag_GetVolumePathNamesForVolumeNameW_is_Null ;
extern PVOID g_SymbolicLinkObjectName_Array ;
extern ULONG g_Redirectd_FileHandle_Array ;
extern ULONG g_CopyLimitKb ;
extern LIST_ENTRY_EX g_pNodeHead_DirHandle ;
extern LIST_ENTRY_EX g_pNodeHead_Handles ;
extern LONG g_FileHandle_Arrays[ 0x100 ] ;
extern char g_bFlag_getinfo_c_windows_winsxs_lastupdatetime__ok ;
extern ULONG g_lpszEnv_00000000_SBIE_ALL_USERS_Length ;
extern ULONG g_lpszEnv_00000000_SBIE_CURRENT_USER_Length ;
extern LPWSTR g_lpszEnv_00000000_SBIE_ALL_USERS ;
extern LPWSTR g_lpszEnv_00000000_SBIE_CURRENT_USER ;

typedef struct _APCINFO_ 
{
/*0x000*/ PIO_APC_ROUTINE ApcRoutine ;
/*0x004*/ PVOID ApcContext ;
/*0x008*/ PIO_STATUS_BLOCK IoStatusBlock ;

} APCINFO, *LPAPCINFO ;


typedef struct _FILE_COM_DIR_INFORMATION {
/*0x000*/ FILE_BOTH_DIR_INFORMATION FileBothDirInfo ;
/*0x060*/ ULONG Reserved1 ;
/*0x064*/ ULONG Reserved2 ;
/*0x068*/ WCHAR FileName[1];

} FILE_COM_DIR_INFORMATION, *LPFILE_COM_DIR_INFORMATION;


typedef struct _DIR_HANDLE_INFO_LITTLEEX_ 
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ ULONG StructLength ;
/*0x00C*/ UNICODE_STRING SearchName ;
/*0x014*/ ULONG Reserved ;
/*0x018*/ FILE_ID_BOTH_DIR_INFORMATION FileDirInfo ;

} DIR_HANDLE_INFO_LITTLEEX, *LPDIR_HANDLE_INFO_LITTLEEX ;


typedef struct _DIR_HANDLE_INFO_LITTLE_  // sizeof - 0x30
{
/*0x000*/ HANDLE hFile ;
/*0x004*/ LPFILE_COM_DIR_INFORMATION FileInfo ;
/*0x008*/ ULONG FileInfoLength ;
/*0x00C*/ LPWSTR wszFileName ;
/*0x010*/ ULONG FileNameLength ;
/*0x014*/ UNICODE_STRING uniFileName ;

/*0x01C*/ BOOLEAN bFlag_dont_need_to_get_nextFileInfo ;
/*0x01D*/ BOOLEAN bFlag2 ;
/*0x01E*/ BOOLEAN bFlag_RestartScan ;
/*0x01F*/ BOOLEAN bFlag_Is_FileBothDirectoryInformation ;

/*0x020*/ BOOL FuckedFlag ;
/*0x024*/ LIST_ENTRY_EX ListEntry ;

} DIR_HANDLE_INFO_LITTLE, *LPDIR_HANDLE_INFO_LITTLE ;


typedef struct _DIR_HANDLE_INFO_ 
{
/*0x000*/ LIST_ENTRY ListEntry ;
/*0x008*/ HANDLE FileHandle ;

/*0x00C*/ BOOLEAN bFlag1 ;
/*0x00D*/ BOOLEAN bFlag2 ;
/*0x00E*/ BOOLEAN bFlag3 ;
/*0x00F*/ BOOLEAN bFlag4 ;

/*0x010*/ UNICODE_STRING SearchName ;
/*0x018*/ DIR_HANDLE_INFO_LITTLE OrignalDataInfo ;
/*0x048*/ DIR_HANDLE_INFO_LITTLE RedirectedDataInfo ;
/*0x078*/ ULONG OrignalPathLength ;
/*0x07C*/ WCHAR OrignalPath[1] ;

} DIR_HANDLE_INFO, *LPDIR_HANDLE_INFO ;


typedef struct _VOLUME_INFO_ 
{
/*0x000*/ BOOL bInited ;
/*0x004*/ LIST_ENTRY_EX NodeHead ;

} VOLUME_INFO, *LPVOLUME_INFO ;

extern VOLUME_INFO g_pNodeHead_xx ;


typedef struct _VOLUME_INFO_LITTLE_ 
{
/*0x000*/ int VolumeNumber ;
/*0x004*/ ULONG VolumePathLength ;
/*0x008*/ WCHAR VolumePath[1] ;

} VOLUME_INFO_LITTLE, *LPVOLUME_INFO_LITTLE ;


typedef struct _VOLUME_ 
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ ULONG FileNameLength ;
/*0x00C*/ ULONG DirectoryNameLength ;
/*0x010*/ LPWSTR DirectoryNameAddr ;
/*0x014*/ WCHAR Buffer[1] ; // 包含FileName 和 DirectoryName

} VOLUME, *LPVOLUME ;


typedef enum _Name_Type_Class_ 
{
	Is_Error_Type           = 0,
	Is_Pipe_Type            = 1, // eg:"\\device\\namedpipe\\", "\\device\\lanmanredirector\\PIPE\\", "\\device\\mup\\;lanmanredirector\\PIPE\\", // "\\device\\mup\\PIPE\\"
	Is_Mailslot_Type        = 2
};

// Subroutines for dealing with the Registry
#define RTL_REGISTRY_WINDOWS_NT 3 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004 
#define RTL_QUERY_REGISTRY_DIRECT 0x00000020 


// call Imitate_Pipe_Communication();
typedef struct _RPC_IN_Imitate_Pipe_Communication_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG CreateOptions ;
/*0x00C */  WCHAR szName[0x40] ;
			WCHAR PipeName[0x30];

} RPC_IN_Imitate_Pipe_Communication, *LPRPC_IN_Imitate_Pipe_Communication ;


typedef struct _RPC_OUT_Imitate_Pipe_Communication_ 
{
/*0x000 */  ULONG ReturnLength ;
/*0x004 */  NTSTATUS Result ;
/*0x008 */  HANDLE RedirectedFileHandle ;
/*0x00C */	ULONG Reserved1 ;
/*0x010 */	PVOID IoStatusBlock_Pointer ;
/*0x014 */	ULONG Reserved2 ;
/*0x018 */	ULONG IoStatusBlock_Information ;

} RPC_OUT_Imitate_Pipe_Communication, *LPRPC_OUT_Imitate_Pipe_Communication ;


// call CreateRedirectedDirectorysEx();
typedef struct _RPC_IN_CreateRedirectedDirectorysEx_ {

/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG ShortNameLength ;
/*0x00C */  WCHAR ShortName[0xC] ;
/*0x024 */	ULONG RedirectedPathLength ;
/*0x028 */	WCHAR RedirectedPath[1] ;

} RPC_IN_CreateRedirectedDirectorysEx, *LPRPC_IN_CreateRedirectedDirectorysEx ;


// call MarkFileTime();
typedef struct _RPC_IN_MarkFileTime_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  FILE_BASIC_INFORMATION FileBasicInfo ;
/*0x02C */	ULONG Reserved ;
/*0x030 */	ULONG RedirectedPathLength ;
/*0x034 */	WCHAR RedirectedPath[1] ;

} RPC_IN_MarkFileTime, *LPRPC_IN_MarkFileTime ;


// call CloseLittleHandle();
typedef struct _RPC_IN_HANDLE_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;

} RPC_IN_HANDLE, *LPRPC_IN_HANDLE ;


// call NtSetInformationFile_FilePipeInformation_Filter();
typedef struct _RPC_IN_FilePipeInformation_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;
/*0x00C*/	ULONG FileInfoLength ;
/*0x010*/	PVOID FileInfo[1] ;

} RPC_IN_FilePipeInformation, *LPRPC_IN_FilePipeInformation ;


typedef struct _RPC_OUT_FilePipeInformation_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ ULONG Reserved1[2] ;
/*0x010*/ PVOID Pointer ;
/*0x014*/ ULONG Reserved2 ;
/*0x018*/ ULONG Information ;

} RPC_OUT_FilePipeInformation, *LPRPC_OUT_FilePipeInformation ;


// call Handler_RedirectedFile()
typedef struct _RPC_IN_RedirectedFile_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG TempNameLength ;
/*0x00C*/	WCHAR TempName[0xC] ;
/*0x024*/	ULONG RedirectedPathLength ;
/*0x028*/	WCHAR RedirectedPath[1] ;

} RPC_IN_RedirectedFile, *LPRPC_IN_RedirectedFile ;


// call NtCloseFilter()
typedef struct _RPC_IN_NtCloseFilter_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG RedirectedPathLength ;
/*0x00C*/	WCHAR RedirectedPath[1] ;

} RPC_IN_NtCloseFilter, *LPRPC_IN_NtCloseFilter ;


// call fake_NtReadFile()
typedef struct _RPC_IN_NtReadFile_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;
/*0x00C*/	ULONG BufferLength ;

} RPC_IN_NtReadFile, *LPRPC_IN_NtReadFile ;


typedef struct _RPC_OUT_NtReadFile_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ PVOID IoStatusBlock_Pointer ;
/*0x00C*/ ULONG Reserved1 ;
/*0x010*/ ULONG IoStatusBlock_Information ;
/*0x014*/ ULONG Reserved2 ;
/*0x018*/ ULONG BufferLength ;
/*0x01C*/ WCHAR Buffer[1] ;

} RPC_OUT_NtReadFile, *LPRPC_OUT_NtReadFile ;


// call fake_NtWriteFile()
typedef struct _RPC_IN_NtWriteFile_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  HANDLE hFile ;
/*0x00C*/	ULONG Length ;
/*0x010*/	WCHAR Buffer[1] ;

} RPC_IN_NtWriteFile, *LPRPC_IN_NtWriteFile ;


typedef struct _RPC_OUT_NtWriteFile_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ PVOID IoStatusBlock_Pointer ;
/*0x00C*/ ULONG Reserved1 ;
/*0x010*/ ULONG IoStatusBlock_Information ;
/*0x014*/ ULONG Reserved2 ;

} RPC_OUT_NtWriteFile, *LPRPC_OUT_NtWriteFile ;

// call NtFsControlFileFilter()
typedef struct _RPC_IN_NtFsControlFileFilter_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG RedirectedPathLength ;
/*0x00C*/   ULONG RedirectedReparsePathLength ;
/*0x010*/	ULONG RedirectedReparsePathOffset ;
/*0x014*/	WCHAR Buffer[1] ;

} RPC_IN_NtFsControlFileFilter, *LPRPC_IN_NtFsControlFileFilter ;


typedef struct _HANDLE_INFO_ 
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ ULONG TickCount ;
/*0x00C*/ ULONG OrignalPathLength ;
/*0x010*/ WCHAR OrignalPath[1] ;

} HANDLE_INFO, *LPHANDLE_INFO ;


typedef struct _RecoverFolder_INFO_ 
{
/*0x000*/ LIST_ENTRY ListEntry ; 
/*0x008*/ ULONG Reserved ;
/*0x00C*/ ULONG PathLength ;
/*0x010*/ WCHAR szPath[1] ;

} RecoverFolder_INFO, *LPRecoverFolder_INFO ;


//
// 沙箱在过滤NtCreateFile & NtOpenFile 时自定义标志位
//

typedef enum _NtCreaetFile_SBFlag_CLASS_ 
{
	SBFILE_DIRECTORY        =  0x00000001,	// 是目录
	SBFILE_0x10				=  0x00000010,
	SBFILE_NON_DIRECTORY    =  0x00000040,	// 是文件
	SBFILE_MarkedAsDirty	=  0x00001000,	// FileInformation_CreationTime == { 0x1B01234, 0xDEAD44A0 };
	SBFILE_ATTRIBUTE_READONLY	=  0x00100000,	// FILE_ATTRIBUTE_READONLY
	SBFILE_ATTRIBUTE_SYSTEM		=  0x00200000,	// FILE_ATTRIBUTE_SYSTEM
	SBFILE_ATTRIBUTE_READONLY_SYSTEM = 0x00300000  //文件后缀是诸如"url,avi,wmv,mpg,mp3,mp4,wmdb"之类会置于 0x00300000标志

} NtCreaetFile_SBFlag_CLASS ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef ULONG (WINAPI* _NtCreateFile_) (
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
extern _NtCreateFile_ g_NtCreateFile_addr ;


typedef ULONG (WINAPI* _NtDeleteFile_) (
    IN POBJECT_ATTRIBUTES ObjectAttributes
	);
extern _NtDeleteFile_ g_NtDeleteFile_addr ;

typedef ULONG (WINAPI* _NtQueryFullAttributesFile_) (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
	);
extern _NtQueryFullAttributesFile_ g_NtQueryFullAttributesFile_addr ;

typedef ULONG (WINAPI* _NtQueryInformationFile_) (
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
	);
extern _NtQueryInformationFile_ g_NtQueryInformationFile_addr ;

typedef ULONG (WINAPI* _NtQueryDirectoryFile_) (
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
extern _NtQueryDirectoryFile_ g_NtQueryDirectoryFile_addr ;

typedef ULONG (WINAPI* _NtSetInformationFile_) (
	IN HANDLE FileHandle,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass
	);
extern _NtSetInformationFile_ g_NtSetInformationFile_addr ;

typedef ULONG (WINAPI* _NtClose_) (
	IN HANDLE  Handle
	);
extern _NtClose_ g_NtClose_addr ;

typedef ULONG (WINAPI* _NtQueryAttributesFile_) (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
	);

typedef ULONG (WINAPI* _NtCreateNamedPipeFile_) (
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

typedef ULONG (WINAPI* _NtCreateMailslotFile_) (
	OUT PHANDLE MailSlotFileHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG FileAttributes,
	IN ULONG ShareAccess,
	IN ULONG MaxMessageSize,
	IN PLARGE_INTEGER TimeOut
	);

typedef ULONG (WINAPI* _NtReadFile_) (
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

typedef ULONG (WINAPI* _NtWriteFile_) (
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

typedef ULONG (WINAPI* _NtFsControlFile_) (
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

typedef ULONG (WINAPI* _NtQueryVolumeInformationFile_) (
	IN HANDLE FileHandle,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	OUT PVOID FsInformation,
	IN ULONG Length,
	IN FS_INFORMATION_CLASS FsInformationClass
	);

typedef ULONG (WINAPI* _WaitNamedPipeW_) (
	IN LPCWSTR lpNamedPipeName,
	IN DWORD nTimeOut
	);

typedef ULONG (WINAPI* _RtlGetCurrentDirectory_U_) (
	ULONG MaximumLength,
	PWSTR Buffer
	);

typedef ULONG (WINAPI* _RtlSetCurrentDirectory_U_) (
	IN PUNICODE_STRING name
	);

typedef ULONG (WINAPI* _RtlGetFullPathName_U_) (
	IN PCWSTR FileName,
	IN ULONG Size,
	IN PWSTR Buffer,
	OUT PWSTR *ShortName
	);

typedef ULONG (WINAPI* _MoveFileWithProgressW_) (
	LPCWSTR			lpExistingFileName,
	LPCWSTR			lpNewFileName,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	DWORD			dwFlags
	);


NTSTATUS
GetFilePath (
	IN PUNICODE_STRING uniFileName,
	IN HANDLE FileHandle,
	OUT LPWSTR* pOrignalPath,
	OUT LPWSTR* pRedirectedPath,
	OUT BOOL* bIsHandlerSelfFilePath
	);

BOOL HandlerPanfu( ULONG a1 );

VOID HandlerFileList();

void Handler_Configration_AutoRecover();

BOOL CsrPopulateDosDevices();

LPWSTR
QuerySymbolicLinkObject (
	IN PVOID _pNode,
	IN LPWSTR szName,
	IN ULONG NameLength,
	OUT BOOL* bFlag
	);

ULONG 
GetNameType (
	IN LPWSTR wszName,
	OUT LPWSTR* Buffer
	);

VOID HandlerDevices();

BOOL 
HandlerDevicesEx (
	IN LPWSTR szNameFile,
	IN LPWSTR szNameDirectory
	);

VOID RemoveVolumeNode( IN LPWSTR szName );

BOOL
GetClearEnviro (
	IN LPWSTR lpszValue,
	IN ULONG MaxLength
	);

BOOL GetPBEnvironment();

NTSTATUS
Handler_PipeMailslot_Communication (
	IN int CreateOptions ,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN PHANDLE FileHandle,
	IN ULONG DesiredAccess,
	IN PISECURITY_DESCRIPTOR SecurityDescriptor,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG ShareAccess,
	IN ULONG CreateDisposition,
	IN LPWSTR OrignalPath,
	IN ULONG NameType,
	IN LPWSTR PipeName
	);

NTSTATUS
Imitate_Pipe_Communication (
	IN LPWSTR szName,
	IN LPWSTR PipeName,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG CreateOptions,
	OUT HANDLE* FileHandle
	);

LPWSTR
Get_Redirected_PipeMailslot_Name (
	IN LPWSTR szPath,
	IN ULONG NameType
	);

NTSTATUS
NtCreateFileFilter (
	IN ULONG Flag,
	IN ULONG CreateDisposition ,
	IN ULONG DesiredAccess, 
	IN ULONG CreateOptions
	);

BOOL
IsDirtyDirectory (
	IN LPWSTR szPath
	);

NTSTATUS 
QueryAttributesFile (
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	OUT ULONG *Flag,
	OUT BOOL *bNotEndOfFile
	);

NTSTATUS 
CreateRedirectedDirectorys ( 
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	);

NTSTATUS
CreateRedirectedDirectorysEx (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	);

NTSTATUS
Copy_OrignalFileData_to_RedirectedFileData (
	IN BOOL bFlag,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath
	);

NTSTATUS
MarkFileTime (
	IN PVOID FileInformation, 
	IN HANDLE hRedirectedFile,
	IN LPWSTR RedirectedPath
	);

NTSTATUS 
DeleteDirectory (
	IN LPWSTR RedirectedPath
	);

NTSTATUS
MarkAsDirtyFile (
	IN HANDLE hFile,
	IN LPWSTR RedirectedPath
	);

NTSTATUS
MarkAsDirtyFileEx (
	IN LPWSTR RedirectedDirectoryPath
	);


NTSTATUS
RemoveDirtyTag (
	IN HANDLE hFile,
	IN LPWSTR RedirectedPath
	);

BOOL
IsWinsxsFile (
	IN LPWSTR szName
	);

BOOL 
IsRecoverFolder (
	IN LPWSTR szName 
	);

BOOL IsArrayEnd ( IN ULONG ptr );

BOOL
InsertFileHandle (
	IN HANDLE FileHandle,
	IN LPWSTR szName,
	BOOL bInsert
	);

BOOL
Handler_RedirectedFile (
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN HANDLE hRedirectedFile
	);

BOOL
CallHandlerDevicesEx (
	IN LPWSTR szName
	);

BOOL
JointPath (
	IN LPWSTR *pOrignalPath
	);

ULONG 
JointPathEx (
	IN LPWSTR *OrignalPath,
	IN ULONG *OrignalPathLength,
	IN ULONG FileNameLength,
	IN LPWSTR DirectoryNameAddr,
	IN ULONG DirectoryNameLength
	);

PVOID 
GetVolumeNode (
	IN LPWSTR szFileName,
	IN ULONG FileNameLength
	);

LPWSTR 
FixupOrignalPath (
	IN PVOID _pNode,
	IN LPWSTR szOrignalPath
	);

void pfnAPC( IN LPAPCINFO APC );

NTSTATUS 
GetDirNode ( 
	IN HANDLE FileHandle,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN BOOLEAN RestartScan,
	IN PUNICODE_STRING FileName,
	OUT LPDIR_HANDLE_INFO *pNode
	);

NTSTATUS 
NtQueryDirectoryFileFilter ( 
	IN LPDIR_HANDLE_INFO pNode, 
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN int FileInformation,
	IN ULONG Length,
	IN int FileInformationClass,
	IN BOOLEAN ReturnSingleEntry
	);

void 
FreeDirNode (
	IN LPDIR_HANDLE_INFO pNode
	);

NTSTATUS 
FixDirNode (
	IN LPWSTR RedirectedPath,
	IN LPWSTR OrignalPath,
	IN LPDIR_HANDLE_INFO pNode
	);

NTSTATUS 
FixDirNodeEx(
	IN LPDIR_HANDLE_INFO_LITTLE pNode, 
	IN PUNICODE_STRING pSearchName,
	IN BOOLEAN bFlag
	);

NTSTATUS 
FixDirNodeExp (
	IN LPDIR_HANDLE_INFO_LITTLE pNode,
	IN PUNICODE_STRING pSearchName,
	IN PFILE_BOTH_DIR_INFORMATION FileDirInfo
	);

NTSTATUS 
Handler_NtQueryDirectoryFile_FileInformation (
	IN LPDIR_HANDLE_INFO_LITTLE DataInfo ,
	IN PUNICODE_STRING SearchName
	);

LPWSTR 
Copy_NtQueryDirectoryFile_FileInformation (
	IN OUT int FileInformationOld,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	IN LPFILE_COM_DIR_INFORMATION FileInformationNew
	);

NTSTATUS 
NtSetInformationFile_FilePipeInformation_Filter (
	IN UCHAR Index,
	IN ULONG Length,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN PVOID FileInformation,
	IN int FileInformationClass
	);

NTSTATUS 
NtSetInformationFile_FileRenameInformation_Filter (
	IN HANDLE FileHandle,
	IN PFILE_RENAME_INFORMATION FileRenameInfo
	);

NTSTATUS 
CloseLittleHandle (
	IN UCHAR Index
	);

void 
NtCloseFilter (
	IN HANDLE hFile,
	OUT PVOID *RpcBuffer
	);

NTSTATUS
NtFsControlFileFilter (
	IN HANDLE FileHandle,
	IN PREPARSE_DATA_BUFFER InputBuffer
	);

///////////////////////////////   END OF FILE   ///////////////////////////////