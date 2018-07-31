/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/19 [19:9:2010 - 10:27]
* MODULE : SandBox\Code\Project\ProteinBoxDLL\HookHelper.cpp
* 
* Description:
*   负责处理所有的Hook相关需求     
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "common.h"
#include "ProteinBoxDLL.h"
#include "PBAlpc.h"
#include "PBAlpcData.h"
#include "PBToken.h"
#include "PBLoadData.h"
#include "PBLoad.h"
#include "PBFiles.h"
#include "PBRegs.h"
#include "PBDynamicData.h"
#include "PBIPStore.h"
#include "PBCreateProcess.h"
#include "PBUser32dll/PBUser32dll.h"
#include "PBAdvapi32dll/PBAdvapi32dll.h"
#include "PBMsidll/PBMsidll.h"
#include "PBOle32dll/PBOle32dll.h"
#include "PBSetupapidll/PBSetupapidll.h"
#include "PBShell32dll/PBShell32dll.h"
#include "PBWtsapi32dll/PBWtsapi32dll.h"
#include "HookHelperData.h"
#include "HookHelper.h"

#pragma warning(disable : 4995 4996 4005)

//////////////////////////////////////////////////////////////////////////

REGINFO g_RegInfo ;
HOOKINFO g_HookInfo ;

LONG g_TotalDynamic_LockCounts = 0 ;

BOOL g_FileHookInitOK			= FALSE ;
BOOL g_bHookRpc					= FALSE ;
BOOL g_bFlag_Is_in_SynTP_exe	= FALSE ;

WCHAR g_szPath_ntvdm[ MAX_PATH ] = L"" ;

_GetModuleFileNameW_ g_GetModuleFileNameW_addr = NULL ;
_MoveFileWithProgressW_ g_MoveFileWithProgressW_addr = NULL;

// Regs
static HOOKINFOLittle g_HookInfoReg_Array [] = 
{
	{ Ntdll_TAG, NtCreateKey_TAG, "NtCreateKey",			0, NULL, fake_NtCreateKey },
	{ Ntdll_TAG, NtOpenKey_TAG, "NtOpenKey",				0, NULL, fake_NtOpenKey },
	{ Ntdll_TAG, NtDeleteKey_TAG, "NtDeleteKey",			0, NULL, fake_NtDeleteKey },
	{ Ntdll_TAG, NtDeleteValueKey_TAG, "NtDeleteValueKey", 0, NULL, fake_NtDeleteValueKey },
	{ Ntdll_TAG, NtSetValueKey_TAG, "NtSetValueKey",		0, NULL, fake_NtSetValueKey },
	{ Ntdll_TAG, NtQueryKey_TAG, "NtQueryKey",				0, NULL, fake_NtQueryKey },
	{ Ntdll_TAG, NtQueryValueKey_TAG, "NtQueryValueKey",	0, NULL, fake_NtQueryValueKey },
	{ Ntdll_TAG, NtQueryMultipleValueKey_TAG, "NtQueryMultipleValueKey", 0, NULL, fake_NtQueryMultipleValueKey },
	{ Ntdll_TAG, NtEnumerateKey_TAG, "NtEnumerateKey",		0, NULL, fake_NtEnumerateKey },
	{ Ntdll_TAG, NtEnumerateValueKey_TAG, "NtEnumerateValueKey", 0, NULL, fake_NtEnumerateValueKey },
	{ Ntdll_TAG, NtNotifyChangeKey_TAG, "NtNotifyChangeKey", 0, NULL, fake_NtNotifyChangeKey },
	{ Ntdll_TAG, NtSaveKey_TAG, "NtSaveKey",				0, NULL, fake_NtSaveKey },
	{ Ntdll_TAG, NtLoadKey_TAG, "NtLoadKey",				0, NULL, fake_NtLoadKey },
	{ Ntdll_TAG, NtRenameKey_TAG, "NtRenameKey",			0, NULL, fake_NtRenameKey },
	{ Ntdll_TAG, NtOpenKeyEx_TAG, "NtOpenKeyEx",			0, NULL, fake_NtOpenKeyEx },
	{ Ntdll_TAG, NtNotifyChangeMultipleKeys_TAG, "NtNotifyChangeMultipleKeys", 0, NULL, fake_NtNotifyChangeMultipleKeys },

};


