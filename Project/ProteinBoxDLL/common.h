#pragma once

//////////////////////////////////////////////////////////////////////////

#define STATUS_ALREADY_COMMITTED		((NTSTATUS)0xC0000021L)
#define STATUS_ALLOCATE_BUCKET			((NTSTATUS)0xC000022FL)
#define STATUS_BUFFER_OVERFLOW			((NTSTATUS)0x80000005L)
#define STATUS_OBJECT_PATH_SYNTAX_BAD	((NTSTATUS)0xC000003BL)
#define STATUS_OBJECT_NAME_NOT_FOUND	((NTSTATUS)0xC0000034L)
#define STATUS_OBJECT_PATH_NOT_FOUND    ((NTSTATUS)0xC000003AL)
#define STATUS_OBJECT_PATH_INVALID      ((NTSTATUS)0xC0000039L)
#define STATUS_CANNOT_DELETE			((NTSTATUS)0xC0000121L)
#define STATUS_NO_MORE_ENTRIES			((NTSTATUS)0x8000001AL)
#define STATUS_KEY_DELETED				((NTSTATUS)0xC000017CL)
#define STATUS_BAD_INITIAL_PC			((NTSTATUS)0xC000000AL)
#define STATUS_PRIVILEGE_NOT_HELD		((NTSTATUS)0xC0000061L)
#define STATUS_INSUFFICIENT_RESOURCES	((NTSTATUS)0xC000009AL) 
#define STATUS_FILE_IS_A_DIRECTORY		((NTSTATUS)0xC00000BAL)
#define STATUS_OBJECT_NAME_COLLISION	((NTSTATUS)0xC0000035L)
#define STATUS_NOT_A_DIRECTORY			((NTSTATUS)0xC0000103L)
#define STATUS_SHARING_VIOLATION		((NTSTATUS)0xC0000043L)
#define STATUS_DIRECTORY_NOT_EMPTY		((NTSTATUS)0xC0000101L)
#define STATUS_DELETE_PENDING			((NTSTATUS)0xC0000056L)
#define STATUS_NO_MORE_FILES			((NTSTATUS)0x80000006L)
#define STATUS_SERVER_SID_MISMATCH		((NTSTATUS)0xC00002A0L)
#define STATUS_BAD_IMPERSONATION_LEVEL	((NTSTATUS)0xC00000A5L)
#define STATUS_NOT_ALL_ASSIGNED			((NTSTATUS)0x00000106L)
#define STATUS_DLL_NOT_FOUND			((NTSTATUS)0xC0000135L)
#define STATUS_DISK_FULL				((NTSTATUS)0xC000007FL)


#define FILE_SUPERSEDE                    0x00000000
#define FILE_OPEN                         0x00000001
#define FILE_CREATE                       0x00000002
#define FILE_OPEN_IF                      0x00000003
#define FILE_OVERWRITE                    0x00000004
#define FILE_OVERWRITE_IF                 0x00000005
#define FILE_MAXIMUM_DISPOSITION          0x00000005

#define FILE_DIRECTORY_FILE               0x00000001
#define FILE_WRITE_THROUGH                0x00000002
#define FILE_SEQUENTIAL_ONLY              0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING    0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT         0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT      0x00000020
#define FILE_NON_DIRECTORY_FILE           0x00000040

#define FILE_OPEN_BY_FILE_ID              0x00002000
#define FILE_DELETE_ON_CLOSE              0x00001000

#define SYMBOLIC_LINK_QUERY 0x0001

#define FSCTL_PIPE_TRANSCEIVE  0x11C017

#define DIRECTORY_QUERY (0x0001)

#define ERROR_INSTALL_SERVICE 0x641

#define NtCurrentProcess() ( (HANDLE)(LONG_PTR) -1 )  

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

#define IS_FLAG_ON(val, flag)  ((BOOL)(val & flag))
#define IS_FLAG_OFF(val, flag)  (!(BOOL)(val & flag))

#define ARRAYSIZEOF(x)	sizeof (x) / sizeof (x[0])

