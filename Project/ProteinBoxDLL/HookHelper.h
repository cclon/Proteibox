#pragma once

#include "PBRegsData.h"
#include "PBFilesData.h"
#include "PBLoad.h"

//////////////////////////////////////////////////////////////////////////

typedef struct _REGINFO_ 
{
	LPWSTR RegSIDPath ; // eg: "\registry\user\S-1-5-21-583907252-261903793-1177238915-1003"
	ULONG_PTR RegSIDPathLength ; // 上面路径的长度

} REGINFO, *LPREGINFO ;

extern REGINFO g_RegInfo ;

typedef enum _REGTAG_ 
{
	NtCreateKey_TAG = 0,
	NtOpenKey_TAG ,
	NtDeleteKey_TAG ,
	NtDeleteValueKey_TAG ,
	NtSetValueKey_TAG ,
	NtQueryKey_TAG ,
	NtQueryValueKey_TAG ,
	NtQueryMultipleValueKey_TAG ,
	NtEnumerateKey_TAG ,
	NtEnumerateValueKey_TAG ,
	NtNotifyChangeKey_TAG ,
	NtSaveKey_TAG ,
	NtLoadKey_TAG ,
	NtRenameKey_TAG ,
	NtOpenKeyEx_TAG ,
	NtNotifyChangeMultipleKeys_TAG ,
};


typedef enum _FILETAG_ 
{
	NtCreateFile_TAG = 0,
	NtOpenFile_TAG ,
	NtQueryAttributesFile_TAG ,
	NtQueryFullAttributesFile_TAG ,
	NtQueryInformationFile_TAG ,
	NtQueryDirectoryFile_TAG ,
	NtSetInformationFile_TAG ,
	NtDeleteFile_TAG ,
	NtClose_TAG ,
	NtCreateNamedPipeFile_TAG ,
	NtCreateMailslotFile_TAG ,
	NtReadFile_TAG ,
	NtWriteFile_TAG ,
	NtFsControlFile_TAG ,
	NtQueryVolumeInformationFile_TAG ,
	WaitNamedPipeW_TAG ,
	RtlGetCurrentDirectory_U_TAG ,
	RtlSetCurrentDirectory_U_TAG ,
	RtlGetFullPathName_U_TAG ,
	MoveFileWithProgressW_TAG ,
};


typedef enum _ALPCTAG_ 
{
	NtCreatePort_TAG = 0,
	NtConnectPort_TAG ,
	NtSecureConnectPort_TAG ,
	NtAlpcCreatePort_TAG ,
	NtAlpcConnectPort_TAG ,
	NtAlpcQueryInformation_TAG ,
	NtAlpcQueryInformationMessage_TAG ,
	NtImpersonateClientOfPort_TAG ,
	NtAlpcImpersonateClientOfPort_TAG ,
	NtCreateEvent_TAG ,
	NtOpenEvent_TAG ,
	NtCreateMutant_TAG ,
	NtOpenMutant_TAG ,
	NtCreateSemaphore_TAG ,
	NtOpenSemaphore_TAG ,
	NtCreateSection_TAG ,
	NtOpenSection_TAG ,
};


typedef enum _TOKENTAG_ 
{
	NtOpenProcess_TAG = 0,
	NtOpenThread_TAG ,
	NtDuplicateObject_TAG ,
	NtQuerySecurityObject_TAG ,
	NtSetSecurityObject_TAG ,
	NtSetInformationToken_TAG ,
	NtAdjustPrivilegesToken_TAG ,

};


typedef enum _TOKENEXTAG_ 
{
	RtlQueryElevationFlags_TAG = 0,
	NtQueryInformationToken_TAG ,
};


typedef enum _LOADTAG_ 
{
	LdrLoadDll_TAG = 0,
	LdrUnloadDll_TAG ,
	NtLoadDriver_TAG ,
	GetModuleFileNameW_TAG ,
};


typedef enum _CONSOLETAG_ 
{
	SetConsoleTitleA_TAG = 0,
	SetConsoleTitleW_TAG ,
	GetConsoleTitleA_TAG ,
	GetConsoleTitleW_TAG ,
};


#define MAXLENGTH 0x30
typedef struct _HOOKINFOLittle_ 
{
	ULONG_PTR DllTag ;	// 函数所属的模块标记
	ULONG_PTR Tag ;		// 标记当前节点单元
	char FunctionName[ MAXLENGTH ]; // 函数名
	BOOL bHooked ;	// 是否被Hook
	PVOID OrignalAddress ;	// 原始地址
	PVOID FakeAddress ;		// 过滤函数地址
} HOOKINFOLittle, *LPHOOKINFOLittle ;


typedef struct _HOOKINFO_ // size - 
{
	BOOL  bInited ;				 // 是否初始化成功

	struct 
	{
		int	 ArrayCounts ;		  // 数组的总个数
		LPHOOKINFOLittle pArray ; // 数组的首地址
	} REGS ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ; 
	} FILES ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} ALPC ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} TOKEN ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} TOKENEX ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} LOAD ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} CONSOLE ;

	struct 
	{
		int	 ArrayCounts ;
		LPHOOKINFOLittle pArray ;
	} SERVICES ;
	
} HOOKINFO, *LPHOOKINFO ;


typedef BOOL (WINAPI *_TotalDynamicInfoFunc_ ) (HMODULE);

typedef struct _TotalDynamicInfo_	// size - 0x20
{
/*0x000*/ WCHAR ModuleName[ 0x50 ] ;
/*0x004*/ _TotalDynamicInfoFunc_ FuncAddress ;
/*0x008*/ HMODULE hModule ;
/*0x00C*/ BOOL bFlag ;
/*0x010*/ CRITICAL_SECTION Lock ;

} TotalDynamicInfo, *LPTotalDynamicInfo ;


extern BOOL g_FileHookInitOK ;
extern BOOL g_bHookRpc ;
extern BOOL g_bFlag_Is_in_SynTP_exe ;
extern HOOKINFO g_HookInfo ;
extern _GetModuleFileNameW_ g_GetModuleFileNameW_addr ;
extern _MoveFileWithProgressW_ g_MoveFileWithProgressW_addr;
extern WCHAR g_szPath_ntvdm[ MAX_PATH ];


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         函数预定义                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL StartHook ();
VOID StopHook ();

BOOL HookOne( LPHOOKINFOLittle Info );

VOID 
InitHookInfoEx (
	IN int TotalCounts,
	IN LPHOOKINFOLittle pArray,
	IN HMODULE* phModuleArray
	);
VOID InitHookInfo();

BOOL Hook_Pharase0_Regs();
BOOL Hook_pharase1_Files();
BOOL Hook_pharase2_Alpc();
BOOL Hook_pharase3_Token();
BOOL Hook_pharase4_Load();
BOOL Hook_pharase5();
BOOL Hook_pharase6();
BOOL Hook_pharase7_CreateProcess();
BOOL Hook_pharase8_RegKey();
BOOL Hook_pharase9_TotalDynamic();


///////////////////////////////   END OF FILE   ///////////////////////////////