// Files
static HOOKINFOLittle g_HookInfoFile_Array [] = 
{
	{ Ntdll_TAG, NtCreateFile_TAG, "NtCreateFile",0, NULL, fake_NtCreateFile },
	{ Ntdll_TAG, NtOpenFile_TAG, "NtOpenFile",0, NULL, fake_NtOpenFile },
	{ Ntdll_TAG, NtQueryAttributesFile_TAG, "NtQueryAttributesFile",0, NULL, fake_NtQueryAttributesFile },
	{ Ntdll_TAG, NtQueryFullAttributesFile_TAG, "NtQueryFullAttributesFile",0, NULL, fake_NtQueryFullAttributesFile },
	{ Ntdll_TAG, NtQueryInformationFile_TAG, "NtQueryInformationFile",0, NULL, fake_NtQueryInformationFile },
	{ Ntdll_TAG, NtQueryDirectoryFile_TAG, "NtQueryDirectoryFile",0, NULL, fake_NtQueryDirectoryFile },
	{ Ntdll_TAG, NtSetInformationFile_TAG, "NtSetInformationFile",0, NULL, fake_NtSetInformationFile },
	{ Ntdll_TAG, NtDeleteFile_TAG, "NtDeleteFile",0, NULL, fake_NtDeleteFile },
	{ Ntdll_TAG, NtClose_TAG, "NtClose",0, NULL, fake_NtClose },
	{ Ntdll_TAG, NtCreateNamedPipeFile_TAG, "NtCreateNamedPipeFile",0, NULL, fake_NtCreateNamedPipeFile },
	{ Ntdll_TAG, NtCreateMailslotFile_TAG, "NtCreateMailslotFile",0, NULL, fake_NtCreateMailslotFile },
	{ Ntdll_TAG, NtReadFile_TAG, "NtReadFile",0, NULL, fake_NtReadFile },
	{ Ntdll_TAG, NtWriteFile_TAG, "NtWriteFile",0, NULL, fake_NtWriteFile },
	{ Ntdll_TAG, NtFsControlFile_TAG, "NtFsControlFile",0, NULL, fake_NtFsControlFile },
	{ Ntdll_TAG, NtQueryVolumeInformationFile_TAG, "NtQueryVolumeInformationFile",0, NULL, fake_NtQueryVolumeInformationFile },
	{ Kernel32_TAG, WaitNamedPipeW_TAG, "WaitNamedPipeW",0, NULL, fake_WaitNamedPipeW },
	{ Ntdll_TAG, RtlGetCurrentDirectory_U_TAG, "RtlGetCurrentDirectory_U",0, NULL, fake_RtlGetCurrentDirectory_U },
	{ Ntdll_TAG, RtlSetCurrentDirectory_U_TAG, "RtlSetCurrentDirectory_U",0, NULL, fake_RtlSetCurrentDirectory_U },
	{ Ntdll_TAG, RtlGetFullPathName_U_TAG, "RtlGetFullPathName_U",0, NULL, fake_RtlGetFullPathName_U },
	{ Kernel32_TAG, MoveFileWithProgressW_TAG, "MoveFileWithProgressW",0, NULL, fake_MoveFileWithProgressW },

};


// Alpc
static HOOKINFOLittle g_HookInfoAlpc_Array [] = 
{
	{ Ntdll_TAG, NtCreatePort_TAG, "NtCreatePort",	0, NULL, fake_NtCreatePort },
	{ Ntdll_TAG, NtConnectPort_TAG, "NtConnectPort", 0, NULL, fake_NtConnectPort },
	{ Ntdll_TAG, NtSecureConnectPort_TAG, "NtSecureConnectPort", 0, NULL, fake_NtSecureConnectPort },
	{ Ntdll_TAG, NtAlpcCreatePort_TAG, "NtAlpcCreatePort", 0, NULL, fake_NtAlpcCreatePort },
	{ Ntdll_TAG, NtAlpcConnectPort_TAG, "NtAlpcConnectPort", 0, NULL, fake_NtAlpcConnectPort },
	{ Ntdll_TAG, NtAlpcQueryInformation_TAG, "NtAlpcQueryInformation", 0, NULL, fake_NtAlpcQueryInformation },
	{ Ntdll_TAG, NtAlpcQueryInformationMessage_TAG, "NtAlpcQueryInformationMessage", 0, NULL, fake_NtAlpcQueryInformationMessage },
	{ Ntdll_TAG, NtImpersonateClientOfPort_TAG, "NtImpersonateClientOfPort", 0, NULL, fake_NtImpersonateClientOfPort },
	{ Ntdll_TAG, NtAlpcImpersonateClientOfPort_TAG, "NtAlpcImpersonateClientOfPort", 0, NULL, fake_NtAlpcImpersonateClientOfPort },
	{ Ntdll_TAG, NtCreateEvent_TAG, "NtCreateEvent", 0, NULL, fake_NtCreateEvent },
	{ Ntdll_TAG, NtOpenEvent_TAG, "NtOpenEvent", 0, NULL, fake_NtOpenEvent },
	{ Ntdll_TAG, NtCreateMutant_TAG, "NtCreateMutant", 0, NULL, fake_NtCreateMutant },
	{ Ntdll_TAG, NtOpenMutant_TAG, "NtOpenMutant", 0, NULL, fake_NtOpenMutant },
	{ Ntdll_TAG, NtCreateSemaphore_TAG, "NtCreateSemaphore", 0, NULL, fake_NtCreateSemaphore },
	{ Ntdll_TAG, NtOpenSemaphore_TAG, "NtOpenSemaphore", 0, NULL, fake_NtOpenSemaphore },
	{ Ntdll_TAG, NtCreateSection_TAG, "NtCreateSection", 0, NULL, fake_NtCreateSection },
	{ Ntdll_TAG, NtOpenSection_TAG, "NtOpenSection", 0, NULL, fake_NtOpenSection },

};


