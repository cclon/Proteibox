/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2008/08/13 
* MODULE : struct.h               
*
***
* Copyright (c) 2008 - 2010 sudami.
* Freely distributable in source or binary for noncommercial purposes.
* TAKE IT EASY,JUST FOR FUN.
*
****************************************************************************************/

#pragma once

#include <ntifs.h>
#include <windef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////


#define dprintf if (DBG) DbgPrint

#define MYTRACE

#define kmalloc(_s)	ExAllocatePoolWithTag(NonPagedPool, _s, 'ADUS')
#define kfree(_p)	if(_p){ ExFreePool(_p); _p = NULL ; }


#define  SPIN_LOCK_INITIAL(_m_)       KeInitializeSpinLock(&_m_)
#define  SPIN_LOCK_WAITFOR(_m_, _i_)  KeAcquireSpinLock(&_m_, &_i_)
#define  SPIN_LOCK_RELEASE(_m_, _i_)  KeReleaseSpinLock(&_m_,  _i_)


#define MAX_PATH 260
#define ARRAYSIZEOF(x)	sizeof (x) / sizeof (x[0])


ULONG g_CR0 ;

#define Writable()                    \
    __asm { push  eax             }    \
    __asm { mov   eax, cr0        }    \
    __asm { mov   g_CR0, eax      }    \
    __asm { and   eax, 0FFFEFFFFh }    \
    __asm { mov   cr0, eax        }    \
    __asm { pop   eax             }

#define UnWritable()                  \
    __asm { push  eax             }    \
    __asm { mov   eax, g_CR0      }    \
    __asm { mov   cr0, eax        }    \
    __asm { pop   eax             }


#define LongAlign(P) (                \
    ((((ULONG)(P)) + 3) & 0xfffffffc) \
)

#define WordAlign(P) (                \
    ((((ULONG)(P)) + 1) & 0xfffffffe) \
)

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

#define PtrOffset(B,O) ((ULONG)((ULONG)(O) - (ULONG)(B)))

#define PROCESS_QUERY_INFORMATION (0x0400)  

#define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }

#define PushRegister()	\
	__asm { pushfd }	\
	__asm { pushad }	

#define PopRegister()	\
	__asm { popad }	\
	__asm { popfd }	

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define HANDLE_FLAG_INHERIT             0x00000001
#define HANDLE_FLAG_PROTECT_FROM_CLOSE  0x00000002


// 判定函数地址x是否在函数y内部,一般屏蔽低3位进行比较,因为一个函数长度一般<0x1000

#define ShiledBits_lower_2	0xFF	// 表明屏蔽地址的低2位
#define ShiledBits_lower_3	0xFFF	// 表明屏蔽地址的低3位

#define IS_addr_x_Inside_y(_x_, _y_, key) \
	(     (  (DWORD)((DWORD)_x_ & ~key) == (DWORD)((DWORD)_y_ & ~key)  ) && (  (DWORD)_x_ > (DWORD)_y_  )     )


extern POBJECT_TYPE *MmSectionObjectType ; 

extern POBJECT_TYPE *PsProcessType ;
extern POBJECT_TYPE *PsThreadType  ;


#define WM_DESTROY			0x0002
#define WM_SETREDRAW		0x000B
#define WM_CLOSE			0x0010
#define WM_QUERYENDSESSION	0x0011
#define WM_QUIT				0x0012
#define WM_ENDSESSION		0x0016
#define WM_NOTIFY			0x004E
#define WM_NCDESTROY		0x0082
#define WM_COMMAND			0x0111
#define WM_SYSCOMMAND		0x0112


