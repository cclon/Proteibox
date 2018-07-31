#pragma once

#include "RPCHandler.h"


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							结构体			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef struct _FILE_BASIC_INFORMATION {                    
    LARGE_INTEGER CreationTime;                             
    LARGE_INTEGER LastAccessTime;                           
    LARGE_INTEGER LastWriteTime;                            
    LARGE_INTEGER ChangeTime;                               
    ULONG FileAttributes;                                   
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;    


typedef struct _RPC_IN_NtCloseFilter_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG RedirectedPathLength ;
/*0x00C*/	WCHAR RedirectedPath[1] ;

} RPC_IN_NtCloseFilter, *LPRPC_IN_NtCloseFilter ;


typedef struct _APINUMBER_0X1700_0X1900_INFO_ 
{
/*0x000*/ LPWSTR szWindir ;
/*0x004*/ LPWSTR szWinsxs ;
/*0x008*/ PSECURITY_DESCRIPTOR lpSecurityDescriptor ;
/*0x00C*/ HANDLE hHeap ;

} APINUMBER_0X1700_0X1900_INFO, *LPAPINUMBER_0X1700_0X1900_INFO ;


typedef struct _RPC_IN_NtFsControlFileFilter_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG RedirectedPathLength ;
/*0x00C*/   ULONG RedirectedReparsePathLength ;
/*0x010*/	ULONG RedirectedReparsePathOffset ;
/*0x014*/	WCHAR Buffer[1] ;

} RPC_IN_NtFsControlFileFilter, *LPRPC_IN_NtFsControlFileFilter ;


// call CreateRedirectedDirectorysEx();
typedef struct _RPC_IN_CreateRedirectedDirectorysEx_ 
{
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


// fake_NtLoadKey()
typedef struct _RPC_IN_NtLoadKey_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  WCHAR szOrignalPath[0x80] ;
/*0x108 */  WCHAR szRedirectedPath[0x80] ;

} RPC_IN_NtLoadKey, *LPRPC_IN_NtLoadKey ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


class CRpcApiNumber0x17001900 : public CRPCHandler
{
public:
	CRpcApiNumber0x17001900(void);
	~CRpcApiNumber0x17001900(void);

public:
	VOID Handler_ApiNumber_0x1700_0x1900( PVOID _pBuffer, LPTOTOAL_DATA pNode );

	PVOID 
	Proc_ApiNumber_0x1700_0x1900 (
		IN PVOID pInfo,
		IN PRPC_IN_HEADER pRpcBuffer, 
		IN int Msg
		);

protected:
	PVOID RpcMarkFileTime( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId );
	PVOID RpcSetFileShortName( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId );
	PVOID RpcNtLoadKey( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId );
	PVOID RpcNtCloseWinsxs( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId );
	PVOID RpcNtFsControlFile( PVOID _pInfo, PVOID _pRpcInBuffer, ULONG dwProcessId );


protected:
	NTSTATUS CallZwCreateFile (
		LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
		ULONG dwProcessId, 
		LPWSTR RedirectedPath, 
		int DesiredAccess, 
		int CreateOptions, 
		HANDLE* FileHandle
		);

	BOOL 
	RpcNtCloseWinsxsDep (
		LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
		LPWSTR szPath1, 
		ULONG Path1Length, 
		LPWSTR szPath2, 
		ULONG Path2Length
		);

	BOOL Mywcstol( LPWSTR lpBuffer, UINT *puiNum );

	NTSTATUS 
	IsRedirectedPathReliable ( 
		LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
		ULONG dwProcessId, 
		LPWSTR szRedirectedPath, 
		LPWSTR szComparePath 
		);

	NTSTATUS 
	RefreshFileDirectoryInfo ( 
		HANDLE hHeap, 
		LPWSTR szDirectoryPath, 
		HANDLE *hDirectory, 
		PFILE_DIRECTORY_INFORMATION *ppFDIold, 
		PFILE_DIRECTORY_INFORMATION *ppFDInew
		);

	NTSTATUS
	RefreshFileDirectoryInfoDep (
		HANDLE hDirectory, 
		PFILE_DIRECTORY_INFORMATION pFDI, 
		PFILE_DIRECTORY_INFORMATION *ppFDInew
		);

	BOOL
	WinsxsCopyFile ( 
		LPAPINUMBER_0X1700_0X1900_INFO pInfo, 
		LPWSTR szPathforRead, 
		LPWSTR szPathforWrite, 
		DWORD dwProcessId
		);

	NTSTATUS 
	WriteLastTimeToFile (
		LPAPINUMBER_0X1700_0X1900_INFO pInfo,
		LPWSTR szPath
		);

	int sub_10079E0(UINT *a1, UINT *a2);

private:
	
};


extern CRpcApiNumber0x17001900 g_CRpcApiNumber0x1700_0x1900 ;


///////////////////////////////   END OF FILE   ///////////////////////////////