// Token
static HOOKINFOLittle g_HookInfoToken_Array [] = 
{
	{ Ntdll_TAG, NtOpenProcess_TAG, "NtOpenProcess", 0, NULL, fake_NtOpenProcess },
	{ Ntdll_TAG, NtOpenThread_TAG, "NtOpenThread", 0, NULL, fake_NtOpenThread },
	{ Ntdll_TAG, NtDuplicateObject_TAG, "NtDuplicateObject", 0, NULL, fake_NtDuplicateObject },
	{ Ntdll_TAG, NtQuerySecurityObject_TAG, "NtQuerySecurityObject", 0, NULL, fake_NtQuerySecurityObject },
	{ Ntdll_TAG, NtSetSecurityObject_TAG, "NtSetSecurityObject", 0, NULL, fake_NtSetSecurityObject },
	{ Ntdll_TAG, NtSetInformationToken_TAG, "NtSetInformationToken", 0, NULL, fake_NtSetInformationToken },
	{ Ntdll_TAG, NtAdjustPrivilegesToken_TAG, "NtAdjustPrivilegesToken", 0, NULL, fake_NtAdjustPrivilegesToken },

};


// TokenEx
static HOOKINFOLittle g_HookInfoTokenEx_Array [] = 
{
	{ Ntdll_TAG, RtlQueryElevationFlags_TAG, "RtlQueryElevationFlags", 0, NULL, fake_RtlQueryElevationFlags },
	{ Ntdll_TAG, NtQueryInformationToken_TAG, "NtQueryInformationToken", 0, NULL, fake_NtQueryInformationToken },
};


// Load
static HOOKINFOLittle g_HookInfoLoad_Array [] = 
{
	{ Ntdll_TAG, LdrLoadDll_TAG, "LdrLoadDll", 0, NULL, fake_LdrLoadDll },
	{ Ntdll_TAG, LdrUnloadDll_TAG, "LdrUnloadDll", 0, NULL, fake_LdrUnloadDll },
	{ Ntdll_TAG, NtLoadDriver_TAG, "NtLoadDriver", 0, NULL, fake_NtLoadDriver },
};


// Console
static HOOKINFOLittle g_HookInfoConsole_Array [] = 
{
	{ Kernel32_TAG, SetConsoleTitleA_TAG, "SetConsoleTitleA", 0, NULL, fake_SetConsoleTitleA },
	{ Kernel32_TAG, SetConsoleTitleW_TAG, "SetConsoleTitleW", 0, NULL, fake_SetConsoleTitleW },
	{ Kernel32_TAG, GetConsoleTitleA_TAG, "GetConsoleTitleA", 0, NULL, fake_GetConsoleTitleA },
	{ Kernel32_TAG, GetConsoleTitleW_TAG, "GetConsoleTitleW", 0, NULL, fake_GetConsoleTitleW },
};


// TotalDynamic
static TotalDynamicInfo g_HookInfoTotalDynamic_Array [] = 
{
	{ L"advapi32.dll", Hook_pharase0_advapi32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"crypt32.dll", Hook_pharase1_crypt32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"hnetcfg.dll", Hook_pharase2_hnetcfgdll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"ws2_32.dll", Hook_pharase3_ws2_32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"msi.dll", Hook_pharase4_msidll, NULL, TRUE, { 0,0,0,0,0,0 } },
 	{ L"ole32.dll", Hook_pharase5_ole32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"pstorec.dll", Hook_pharase6_pstorecdll, NULL, FALSE, { 0,0,0,0,0,0 } }, // !
	{ L"secur32.dll", Hook_pharase7_secur32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"setupapi.dll", Hook_pharase8_setupapidll, NULL, FALSE, { 0,0,0,0,0,0 } }, // CM_xx 系列
	{ L"cfgmgr32.dll", Hook_pharase9_cfgmgr32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"shell32.dll", Hook_pharase10_shell32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
	{ L"zipfldr.dll", Hook_pharase11_zipfldrdll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"user32.dll", Hook_pharase12_user32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"sfc_os.dll", Hook_pharase13_sfc_osdll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"wtsapi32.dll", Hook_pharase14_wtsapi32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"winsta.dll", Hook_pharase15_winstadll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"wevtapi.dll", Hook_pharase16_wevtapidll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"sxs.dll", Hook_pharase17_sxsdll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"gdi32.dll", Hook_pharase18_gdi32dll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"sechost.dll", Hook_pharase19_sechostdll, NULL, FALSE, { 0,0,0,0,0,0 } },
 	{ L"MsgPlusLive.dll", Hook_pharase20_MsgPlusLivedll, NULL, FALSE, { 0,0,0,0,0,0 } },
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL StartHook ()
{
	MYTRACE( L"enter StartHook(); Cur TickCout:%d ", GetTickCount() );

	// 1. 初始化Hook所需信息
	InitHookInfo();

	// 2.1 操作注册表相关
	if ( FALSE == Hook_Pharase0_Regs() )
	{
		MYTRACE( L"error! | StartHook() - Hook_Pharase0_Regs();|  \n" );
		return FALSE ;
	}

	// 2.2 操作文件相关
	if ( FALSE == Hook_pharase1_Files() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase1_Files();|  \n" );
		return FALSE ;
	}

	// 2.3 操作Alpc相关
	if ( FALSE == Hook_pharase2_Alpc() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase2_Alpc();|  \n" );
		return FALSE ;
	}

	// 2.4 操作Token相关
	if ( FALSE == Hook_pharase3_Token() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase3_Token();|  \n" );
		return FALSE ;
	}

	// 2.5 操作Load相关
	if ( FALSE == Hook_pharase4_Load() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}

	// 2.6 
	if ( FALSE == Hook_pharase5() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}

	// 2.7 
	if ( FALSE == Hook_pharase6() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}

	// 2.8 操作CreateProcess相关
	if ( FALSE == Hook_pharase7_CreateProcess() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}

	// 2.9 操作RegKey相关
	if ( FALSE == Hook_pharase8_RegKey() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}

	// 2.10 操作Dynamic相关
	MYTRACE( L"enter Hook_pharase9_TotalDynamic(); Cur TickCout:%d ", GetTickCount() );
	if ( FALSE == Hook_pharase9_TotalDynamic() )
	{
		MYTRACE( L"error! | StartHook() - Hook_pharase4_Load();|  \n" );
		return FALSE ;
	}
	MYTRACE( L"leave Hook_pharase9_TotalDynamic(); Cur TickCout:%d ", GetTickCount() );

	g_bDoWork_OK = TRUE;
	HandlerIEEmbedding();

	MYTRACE( L"leave StartHook(); \n" );
	return TRUE;
}


VOID StopHook ()
{
	LPHOOKINFOLittle pArray = NULL ;
	int i = 0, TotalCounts = 0 ;

	// 1. UnHook Regs
	TotalCounts = g_HookInfo.REGS.ArrayCounts ;
	pArray		= g_HookInfo.REGS.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	// 2. UnHook Files
	TotalCounts = g_HookInfo.FILES.ArrayCounts ;
	pArray		= g_HookInfo.FILES.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	// 3. UnHook Alpc
	TotalCounts = g_HookInfo.ALPC.ArrayCounts ;
	pArray		= g_HookInfo.ALPC.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	// 4. UnHook Token
	TotalCounts = g_HookInfo.TOKEN.ArrayCounts ;
	pArray		= g_HookInfo.TOKEN.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	// 5. UnHook TokenEx
	TotalCounts = g_HookInfo.TOKENEX.ArrayCounts ;
	pArray		= g_HookInfo.TOKENEX.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	// 6. UnHook Load
	TotalCounts = g_HookInfo.LOAD.ArrayCounts ;
	pArray		= g_HookInfo.LOAD.pArray ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( FALSE == pArray[ i ].bHooked ) { continue ; }

		Mhook_Unhook( (PVOID*)&pArray[ i ].OrignalAddress );
	}

	return ;
}


BOOL HookOne( LPHOOKINFOLittle Info )
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:35]

