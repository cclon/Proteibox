#pragma once


//////////////////////////////////////////////////////////////////////////


extern ULONG g_ImageFileName_Offset ;

#define _key_unknown_executable_image_for_QueryNameString_ 0xfedcba99

//////////////////////////////////////////////////////////////////////////

typedef struct tagNameInfo {

/*0x000 */	OBJECT_NAME_INFORMATION NameInfo;
/*0x008 */  WCHAR pBuffer[1] ; // 文件全路径

} Name_Info, *LPName_Info ;


enum _PBDLLPATHTYPE_
{
	PBDLLPathType_Nothing = 0,
	PBDLLPathType_Reg,
	PBDLLPathType_Ini,
};

#define IsPBDLLPathTypeValid(_Tag) (BOOL)(_Tag >= PBDLLPathType_Reg && _Tag <= PBDLLPathType_Ini)


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
GetEProcessByName_QueryInfo (
  IN  WCHAR* processname, 
  OUT PVOID* proc
  );

VOID 
NtPath2DosPathA (
	IN LPCSTR szNtPath,
	OUT LPSTR szDosPath,
	IN DWORD cchSize
	);

VOID 
NtPath2DosPathW (
	IN LPCWSTR szNtPath,
	OUT LPWSTR szDosPath,
	IN DWORD cchSize
	);

DWORD
QueryDosDeviceW(
	IN LPCWSTR lpDeviceName,
	OUT LPWSTR lpTargetPath,
	IN DWORD ucchMax
	);

NTSTATUS
PutFile(
	IN WCHAR* filename,
	IN CHAR* buffer,
	IN ULONG buffersize 
	) ;


BOOL
OpenEvent (
	IN LPWSTR lpEventName,
	IN ACCESS_MASK DesiredAccess,
	OUT PHANDLE hEvent
	);

BOOL
SetEvent (
	IN LPWSTR lpEventName,
	IN ACCESS_MASK DesiredAccess
	);

BOOL
w2a (
	IN LPWSTR pInBuffer,
	OUT LPSTR pOutBuffer,
	IN ULONG MaxLength
	);

BOOL
a2w (
	IN LPSTR pInBuffer,
	OUT LPWSTR pOutBuffer,
	IN ULONG MaxLength
	);

BOOL InitResource( IN PERESOURCE* rc );
VOID EnterCrit( IN PERESOURCE rc );
VOID LeaveCrit( IN PERESOURCE rc );


BOOL
MyRtlCompareUnicodeString (
	IN LPName_Info StringA, 
	IN LPWSTR StringB,
	IN ULONG MaximumLength
	);

NTSTATUS 
QueryNameString (
	IN PVOID DeviceObject, 
	OUT PUNICODE_STRING pNameInfo,
	OUT ULONG *pNameLength
	);

PVOID
SepQueryNameString(
    IN PVOID Object,
	OUT BOOL* bHasNoName
    );

BOOL 
WildcardCmpA (
	IN const char *wild,
	IN const char *string
	) ;

BOOL
StringMatchA (
	IN PCHAR ParentString,
	IN PCHAR ChildrenString,
	IN BOOL bUseWildcards
	);

BOOL
StringMatchW (
	IN PWCHAR ParentString,
	IN PWCHAR ChildrenString,
	IN BOOL bUseWildcards
	);

BOOL
GetHandleInformation (
	IN HANDLE hObject,
	OUT LPDWORD lpdwFlags
	);

BOOL
SetHandleInformation (
	IN HANDLE hObject,
	IN DWORD dwFlags
	);

NTSTATUS
ZwCloseSafe (
    IN HANDLE Handle,
	IN BOOL Check
    );

void GetProcessNameOffset();

BOOL
GetProcessImageFileName (
	IN HANDLE ProcessId, 
	OUT PUNICODE_STRING* lppImageFileName,
	OUT LPWSTR* lpImageFileShortName
	);

BOOL 
GetProcessFullPathFromPeb (
	IN HANDLE ProcessHandle ,
	OUT PUNICODE_STRING *pFullName
	);

BOOL 
GetProcessFullPathFromPebEx (
	IN PVOID Eprocess ,
	IN ULONG BufferLength,
	OUT PUNICODE_STRING pFullName
	);

BOOL 
GetProcessFullPathFromPebExp (
	IN PVOID eprocess ,
	IN PUNICODE_STRING namepool
	);

PVOID
GetFileObject(
	IN PVOID Object
	);

ULONG AddRef( LONG Ref );
ULONG DecRef( LONG Ref );

BOOL IsWrittenKernelAddr( IN PVOID StartAddr, IN ULONG_PTR length );

BOOL GetSessionId( ULONG *pSessionId );

///////////////////////////////   END OF FILE   ///////////////////////////////