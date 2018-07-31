#pragma once

//////////////////////////////////////////////////////////////////////////

// 供Handler_GetModuleFileNameW_Total()函数使用,处理和GetModuleFileNameW / LdrLoadDll / LdrUnloadDll 相关API

typedef struct _GetModuleFileName_INFO_ {

/*0x000 */ struct _GetModuleFileName_INFO_* pFlink ;		// 上个节点
/*0x004 */ struct _GetModuleFileName_INFO_* pBlink ;		// 下个节点 
/*0x008 */ ULONG nCounts ;		// 结点总数
/*0x00C */ PVOID hModule ;		// 模块基址
/*0x010 */ ULONG NameLength;	// 名字长度
/*0x014 */ WCHAR lpModulePath[1] ;    // 名字. eg: D:\1.exe

} GetModuleFileName_INFO, *LPGetModuleFileName_INFO ;


// 对应 Handler_GetModuleFileNameW_Total()函数参数一(Flag)的3种情况
typedef enum _GetModuleFileName_Flag_ 
{
	_GetModuleFileName_Flag_Add_ = 0,
	_GetModuleFileName_Flag_Del_  ,
	_GetModuleFileName_Flag_Find_ ,

} GetModuleFileName_Flag ;


// call Walk_c_windows_winsxs_Total_dep_phase1()
typedef struct _RPC_IN_WINSXS_ 
{
/*0x000 */  RPC_IN_HEADER RpcHeader ;
/*0x008 */  ULONG NextBufferOffset ; // 标记下个字符串的偏移
/*0x00C*/   ULONG PathLength ; // 下个字符串 szPath 的长度
/*0x010*/	ULONG FuckedBufferoffset ; // FuckedBufferoffset = 2 * WinsxsPathLength + 0x28 + 2 * wcslen(szPath) + 8;
/*0x014*/	ULONG FuckedLength ;
/*0x018*/	ULONG WinsxsPathLength ; // 第一个字符串 szWinsxsPath 的长度
/*0x01c*/   WCHAR Buffer[1] ; // 其中包含3个字符串 szWinsxsPath / szPath / FuckedBuffer

} RPC_IN_WINSXS, *LPRPC_IN_WINSXS ;


typedef struct _RPC_OUT_WINSXS_ 
{
/*0x000*/ RPC_OUT_HEADER RpcHeader ;
/*0x008*/ ULONG WriteSize ;
/*0x00C*/ PVOID WriteBuffer ;

} RPC_OUT_WINSXS, *LPRPC_OUT_WINSXS ;


typedef struct _AddressOfEntryPoint_THREAD_INFO_ 
{
/*0x000 */ HANDLE hCallerThread ;
/*0x004 */ HANDLE hEvent ;
/*0x008 */ HANDLE hWorkThread ;

} AddressOfEntryPoint_THREAD_INFO, *LPAddressOfEntryPoint_THREAD_INFO ;

extern LIST_ENTRY_EX g_Node_GetModuleFileName ;
extern CRITICAL_SECTION g_lock_GetModuleFileName ;
extern DWORD g_OldProtect ;
extern BYTE g_AddressOfEntryPoint_OrigData[ 5 ] ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID 
Walk_c_windows_winsxs_Total (
	IN LPWSTR lpFileName,
	IN LPWSTR lpSearchPath,
	IN int bFlag
	);

LPWSTR 
Handler_GetModuleFileNameW_Total (
	IN int Flag,
	IN PVOID hModule,
	OUT PULONG pNameLength,
	OUT BOOL* bIsLocked
	);

LPWSTR 
Handler_winsxs_lastupdatetime (
	IN BOOL bFlag_tranlateTodos
	);

PVOID 
Handler_Read_manifest (
	IN LPWSTR lpFileName,
	IN ULONG ReadSize
	);

void 
Handler_Write_manifest (
	IN LPWSTR lpFileName,
	IN PFILE_POSITION_INFORMATION FilePosition,
	IN ULONG WriteSize
	);

ULONG 
ParseManifest (
	IN PVOID pBuffer,
	IN ULONG WriteSize
	);

VOID 
Walk_c_windows_winsxs_Total_dep (
	IN HANDLE hFile,
	IN int bFlag,
	IN LPWSTR szPath,
	IN LPWSTR lpSearchPath,
	IN LPWSTR szWinsxsPath,
	OUT PULONG FilePosition,
	OUT PULONG WriteSize
	);

PVOID 
Walk_c_windows_winsxs_Total_dep_phase1 (
	IN PCHAR FileData,
	IN LPWSTR szPath,
	IN LPWSTR szWinsxsPath,
	OUT LPWSTR *OutFuckedBuffer,
	OUT PULONG OutFuckedLength
	);

ULONG 
Walk_c_windows_winsxs_Total_dep_phase2 (
	IN int FuckedBuffer,
	IN PIMAGE_DOS_HEADER PeAddr
	);

VOID 
Walk_c_windows_winsxs_Total_dep_phase3 (
	IN LPRPC_OUT_WINSXS RpcBuffer,
	IN LPWSTR lpSearchPath,
	IN int bFlag
	);

VOID 
Walk_c_windows_winsxs_Total_dep_phase4 (
	IN int peAddr,
	IN LPWSTR lpSearchPath,
	IN int Flag
	);

int 
sub_7D2523A0 (
	IN PIMAGE_DOS_HEADER PeAddr,
	OUT LPWSTR *OutFuckedBuffer,
	OUT int *OutFuckedLength,
	OUT int *a4
	);

NTSTATUS 
MyWriteFile (
	IN LPWSTR lpFileName,
	IN PVOID pBuffer,
	IN ULONG Length
	);

PVOID 
MyReadFile (
	IN LPWSTR szFileName
	);

VOID Hook_AddressOfEntryPoint();

VOID 
HandlerManagedCode (
	IN LPWSTR szDllName,
	IN PIMAGE_IMPORT_DESCRIPTOR piid
	);

VOID HandlerConsole();

///////////////////////////////   END OF FILE   ///////////////////////////////