Routine Description:
  对单个函数节点进行挂钩  
    
Arguments:
  Info - 结构体指针,包含了函数的原始地址/过滤地址 等信息 
 
--*/
{
	if ( NULL == Info ) { return FALSE ; }
	if ( Info->bHooked ) { return TRUE ; }
	if ( NULL == Info->OrignalAddress || NULL == Info->FakeAddress ) { return TRUE ; }

	return Mhook_SetHook( (PVOID*)&Info->OrignalAddress, Info->FakeAddress );
}


VOID InitHookInfo()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:33]

Routine Description:
  初始化Hook数据   

--*/
{
	// 1. 初始化注册表SID全路径
	RtlZeroMemory( &g_RegInfo, sizeof(g_RegInfo) );

	g_RegInfo.RegSIDPathLength = wcslen( g_BoxInfo.SID ) + 0xF ;
	g_RegInfo.RegSIDPath = (LPWSTR) kmalloc( g_RegInfo.RegSIDPathLength );

	wcscpy( g_RegInfo.RegSIDPath, L"\\registry\\user\\" );
	wcscat( g_RegInfo.RegSIDPath, g_BoxInfo.SID );

	// 2. 初始化数组
	if ( g_HookInfo.bInited ) { return ; }

	RtlZeroMemory( &g_HookInfo, sizeof(g_HookInfo) );

	g_HookInfo.REGS.pArray = g_HookInfoReg_Array ;
	g_HookInfo.REGS.ArrayCounts = ARRAYSIZEOF( g_HookInfoReg_Array );

	g_HookInfo.FILES.pArray = g_HookInfoFile_Array ;
	g_HookInfo.FILES.ArrayCounts = ARRAYSIZEOF( g_HookInfoFile_Array );

	g_HookInfo.ALPC.pArray = g_HookInfoAlpc_Array ;
	g_HookInfo.ALPC.ArrayCounts = ARRAYSIZEOF( g_HookInfoAlpc_Array );

	g_HookInfo.TOKEN.pArray = g_HookInfoToken_Array ;
	g_HookInfo.TOKEN.ArrayCounts = ARRAYSIZEOF( g_HookInfoToken_Array );

	g_HookInfo.TOKENEX.pArray = g_HookInfoTokenEx_Array ;
	g_HookInfo.TOKENEX.ArrayCounts = ARRAYSIZEOF( g_HookInfoTokenEx_Array );

	g_HookInfo.LOAD.pArray = g_HookInfoLoad_Array ;
	g_HookInfo.LOAD.ArrayCounts = ARRAYSIZEOF( g_HookInfoLoad_Array );

	g_HookInfo.CONSOLE.pArray = g_HookInfoConsole_Array ;
	g_HookInfo.CONSOLE.ArrayCounts = ARRAYSIZEOF( g_HookInfoConsole_Array );

	// 3. 保存原始函数地址
	HMODULE* phModuleArray = __ProcessNode->DllBaseAddr.hModuleArrays;

	InitializeCriticalSection( &g_cs_Regedit );

	InitHookInfoEx( g_HookInfo.REGS.ArrayCounts, g_HookInfo.REGS.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.FILES.ArrayCounts, g_HookInfo.FILES.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.ALPC.ArrayCounts, g_HookInfo.ALPC.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.TOKEN.ArrayCounts, g_HookInfo.TOKEN.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.TOKENEX.ArrayCounts, g_HookInfo.TOKENEX.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.LOAD.ArrayCounts, g_HookInfo.LOAD.pArray, phModuleArray );
	InitHookInfoEx( g_HookInfo.CONSOLE.ArrayCounts, g_HookInfo.CONSOLE.pArray, phModuleArray );

	g_HookInfo.bInited = TRUE ;
	return ;
}