#define _ProteinBoxRpcSs_exe_A_		"ProteinBoxRpcSs.exe"
#define _ProteinBoxRpcSs_exe_W_		L"ProteinBoxRpcSs.exe"

#define _PBServiceNameA_	"PBSvc"
#define _PBServiceNameW_	L"PBSvc"

#define _FuckedTag_		'tzuk'

extern BOOL g_bDoWork_OK ;
extern BOOL g_bIs_Inside_iexplore_exe ;
extern BOOL g_bIs_Inside_Explorer_exe ;
extern BOOL g_bIs_Inside_PBRpcSs_exe ;
extern BOOL g_bIs_Inside_PBCrypto_exe ;
extern BOOL g_bSID_is_S_1_5_18 ;	

extern PISECURITY_DESCRIPTOR g_SecurityDescriptor ;

extern const LPWSTR g_PBRpcSs_exe ;

extern const LPWSTR g_PBDcomLaunch_exe ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


//
// 微软定义的结构体
//

typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR  DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION {               
	ULONG FileAttributes;                                       
	ULONG ReparseTag;                                           
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;  

typedef struct _FILE_ID_BOTH_DIR_INFORMATION { // size - 0x6C
/* 0x000 */    ULONG NextEntryOffset;
/* 0x004 */    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
/* 0x038 */    ULONG FileAttributes;
/* 0x03C */    ULONG FileNameLength;
/* 0x040 */    ULONG EaSize;
/* 0x044 */    CCHAR ShortNameLength;
/* 0x046 */    WCHAR ShortName[12];
/* 0x060 */    LARGE_INTEGER FileId;
/* 0x068 */    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef unsigned __int64 QWORD, *PQWORD;

typedef struct _FILE_BASIC_INFORMATION {                    
	LARGE_INTEGER CreationTime;                             
	LARGE_INTEGER LastAccessTime;                           
	LARGE_INTEGER LastWriteTime;                            
	LARGE_INTEGER ChangeTime;                               
	ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;    

typedef struct _FILE_RENAME_INFORMATION {
	BOOLEAN Replace;
	HANDLE RootDir;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;


typedef enum _FSINFOCLASS {
	FileFsVolumeInformation       = 1,
	FileFsLabelInformation,      // 2
	FileFsSizeInformation,       // 3
	FileFsDeviceInformation,     // 4
	FileFsAttributeInformation,  // 5
	FileFsControlInformation,    // 6
	FileFsFullSizeInformation,   // 7
	FileFsObjectIdInformation,   // 8
	FileFsDriverPathInformation, // 9
	FileFsMaximumInformation
} FS_INFORMATION_CLASS ;


typedef enum _ALPC_PORT_INFORMATION_CLASS_ 
{
	AlpcBasicInformation  = 0,
	AlpcPortInformation  = 1,
	AlpcAssociateCompletionPortInformation  = 2,
	AlpcConnectedSIDInformation  = 3,
	AlpcServerInformation  = 4,
	AlpcMessageZoneInformation  = 5,
	AlpcRegisterCompletionListInformation  = 6,
	AlpcUnregisterCompletionListInformation  = 7,
	AlpcAdjustCompletionListConcurrencyCountInformation  = 8

} ALPC_PORT_INFORMATION_CLASS ;


typedef enum _ALPC_MESSAGE_INFORMATION_CLASS_
{
	AlpcMessageSidInformation  = 0,
	AlpcMessageTokenModifiedIdInformation  = 1
} ALPC_MESSAGE_INFORMATION_CLASS ;


typedef struct _ALPC_PORT_ATTRIBUTES                 // 10 elements, 0x48 bytes (sizeof)
{
	/*0x000*/     ULONG32      Flags;
	/*0x004*/     struct _SECURITY_QUALITY_OF_SERVICE SecurityQos; // 4 elements, 0xC bytes (sizeof)
	/*0x010*/     UINT64       MaxMessageLength;
	/*0x018*/     UINT64       MemoryBandwidth;
	/*0x020*/     UINT64       MaxPoolUsage;
	/*0x028*/     UINT64       MaxSectionSize;
	/*0x030*/     UINT64       MaxViewSize;
	/*0x038*/     UINT64       MaxTotalSectionSize;
	/*0x040*/     ULONG32      DupObjectTypes;
	/*0x044*/     ULONG32      Reserved;
}ALPC_PORT_ATTRIBUTES, *LPALPC_PORT_ATTRIBUTES;


typedef struct _ALPC_MESSAGE_ATTRIBUTES // 2 elements, 0x8 bytes (sizeof)
{
	/*0x000*/     ULONG32      AllocatedAttributes;
	/*0x004*/     ULONG32      ValidAttributes;
}ALPC_MESSAGE_ATTRIBUTES, *LPALPC_MESSAGE_ATTRIBUTES;


typedef struct _FILE_POSITION_INFORMATION {                 
	LARGE_INTEGER CurrentByteOffset;                        
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;  


//
// 自定义结构体
//

typedef struct _LIST_ENTRY_EX_ 
{
	struct _LIST_ENTRY *Flink;
	struct _LIST_ENTRY *Blink;

	ULONG TotalCounts ;

} LIST_ENTRY_EX, *LPLIST_ENTRY_EX ;


#define _WhiteOrBlack_Flag_LowerLimit_	0x60
#define _WhiteOrBlack_Flag_UperLimit_	0x70

#define IsWBFlagOK( l )	  ( l >= _WhiteOrBlack_Flag_LowerLimit_ && l <= _WhiteOrBlack_Flag_UperLimit_ ) 

enum _WhiteOrBlack_Flag_ 
{
	WhiteOrBlack_Flag_XFilePath = _WhiteOrBlack_Flag_LowerLimit_ ,
	WhiteOrBlack_Flag_XRegKey,
	WhiteOrBlack_Flag_XIpcPath,
	WhiteOrBlack_Flag_XClassName,
};


typedef struct _XFile_INFORMATION_CLASS_ { // Size - 0x54

/*0x000 */  DWORD g_pBuffer_All ;

/*0x004 */  BYTE bFlag_BuildOK_0x66 ; // 属于标志位 0x66
/*0x005 */  BYTE bFlag_BuildOK_0x6B ; // 属于标志位 0x6B
/*0x006 */  BYTE bFlag_BuildOK_0x69 ; // 属于标志位 0x69
/*0x007 */  BYTE bFlag_BuildOK_0x77 ; // 属于标志位 0x77

// _________ 0x008 ~ 0x01C 属于标志位 0x66 _________
/*0x008 */  PVOID pFileWhiteList_LastOne_Device ;  // 尾结点		<本单元保存白名单: g_WhiteList_Device[]>
/*0x00C */  PVOID pFileWhiteList_FirstOne_Device ; // 首结点
/*0x010 */  DWORD nNodeCounts_Device ;	     // 本小链表单元的结点个数

// 保存禁止沙箱中的程序直接访问的所有文件信息,以双向链表的形式关联起来
/*0x014 */  PVOID pFileBlackList_LastOne_ClosedFilePath ;  // 尾结点	<本单元保存黑名单: g_BlackList_CloseFilePath[]>
/*0x018 */  PVOID pFileBlackList_FirstOne_ClosedFilePath ; // 首结点
/*0x01C */  DWORD nNodeCounts_ClosedFilePath ;        // 本小链表单元的结点个数

// _________ 0x020 ~ 0x034 属于标志位 0x6B _________
/*0x020 */  PVOID pFileWhiteList_LastOne_ReadKeyPath ;  // 尾结点		<本单元保存白名单: g_WhiteList_ReadKeyPath[]>
/*0x024 */  PVOID pFileWhiteList_FirstOne_ReadKeyPath ; // 首结点
/*0x028 */  DWORD nNodeCounts_ReadKeyPath ;	     // 本小链表单元的结点个数

/*0x02C */  PVOID pFileBlackList_LastOne_ClosedKeyPath ;  // 尾结点		<本单元保存黑名单: g_BlackList_ClosedKeyPath[]>
/*0x030 */  PVOID pFileBlackList_FirstOne_ClosedKeyPath ; // 首结点
/*0x034 */  DWORD nNodeCounts_ClosedKeyPath ;	     // 本小链表单元的结点个数

// _________ 0x038 ~ 0x04C 属于标志位 0x69 _________
/*0x038 */  PVOID pFileWhiteList_LastOne_BaseNamedObjects ;  // 尾结点		<本单元保存白名单: g_WhiteList_BaseNamedObjects[]>
/*0x03C */  PVOID pFileWhiteList_FirstOne_BaseNamedObjects ; // 首结点
/*0x040 */  DWORD nNodeCounts_5 ;	     // 本小链表单元的结点个数

/*0x044 */  PVOID pFileBlackList_LastOne_6 ;  // 尾结点		[本单元保存黑名单]
/*0x048 */  PVOID pFileBlackList_FirstOne_6 ; // 首结点
/*0x04C */  DWORD nNodeCounts_6 ;	     // 本小链表单元的结点个数

// _________ 0x050 属于标志位 0x77 _________
/*0x050 */  PVOID pFileWhiteList_LastOne_7 ;  // 尾结点

} XFile_INFORMATION_CLASS, *PXFile_INFORMATION_CLASS ;



//
// 进程总结构体 
//

typedef enum _DLLTAG_
{
	Ntdll_TAG = 0,
	Kernel32_TAG,
	KernelBase_TAG,

	Nothing_TAG = 0x10,
};

typedef struct _DLL_BASE_ADDRESS_ 
{
	HMODULE hModule_Self ;
	HMODULE hModuleArrays[3];
} DLL_BASE_ADDRESS, *LPDLL_BASE_ADDRESS ;


typedef struct UNICODE_STRING_EX
{
	UNICODE_STRING uniBuffer;
	WCHAR Data[1] ;

} UNICODE_STRING_EX, *LPUNICODE_STRING_EX ;


typedef struct _PROCESSNODE_ 
{
	DLL_BASE_ADDRESS DllBaseAddr ;

} PROCESSNODE, *LPPROCESSNODE ;

extern LPPROCESSNODE __ProcessNode ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL GetProcessToken( PUNICODE_STRING SidName );

PVOID kmalloc ( ULONG length );

VOID kfree ( PVOID ptr );

void kfreeExp( PVOID pBuffer );

VOID 
WhiteOrBlack (
	IN int Flag,
	IN LPCWSTR szPath,
	OUT BOOL* bIsWhite,
	OUT BOOL* bIsBlack
	);

VOID 
WhiteOrBlackEx (
	IN LPWSTR szPath,
	OUT BOOL* bIsWhite,
	OUT BOOL* bIsBlack
	);

BOOL
MmIsAddressValid(
	IN PVOID ptr,
	IN ULONG length
	);

LPLIST_ENTRY_EX 
ClearStruct( 
	IN LPLIST_ENTRY_EX pNode
	);

PVOID
InsertListA (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pNodeCurrent,
	IN PLIST_ENTRY pNodeNew
	);

PVOID 
InsertListB (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pNodeCurrent,
	IN PLIST_ENTRY pNodeNew
	);


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}


PLIST_ENTRY
RemoveEntryListEx (
	IN LPLIST_ENTRY_EX pNodeHead,
	IN PLIST_ENTRY pCurrentNode
	);


PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}



LONG
NTAPI
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

WCHAR NTAPI
RtlUpcaseUnicodeChar (
	IN WCHAR Source
	);

int 
RtlCompareUnicodeStringDummy (
	IN LPWSTR szNameA, 
	IN LPWSTR szNameB,
	IN USHORT Length
	);

PIMAGE_NT_HEADERS
SRtlImageNtHeader (
	IN PVOID BaseAddress
	);

LPVOID HeapAllocEx( IN DWORD Size );


///////////////////////////////   END OF FILE   ///////////////////////////////