#define THREAD_TERMINATE               (0x0001)  
#define THREAD_SUSPEND_RESUME          (0x0002)  
#define THREAD_GET_CONTEXT             (0x0008)  
#define THREAD_SET_CONTEXT             (0x0010)  
#define THREAD_SET_INFORMATION         (0x0020)  
#define THREAD_QUERY_INFORMATION       (0x0040)  
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_DIRECT_IMPERSONATION    (0x0200)

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+						   结构体						      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectTypesInformation,
	ObjectHandleFlagInformation,
	ObjectSessionInformation,
	MaxObjectInfoClass
} OBJECT_INFORMATION_CLASS;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemCpuInformation = 1,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3, /* was SystemTimeInformation */
	Unknown4,
	SystemProcessInformation = 5,
	Unknown6,
	Unknown7,
	SystemProcessorPerformanceInformation = 8,
	Unknown9,
	Unknown10,
	SystemModuleInformation = 11,
	Unknown12,
	Unknown13,
	Unknown14,
	Unknown15,
	SystemHandleInformation = 16,
	Unknown17,
	SystemPageFileInformation = 18,
	Unknown19,
	Unknown20,
	SystemCacheInformation = 21,
	Unknown22,
	SystemInterruptInformation = 23,
	SystemDpcBehaviourInformation = 24,
	SystemFullMemoryInformation = 25,
	SystemNotImplemented6 = 25,
	SystemLoadImage = 26,
	SystemUnloadImage = 27,
	SystemTimeAdjustmentInformation = 28,
	SystemTimeAdjustment = 28,
	SystemSummaryMemoryInformation = 29,
	SystemNotImplemented7 = 29,
	SystemNextEventIdInformation = 30,
	SystemNotImplemented8 = 30,
	SystemEventIdsInformation = 31,
	SystemCrashDumpInformation = 32,
	SystemExceptionInformation = 33,
	SystemCrashDumpStateInformation = 34,
	SystemKernelDebuggerInformation = 35,
	SystemContextSwitchInformation = 36,
	SystemRegistryQuotaInformation = 37,
	SystemCurrentTimeZoneInformation = 44,
	SystemTimeZoneInformation = 44,
	SystemLookasideInformation = 45,
	SystemSetTimeSlipEvent = 46,
	SystemCreateSession = 47,
	SystemDeleteSession = 48,
	SystemInvalidInfoClass4 = 49,
	SystemRangeStartInformation = 50,
	SystemVerifierInformation = 51,
	SystemAddVerifier = 52,
	SystemSessionProcessesInformation	= 53,
	SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS ;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER SpareLi2;
	LARGE_INTEGER SpareLi3;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SpareUl2;
	ULONG SpareUl3;
	ULONG PeakVirtualSize;
	ULONG VirtualSize;
	ULONG PageFaultCount;
	ULONG PeakWorkingSetSize;
	ULONG WorkingSetSize;
	ULONG QuotaPeakPagedPoolUsage;
	ULONG QuotaPagedPoolUsage;
	ULONG QuotaPeakNonPagedPoolUsage;
	ULONG QuotaNonPagedPoolUsage;
	ULONG PagefileUsage;
	ULONG PeakPagefileUsage;
	ULONG PrivatePageCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;


typedef struct _OBJECT_HANDLE_ATTRIBUTE_INFORMATION
{
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_ATTRIBUTE_INFORMATION, *POBJECT_HANDLE_ATTRIBUTE_INFORMATION ;


typedef struct _CURDIR
{
	UNICODE_STRING DosPath;
	PVOID Handle;
}CURDIR, *PCURDIR;


typedef struct _RTL_DRIVE_LETTER_CURDIR
{
	WORD Flags;
	WORD Length;
	ULONG TimeStamp;
	STRING DosPath;
}RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;


typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	ULONG MaximumLength;
	ULONG Length;
	ULONG Flags;
	ULONG DebugFlags;
	PVOID ConsoleHandle;
	ULONG ConsoleFlags;
	PVOID StandardInput;
	PVOID StandardOutput;
	PVOID StandardError;
	CURDIR CurrentDirectory;
	UNICODE_STRING DllPath;
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
	PVOID Environment;
	ULONG StartingX;
	ULONG StartingY;
	ULONG CountX;
	ULONG CountY;
	ULONG CountCharsX;
	ULONG CountCharsY;
	ULONG FillAttribute;
	ULONG WindowFlags;
	ULONG ShowWindowFlags;
	UNICODE_STRING WindowTitle;
	UNICODE_STRING DesktopInfo;
	UNICODE_STRING ShellInfo;
	UNICODE_STRING RuntimeData;
	RTL_DRIVE_LETTER_CURDIR CurrentDirectores;
}RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;


typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG_PTR AffinityMask;
	KPRIORITY Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION;
typedef THREAD_BASIC_INFORMATION *PTHREAD_BASIC_INFORMATION;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS
ZwQuerySystemInformation(
    IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength
	);

NTSTATUS 
ZwOpenProcess(
   OUT PHANDLE ProcessHandle, 
   IN ACCESS_MASK DesiredAccess, 
   IN POBJECT_ATTRIBUTES ObjectAttributes, 
   IN PCLIENT_ID ClientId
   );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessToken (
    IN HANDLE       ProcessHandle,
    IN ACCESS_MASK  DesiredAccess,
    OUT PHANDLE     TokenHandle
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadToken (
    IN HANDLE       ThreadHandle,
    IN ACCESS_MASK  DesiredAccess,
    IN BOOLEAN      OpenAsSelf,
    OUT PHANDLE     TokenHandle
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateToken (
    IN HANDLE               ExistingTokenHandle,
    IN ACCESS_MASK          DesiredAccess,
    IN POBJECT_ATTRIBUTES   ObjectAttributes,
    IN BOOLEAN              EffectiveOnly,
    IN TOKEN_TYPE           TokenType,
    OUT PHANDLE             NewTokenHandle
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwAdjustPrivilegesToken (
    IN HANDLE               TokenHandle,
    IN BOOLEAN              DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES    NewState,
    IN ULONG                BufferLength,
    OUT PTOKEN_PRIVILEGES   PreviousState OPTIONAL,
    OUT PULONG              ReturnLength
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING      SubsystemName,
    IN PVOID                HandleId,
    IN PUNICODE_STRING      ObjectTypeName,
    IN PUNICODE_STRING      ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK          DesiredAccess,
    IN PGENERIC_MAPPING     GenericMapping,
    IN BOOLEAN              ObjectCreation,
    OUT PACCESS_MASK        GrantedAccess,
    OUT PBOOLEAN            AccessStatus,
    OUT PBOOLEAN            GenerateOnClose
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwYieldExecution (
    VOID
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess (
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID           ProcessInformation,
    IN ULONG            ProcessInformationLength,
    OUT PULONG          ReturnLength OPTIONAL
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationObject (
	IN HANDLE ObjectHandle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN PVOID ObjectInformation,
    IN ULONG Length
	);

// ZwProtectVirtualMemory won't resolve! Need to extract from dll manually.

NTSYSAPI 
NTSTATUS 
NTAPI 
ZwPulseEvent( 
	HANDLE h, 
	PULONG p 
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject(
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSYSAPI
NTSTATUS
NTAPI
ZwLoadKey (
    IN POBJECT_ATTRIBUTES KeyObjectAttributes,
    IN POBJECT_ATTRIBUTES FileObjectAttributes
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId
	);


NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(
    __in HANDLE ProcessId,
    __deref_out PEPROCESS *Process
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationProcess (
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID            ProcessInformation,
    IN ULONG            ProcessInformationLength
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryObject (
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG ObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSTATUS NTAPI
RtlConvertSidToUnicodeString(
	PUNICODE_STRING String,
	PSID Sid_,
	BOOLEAN AllocateBuffer
	);

NTSTATUS
NTAPI
ObOpenObjectByName(IN POBJECT_ATTRIBUTES ObjectAttributes,
                   IN POBJECT_TYPE ObjectType,
                   IN KPROCESSOR_MODE AccessMode,
                   IN PACCESS_STATE PassedAccessState,
                   IN ACCESS_MASK DesiredAccess,
                   IN OUT PVOID ParseContext,
                   OUT PHANDLE Handle
				   );

NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace(
    __in ULONG FramesToSkip,
    __in ULONG FramesToCapture,
    __out_ecount(FramesToCapture) PVOID *BackTrace,
    __out_opt PULONG BackTraceHash
   );

NTSYSAPI
NTSTATUS
NTAPI
ZwResetEvent (
    IN HANDLE   EventHandle,
    OUT PLONG   PreviousState OPTIONAL
	);

NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSymbolicLinkObject(
    __out PHANDLE LinkHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in PUNICODE_STRING LinkTarget
    );

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							函数指针				          +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef NTSYSAPI NTSTATUS (*_ZwCreateToken_)(
	OUT PHANDLE TokenHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN TOKEN_TYPE TokenType,
	IN PLUID AuthenticationId,
	IN PLARGE_INTEGER ExpirationTime,
	IN PTOKEN_USER User,
	IN PTOKEN_GROUPS Groups,
	IN PTOKEN_PRIVILEGES Privileges,
	IN PTOKEN_OWNER Owner OPTIONAL,
	IN PTOKEN_PRIMARY_GROUP PrimaryGroup,
	IN PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
	IN PTOKEN_SOURCE TokenSource
	);

extern _ZwCreateToken_ g_ZwCreateToken_addr ;


typedef NTSYSAPI NTSTATUS (*_ZwProtectVirtualMemory_)(
	IN HANDLE ProcessHandle,
	IN PVOID *BaseAddress,
	IN ULONG *NumberOfBytesToProtect,
	IN ULONG NewAccessProtection,
	OUT PULONG OldAccessProtection
	) ;

extern _ZwProtectVirtualMemory_ g_ZwProtectVirtualMemory_addr ;


typedef NTSYSAPI NTSTATUS (*_ZwSetInformationToken_)(
    __in HANDLE TokenHandle,
    __in TOKEN_INFORMATION_CLASS TokenInformationClass,
    __in_bcount(TokenInformationLength) PVOID TokenInformation,
    __in ULONG TokenInformationLength
    );

extern _ZwSetInformationToken_ g_ZwSetInformationToken_addr ;


typedef NTSYSAPI NTSTATUS (*_ZwQueryInformationThread_)(
	IN HANDLE ThreadHandle,
	IN THREADINFOCLASS ThreadInformationClass,
	OUT PVOID ThreadInformation,
	IN ULONG ThreadInformationLength,
	OUT PULONG ReturnLength
	);

extern _ZwQueryInformationThread_ g_ZwQueryInformationThread_addr ;


typedef NTSTATUS (NTAPI *_RtlAddAccessAllowedAceEx_) (
	IN OUT PACL Acl,
	IN ULONG Revision,
	IN ULONG Flags,
	IN ACCESS_MASK AccessMask,
	IN PSID Sid
	);

extern _RtlAddAccessAllowedAceEx_ g_RtlAddAccessAllowedAceEx_addr ;


typedef HANDLE (NTAPI *_PsGetProcessId_) (PEPROCESS Process);
extern _PsGetProcessId_ g_PsGetProcessId_addr ;


typedef HWND (NTAPI *_NtUserGetForegroundWindow_)( VOID );
extern _NtUserGetForegroundWindow_ g_NtUserGetForegroundWindow_addr ;


typedef UINT (NTAPI *_NtUserSendInput_)(
	IN UINT cInputs,
	IN int pInputs,
	IN int cbSize
	);

extern _NtUserSendInput_ g_NtUserSendInput_addr ;


typedef HANDLE (NTAPI *_NtUserQueryWindow_)( IN HWND hwnd, IN /*WINDOWINFOCLASS*/ int WindowInfo );
extern _NtUserQueryWindow_ g_NtUserQueryWindow_addr ;


typedef int (NTAPI *_NtUserGetClassName_)(  IN HWND hwnd, IN BOOL bAnsi, IN OUT PUNICODE_STRING pstrClassName);
extern _NtUserGetClassName_ g_NtUserGetClassName_addr ;


extern BOOL g_Driver_Inited_phrase1 ;

///////////////////////////////   END OF FILE   ///////////////////////////////