VOID 
InitHookInfoEx (
	IN int TotalCounts,
	IN LPHOOKINFOLittle pArray,
	IN HMODULE* phModuleArray
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/13 [13:1:2012 - 14:24]

Routine Description:
  初始化Hook结点，获取原始函数地址
    
Arguments:
  TotalCounts - 结点中的函数个数
  pArray - 待初始化的Hook结点
  phModuleArray - 模块基址数组
    
--*/
{
	for( int i=0; i<TotalCounts; i++ )
	{
		pArray[i].OrignalAddress = (PVOID) GetProcAddress( phModuleArray[pArray[i].DllTag], pArray[i].FunctionName );
	}

	return;
}


BOOL Hook_Pharase0_Regs()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:33]

Routine Description:
  对注册表API进行挂钩    
    
--*/
{
	BOOL bRet = TRUE ;
	LPHOOKINFOLittle pArray = NULL ;
	int i = 0, TotalCounts = 0 ;

	TotalCounts = g_HookInfo.REGS.ArrayCounts ;
	pArray		= g_HookInfo.REGS.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_Pharase0_Regs() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	// 保存部分ssdt 原始地址至全局变量
	if ( NULL == g_NtQueryKey_addr ) { g_NtQueryKey_addr = (_NtQueryKey_) GetRegFunc(NtQueryKey) ; }
	if ( NULL == g_NtOpenKey_addr ) { g_NtOpenKey_addr = (_NtOpenKey_) GetRegFunc( NtOpenKey ) ; }
	if ( NULL == g_NtCreateKey_addr ) { g_NtCreateKey_addr = (_NtCreateKey_) GetRegFunc( NtCreateKey ) ; }
	if ( NULL == g_NtDeleteKey_addr ) { g_NtDeleteKey_addr = (_NtDeleteKey_) GetRegFunc(NtDeleteKey) ; }
	if ( NULL == g_NtEnumerateKey_addr ) { g_NtEnumerateKey_addr = (_NtEnumerateKey_) GetRegFunc(NtEnumerateKey) ; }
	if ( NULL == g_NtEnumerateValueKey_addr ) { g_NtEnumerateValueKey_addr = (_NtEnumerateValueKey_) GetRegFunc(NtEnumerateValueKey) ; }

	if ( 0 == wcsicmp( g_BoxInfo.ProcessName, _ProteinBoxRpcSs_exe_W_ ) )
	{
		g_bIs_Inside_PBRpcSs_exe = TRUE ;
	}
	
	return TRUE ;
}


BOOL Hook_pharase1_Files()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:33]

Routine Description:
  对文件API进行挂钩    
    
--*/
{
	BOOL bRet = TRUE ;
	LPHOOKINFOLittle pArray = NULL ;
	int i = 0, TotalCounts = 0 ;

	// 1. 为文件操作准备黑白名单
	InitializeCriticalSection( &g_Lock_DirHandle );
	InitializeCriticalSection( &g_Lock_RtlGetCurrentDirectory );

	HandlerFileList();

	CsrPopulateDosDevices();

	// 2. 获取系统所有磁盘,分区名,保存至全局变量
	if ( FALSE == HandlerPanfu( 0xFFFFFFFF ) ) { return FALSE ; }

	// 3. 获取沙箱自己的变量内容
	if ( FALSE == GetPBEnvironment() ) { return FALSE ; }

	// 4.1 获取配置文件中关于"删除沙箱时自动恢复的文件路径"的相关信息
	Handler_Configration_AutoRecover();

	// TODO{sudami}: 4.2 获取配置文件中的其他信息 即对 g_CopyLimitKb 进行赋值



	// 4.3
	if ( 0 == wcsicmp( g_BoxInfo.ProcessName, L"PBCrypto.exe" ) )
	{ 
		g_bIs_Inside_PBCrypto_exe = TRUE ;
	}

	// 5.1 Inline Hook文件相关的IAT函数
	TotalCounts = g_HookInfo.FILES.ArrayCounts ;
	pArray		= g_HookInfo.FILES.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase1_Files() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	// 5.2 保存部分ssdt 原始地址至全局变量
	if ( NULL == g_NtCreateFile_addr ) { g_NtCreateFile_addr = (_NtCreateFile_) GetFileFunc(NtCreateFile); }
	if ( NULL == g_NtDeleteFile_addr ) { g_NtDeleteFile_addr = (_NtDeleteFile_) GetFileFunc(NtDeleteFile); }
	if ( NULL == g_NtQueryFullAttributesFile_addr ) { g_NtQueryFullAttributesFile_addr = (_NtQueryFullAttributesFile_) GetFileFunc(NtQueryFullAttributesFile); }
	if ( NULL == g_NtQueryInformationFile_addr ) { g_NtQueryInformationFile_addr = (_NtQueryInformationFile_) GetFileFunc(NtQueryInformationFile); }
	if ( NULL == g_NtQueryDirectoryFile_addr ) { g_NtQueryDirectoryFile_addr = (_NtQueryDirectoryFile_) GetFileFunc(NtQueryDirectoryFile); }
	if ( NULL == g_NtSetInformationFile_addr ) { g_NtSetInformationFile_addr = (_NtSetInformationFile_) GetFileFunc(NtSetInformationFile); }
	if ( NULL == g_NtClose_addr ) { g_NtClose_addr = (_NtClose_) GetFileFunc(NtClose); }

	g_FileHookInitOK = TRUE ;
	return TRUE ;
}


BOOL Hook_pharase2_Alpc()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 10:33]

Routine Description:
  对alpc API进行挂钩    
    
--*/
{
	BOOL bRet = TRUE ;
	int i = 0, TotalCounts = 0 ;
	LPHOOKINFOLittle pArray = NULL ;

	HandlerAlpcList();

	// Inline Hook文件相关的IAT函数
	TotalCounts = g_HookInfo.ALPC.ArrayCounts ;
	pArray		= g_HookInfo.ALPC.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase2_Alpc() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	CreateObjectLink();
	return TRUE ;
}


BOOL Hook_pharase3_Token()
{
	BOOL bRet = TRUE ;
	int i = 0, TotalCounts = 0 ;
	LPHOOKINFOLittle pArray = NULL ;
	PVOID Orignal_RtlQueryElevationFlags_addr = NULL ;

	g_NtQueryInformationToken_addr = (_NtQueryInformationToken_) GetTokenExFunc(NtQueryInformationToken) ;

	// 1. Inline Hook Toeken相关的IAT函数
	TotalCounts = g_HookInfo.TOKEN.ArrayCounts ;
	pArray		= g_HookInfo.TOKEN.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase3_Token() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	// 2.1 其他初始化工作
	Orignal_RtlQueryElevationFlags_addr = g_HookInfoTokenEx_Array[ RtlQueryElevationFlags_TAG ].OrignalAddress;
	if ( NULL == Orignal_RtlQueryElevationFlags_addr ) { return TRUE; }

	g_bHookRpc = TRUE ;
	bRet = FALSE ;

	if ( 0 == _wcsicmp( g_BoxInfo.ProcessName, L"SynTPEnh.exe" ) || 0 == _wcsicmp( g_BoxInfo.ProcessName, L"SynTPEnh.exe" ) )
	{
		bRet = g_bFlag_Is_in_SynTP_exe = TRUE ;
	}

	// 2.2 若当前进程不在 IE / Syn*.exe中,直接返回
	if ( FALSE == g_bIs_Inside_Explorer_exe && FALSE == bRet ) { return TRUE ; }

	// 3. Inline Hook ToekenEx相关的IAT函数
	TotalCounts = g_HookInfo.TOKENEX.ArrayCounts ;
	pArray		= g_HookInfo.TOKENEX.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase3_Token() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	return bRet;
}


BOOL Hook_pharase4_Load()
{
	BOOL bRet = TRUE, bIsLocked = FALSE ;
	int i = 0, TotalCounts = 0 ;
	LPHOOKINFOLittle pArray = NULL ;
	HMODULE hModule_kernel32_dll = NULL ;

	// 1.0
	ClearStruct( &g_Node_GetModuleFileName );
	InitializeCriticalSection( &g_lock_GetModuleFileName );

	// 1.1 Inline Hook Toeken相关的IAT函数
	TotalCounts = g_HookInfo.LOAD.ArrayCounts ;
	pArray		= g_HookInfo.LOAD.pArray ;

	for( i; i<TotalCounts; i++ )
	{
		bRet = HookOne( &pArray[ i ] );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase4_Load() - HookOne(); | \"%s\" \n", pArray[ i ].FunctionName );
			return FALSE ;
		}
	}

	// 2. hook GetModuleFileNameW
	hModule_kernel32_dll = __ProcessNode->DllBaseAddr.hModuleArrays[KernelBase_TAG] ;
	if ( NULL == hModule_kernel32_dll ) { hModule_kernel32_dll = __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG] ; }

	g_GetModuleFileNameW_addr = (_GetModuleFileNameW_) GetProcAddress( hModule_kernel32_dll, "GetModuleFileNameW" );
	bRet = Mhook_SetHook( (PVOID*)&g_GetModuleFileNameW_addr, fake_GetModuleFileNameW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase4_Load() - Mhook_SetHook(); | \"GetModuleFileNameW\" \n" );
		return FALSE ;
	}

	// 3.
	Handler_GetModuleFileNameW_Total( _GetModuleFileName_Flag_Add_, NULL, NULL, &bIsLocked );
	if ( bIsLocked ) { LeaveCriticalSection(&g_lock_GetModuleFileName); }

	Hook_AddressOfEntryPoint();
	return TRUE;
}


BOOL Hook_pharase5()
{
	BOOL bRet = TRUE ;

	g_NtQuerySystemInformation_addr = (_NtQuerySystemInformation_) GetProcAddress( __ProcessNode->DllBaseAddr.hModuleArrays[Ntdll_TAG], "ZwQuerySystemInformation" );
	bRet = Mhook_SetHook( (PVOID*)&g_NtQuerySystemInformation_addr, fake_NtQuerySystemInformation );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase5() - Mhook_SetHook(); | \"NtQuerySystemInformation\" \n" );
		return FALSE ;
	}

	ULONG SetLocaleInfoW_addr = (ULONG) SetLocaleInfoW ;
	bRet = Mhook_SetHook( (PVOID*)&SetLocaleInfoW_addr, fake_SetLocaleInfo );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase5() - Mhook_SetHook(); | \"SetLocaleInfoW\" \n" );
		return FALSE ;
	}

	ULONG SetLocaleInfoA_addr = (ULONG) SetLocaleInfoA ;

	bRet = Mhook_SetHook( (PVOID*)&SetLocaleInfoA_addr, fake_SetLocaleInfo );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase5() - Mhook_SetHook(); | \"SetLocaleInfoA\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase6()
{
	BOOL bRet = TRUE ;
	HMODULE hModuleKernel32 = __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG] ;
	HMODULE hModuleNtdll = __ProcessNode->DllBaseAddr.hModuleArrays[Ntdll_TAG] ;
	DWORD NtCreateTransaction_addr = 0, NtOpenTransaction_addr = 0 ;
	DWORD NtCommitTransaction_addr = 0, NtRollbackTransaction_addr = 0 ;

	if ( NULL == hModuleKernel32 ) { return FALSE; }

	g_CreateActCtxA_addr = (_CreateActCtxA_) GetProcAddress( hModuleKernel32, "CreateActCtxA" );
	g_CreateActCtxW_addr = (_CreateActCtxW_) GetProcAddress( hModuleKernel32, "CreateActCtxW" );
	g_QueryActCtxSettingsW_addr = (_QueryActCtxSettingsW_) GetProcAddress( hModuleKernel32, "QueryActCtxSettingsW" );

	if ( g_CreateActCtxA_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CreateActCtxA_addr, fake_CreateActCtxA );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"CreateActCtxA\" \n" );
			return FALSE ;
		}
	}

	if ( g_CreateActCtxW_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_CreateActCtxA_addr, fake_CreateActCtxW );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"CreateActCtxW\" \n" );
			return FALSE ;
		}
	}

	if ( wcsicmp( g_BoxInfo.ProcessName, L"TrustedInstaller.exe" ) ) { return TRUE; }

	//
	// 针对进程"TrustedInstaller.exe"（微软升级或者安装某些微软发布的安装包）进行额外的处理,4个过滤函数都是空函数，不响应任何消息
	//
	g_Is_NtSetValueKey_Ready = TRUE ;

	NtCreateTransaction_addr	= (DWORD) GetProcAddress( hModuleNtdll, "NtCreateTransaction" );
	NtOpenTransaction_addr		= (DWORD) GetProcAddress( hModuleNtdll, "NtOpenTransaction" );
	NtCommitTransaction_addr	= (DWORD) GetProcAddress( hModuleNtdll, "NtCommitTransaction" );
	NtRollbackTransaction_addr	= (DWORD) GetProcAddress( hModuleNtdll, "NtRollbackTransaction" );

	if ( NtCreateTransaction_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&NtCreateTransaction_addr, fake_NtCreateTransaction );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"NtCreateTransaction\" \n" );
			return FALSE ;
		}
	}

	if ( NtOpenTransaction_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&NtOpenTransaction_addr, fake_NtOpenTransaction );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"NtOpenTransaction\" \n" );
			return FALSE ;
		}
	}

	if ( NtCommitTransaction_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&NtCommitTransaction_addr, fake_NtSaveKey );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"NtCommitTransaction\" \n" );
			return FALSE ;
		}
	}

	if ( NtRollbackTransaction_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&NtRollbackTransaction_addr, fake_NtSaveKey );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase6() - Mhook_SetHook(); | \"NtRollbackTransaction\" \n" );
			return FALSE ;
		}
	}
	
	return TRUE;
}


BOOL Hook_pharase7_CreateProcess()
{
	BOOL bRet = TRUE ;
	STRING AnsiString ;
	HMODULE hModule_ntdll    = __ProcessNode->DllBaseAddr.hModuleArrays[Ntdll_TAG] ;
	HMODULE hModule_kernel32 = __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG] ;

	// 获取“ntvdm.exe”的全路径。（Windows 16-bit Virtual Machine）
	memset( g_szPath_ntvdm, 0, MAX_PATH );
	GetSystemWindowsDirectoryW( g_szPath_ntvdm, 0x104 );
	
	if ( NULL == g_szPath_ntvdm ) { wcscpy( g_szPath_ntvdm, L"C:\\WINDOWS" ); }
	if ( '\\' == g_szPath_ntvdm[ wcslen(g_szPath_ntvdm) ] ) { g_szPath_ntvdm[ wcslen(g_szPath_ntvdm) ] = 0; }

	wcscat( g_szPath_ntvdm, L"\\system32\\ntvdm.exe" );

	// Hook
	g_RtlCreateProcessParameters_addr  = (_RtlCreateProcessParameters_) GetProcAddress( hModule_ntdll, "RtlCreateProcessParameters" );
	g_RtlCreateProcessParametersEx_addr	= (_RtlCreateProcessParametersEx_) GetProcAddress( hModule_ntdll, "RtlCreateProcessParametersEx" );

	if ( NULL == g_RtlCreateProcessParameters_addr ) { return FALSE; }
	bRet = Mhook_SetHook( (PVOID*)&g_RtlCreateProcessParameters_addr, fake_RtlCreateProcessParameters );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"RtlCreateProcessParameters\" \n" );
		return FALSE ;
	}
	
	if ( g_RtlCreateProcessParametersEx_addr )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_RtlCreateProcessParametersEx_addr, fake_RtlCreateProcessParametersEx );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"RtlCreateProcessParametersEx\" \n" );
			return FALSE ;
		}
	}

	RtlInitString( &AnsiString, "CreateProcessA" );
	LdrGetProcedureAddress( hModule_kernel32, &AnsiString, 0, (PVOID*)&g_CreateProcessA_addr );

	RtlInitString( &AnsiString, "CreateProcessW" );
	LdrGetProcedureAddress( hModule_kernel32, &AnsiString, 0, (PVOID*)&g_CreateProcessW_addr );

	bRet = Mhook_SetHook( (PVOID*)&g_CreateProcessA_addr, fake_CreateProcessA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"CreateProcessA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateProcessW_addr, fake_CreateProcessW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"CreateProcessW\" \n" );
		return FALSE ;
	}

	if ( 0 == wcsicmp( g_BoxInfo.ProcessName, L"ServiceModelReg.exe" ) )
	{
		bRet = Mhook_SetHook( (PVOID*)&g_ExitProcess_addr, fake_ExitProcess );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"ExitProcess\" \n" );
			return FALSE ;
		}
	}

	g_WinExec_addr = (_WinExec_) GetProcAddress( hModule_kernel32, "WinExec" );
	bRet = Mhook_SetHook( (PVOID*)&g_WinExec_addr, fake_WinExec );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_pharase7_CreateProcess() - Mhook_SetHook(); | \"WinExec\" \n" );
		return FALSE ;
	}

	return TRUE;
}


BOOL Hook_pharase8_RegKey()
{
	if ( Handler_RegKey_PBAutoExec(0) != '1' )
	{
		Handler_RegKey_RegLink();

		Handler_RegKey_DisableDCOM();

		Handler_RegKey_NukeOnDelete_Recycle();

		Handler_RegKey_BrowseNewProcess();

		Handler_RegKey_Elimination();

		Handler_RegKey_clsid_1();

		Handler_RegKey_clsid_2();

		Handler_RegKey_PBAutoExec( '1' );
	}

	HandlerSelfAutoExec();

	if ( g_KeyHandle_PBAutoExec ) 
	{ 
		ZwClose( g_KeyHandle_PBAutoExec );
		g_KeyHandle_PBAutoExec = NULL;
	}

	return TRUE;
}


static BOOL g_pharase9_TotalDynamic_InitOnce = TRUE ;

BOOL Hook_pharase9_TotalDynamic()
{
	BOOL bRet = TRUE ;
	HMODULE hModule = NULL ;
	int i = 0, TotalCounts = 0 ;
	TotalDynamicInfo pArray ;

	if ( g_TotalDynamic_LockCounts ) { return TRUE; }

	TotalCounts = ARRAYSIZEOF( g_HookInfoTotalDynamic_Array );
	
	if ( g_pharase9_TotalDynamic_InitOnce )
	{
		g_pharase9_TotalDynamic_InitOnce = FALSE ;
		for( i; i<TotalCounts; i++ )
		{
			InitializeCriticalSection( &g_HookInfoTotalDynamic_Array[ i ].Lock );
		}
	}

	i = 0 ;
	for( i; i<TotalCounts; i++ )
	{
		pArray = g_HookInfoTotalDynamic_Array[ i ];
		hModule = GetModuleHandleW( pArray.ModuleName );

		EnterCriticalSection( &pArray.Lock );
		InterlockedIncrement( &g_TotalDynamic_LockCounts );

		// 1. 若当前DLL在内存中,且没有被IAT HOOK(判断条件即是结构体中的hModule成员变量还没有被填充),则进行HOOK并填充结构体
		if ( hModule )
		{
			if ( NULL == pArray.hModule )
			{
				if ( pArray.FuncAddress(hModule) ) 
					g_HookInfoTotalDynamic_Array[ i ].hModule = hModule ;
				else
					bRet = FALSE ;
			}
		}
		else
		{
			if ( pArray.hModule )
			{
				if ( pArray.bFlag ) { pArray.FuncAddress(NULL); }
				g_HookInfoTotalDynamic_Array[ i ].hModule = NULL ;
			}
		}

		InterlockedDecrement( &g_TotalDynamic_LockCounts );
		LeaveCriticalSection( &pArray.Lock );
	}

	return bRet;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
